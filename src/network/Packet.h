#pragma once

#include <iosfwd>
#include <memory>

#include "java/String.h"
#include "java/Type.h"

class NetHandler;
class PacketDataInput;
class PacketDataOutput;

class Packet
{
public:
	using Factory = std::unique_ptr<Packet> (*)();

	const long_t creationTimeMillis;
	bool isChunkDataPacket = false;

protected:
	explicit Packet(int_t packetId);

public:
	virtual ~Packet() = default;

	int_t getPacketId() const;
	virtual void readPacketData(PacketDataInput &input) = 0;
	virtual void writePacketData(PacketDataOutput &output) = 0;
	virtual void processPacket(NetHandler &handler) = 0;
	virtual int_t getPacketSize() const = 0;

	static std::unique_ptr<Packet> getNewPacket(int_t packetId);
	static std::unique_ptr<Packet> readPacket(PacketDataInput &input, bool serverHandler);
	static void writePacket(Packet &packet, PacketDataOutput &output);
	static void writeString(const jstring &value, PacketDataOutput &output);
	static jstring readString(PacketDataInput &input, int_t maximumLength);

	static void addIdClassMapping(int_t packetId, bool clientPacket, bool serverPacket,
		Factory factory, const char *className);

private:
	const int_t packetId;
	static void ensureRegistry();
};
