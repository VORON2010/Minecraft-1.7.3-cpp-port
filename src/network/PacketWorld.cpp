#include "network/PacketWorld.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

#include "network/NetHandler.h"
#include "network/PacketDataStream.h"
#include "util/Mth.h"
#include "world/entity/Entity.h"
#include "world/entity/EntityLightningBolt.h"

#include "zlib.h"

namespace
{

int_t javaDoubleToInt(double value)
{
	if (std::isnan(value))
		return 0;
	if (value <= std::numeric_limits<int_t>::min())
		return std::numeric_limits<int_t>::min();
	if (value >= std::numeric_limits<int_t>::max())
		return std::numeric_limits<int_t>::max();
	return static_cast<int_t>(value);
}

int_t javaIntAdd(int_t left, int_t right)
{
	uint_t resultBits = static_cast<uint_t>(left) + static_cast<uint_t>(right);
	int_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

int_t javaIntSubtract(int_t left, int_t right)
{
	uint_t resultBits = static_cast<uint_t>(left) - static_cast<uint_t>(right);
	int_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

template<typename T>
std::unique_ptr<Packet> makePacket()
{
	return std::make_unique<T>();
}

const jstring bedNotValid = u"tile.bed.notValid";

}

Packet50PreChunk::Packet50PreChunk() : Packet(50)
{
	isChunkDataPacket = false;
}

void Packet50PreChunk::readPacketData(PacketDataInput &input)
{
	xPosition = input.readInt();
	yPosition = input.readInt();
	mode = input.read() != 0;
}

void Packet50PreChunk::writePacketData(PacketDataOutput &output)
{
	output.writeInt(xPosition);
	output.writeInt(yPosition);
	output.write(mode ? 1 : 0);
}

void Packet50PreChunk::processPacket(NetHandler &handler)
{
	handler.handlePreChunk(*this);
}

int_t Packet50PreChunk::getPacketSize() const
{
	return 9;
}

Packet51MapChunk::Packet51MapChunk() : Packet(51)
{
	isChunkDataPacket = true;
}

void Packet51MapChunk::readPacketData(PacketDataInput &input)
{
	xPosition = input.readInt();
	yPosition = input.readShort();
	zPosition = input.readInt();
	xSize = input.read() + 1;
	ySize = input.read() + 1;
	zSize = input.read() + 1;
	chunkSize = input.readInt();
	if (chunkSize < 0)
		throw std::length_error("Negative compressed chunk size");

	std::vector<byte_t> compressedChunk(static_cast<std::size_t>(chunkSize));
	input.readFully(compressedChunk.data(), compressedChunk.size());
	chunk.assign(static_cast<std::size_t>(xSize * ySize * zSize * 5 / 2), 0);

	if (chunk.empty())
		return;

	z_stream inflater{};
	inflater.next_in = reinterpret_cast<Bytef *>(compressedChunk.data());
	inflater.avail_in = static_cast<uInt>(compressedChunk.size());
	inflater.next_out = reinterpret_cast<Bytef *>(chunk.data());
	inflater.avail_out = static_cast<uInt>(chunk.size());
	if (inflateInit(&inflater) != Z_OK)
		throw std::runtime_error("Unable to initialize chunk inflater");

	int result = inflate(&inflater, Z_PARTIAL_FLUSH);
	inflateEnd(&inflater);
	if (result == Z_DATA_ERROR)
		throw std::runtime_error("Bad compressed data format");
	if (result != Z_OK && result != Z_STREAM_END && result != Z_NEED_DICT && result != Z_BUF_ERROR)
		throw std::runtime_error("Unable to inflate chunk data");
}

void Packet51MapChunk::writePacketData(PacketDataOutput &output)
{
	output.writeInt(xPosition);
	output.writeShort(yPosition);
	output.writeInt(zPosition);
	output.write(xSize - 1);
	output.write(ySize - 1);
	output.write(zSize - 1);
	output.writeInt(chunkSize);
	if (chunkSize < 0 || static_cast<std::size_t>(chunkSize) > chunk.size())
		throw std::out_of_range("Compressed chunk size exceeds chunk data");
	output.write(chunk.data(), static_cast<std::size_t>(chunkSize));
}

void Packet51MapChunk::processPacket(NetHandler &handler)
{
	handler.handleMapChunk(*this);
}

int_t Packet51MapChunk::getPacketSize() const
{
	return javaIntAdd(17, chunkSize);
}

Packet52MultiBlockChange::Packet52MultiBlockChange() : Packet(52)
{
	isChunkDataPacket = true;
}

void Packet52MultiBlockChange::readPacketData(PacketDataInput &input)
{
	xPosition = input.readInt();
	zPosition = input.readInt();
	size = static_cast<int_t>(input.readShort()) & 65535;
	coordinateArray.assign(static_cast<std::size_t>(size), 0);
	typeArray.assign(static_cast<std::size_t>(size), 0);
	metadataArray.assign(static_cast<std::size_t>(size), 0);

	for (int_t i = 0; i < size; ++i)
		coordinateArray[static_cast<std::size_t>(i)] = input.readShort();

	input.readFully(typeArray.data(), typeArray.size());
	input.readFully(metadataArray.data(), metadataArray.size());
}

void Packet52MultiBlockChange::writePacketData(PacketDataOutput &output)
{
	output.writeInt(xPosition);
	output.writeInt(zPosition);
	output.writeShort(static_cast<short_t>(size));

	for (int_t i = 0; i < size; ++i)
		output.writeShort(coordinateArray.at(static_cast<std::size_t>(i)));

	output.write(typeArray.data(), typeArray.size());
	output.write(metadataArray.data(), metadataArray.size());
}

void Packet52MultiBlockChange::processPacket(NetHandler &handler)
{
	handler.handleMultiBlockChange(*this);
}

int_t Packet52MultiBlockChange::getPacketSize() const
{
	return 10 + size * 4;
}

Packet53BlockChange::Packet53BlockChange() : Packet(53)
{
	isChunkDataPacket = true;
}

void Packet53BlockChange::readPacketData(PacketDataInput &input)
{
	xPosition = input.readInt();
	yPosition = input.read();
	zPosition = input.readInt();
	type = input.read();
	metadata = input.read();
}

void Packet53BlockChange::writePacketData(PacketDataOutput &output)
{
	output.writeInt(xPosition);
	output.write(yPosition);
	output.writeInt(zPosition);
	output.write(type);
	output.write(metadata);
}

void Packet53BlockChange::processPacket(NetHandler &handler)
{
	handler.handleBlockChange(*this);
}

int_t Packet53BlockChange::getPacketSize() const
{
	return 11;
}

Packet54PlayNoteBlock::Packet54PlayNoteBlock() : Packet(54)
{
}

void Packet54PlayNoteBlock::readPacketData(PacketDataInput &input)
{
	xLocation = input.readInt();
	yLocation = input.readShort();
	zLocation = input.readInt();
	instrumentType = input.read();
	pitch = input.read();
}

void Packet54PlayNoteBlock::writePacketData(PacketDataOutput &output)
{
	output.writeInt(xLocation);
	output.writeShort(yLocation);
	output.writeInt(zLocation);
	output.write(instrumentType);
	output.write(pitch);
}

void Packet54PlayNoteBlock::processPacket(NetHandler &handler)
{
	handler.handleNotePlay(*this);
}

int_t Packet54PlayNoteBlock::getPacketSize() const
{
	return 12;
}

Packet60Explosion::Packet60Explosion() : Packet(60)
{
}

void Packet60Explosion::readPacketData(PacketDataInput &input)
{
	explosionX = input.readDouble();
	explosionY = input.readDouble();
	explosionZ = input.readDouble();
	explosionSize = input.readFloat();
	int_t count = input.readInt();
	destroyedBlockPositions.clear();
	int_t baseX = javaDoubleToInt(explosionX);
	int_t baseY = javaDoubleToInt(explosionY);
	int_t baseZ = javaDoubleToInt(explosionZ);

	for (int_t i = 0; i < count; ++i)
	{
		int_t x = javaIntAdd(input.readByte(), baseX);
		int_t y = javaIntAdd(input.readByte(), baseY);
		int_t z = javaIntAdd(input.readByte(), baseZ);
		destroyedBlockPositions.emplace(x, y, z);
	}
}

void Packet60Explosion::writePacketData(PacketDataOutput &output)
{
	output.writeDouble(explosionX);
	output.writeDouble(explosionY);
	output.writeDouble(explosionZ);
	output.writeFloat(explosionSize);
	output.writeInt(static_cast<int_t>(destroyedBlockPositions.size()));
	int_t baseX = javaDoubleToInt(explosionX);
	int_t baseY = javaDoubleToInt(explosionY);
	int_t baseZ = javaDoubleToInt(explosionZ);

	for (const TilePos &position : destroyedBlockPositions)
	{
		output.writeByte(javaIntSubtract(position.x, baseX));
		output.writeByte(javaIntSubtract(position.y, baseY));
		output.writeByte(javaIntSubtract(position.z, baseZ));
	}
}

void Packet60Explosion::processPacket(NetHandler &handler)
{
	handler.func_12245_a(*this);
}

int_t Packet60Explosion::getPacketSize() const
{
	return 32 + static_cast<int_t>(destroyedBlockPositions.size()) * 3;
}

Packet61DoorChange::Packet61DoorChange() : Packet(61)
{
}

void Packet61DoorChange::readPacketData(PacketDataInput &input)
{
	field_28050_a = input.readInt();
	field_28053_c = input.readInt();
	field_28052_d = input.readByte();
	field_28051_e = input.readInt();
	field_28049_b = input.readInt();
}

void Packet61DoorChange::writePacketData(PacketDataOutput &output)
{
	output.writeInt(field_28050_a);
	output.writeInt(field_28053_c);
	output.writeByte(field_28052_d);
	output.writeInt(field_28051_e);
	output.writeInt(field_28049_b);
}

void Packet61DoorChange::processPacket(NetHandler &handler)
{
	handler.func_28115_a(*this);
}

int_t Packet61DoorChange::getPacketSize() const
{
	return 20;
}

const std::array<const jstring *, 3> Packet70Bed::field_25020_a = {
	&bedNotValid,
	nullptr,
	nullptr
};

Packet70Bed::Packet70Bed() : Packet(70)
{
}

void Packet70Bed::readPacketData(PacketDataInput &input)
{
	field_25019_b = input.readByte();
}

void Packet70Bed::writePacketData(PacketDataOutput &output)
{
	output.writeByte(field_25019_b);
}

void Packet70Bed::processPacket(NetHandler &handler)
{
	handler.func_25118_a(*this);
}

int_t Packet70Bed::getPacketSize() const
{
	return 1;
}

Packet71Weather::Packet71Weather() : Packet(71)
{
}

Packet71Weather::Packet71Weather(Entity &entity) : Packet(71)
{
	field_27054_a = entity.entityId;
	field_27053_b = Mth::floor(entity.x * 32.0);
	field_27057_c = Mth::floor(entity.y * 32.0);
	field_27056_d = Mth::floor(entity.z * 32.0);
	if (dynamic_cast<EntityLightningBolt *>(&entity) != nullptr)
		field_27055_e = 1;
}

void Packet71Weather::readPacketData(PacketDataInput &input)
{
	field_27054_a = input.readInt();
	field_27055_e = input.readByte();
	field_27053_b = input.readInt();
	field_27057_c = input.readInt();
	field_27056_d = input.readInt();
}

void Packet71Weather::writePacketData(PacketDataOutput &output)
{
	output.writeInt(field_27054_a);
	output.writeByte(field_27055_e);
	output.writeInt(field_27053_b);
	output.writeInt(field_27057_c);
	output.writeInt(field_27056_d);
}

void Packet71Weather::processPacket(NetHandler &handler)
{
	handler.handleWeather(*this);
}

int_t Packet71Weather::getPacketSize() const
{
	return 17;
}

Packet130UpdateSign::Packet130UpdateSign() : Packet(130)
{
	isChunkDataPacket = true;
}

Packet130UpdateSign::Packet130UpdateSign(int_t xPosition, int_t yPosition, int_t zPosition,
	std::shared_ptr<const std::array<jstring, 4>> signLines)
	: Packet(130), referencedSignLines(std::move(signLines)), xPosition(xPosition),
	  yPosition(yPosition), zPosition(zPosition)
{
	isChunkDataPacket = true;
}

const jstring &Packet130UpdateSign::getSignLine(int_t line) const
{
	if (referencedSignLines)
		return referencedSignLines->at(static_cast<std::size_t>(line));
	return signLines.at(static_cast<std::size_t>(line));
}

void Packet130UpdateSign::readPacketData(PacketDataInput &input)
{
	xPosition = input.readInt();
	yPosition = input.readShort();
	zPosition = input.readInt();
	referencedSignLines.reset();
	signLines.assign(4, jstring());

	for (int_t i = 0; i < 4; ++i)
		signLines[static_cast<std::size_t>(i)] = readString(input, 15);
}

void Packet130UpdateSign::writePacketData(PacketDataOutput &output)
{
	output.writeInt(xPosition);
	output.writeShort(yPosition);
	output.writeInt(zPosition);

	for (int_t i = 0; i < 4; ++i)
		writeString(getSignLine(i), output);
}

void Packet130UpdateSign::processPacket(NetHandler &handler)
{
	handler.handleSignUpdate(*this);
}

int_t Packet130UpdateSign::getPacketSize() const
{
	int_t result = 0;
	for (int_t i = 0; i < 4; ++i)
		result += static_cast<int_t>(getSignLine(i).size());
	return result;
}

Packet131MapData::Packet131MapData() : Packet(131)
{
	isChunkDataPacket = true;
}

void Packet131MapData::readPacketData(PacketDataInput &input)
{
	field_28055_a = input.readShort();
	field_28054_b = input.readShort();
	int_t length = static_cast<int_t>(input.readByte()) & 255;
	field_28056_c.assign(static_cast<std::size_t>(length), 0);
	input.readFully(field_28056_c.data(), field_28056_c.size());
}

void Packet131MapData::writePacketData(PacketDataOutput &output)
{
	output.writeShort(field_28055_a);
	output.writeShort(field_28054_b);
	output.writeByte(static_cast<int_t>(field_28056_c.size()));
	output.write(field_28056_c.data(), field_28056_c.size());
}

void Packet131MapData::processPacket(NetHandler &handler)
{
	handler.func_28116_a(*this);
}

int_t Packet131MapData::getPacketSize() const
{
	return 4 + static_cast<int_t>(field_28056_c.size());
}

void registerWorldPackets()
{
	Packet::addIdClassMapping(50, true, false, &makePacket<Packet50PreChunk>, "Packet50PreChunk");
	Packet::addIdClassMapping(51, true, false, &makePacket<Packet51MapChunk>, "Packet51MapChunk");
	Packet::addIdClassMapping(52, true, false, &makePacket<Packet52MultiBlockChange>, "Packet52MultiBlockChange");
	Packet::addIdClassMapping(53, true, false, &makePacket<Packet53BlockChange>, "Packet53BlockChange");
	Packet::addIdClassMapping(54, true, false, &makePacket<Packet54PlayNoteBlock>, "Packet54PlayNoteBlock");
	Packet::addIdClassMapping(60, true, false, &makePacket<Packet60Explosion>, "Packet60Explosion");
	Packet::addIdClassMapping(61, true, false, &makePacket<Packet61DoorChange>, "Packet61DoorChange");
	Packet::addIdClassMapping(70, true, false, &makePacket<Packet70Bed>, "Packet70Bed");
	Packet::addIdClassMapping(71, true, false, &makePacket<Packet71Weather>, "Packet71Weather");
	Packet::addIdClassMapping(130, true, true, &makePacket<Packet130UpdateSign>, "Packet130UpdateSign");
	Packet::addIdClassMapping(131, true, false, &makePacket<Packet131MapData>, "Packet131MapData");
}
