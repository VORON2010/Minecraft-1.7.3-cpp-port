#include "client/gui/CreativeInventoryScreen.h"
#include "client/Minecraft.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/player/Player.h"
#include "world/level/tile/Tile.h"
#include "world/item/Item.h"
#include "client/renderer/Tesselator.h"
#include "util/Mth.h"
#include "client/Lighting.h"
#include "lwjgl/Keyboard.h"

// Texture layout (creative_list.png 256x256):
//   Main panel body:      (0,0)   176x164
//   Scrollbar thumb:      (208,0)   9x16
//
//   Grid slots:  7 rows x 8 cols, item content at (9,18), 18px spacing
//   Hotbar:      9 slots, item content at (9,146), 18px spacing
//   Scrollbar track:  x=170, y=14..137  (height=124, thumb=16 => range=108)

static constexpr int_t PANEL_W = 176;
static constexpr int_t PANEL_H = 208;

static constexpr int_t GRID_X = 8;
static constexpr int_t GRID_Y = 18;
static constexpr int_t SLOT_SIZE = 18;

static constexpr int_t HOTBAR_Y = 184;


// Tiles to skip (would crash or look broken in inventory)
static bool shouldSkipTile(int id) {
    switch (id) {
        case 0:   // air
        case 7:   // bedrock
        case 8: case 9:   // water
        case 10: case 11: // lava
        case 26:  // bed
        case 29: case 33: case 34: case 36: // pistons
        case 51:  // fire
        case 52:  // mob spawner
        case 55:  // redstone wire
        case 59:  // crops
        case 60:  // farmland
        case 63: case 68: // signs
        case 64: case 71: // doors
        case 74:  // redstone ore glowing
        case 75:  // redstone torch off
        case 83:  // reed
        case 90:  // portal
        case 93: case 94: // repeaters
            return true;
        default:
            return false;
    }
}

CreativeInventoryScreen::CreativeInventoryScreen(Minecraft &mc) : Screen(mc) {

    // Populate tiles (blocks)
    for (int i = 1; i < 256; i++) {
        if (Tile::tiles[i] == nullptr) continue;
        if (shouldSkipTile(i)) continue;

        if (i == 35) { // Wool variants
            for (int d = 0; d <= 15; d++) allItems.emplace_back(i, 1, d);
        } else if (i == 17 || i == 18) { // Log, Leaves variants
            for (int d = 0; d <= 2; d++) allItems.emplace_back(i, 1, d);
        } else if (i == 44) { // Slab variants
            for (int d = 0; d <= 3; d++) allItems.emplace_back(i, 1, d);
        } else {
            allItems.emplace_back(i, 1, 0);
        }
    }
    // Populate items
    for (int i = 256; i < 32000; i++) {
        if (Item::items[i] == nullptr) continue;

        if (i == 351) { // Dyes
            for (int d = 0; d <= 15; d++) allItems.emplace_back(i, 1, d);
        } else if (i == 263) { // Coal/Charcoal
            allItems.emplace_back(i, 1, 0);
            allItems.emplace_back(i, 1, 1);
        } else {
            allItems.emplace_back(i, 1, 0);
        }
    }
}

void CreativeInventoryScreen::init() {
    guiLeft = (width - PANEL_W) / 2;
    guiTop = (height - PANEL_H) / 2;
}

int_t CreativeInventoryScreen::getMaxScrollRows() const {
    int_t totalRows = (static_cast<int_t>(allItems.size()) + COLUMNS - 1) / COLUMNS;
    int_t maxRows = totalRows - ROWS;
    return maxRows > 0 ? maxRows : 0;
}

int_t CreativeInventoryScreen::getStartRow() const {
    int_t maxRows = getMaxScrollRows();
    int_t row = static_cast<int_t>(scrollPosition * maxRows + 0.5f);
    if (row < 0) row = 0;
    if (row > maxRows) row = maxRows;
    return row;
}

void CreativeInventoryScreen::render(int_t xm, int_t ym, float a) {

    renderBackground();

    // Draw main panel background
    int_t tex = minecraft.textures.loadTexture(u"/gui/allitems.png");
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    blit(guiLeft, guiTop, 0, 0, PANEL_W, PANEL_H);

    // Scrollbar rendering
    int_t trackX = guiLeft + 156;
    int_t trackY = guiTop + 18;
    int_t trackHeight = 144; // 8 rows * 18
    int_t thumbHeight = 15;
    int_t thumbY = trackY + static_cast<int_t>(scrollPosition * (trackHeight - thumbHeight));
    blit(trackX, thumbY, 2, 209, 12, 15);

    // Setup lighting exactly like InventoryScreen does (rotated light for bright items)
    glPushMatrix();
    glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
    Lighting::turnOn();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(static_cast<float>(guiLeft), static_cast<float>(guiTop), 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_RESCALE_NORMAL);

    int_t startRow = getStartRow();

    // Render grid items (7 rows x 9 cols)
    int_t hoveredX = -1, hoveredY = -1;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLUMNS; col++) {
            int_t itemIndex = (startRow + row) * COLUMNS + col;
            int_t x = GRID_X + col * SLOT_SIZE;
            int_t y = GRID_Y + row * SLOT_SIZE;

            if (itemIndex < static_cast<int_t>(allItems.size())) {
                renderSlot(&allItems[itemIndex], x, y);
                int_t ax = guiLeft + x, ay = guiTop + y;
                if (xm >= ax && xm < ax + 16 && ym >= ay && ym < ay + 16) {
                    hoveredX = x; hoveredY = y;
                }
            }
        }
    }

    // Render hotbar items
    for (int col = 0; col < 9; col++) {
        int_t x = GRID_X + col * SLOT_SIZE;
        int_t y = HOTBAR_Y;
        ItemInstance* stack = minecraft.player->inventory.getItem(col);
        if (stack) renderSlot(stack, x, y);
        int_t ax = guiLeft + x, ay = guiTop + y;
        if (xm >= ax && xm < ax + 16 && ym >= ay && ym < ay + 16) {
            hoveredX = x; hoveredY = y;
        }
    }

    // Draw hover highlight
    if (hoveredX >= 0 && hoveredY >= 0) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
        fillGradient(hoveredX, hoveredY, hoveredX + 16, hoveredY + 16, 0x80FFFFFF, 0x80FFFFFF);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }

    // Render carried (cursor) item
    ItemInstance* cursorItem = minecraft.player->inventory.getCarried();
    if (cursorItem && !cursorItem->isEmpty()) {
        int_t relX = xm - guiLeft;
        int_t relY = ym - guiTop;
        glTranslatef(0.0f, 0.0f, 32.0f);
        static ItemRenderer cursorRenderer(EntityRenderDispatcher::instance);
        cursorRenderer.renderGuiItem(font, minecraft.textures, *cursorItem, relX - 8, relY - 8);
        cursorRenderer.renderGuiItemDecorations(font, minecraft.textures, *cursorItem, relX - 8, relY - 8);
    }

    glDisable(GL_RESCALE_NORMAL);
    Lighting::turnOff();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glPopMatrix();
}

void CreativeInventoryScreen::renderSlot(ItemInstance *stack, int_t x, int_t y) {
    if (!stack || stack->isEmpty()) return;

    static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);
    itemRenderer.renderGuiItem(font, minecraft.textures, *stack, x, y);
    itemRenderer.renderGuiItemDecorations(font, minecraft.textures, *stack, x, y);
}

int_t CreativeInventoryScreen::getSlotAtPosition(int_t x, int_t y) const {
    int_t relX = x - (guiLeft + GRID_X);
    int_t relY = y - (guiTop + GRID_Y);
    if (relX >= 0 && relX < COLUMNS * SLOT_SIZE && relY >= 0 && relY < ROWS * SLOT_SIZE) {
        int_t col = relX / SLOT_SIZE;
        int_t row = relY / SLOT_SIZE;
        return row * COLUMNS + col;
    }
    return -1;
}

int_t CreativeInventoryScreen::getHotbarSlotAtPosition(int_t x, int_t y) const {
    int_t relX = x - (guiLeft + GRID_X);
    int_t relY = y - (guiTop + HOTBAR_Y);
    if (relX >= 0 && relX < 9 * SLOT_SIZE && relY >= 0 && relY < SLOT_SIZE) {
        return relX / SLOT_SIZE;
    }
    return -1;
}

void CreativeInventoryScreen::mouseClicked(int_t x, int_t y, int_t buttonNum) {
    // Scrollbar drag removed — use mouse wheel to scroll
    int_t gridSlot = getSlotAtPosition(x, y);
    int_t hotbarSlot = getHotbarSlotAtPosition(x, y);

    auto& inv = minecraft.player->inventory;
    ItemInstance* cursorItem = inv.getCarried();

    if (gridSlot != -1) {
        int_t startRow = getStartRow();
        int_t itemIndex = startRow * COLUMNS + gridSlot;
        if (itemIndex >= 0 && itemIndex < static_cast<int_t>(allItems.size())) {
            if (cursorItem) {
                inv.setCarriedNull();
            } else {
                // Left click = 1 item, middle click = stack of 64
                int_t count = (buttonNum == 2) ? 64 : 1;
                inv.setCarried(ItemInstance(allItems[itemIndex].itemID, count, allItems[itemIndex].itemDamage));
            }
        }
    } else if (hotbarSlot != -1) {
        if (cursorItem) {
            inv.setItem(hotbarSlot, *cursorItem);
            inv.setCarriedNull();
        } else {
            ItemInstance* slot = inv.getItem(hotbarSlot);
            if (slot) {
                inv.setCarried(*slot);
                inv.setItem(hotbarSlot, ItemInstance());
            }
        }
    } else {
        // Clicked outside — delete cursor item
        if (cursorItem) {
            inv.setCarriedNull();
        }
    }
}

void CreativeInventoryScreen::mouseReleased(int_t x, int_t y, int_t buttonNum) {
}

void CreativeInventoryScreen::mouseScrolled(int_t x, int_t y, int_t scrollAmount) {
    int_t maxRows = getMaxScrollRows();
    if (maxRows <= 0) return;

    if (scrollAmount != 0) {
        scrollPosition -= static_cast<float>(scrollAmount) / static_cast<float>(maxRows);
    }

    if (scrollPosition < 0.0f) scrollPosition = 0.0f;
    if (scrollPosition > 1.0f) scrollPosition = 1.0f;
}

void CreativeInventoryScreen::keyPressed(char_t eventCharacter, int_t eventKey) {
    if (eventKey == lwjgl::Keyboard::KEY_ESCAPE || eventKey == minecraft.options.keyInventory.key) {
        minecraft.setScreen(nullptr);
    }
}
