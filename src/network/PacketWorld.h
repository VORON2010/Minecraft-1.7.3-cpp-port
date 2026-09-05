#pragma once

#include <array>
#include <memory>
#include <vector>

#include "network/Packet.h"
#include "world/level/TilePos.h"

class Entity;

class Packet50PreChunk : public Packet
{
public:
	int_t xPosition = 0;
	int_t yPosition = 0;
	bool mode = false;

	Packet50PreChunk();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet51MapChunk : public Packet
{
public:
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t xSize = 0;
	int_t ySize = 0;
	int_t zSize = 0;
	std::vector<byte_t> chunk;

	Packet51MapChunk();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;

private:
	friend int runNetworkSmoke();
	int_t chunkSize = 0;
};

class Packet52MultiBlockChange : public Packet
{
public:
	int_t xPosition = 0;
	int_t zPosition = 0;
	std::vector<short_t> coordinateArray;
	std::vector<byte_t> typeArray;
	std::vector<byte_t> metadataArray;
	int_t size = 0;

	Packet52MultiBlockChange();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet53BlockChange : public Packet
{
public:
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t type = 0;
	int_t metadata = 0;

	Packet53BlockChange();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet54PlayNoteBlock : public Packet
{
public:
	int_t xLocation = 0;
	int_t yLocation = 0;
	int_t zLocation = 0;
	int_t instrumentType = 0;
	int_t pitch = 0;

	Packet54PlayNoteBlock();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet60Explosion : public Packet
{
public:
	double explosionX = 0.0;
	double explosionY = 0.0;
	double explosionZ = 0.0;
	float explosionSize = 0.0f;
	JavaTilePosSet destroyedBlockPositions;

	Packet60Explosion();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet61DoorChange : public Packet
{
public:
	int_t field_28050_a = 0;
	int_t field_28049_b = 0;
	int_t field_28053_c = 0;
	int_t field_28052_d = 0;
	int_t field_28051_e = 0;

	Packet61DoorChange();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet70Bed : public Packet
{
public:
	static const std::array<const jstring *, 3> field_25020_a;
	int_t field_25019_b = 0;

	Packet70Bed();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet71Weather : public Packet
{
public:
	int_t field_27054_a = 0;
	int_t field_27053_b = 0;
	int_t field_27057_c = 0;
	int_t field_27056_d = 0;
	int_t field_27055_e = 0;

	Packet71Weather();
	explicit Packet71Weather(Entity &entity);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet130UpdateSign : public Packet
{
private:
	std::shared_ptr<const std::array<jstring, 4>> referencedSignLines;
	const jstring &getSignLine(int_t line) const;

public:
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	std::vector<jstring> signLines;

	Packet130UpdateSign();
	Packet130UpdateSign(int_t xPosition, int_t yPosition, int_t zPosition,
		std::shared_ptr<const std::array<jstring, 4>> signLines);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet131MapData : public Packet
{
public:
	short_t field_28055_a = 0;
	short_t field_28054_b = 0;
	std::vector<byte_t> field_28056_c;

	Packet131MapData();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

void registerWorldPackets();
