#include "network/NetworkManager.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <iostream>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <streambuf>
#include <system_error>
#include <thread>
#include <utility>

#include "java/String.h"
#include "java/System.h"
#include "network/NetHandler.h"
#include "network/Packet.h"
#include "network/PacketDataStream.h"

namespace
{

#if defined(_WIN32)
using NativeSocket = SOCKET;
const NativeSocket INVALID_NATIVE_SOCKET = INVALID_SOCKET;
#else
using NativeSocket = int;
const NativeSocket INVALID_NATIVE_SOCKET = -1;
#endif

struct ConnectedSocket
{
	NativeSocket socket;
	int family;
};

int getLastSocketError()
{
#if defined(_WIN32)
	return WSAGetLastError();
#else
	return errno;
#endif
}

bool isInterruptedSocketError(int error)
{
#if defined(_WIN32)
	return error == WSAEINTR;
#else
	return error == EINTR;
#endif
}

std::string socketErrorMessage(int error)
{
	return std::system_category().message(error);
}

void closeNativeSocket(NativeSocket socket)
{
#if defined(_WIN32)
	closesocket(socket);
#else
	close(socket);
#endif
}

void shutdownNativeSocket(NativeSocket socket)
{
#if defined(_WIN32)
	shutdown(socket, SD_BOTH);
#else
	shutdown(socket, SHUT_RDWR);
#endif
}

#if defined(_WIN32)
void ensureSocketsInitialized()
{
	static std::once_flag initializationFlag;
	std::call_once(initializationFlag, []()
	{
		WSADATA data;
		int result = WSAStartup(MAKEWORD(2, 2), &data);
		if (result != 0)
			throw std::runtime_error("WSAStartup failed: " + socketErrorMessage(result));
	});
}
#endif

std::string addressResolutionError(int error)
{
#if defined(_WIN32)
	const char *message = gai_strerrorA(error);
#else
	const char *message = gai_strerror(error);
#endif
	return message == nullptr ? std::to_string(error) : std::string(message);
}

void configureSocket(NativeSocket socket, int family)
{
	try
	{
#if defined(_WIN32)
		DWORD timeout = 30000;
		if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,
			reinterpret_cast<const char *>(&timeout), sizeof(timeout)) != 0)
		{
			throw std::runtime_error(socketErrorMessage(getLastSocketError()));
		}
#else
		timeval timeout;
		timeout.tv_sec = 30;
		timeout.tv_usec = 0;
		if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
			throw std::runtime_error(socketErrorMessage(getLastSocketError()));
#endif

		int trafficClass = 24;
		int result = 0;
		if (family == AF_INET)
		{
#if defined(_WIN32)
			result = setsockopt(socket, IPPROTO_IP, IP_TOS,
				reinterpret_cast<const char *>(&trafficClass), sizeof(trafficClass));
#else
			result = setsockopt(socket, IPPROTO_IP, IP_TOS, &trafficClass, sizeof(trafficClass));
#endif
		}
#if defined(IPV6_TCLASS)
		else if (family == AF_INET6)
		{
#if defined(_WIN32)
			result = setsockopt(socket, IPPROTO_IPV6, IPV6_TCLASS,
				reinterpret_cast<const char *>(&trafficClass), sizeof(trafficClass));
#else
			result = setsockopt(socket, IPPROTO_IPV6, IPV6_TCLASS,
				&trafficClass, sizeof(trafficClass));
#endif
		}
#endif
		if (result != 0)
			throw std::runtime_error(socketErrorMessage(getLastSocketError()));
	}
	catch (const std::exception &error)
	{
		std::cerr << error.what() << std::endl;
	}
}

ConnectedSocket connectBlocking(const std::string &host, std::uint16_t port)
{
#if defined(_WIN32)
	ensureSocketsInitialized();
#endif

	addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo *addresses = nullptr;
	std::string service = std::to_string(port);
	int lookupResult = getaddrinfo(host.c_str(), service.c_str(), &hints, &addresses);
	if (lookupResult != 0)
		throw std::runtime_error("Unable to resolve " + host + ": " + addressResolutionError(lookupResult));

	NativeSocket connected = INVALID_NATIVE_SOCKET;
	int connectedFamily = AF_UNSPEC;
	int lastError = 0;
	for (addrinfo *address = addresses; address != nullptr; address = address->ai_next)
	{
		NativeSocket candidate = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
		if (candidate == INVALID_NATIVE_SOCKET)
		{
			lastError = getLastSocketError();
			continue;
		}

#if defined(_WIN32)
		int result = connect(candidate, address->ai_addr, static_cast<int>(address->ai_addrlen));
#else
		int result = connect(candidate, address->ai_addr, address->ai_addrlen);
#endif
		if (result == 0)
		{
			connected = candidate;
			connectedFamily = address->ai_family;
			break;
		}

		lastError = getLastSocketError();
		closeNativeSocket(candidate);
	}
	freeaddrinfo(addresses);

	if (connected == INVALID_NATIVE_SOCKET)
	{
		throw std::runtime_error("Unable to connect to " + host + ":" + service + ": "
			+ socketErrorMessage(lastError));
	}

	configureSocket(connected, connectedFamily);
#if !defined(_WIN32) && defined(SO_NOSIGPIPE)
	int noSigPipe = 1;
	setsockopt(connected, SOL_SOCKET, SO_NOSIGPIPE, &noSigPipe, sizeof(noSigPipe));
#endif
	return ConnectedSocket{connected, connectedFamily};
}

class SocketState
{
private:
	std::mutex stateMutex;
	std::condition_variable operationsFinished;
	NativeSocket socket;
	bool closing = false;
	int activeOperations = 0;

	bool beginOperation(NativeSocket &result)
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (closing || socket == INVALID_NATIVE_SOCKET)
			return false;
		++activeOperations;
		result = socket;
		return true;
	}

	void endOperation()
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		--activeOperations;
		if (activeOperations == 0)
			operationsFinished.notify_all();
	}

public:
	explicit SocketState(const ConnectedSocket &connected) : socket(connected.socket)
	{
	}

	~SocketState()
	{
		closeSocket();
	}

	std::streamsize receive(char *data, std::streamsize size)
	{
		NativeSocket activeSocket;
		if (!beginOperation(activeSocket))
			return 0;

		int result;
		int error = 0;
		do
		{
			int amount = static_cast<int>(std::min<std::streamsize>(
				size, std::numeric_limits<int>::max()));
			result = recv(activeSocket, data, amount, 0);
			if (result < 0)
				error = getLastSocketError();
		} while (result < 0 && isInterruptedSocketError(error));

		endOperation();
		if (result < 0)
			throw std::runtime_error(socketErrorMessage(error));
		return result;
	}

	void sendAll(const char *data, std::streamsize size)
	{
		NativeSocket activeSocket;
		if (!beginOperation(activeSocket))
			throw std::runtime_error("Socket is closed");

		std::streamsize sent = 0;
		int error = 0;
		while (sent < size)
		{
			int amount = static_cast<int>(std::min<std::streamsize>(
				size - sent, std::numeric_limits<int>::max()));
#if defined(_WIN32) || !defined(MSG_NOSIGNAL)
			int result = send(activeSocket, data + sent, amount, 0);
#else
			int result = static_cast<int>(send(activeSocket, data + sent, amount, MSG_NOSIGNAL));
#endif
			if (result > 0)
			{
				sent += result;
				continue;
			}
			if (result < 0)
			{
				error = getLastSocketError();
				if (isInterruptedSocketError(error))
					continue;
			}
			break;
		}

		endOperation();
		if (sent != size)
		{
			if (error == 0)
				throw std::runtime_error("Socket closed while writing packet data");
			throw std::runtime_error(socketErrorMessage(error));
		}
	}

	void closeSocket()
	{
		NativeSocket socketToClose;
		{
			std::unique_lock<std::mutex> lock(stateMutex);
			if (closing)
			{
				operationsFinished.wait(lock, [this]()
				{
					return socket == INVALID_NATIVE_SOCKET;
				});
				return;
			}
			if (socket == INVALID_NATIVE_SOCKET)
				return;
			closing = true;
			socketToClose = socket;
		}

		shutdownNativeSocket(socketToClose);

		{
			std::unique_lock<std::mutex> lock(stateMutex);
			operationsFinished.wait(lock, [this]()
			{
				return activeOperations == 0;
			});
			socket = INVALID_NATIVE_SOCKET;
		}
		closeNativeSocket(socketToClose);
		operationsFinished.notify_all();
	}
};

class SocketInputBuffer : public std::streambuf
{
private:
	SocketState &socket;
	char buffer[4096];

protected:
	int_type underflow() override
	{
		if (gptr() != nullptr && gptr() < egptr())
			return traits_type::to_int_type(*gptr());

		std::streamsize received = socket.receive(buffer, sizeof(buffer));
		if (received == 0)
			return traits_type::eof();
		setg(buffer, buffer, buffer + received);
		return traits_type::to_int_type(*gptr());
	}

public:
	explicit SocketInputBuffer(SocketState &socket) : socket(socket)
	{
		setg(buffer, buffer, buffer);
	}
};

class SocketOutputBuffer : public std::streambuf
{
private:
	SocketState &socket;
	char buffer[5120];

	bool flushBuffer()
	{
		std::streamsize size = pptr() - pbase();
		if (size != 0)
			socket.sendAll(pbase(), size);
		setp(buffer, buffer + sizeof(buffer));
		return true;
	}

protected:
	int_type overflow(int_type value) override
	{
		flushBuffer();
		if (!traits_type::eq_int_type(value, traits_type::eof()))
		{
			*pptr() = traits_type::to_char_type(value);
			pbump(1);
		}
		return traits_type::not_eof(value);
	}

	std::streamsize xsputn(const char *data, std::streamsize size) override
	{
		std::streamsize written = 0;
		while (written < size)
		{
			std::streamsize available = epptr() - pptr();
			if (available == 0)
			{
				flushBuffer();
				available = epptr() - pptr();
			}

			std::streamsize remaining = size - written;
			if (pptr() == pbase() && remaining >= static_cast<std::streamsize>(sizeof(buffer)))
			{
				socket.sendAll(data + written, remaining);
				written += remaining;
				continue;
			}

			std::streamsize amount = std::min(available, remaining);
			std::memcpy(pptr(), data + written, static_cast<std::size_t>(amount));
			pbump(static_cast<int>(amount));
			written += amount;
		}
		return written;
	}

	int sync() override
	{
		flushBuffer();
		return 0;
	}

public:
	explicit SocketOutputBuffer(SocketState &socket) : socket(socket)
	{
		setp(buffer, buffer + sizeof(buffer));
	}
};

class WorkerSignal
{
private:
	std::mutex mutex;
	std::condition_variable condition;
	std::uint64_t sequence = 0;

public:
	void wake()
	{
		{
			std::lock_guard<std::mutex> lock(mutex);
			++sequence;
		}
		condition.notify_one();
	}

	void waitForOneHundredMilliseconds(std::uint64_t &lastSequence)
	{
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait_for(lock, std::chrono::milliseconds(100), [this, &lastSequence]()
		{
			return sequence != lastSequence;
		});
		lastSequence = sequence;
	}
};

void joinThread(std::thread &thread)
{
	if (!thread.joinable())
		return;
	if (thread.get_id() == std::this_thread::get_id())
	{
		thread.detach();
		return;
	}
	thread.join();
}

class NetworkThreadCounter
{
private:
	int_t &counter;

public:
	explicit NetworkThreadCounter(int_t &counter) : counter(counter)
	{
		std::lock_guard<std::mutex> lock(NetworkManager::threadSyncObject);
		++counter;
	}

	~NetworkThreadCounter()
	{
		std::lock_guard<std::mutex> lock(NetworkManager::threadSyncObject);
		--counter;
	}
};

}

std::mutex NetworkManager::threadSyncObject;
int_t NetworkManager::numReadThreads = 0;
int_t NetworkManager::numWriteThreads = 0;
std::array<std::atomic<int_t>, 256> NetworkManager::field_28145_d{};
std::array<std::atomic<int_t>, 256> NetworkManager::field_28144_e{};

class NetworkManager::Impl
{
private:
	NetworkManager &manager;
	NetHandler &netHandler;
	SocketState socket;
	SocketInputBuffer inputBuffer;
	SocketOutputBuffer outputBuffer;
	std::istream inputStream;
	std::ostream outputStream;
	PacketDataInput socketInputStream;
	PacketDataOutput socketOutputStream;

	std::atomic<bool> isRunning{true};
	std::atomic<bool> isServerTerminating{false};
	std::atomic<bool> isTerminating{false};
	std::mutex sendQueueLock;
	std::deque<std::unique_ptr<Packet>> readPackets;
	std::mutex readPacketsLock;
	std::deque<std::unique_ptr<Packet>> dataPackets;
	std::deque<std::unique_ptr<Packet>> chunkDataPackets;
	int_t timeSinceLastRead = 0;
	int_t sendQueueByteLength = 0;
	int_t field_20100_w = 50;

	std::mutex shutdownMutex;
	jstring terminationReason;
	std::vector<jstring> terminationArguments;

	WorkerSignal readSignal;
	WorkerSignal writeSignal;
	std::mutex startMutex;
	std::condition_variable startCondition;
	bool threadsStarted = false;
	std::thread readThread;
	std::thread writeThread;
	std::thread terminationThread;

	std::mutex gracefulCloseMutex;
	std::condition_variable gracefulCloseCondition;
	bool destroying = false;
	std::thread gracefulCloseThread;

	void waitForThreadsToStart()
	{
		std::unique_lock<std::mutex> lock(startMutex);
		startCondition.wait(lock, [this]()
		{
			return threadsStarted;
		});
	}

	void startThreads()
	{
		try
		{
			readThread = std::thread([this]()
			{
				readThreadRun();
			});
			writeThread = std::thread([this]()
			{
				writeThreadRun();
			});
		}
		catch (...)
		{
			isRunning.store(false);
			{
				std::lock_guard<std::mutex> lock(startMutex);
				threadsStarted = true;
			}
			startCondition.notify_all();
			socket.closeSocket();
			joinThread(readThread);
			joinThread(writeThread);
			throw;
		}

		{
			std::lock_guard<std::mutex> lock(startMutex);
			threadsStarted = true;
		}
		startCondition.notify_all();
	}

	void onNetworkError(const std::exception &error)
	{
		std::cerr << error.what() << std::endl;
		std::vector<jstring> arguments;
		arguments.push_back(String::fromUTF8("Internal exception: " + std::string(error.what())));
		networkShutdown(u"disconnect.genericReason", arguments);
	}

	void onUnknownNetworkError()
	{
		std::runtime_error error("Unknown exception");
		onNetworkError(error);
	}

	bool sendPacket()
	{
		try
		{
			bool sentPacket = false;
			std::unique_ptr<Packet> packet;
			{
				std::lock_guard<std::mutex> lock(sendQueueLock);
				if (!dataPackets.empty())
				{
					int_t delay = manager.chunkDataSendCounter.load();
					if (delay == 0 || System::currentTimeMillis()
						- dataPackets.front()->creationTimeMillis >= delay)
					{
						packet = std::move(dataPackets.front());
						dataPackets.pop_front();
						sendQueueByteLength -= packet->getPacketSize() + 1;
					}
				}
			}

			if (packet)
			{
				Packet::writePacket(*packet, socketOutputStream);
				int_t packetId = packet->getPacketId();
				if (packetId < 0 || packetId >= static_cast<int_t>(NetworkManager::field_28144_e.size()))
					throw std::out_of_range("Bad packet id " + std::to_string(packetId));
				NetworkManager::field_28144_e[packetId].fetch_add(packet->getPacketSize() + 1);
				sentPacket = true;
			}

			int_t previousChunkDelay = field_20100_w;
			field_20100_w = static_cast<int_t>(static_cast<uint_t>(field_20100_w) - 1U);
			bool maySendChunk = previousChunkDelay <= 0;
			packet.reset();
			if (maySendChunk)
			{
				std::lock_guard<std::mutex> lock(sendQueueLock);
				if (!chunkDataPackets.empty())
				{
					int_t delay = manager.chunkDataSendCounter.load();
					if (delay == 0 || System::currentTimeMillis()
						- chunkDataPackets.front()->creationTimeMillis >= delay)
					{
						packet = std::move(chunkDataPackets.front());
						chunkDataPackets.pop_front();
						sendQueueByteLength -= packet->getPacketSize() + 1;
					}
				}
			}

			if (packet)
			{
				Packet::writePacket(*packet, socketOutputStream);
				int_t packetId = packet->getPacketId();
				if (packetId < 0 || packetId >= static_cast<int_t>(NetworkManager::field_28144_e.size()))
					throw std::out_of_range("Bad packet id " + std::to_string(packetId));
				NetworkManager::field_28144_e[packetId].fetch_add(packet->getPacketSize() + 1);
				field_20100_w = 0;
				sentPacket = true;
			}
			return sentPacket;
		}
		catch (const std::exception &error)
		{
			if (!isTerminating.load())
				onNetworkError(error);
		}
		catch (...)
		{
			if (!isTerminating.load())
				onUnknownNetworkError();
		}
		return false;
	}

	bool readPacket()
	{
		try
		{
			std::unique_ptr<Packet> packet = Packet::readPacket(
				socketInputStream, netHandler.isServerHandler());
			if (!packet)
			{
				networkShutdown(u"disconnect.endOfStream", std::vector<jstring>());
				return false;
			}

			int_t packetId = packet->getPacketId();
			if (packetId < 0 || packetId >= static_cast<int_t>(NetworkManager::field_28145_d.size()))
				throw std::out_of_range("Bad packet id " + std::to_string(packetId));
			NetworkManager::field_28145_d[packetId].fetch_add(packet->getPacketSize() + 1);
			{
				std::lock_guard<std::mutex> lock(readPacketsLock);
				readPackets.push_back(std::move(packet));
			}
			return true;
		}
		catch (const std::exception &error)
		{
			if (!isTerminating.load())
				onNetworkError(error);
		}
		catch (...)
		{
			if (!isTerminating.load())
				onUnknownNetworkError();
		}
		return false;
	}

	void readThreadRun()
	{
		waitForThreadsToStart();
		NetworkThreadCounter counter(NetworkManager::numReadThreads);
		std::uint64_t lastWakeSequence = 0;
		while (isRunning.load() && !isServerTerminating.load())
		{
			while (readPacket())
			{
			}
			readSignal.waitForOneHundredMilliseconds(lastWakeSequence);
		}
	}

	void writeThreadRun()
	{
		waitForThreadsToStart();
		NetworkThreadCounter counter(NetworkManager::numWriteThreads);
		std::uint64_t lastWakeSequence = 0;
		try
		{
			while (isRunning.load())
			{
				while (sendPacket())
				{
				}
				writeSignal.waitForOneHundredMilliseconds(lastWakeSequence);
				socketOutputStream.flush();
			}
		}
		catch (const std::exception &error)
		{
			if (!isTerminating.load())
				onNetworkError(error);
			std::cerr << error.what() << std::endl;
		}
		catch (...)
		{
			if (!isTerminating.load())
				onUnknownNetworkError();
			std::cerr << "Unknown exception" << std::endl;
		}
	}

	void destroy()
	{
		{
			std::lock_guard<std::mutex> lock(gracefulCloseMutex);
			destroying = true;
		}
		gracefulCloseCondition.notify_all();

		if (isRunning.load())
			networkShutdown(u"disconnect.closed", std::vector<jstring>());
		readSignal.wake();
		writeSignal.wake();

		joinThread(gracefulCloseThread);
		joinThread(terminationThread);
		joinThread(readThread);
		joinThread(writeThread);
		socket.closeSocket();
	}

public:
	Impl(NetworkManager &manager, const std::string &host, std::uint16_t port,
		NetHandler &netHandler, const std::string &connectionName)
		: manager(manager), netHandler(netHandler), socket(connectBlocking(host, port)),
		inputBuffer(socket), outputBuffer(socket), inputStream(&inputBuffer),
		outputStream(&outputBuffer), socketInputStream(inputStream),
		socketOutputStream(outputStream)
	{
		(void)connectionName;
		inputStream.exceptions(std::ios::badbit);
		outputStream.exceptions(std::ios::badbit);
		startThreads();
	}

	~Impl()
	{
		destroy();
	}

	void addToSendQueue(std::unique_ptr<Packet> packet)
	{
		if (isServerTerminating.load())
			return;
		if (!packet)
			throw std::invalid_argument("packet");

		std::lock_guard<std::mutex> lock(sendQueueLock);
		sendQueueByteLength += packet->getPacketSize() + 1;
		if (packet->isChunkDataPacket)
			chunkDataPackets.push_back(std::move(packet));
		else
			dataPackets.push_back(std::move(packet));
	}

	void wakeThreads()
	{
		readSignal.wake();
		writeSignal.wake();
	}

	void networkShutdown(const jstring &reason, const std::vector<jstring> &arguments)
	{
		std::lock_guard<std::mutex> lock(shutdownMutex);
		if (!isRunning.load())
			return;

		terminationReason = reason;
		terminationArguments = arguments;
		isTerminating.store(true);
		isRunning.store(false);
		socket.closeSocket();

		try
		{
			terminationThread = std::thread([this]()
			{
				joinThread(readThread);
				joinThread(writeThread);
			});
		}
		catch (const std::exception &error)
		{
			std::cerr << error.what() << std::endl;
		}
	}

	void processReadPackets()
	{
		{
			std::lock_guard<std::mutex> lock(sendQueueLock);
			if (sendQueueByteLength > 1048576)
				networkShutdown(u"disconnect.overflow", std::vector<jstring>());
		}

		bool readQueueEmpty;
		{
			std::lock_guard<std::mutex> lock(readPacketsLock);
			readQueueEmpty = readPackets.empty();
		}
		if (readQueueEmpty)
		{
			if (timeSinceLastRead++ == 1200)
				networkShutdown(u"disconnect.timeout", std::vector<jstring>());
		}
		else
		{
			timeSinceLastRead = 0;
		}

		int_t packetsToProcess = 100;
		while (packetsToProcess-- >= 0)
		{
			std::unique_ptr<Packet> packet;
			{
				std::lock_guard<std::mutex> lock(readPacketsLock);
				if (readPackets.empty())
					break;
				packet = std::move(readPackets.front());
				readPackets.pop_front();
			}
			packet->processPacket(netHandler);
		}

		wakeThreads();

		{
			std::lock_guard<std::mutex> lock(readPacketsLock);
			readQueueEmpty = readPackets.empty();
		}
		if (isTerminating.load() && readQueueEmpty)
		{
			jstring reason;
			std::vector<jstring> arguments;
			{
				std::lock_guard<std::mutex> lock(shutdownMutex);
				reason = terminationReason;
				arguments = terminationArguments;
			}
			netHandler.handleErrorMessage(reason, arguments);
		}
	}

	void func_28142_c()
	{
		wakeThreads();
		isServerTerminating.store(true);
		readSignal.wake();

		std::lock_guard<std::mutex> lock(gracefulCloseMutex);
		if (gracefulCloseThread.joinable())
			return;
		gracefulCloseThread = std::thread([this]()
		{
			std::unique_lock<std::mutex> closeLock(gracefulCloseMutex);
			if (gracefulCloseCondition.wait_for(closeLock, std::chrono::milliseconds(2000),
				[this]() { return destroying; }))
			{
				return;
			}
			closeLock.unlock();

			if (isRunning.load())
			{
				writeSignal.wake();
				networkShutdown(u"disconnect.closed", std::vector<jstring>());
			}
		});
	}
};

NetworkManager::NetworkManager(const std::string &host, std::uint16_t port,
	NetHandler &netHandler, const std::string &connectionName)
	: chunkDataSendCounter(0),
	impl(new Impl(*this, host, port, netHandler, connectionName))
{
}

NetworkManager::~NetworkManager() = default;

void NetworkManager::addToSendQueue(std::unique_ptr<Packet> packet)
{
	impl->addToSendQueue(std::move(packet));
}

void NetworkManager::wakeThreads()
{
	impl->wakeThreads();
}

void NetworkManager::networkShutdown(const jstring &reason,
	const std::vector<jstring> &arguments)
{
	impl->networkShutdown(reason, arguments);
}

void NetworkManager::processReadPackets()
{
	impl->processReadPackets();
}

void NetworkManager::func_28142_c()
{
	impl->func_28142_c();
}
