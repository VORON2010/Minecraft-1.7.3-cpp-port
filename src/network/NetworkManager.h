#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "java/String.h"
#include "java/Type.h"

class NetHandler;
class Packet;

class NetworkManager
{
public:
	static std::mutex threadSyncObject;
	static int_t numReadThreads;
	static int_t numWriteThreads;
	static std::array<std::atomic<int_t>, 256> field_28145_d;
	static std::array<std::atomic<int_t>, 256> field_28144_e;

	std::atomic<int_t> chunkDataSendCounter;

	NetworkManager(const std::string &host, std::uint16_t port, NetHandler &netHandler,
		const std::string &connectionName = "Client");
	~NetworkManager();

	NetworkManager(const NetworkManager &) = delete;
	NetworkManager &operator=(const NetworkManager &) = delete;

	void addToSendQueue(std::unique_ptr<Packet> packet);
	void wakeThreads();
	void networkShutdown(const jstring &reason,
		const std::vector<jstring> &arguments = std::vector<jstring>());
	void processReadPackets();
	void func_28142_c();

private:
	class Impl;
	std::unique_ptr<Impl> impl;
};
