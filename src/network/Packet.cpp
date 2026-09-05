#include "network/Packet.h"

#include <cstring>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "java/System.h"
#include "network/PacketCore.h"
#include "network/PacketDataStream.h"
#include "network/PacketEntity.h"
#include "network/PacketWindow.h"
#include "network/PacketWorld.h"

namespace
{

int_t javaIntAdd(int_t left, int_t right)
{
	uint_t resultBits = static_cast<uint_t>(left) + static_cast<uint_t>(right);
	int_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

long_t javaLongAdd(long_t left, long_t right)
{
	ulong_t resultBits = static_cast<ulong_t>(left) + static_cast<ulong_t>(right);
	long_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

struct PacketRegistration
{
	bool clientPacket;
	bool serverPacket;
	Packet::Factory factory;
	std::string className;
};

struct PacketCounter
{
	int_t totalPackets = 0;
	long_t totalBytes = 0;

	void addPacket(int_t size)
	{
		totalPackets = javaIntAdd(totalPackets, 1);
		totalBytes = javaLongAdd(totalBytes, size);
	}
};

std::unordered_map<int_t, PacketRegistration> packetIdToRegistration;
std::unordered_map<std::string, int_t> packetClassToId;
std::unordered_set<int_t> clientPacketIds;
std::unordered_set<int_t> serverPacketIds;
std::unordered_map<int_t, PacketCounter> packetStats;
int_t totalPacketsCount = 0;
std::once_flag registryOnce;

}

Packet::Packet(int_t packetId)
	: creationTimeMillis(System::currentTimeMillis()), packetId(packetId)
{
}

int_t Packet::getPacketId() const
{
	return packetId;
}

void Packet::ensureRegistry()
{
	std::call_once(registryOnce, []()
	{
		registerCorePackets();
		registerEntityPackets();
		registerWorldPackets();
		registerWindowPackets();
	});
}

void Packet::addIdClassMapping(int_t packetId, bool clientPacket, bool serverPacket,
	Factory factory, const char *className)
{
	if (packetIdToRegistration.find(packetId) != packetIdToRegistration.end())
		throw std::invalid_argument("Duplicate packet id:" + std::to_string(packetId));

	std::string name(className);
	if (packetClassToId.find(name) != packetClassToId.end())
		throw std::invalid_argument("Duplicate packet class:" + name);

	packetIdToRegistration.emplace(packetId, PacketRegistration{clientPacket, serverPacket, factory, name});
	packetClassToId.emplace(name, packetId);
	if (clientPacket)
		clientPacketIds.insert(packetId);
	if (serverPacket)
		serverPacketIds.insert(packetId);
}

std::unique_ptr<Packet> Packet::getNewPacket(int_t packetId)
{
	ensureRegistry();
	auto registration = packetIdToRegistration.find(packetId);
	if (registration == packetIdToRegistration.end())
		return nullptr;

	try
	{
		return registration->second.factory();
	}
	catch (const std::exception &error)
	{
		std::cerr << error.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown exception constructing packet" << std::endl;
	}

	std::cout << "Skipping packet with id " << packetId << std::endl;
	return nullptr;
}

std::unique_ptr<Packet> Packet::readPacket(PacketDataInput &input, bool serverHandler)
{
	ensureRegistry();
	int_t packetId = input.read();
	if (packetId == -1)
		return nullptr;

	const auto &allowedIds = serverHandler ? serverPacketIds : clientPacketIds;
	if (allowedIds.find(packetId) == allowedIds.end())
		throw std::runtime_error("Bad packet id " + std::to_string(packetId));

	std::unique_ptr<Packet> packet = getNewPacket(packetId);
	if (!packet)
		throw std::runtime_error("Bad packet id " + std::to_string(packetId));

	try
	{
		packet->readPacketData(input);
	}
	catch (const PacketEndOfStream &)
	{
		std::cout << "Reached end of stream" << std::endl;
		return nullptr;
	}

	packetStats[packetId].addPacket(packet->getPacketSize());
	totalPacketsCount = javaIntAdd(totalPacketsCount, 1);
	if (totalPacketsCount % 1000 == 0)
	{
	}

	return packet;
}

void Packet::writePacket(Packet &packet, PacketDataOutput &output)
{
	output.write(packet.getPacketId());
	packet.writePacketData(output);
}

void Packet::writeString(const jstring &value, PacketDataOutput &output)
{
	if (value.size() > 32767)
		throw std::runtime_error("String too big");

	output.writeShort(static_cast<int_t>(value.size()));
	for (uchar_t character : value)
		output.writeChar(character);
}

jstring Packet::readString(PacketDataInput &input, int_t maximumLength)
{
	short_t length = input.readShort();
	if (length > maximumLength)
	{
		throw std::runtime_error("Received string length longer than maximum allowed (" +
			std::to_string(length) + " > " + std::to_string(maximumLength) + ")");
	}
	if (length < 0)
		throw std::runtime_error("Received string length is less than zero! Weird string!");

	jstring result;
	result.reserve(static_cast<std::size_t>(length));
	for (int_t i = 0; i < length; ++i)
		result.push_back(input.readChar());
	return result;
}
