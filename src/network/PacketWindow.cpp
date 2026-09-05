#include "network/PacketWindow.h"

#include <stdexcept>

#include "network/NetHandler.h"
#include "network/PacketDataStream.h"

namespace
{

template<typename T>
std::unique_ptr<Packet> makePacket()
{
	return std::make_unique<T>();
}

std::unique_ptr<ItemInstance> readItemStack(PacketDataInput &input)
{
	short_t itemId = input.readShort();
	if (itemId < 0)
		return nullptr;
	byte_t stackSize = input.readByte();
	short_t itemDamage = input.readShort();
	return std::make_unique<ItemInstance>(itemId, stackSize, itemDamage);
}

void writeItemStack(const ItemInstance *itemStack, PacketDataOutput &output)
{
	if (!itemStack)
	{
		output.writeShort(-1);
		return;
	}
	output.writeShort(itemStack->itemID);
	output.writeByte(itemStack->stackSize);
	output.writeShort(itemStack->getAuxValue());
}

}

Packet100OpenWindow::Packet100OpenWindow() : Packet(100)
{
}

void Packet100OpenWindow::readPacketData(PacketDataInput &input)
{
	windowId = input.readByte();
	inventoryType = input.readByte();
	windowTitle = input.readUTF();
	slotsCount = input.readByte();
}

void Packet100OpenWindow::writePacketData(PacketDataOutput &output)
{
	output.writeByte(windowId);
	output.writeByte(inventoryType);
	output.writeUTF(windowTitle);
	output.writeByte(slotsCount);
}

void Packet100OpenWindow::processPacket(NetHandler &handler)
{
	handler.func_20087_a(*this);
}

int_t Packet100OpenWindow::getPacketSize() const
{
	return 3 + static_cast<int_t>(windowTitle.size());
}

Packet101CloseWindow::Packet101CloseWindow() : Packet(101)
{
}

Packet101CloseWindow::Packet101CloseWindow(int_t windowId) : Packet(101), windowId(windowId)
{
}

void Packet101CloseWindow::readPacketData(PacketDataInput &input)
{
	windowId = input.readByte();
}

void Packet101CloseWindow::writePacketData(PacketDataOutput &output)
{
	output.writeByte(windowId);
}

void Packet101CloseWindow::processPacket(NetHandler &handler)
{
	handler.func_20092_a(*this);
}

int_t Packet101CloseWindow::getPacketSize() const
{
	return 1;
}

Packet102WindowClick::Packet102WindowClick() : Packet(102)
{
}

Packet102WindowClick::Packet102WindowClick(int_t windowId, int_t inventorySlot, int_t mouseClick,
	bool shiftClick, const ItemInstance *itemStack, short_t action)
	: Packet(102), window_Id(windowId), inventorySlot(inventorySlot), mouseClick(mouseClick), action(action),
		field_27050_f(shiftClick)
{
	if (itemStack)
		referencedItemStack = itemStack->retainReference();
}

void Packet102WindowClick::readPacketData(PacketDataInput &input)
{
	window_Id = input.readByte();
	inventorySlot = input.readShort();
	mouseClick = input.readByte();
	action = input.readShort();
	field_27050_f = input.readBoolean();
	referencedItemStack.reset();
	itemStack = readItemStack(input);
}

void Packet102WindowClick::writePacketData(PacketDataOutput &output)
{
	output.writeByte(window_Id);
	output.writeShort(inventorySlot);
	output.writeByte(mouseClick);
	output.writeShort(action);
	output.writeBoolean(field_27050_f);
	if (referencedItemStack)
	{
		ItemInstanceReference::Snapshot item = referencedItemStack->get();
		output.writeShort(item.itemID);
		output.writeByte(item.stackSize);
		output.writeShort(item.itemDamage);
	}
	else
	{
		writeItemStack(itemStack.get(), output);
	}
}

void Packet102WindowClick::processPacket(NetHandler &handler)
{
	handler.func_20091_a(*this);
}

int_t Packet102WindowClick::getPacketSize() const
{
	return 11;
}

Packet103SetSlot::Packet103SetSlot() : Packet(103)
{
}

void Packet103SetSlot::readPacketData(PacketDataInput &input)
{
	windowId = input.readByte();
	itemSlot = input.readShort();
	myItemStack = readItemStack(input);
}

void Packet103SetSlot::writePacketData(PacketDataOutput &output)
{
	output.writeByte(windowId);
	output.writeShort(itemSlot);
	writeItemStack(myItemStack.get(), output);
}

void Packet103SetSlot::processPacket(NetHandler &handler)
{
	handler.func_20088_a(*this);
}

int_t Packet103SetSlot::getPacketSize() const
{
	return 8;
}

Packet104WindowItems::Packet104WindowItems() : Packet(104)
{
}

void Packet104WindowItems::readPacketData(PacketDataInput &input)
{
	windowId = input.readByte();
	short_t count = input.readShort();
	if (count < 0)
		throw std::runtime_error("Negative array size");
	itemStack.clear();
	itemStack.resize(static_cast<std::size_t>(count));
	for (int_t i = 0; i < count; ++i)
		itemStack[static_cast<std::size_t>(i)] = readItemStack(input);
}

void Packet104WindowItems::writePacketData(PacketDataOutput &output)
{
	output.writeByte(windowId);
	output.writeShort(static_cast<int_t>(itemStack.size()));
	for (const auto &item : itemStack)
		writeItemStack(item.get(), output);
}

void Packet104WindowItems::processPacket(NetHandler &handler)
{
	handler.func_20094_a(*this);
}

int_t Packet104WindowItems::getPacketSize() const
{
	return 3 + static_cast<int_t>(itemStack.size()) * 5;
}

Packet105UpdateProgressbar::Packet105UpdateProgressbar() : Packet(105)
{
}

void Packet105UpdateProgressbar::readPacketData(PacketDataInput &input)
{
	windowId = input.readByte();
	progressBar = input.readShort();
	progressBarValue = input.readShort();
}

void Packet105UpdateProgressbar::writePacketData(PacketDataOutput &output)
{
	output.writeByte(windowId);
	output.writeShort(progressBar);
	output.writeShort(progressBarValue);
}

void Packet105UpdateProgressbar::processPacket(NetHandler &handler)
{
	handler.func_20090_a(*this);
}

int_t Packet105UpdateProgressbar::getPacketSize() const
{
	return 5;
}

Packet106Transaction::Packet106Transaction() : Packet(106)
{
}

Packet106Transaction::Packet106Transaction(int_t windowId, short_t action, bool accepted)
	: Packet(106), windowId(windowId), field_20028_b(action), field_20030_c(accepted)
{
}

void Packet106Transaction::readPacketData(PacketDataInput &input)
{
	windowId = input.readByte();
	field_20028_b = input.readShort();
	field_20030_c = input.readByte() != 0;
}

void Packet106Transaction::writePacketData(PacketDataOutput &output)
{
	output.writeByte(windowId);
	output.writeShort(field_20028_b);
	output.writeByte(field_20030_c ? 1 : 0);
}

void Packet106Transaction::processPacket(NetHandler &handler)
{
	handler.func_20089_a(*this);
}

int_t Packet106Transaction::getPacketSize() const
{
	return 4;
}

Packet200Statistic::Packet200Statistic() : Packet(200)
{
}

void Packet200Statistic::readPacketData(PacketDataInput &input)
{
	field_27052_a = input.readInt();
	field_27051_b = input.readByte();
}

void Packet200Statistic::writePacketData(PacketDataOutput &output)
{
	output.writeInt(field_27052_a);
	output.writeByte(field_27051_b);
}

void Packet200Statistic::processPacket(NetHandler &handler)
{
	handler.func_27245_a(*this);
}

int_t Packet200Statistic::getPacketSize() const
{
	return 6;
}

void registerWindowPackets()
{
	Packet::addIdClassMapping(100, true, false, &makePacket<Packet100OpenWindow>, "Packet100OpenWindow");
	Packet::addIdClassMapping(101, true, true, &makePacket<Packet101CloseWindow>, "Packet101CloseWindow");
	Packet::addIdClassMapping(102, false, true, &makePacket<Packet102WindowClick>, "Packet102WindowClick");
	Packet::addIdClassMapping(103, true, false, &makePacket<Packet103SetSlot>, "Packet103SetSlot");
	Packet::addIdClassMapping(104, true, false, &makePacket<Packet104WindowItems>, "Packet104WindowItems");
	Packet::addIdClassMapping(105, true, false, &makePacket<Packet105UpdateProgressbar>, "Packet105UpdateProgressbar");
	Packet::addIdClassMapping(106, true, true, &makePacket<Packet106Transaction>, "Packet106Transaction");
	Packet::addIdClassMapping(200, true, false, &makePacket<Packet200Statistic>, "Packet200Statistic");
}
