#pragma once

#include <memory>
#include <vector>

#include "network/Packet.h"
#include "world/item/ItemInstance.h"

class Packet100OpenWindow : public Packet
{
public:
	int_t windowId = 0;
	int_t inventoryType = 0;
	jstring windowTitle;
	int_t slotsCount = 0;

	Packet100OpenWindow();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet101CloseWindow : public Packet
{
public:
	int_t windowId = 0;

	Packet101CloseWindow();
	explicit Packet101CloseWindow(int_t windowId);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet102WindowClick : public Packet
{
private:
	std::shared_ptr<const ItemInstanceReference> referencedItemStack;

public:
	int_t window_Id = 0;
	int_t inventorySlot = 0;
	int_t mouseClick = 0;
	short_t action = 0;
	std::unique_ptr<ItemInstance> itemStack;
	bool field_27050_f = false;

	Packet102WindowClick();
	Packet102WindowClick(int_t windowId, int_t inventorySlot, int_t mouseClick, bool shiftClick,
		const ItemInstance *itemStack, short_t action);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet103SetSlot : public Packet
{
public:
	int_t windowId = 0;
	int_t itemSlot = 0;
	std::unique_ptr<ItemInstance> myItemStack;

	Packet103SetSlot();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet104WindowItems : public Packet
{
public:
	int_t windowId = 0;
	std::vector<std::unique_ptr<ItemInstance>> itemStack;

	Packet104WindowItems();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet105UpdateProgressbar : public Packet
{
public:
	int_t windowId = 0;
	int_t progressBar = 0;
	int_t progressBarValue = 0;

	Packet105UpdateProgressbar();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet106Transaction : public Packet
{
public:
	int_t windowId = 0;
	short_t field_20028_b = 0;
	bool field_20030_c = false;

	Packet106Transaction();
	Packet106Transaction(int_t windowId, short_t action, bool accepted);
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

class Packet200Statistic : public Packet
{
public:
	int_t field_27052_a = 0;
	int_t field_27051_b = 0;

	Packet200Statistic();
	void readPacketData(PacketDataInput &input) override;
	void writePacketData(PacketDataOutput &output) override;
	void processPacket(NetHandler &handler) override;
	int_t getPacketSize() const override;
};

void registerWindowPackets();
