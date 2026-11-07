#include "pc/OpenGL.h"
#include "client/gui/CreativeInventoryScreen.h"
#include "client/Minecraft.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/player/Player.h"
#include "world/level/tile/Tile.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/tile/AllTiles.h"
#include "client/renderer/Tesselator.h"
#include "util/Mth.h"
#include "client/Lighting.h"
#include "client/gui/ContainerScreen.h"
#include "client/Options.h"
#include "client/gui/RecipeScreen.h"
#include "lwjgl/Keyboard.h"

// 1.0 CreativeInventoryScreen: allitems.png, 8x9 grid, hotbar at y 184, scroll bar at x 155

static constexpr int_t PANEL_W = 176;
static constexpr int_t PANEL_H = 208;

static constexpr int_t GRID_X = 8;
static constexpr int_t GRID_Y = 18;
static constexpr int_t SLOT_SIZE = 18;

static constexpr int_t HOTBAR_Y = 184;

CreativeInventoryScreen::CreativeInventoryScreen(Minecraft &mc) : Screen(mc) {
    // 1.0 CreativePlayerMenu: this exact block list, then every item but potions, then the dyes
    const Tile *blocks[] = {
        &Tile::cobblestone, &Tile::rock, &Tile::diamondOre, &Tile::goldOre, &Tile::ironOre, &Tile::coalOre, &Tile::lapisOre, &Tile::redstoneOre,
        &Tile::stoneBrick, &Tile::stoneBrick, &Tile::stoneBrick, &Tile::clay, &Tile::diamondBlock, &Tile::goldBlock, &Tile::ironBlock, &Tile::bedrock,
        &Tile::lapisBlock, &Tile::brick, &Tile::mossyCobblestone, &Tile::slabSingle, &Tile::slabSingle, &Tile::slabSingle, &Tile::slabSingle, &Tile::slabSingle,
        &Tile::slabSingle, &Tile::obsidian, &Tile::netherrack, &Tile::soulSand, &Tile::glowstone, &Tile::treeTrunk, &Tile::treeTrunk, &Tile::treeTrunk,
        &Tile::leaves, &Tile::leaves, &Tile::leaves, &Tile::dirt, &Tile::grass, &Tile::sand, &Tile::sandstone, &Tile::gravel,
        &Tile::cobweb, &Tile::wood, &Tile::sapling, &Tile::sapling, &Tile::sapling, &Tile::deadBush, &Tile::sponge, &Tile::ice,
        &Tile::snow, &Tile::flower, &Tile::rose, &Tile::brownMushroom, &Tile::redMushroom, &Tile::cactus, &Tile::melon, &Tile::pumpkin,
        &Tile::jackOLantern, &Tile::vine, &Tile::ironBars, &Tile::glassPane, &Tile::netherBrick, &Tile::netherFence, &Tile::stairsNetherBrick, &Tile::endStone,
        &Tile::mycelium, &Tile::waterlily, &Tile::tallGrass, &Tile::tallGrass, &Tile::chest, &Tile::workBench, &Tile::glass, &Tile::tnt,
        &Tile::bookshelf,
        &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool,
        &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool, &Tile::wool,
        &Tile::dispenser, &Tile::furnace, &Tile::noteBlock, &Tile::jukebox, &Tile::pistonStickyBase, &Tile::pistonBase, &Tile::fence, &Tile::fenceGate,
        &Tile::ladder, &Tile::rail, &Tile::railPowered, &Tile::railDetector, &Tile::torch, &Tile::stairsWood, &Tile::stairsStone, &Tile::stairsBrick,
        &Tile::stairsStoneBrick, &Tile::lever, &Tile::pressurePlateStone, &Tile::pressurePlateWood, &Tile::torchRedstoneActive, &Tile::buttonStone, &Tile::trapdoor,
    };
    int_t wool = 0, slab = 0, log = 0, sapling = 0, stoneBrick = 0, leaves = 0, tallGrass = 1;
    for (const Tile *tile : blocks) {
        int_t data = 0;
        if (tile == &Tile::wool) data = wool++;
        else if (tile == &Tile::slabSingle) data = slab++;
        else if (tile == &Tile::treeTrunk) data = log++;
        else if (tile == &Tile::sapling) data = sapling++;
        else if (tile == &Tile::stoneBrick) data = stoneBrick++;
        else if (tile == &Tile::tallGrass) data = tallGrass++;
        else if (tile == &Tile::leaves) data = leaves++;
        allItems.emplace_back(tile->id, 1, data);
    }
    for (int_t i = 256; i < static_cast<int_t>(Item::items.size()); i++) {
        if (Item::items[i] != nullptr)
            allItems.emplace_back(i, 1, 0);
    }
    for (int_t d = 1; d < 16; d++)
        allItems.emplace_back(Items::dyePowder->getShiftedIndex(), 1, d);
}

void CreativeInventoryScreen::init() {
    guiLeft = (width - PANEL_W) / 2;
    guiTop = (height - PANEL_H) / 2;
}

int_t CreativeInventoryScreen::getMaxScrollRows() const {
    // 1.0: items.size() / 8 - 8 + 1
    int_t maxRows = static_cast<int_t>(allItems.size()) / COLUMNS - 8 + 1;
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

    if (isDragging) {
        if (!lwjgl::Mouse::isButtonDown(0)) {
            isDragging = false;
        } else {
            int_t top = guiTop + 17;
            int_t bottom = top + 160 + 2;
            float newScroll = static_cast<float>(ym - (top + 8)) / (static_cast<float>(bottom - top) - 16.0f);
            scrollPosition = std::max(0.0f, std::min(1.0f, newScroll));
        }
    }

    // Draw main panel background
    int_t tex = minecraft.textures.loadTexture(u"/gui/allitems.png");
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    blit(guiLeft, guiTop, 0, 0, PANEL_W, PANEL_H);

    // Scroll bar thumb (1.0 renderMenuBackground)
    int_t top = guiTop + 17;
    int_t bottom = top + 160 + 2;
    blit(guiLeft + 154, guiTop + 17 + static_cast<int_t>((bottom - top - 17) * scrollPosition), 0, 208, 16, 16);
    font.draw(u"Item selection", guiLeft + 8, guiTop + 6, 4210752);

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

    // Tooltips
    if (!cursorItem || cursorItem->isEmpty()) {
        int_t hoveredItemIndex = getSlotAtPosition(xm, ym);
        if (hoveredItemIndex == -1) {
            int_t hotbarCol = getHotbarSlotAtPosition(xm, ym);
            if (hotbarCol != -1) {
                ItemInstance* hStack = minecraft.player->inventory.getItem(hotbarCol);
                if (hStack && !hStack->isEmpty()) {
                    auto lines = ContainerScreen::getTooltipLines(*hStack, &minecraft.options);
                    if (!lines.empty())
                        renderTooltip(lines, xm + 12, ym - 12);
                }
            }
        } else {
            if (hoveredItemIndex >= 0 && hoveredItemIndex < static_cast<int_t>(allItems.size())) {
                auto lines = ContainerScreen::getTooltipLines(allItems[hoveredItemIndex], &minecraft.options);
                if (!lines.empty())
                    renderTooltip(lines, xm + 12, ym - 12);
            }
        }
    }
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
        return (getStartRow() + row) * COLUMNS + col;
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
    int_t barX = guiLeft + 155;
    int_t barTop = guiTop + 17;
    if (buttonNum == 0 && x >= barX && x < barX + 14 && y >= barTop && y < barTop + 160 + 2) {
        isDragging = true;
        return;
    }
    bool quickMove = lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) || lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT);
    int_t gridSlot = getSlotAtPosition(x, y);
    int_t hotbarSlot = getHotbarSlotAtPosition(x, y);

    auto &inv = minecraft.player->inventory;
    ItemInstance *carried = inv.getCarried();
    bool hasCarried = carried != nullptr && !carried->isEmpty();

    if (gridSlot != -1) {
        // 1.0: the item list hands out copies; clicking with the same item grows or shrinks it
        ItemInstance *slotItem = gridSlot < static_cast<int_t>(allItems.size()) ? &allItems[gridSlot] : nullptr;
        if (hasCarried && slotItem != nullptr && carried->itemID == slotItem->itemID) {
            if (buttonNum == 0) {
                if (quickMove)
                    carried->stackSize = carried->getMaxStackSize();
                else if (carried->stackSize < carried->getMaxStackSize())
                    carried->stackSize = carried->stackSize + 1;
            } else if (carried->stackSize <= 1) {
                inv.setCarriedNull();
            } else {
                carried->stackSize = carried->stackSize - 1;
            }
        } else if (hasCarried || slotItem == nullptr) {
            inv.setCarriedNull();
        } else {
            ItemInstance copy(slotItem->itemID, 1, slotItem->itemDamage);
            if (quickMove)
                copy.stackSize = copy.getMaxStackSize();
            inv.setCarried(copy);
        }
    } else if (hotbarSlot != -1) {
        // hotbar slots behave like normal inventory slots
        ItemInstance *slot = inv.getItem(hotbarSlot);
        bool slotEmpty = slot == nullptr || slot->isEmpty();
        if (buttonNum == 0) {
            if (hasCarried && !slotEmpty && slot->itemID == carried->itemID && slot->itemDamage == carried->itemDamage) {
                int_t room = slot->getMaxStackSize() - slot->stackSize;
                int_t move = std::min<int_t>(room, carried->stackSize);
                slot->stackSize = slot->stackSize + move;
                carried->stackSize = carried->stackSize - move;
                if (carried->stackSize <= 0)
                    inv.setCarriedNull();
            } else {
                ItemInstance oldSlot = slotEmpty ? ItemInstance() : *slot;
                if (hasCarried)
                    inv.setItem(hotbarSlot, *carried);
                else
                    inv.setItem(hotbarSlot, ItemInstance());
                if (slotEmpty)
                    inv.setCarriedNull();
                else
                    inv.setCarried(oldSlot);
            }
        } else if (buttonNum == 1) {
            if (hasCarried) {
                if (slotEmpty) {
                    inv.setItem(hotbarSlot, ItemInstance(carried->itemID, 1, carried->itemDamage));
                    carried->stackSize = carried->stackSize - 1;
                } else if (slot->itemID == carried->itemID && slot->itemDamage == carried->itemDamage && slot->stackSize < slot->getMaxStackSize()) {
                    slot->stackSize = slot->stackSize + 1;
                    carried->stackSize = carried->stackSize - 1;
                }
                if (carried->stackSize <= 0)
                    inv.setCarriedNull();
            } else if (!slotEmpty) {
                int_t half = (slot->stackSize + 1) / 2;
                inv.setCarried(ItemInstance(slot->itemID, half, slot->itemDamage));
                slot->stackSize = slot->stackSize - half;
                if (slot->stackSize <= 0)
                    inv.setItem(hotbarSlot, ItemInstance());
            }
        }
    } else if (hasCarried && (x < guiLeft || y < guiTop || x >= guiLeft + PANEL_W || y >= guiTop + PANEL_H)) {
        // 1.0: clicking outside the window throws the carried item
        if (buttonNum == 0) {
            minecraft.player->drop(*carried);
            inv.setCarriedNull();
        } else if (buttonNum == 1) {
            ItemInstance one(carried->itemID, 1, carried->itemDamage);
            minecraft.player->drop(one);
            carried->stackSize = carried->stackSize - 1;
            if (carried->stackSize <= 0)
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

    if (eventKey == lwjgl::Keyboard::KEY_F4) {
        int_t xm = lwjgl::Mouse::getEventX() * width / minecraft.width;
        int_t ym = height - lwjgl::Mouse::getEventY() * height / minecraft.height - 1;

        int_t itemIndex = getSlotAtPosition(xm, ym);
        if (itemIndex >= 0 && itemIndex < static_cast<int_t>(allItems.size())) {
            minecraft.setScreen(Util::make_shared<RecipeScreen>(minecraft, minecraft.screen, allItems[itemIndex].itemID, allItems[itemIndex].itemDamage));
        } else {
            int_t hotbarCol = getHotbarSlotAtPosition(xm, ym);
            if (hotbarCol != -1) {
                ItemInstance* stack = minecraft.player->inventory.getItem(hotbarCol);
                if (stack && !stack->isEmpty()) {
                    minecraft.setScreen(Util::make_shared<RecipeScreen>(minecraft, minecraft.screen, stack->itemID, stack->itemDamage));
                }
            }
        }
    }
}
