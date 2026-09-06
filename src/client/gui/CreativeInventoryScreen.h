#pragma once

#include "client/gui/Screen.h"
#include "world/item/ItemInstance.h"
#include <vector>

class CreativeInventoryScreen : public Screen {
private:
    std::vector<ItemInstance> allItems;
    float scrollPosition = 0.0f;
    int_t guiLeft = 0;
    int_t guiTop = 0;

    static constexpr int_t COLUMNS = 8;
    static constexpr int_t ROWS = 8;
    static constexpr int_t SLOTS_PER_PAGE = COLUMNS * ROWS;

public:
    CreativeInventoryScreen(Minecraft &mc);
    void init() override;
    void render(int_t xm, int_t ym, float a) override;
    void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
    void mouseReleased(int_t x, int_t y, int_t buttonNum) override;
    void mouseScrolled(int_t x, int_t y, int_t scrollAmount) override;
    void keyPressed(char_t eventCharacter, int_t eventKey) override;
    bool isPauseScreen() override { return false; }

private:
    void renderSlot(ItemInstance *stack, int_t x, int_t y);
    int_t getSlotAtPosition(int_t x, int_t y) const;
    int_t getHotbarSlotAtPosition(int_t x, int_t y) const;
    int_t getMaxScrollRows() const;
    int_t getStartRow() const;
};
