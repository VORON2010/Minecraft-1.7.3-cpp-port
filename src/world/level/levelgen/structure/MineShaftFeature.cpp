#include "world/level/levelgen/structure/MineShaftFeature.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>

#include "java/Random.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/biome/Biome.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/material/Material.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/ChestTileEntity.h"
#include "world/level/tile/entity/MobSpawnerTileEntity.h"
#include "world/entity/npc/Villager.h"

namespace
{
struct MathHelper
{
	static int_t nextInt(Random &random, int_t a, int_t b) { return a >= b ? a : random.nextInt(b - a + 1) + a; }
};

template <typename T>
T jremove(std::vector<T> &v, int_t i)
{
	T x = v[static_cast<size_t>(i)];
	v.erase(v.begin() + i);
	return x;
}
}

#include "world/level/tile/BedTile.h"
#include "world/level/tile/BookshelfTile.h"
#include "world/level/tile/ButtonTile.h"
#include "world/level/tile/CactusTile.h"
#include "world/level/tile/CakeTile.h"
#include "world/level/tile/ChestTile.h"
#include "world/level/tile/ClayTile.h"
#include "world/level/tile/ClothTile.h"
#include "world/level/tile/CropsTile.h"
#include "world/level/tile/DeadBushTile.h"
#include "world/level/tile/DetectorRailTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/DispenserTile.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/EndPortalFrameTile.h"
#include "world/level/tile/FarmlandTile.h"
#include "world/level/tile/FenceGateTile.h"
#include "world/level/tile/FenceTile.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/FlowerTile.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/GlassTile.h"
#include "world/level/tile/GlowStoneTile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/GravelTile.h"
#include "world/level/tile/HugeMushroomTile.h"
#include "world/level/tile/IceTile.h"
#include "world/level/tile/InfestedTile.h"
#include "world/level/tile/JukeboxTile.h"
#include "world/level/tile/LadderTile.h"
#include "world/level/tile/LeafTile.h"
#include "world/level/tile/LeverTile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/LockedChestTile.h"
#include "world/level/tile/MelonTile.h"
#include "world/level/tile/MobSpawnerTile.h"
#include "world/level/tile/MushroomTile.h"
#include "world/level/tile/MyceliumTile.h"
#include "world/level/tile/NotGateTile.h"
#include "world/level/tile/NoteTile.h"
#include "world/level/tile/OreTile.h"
#include "world/level/tile/PaneTile.h"
#include "world/level/tile/PistonBaseTile.h"
#include "world/level/tile/PistonExtensionTile.h"
#include "world/level/tile/PistonMovingTile.h"
#include "world/level/tile/PistonTextures.h"
#include "world/level/tile/PortalTile.h"
#include "world/level/tile/PressurePlateTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/level/tile/RailLogic.h"
#include "world/level/tile/RailTile.h"
#include "world/level/tile/RedStoneDustTile.h"
#include "world/level/tile/RedstoneOreTile.h"
#include "world/level/tile/ReedTile.h"
#include "world/level/tile/RepeaterTile.h"
#include "world/level/tile/SandStoneTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/SaplingTile.h"
#include "world/level/tile/SignTile.h"
#include "world/level/tile/SlabTile.h"
#include "world/level/tile/SnowBlockTile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/SoulSandTile.h"
#include "world/level/tile/SpongeTile.h"
#include "world/level/tile/StairTile.h"
#include "world/level/tile/StemTile.h"
#include "world/level/tile/StepSound.h"
#include "world/level/tile/StoneBrickTile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/TNTTile.h"
#include "world/level/tile/TallGrassTile.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/TorchTile.h"
#include "world/level/tile/TransparentTile.h"
#include "world/level/tile/TrapDoorTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/VineTile.h"
#include "world/level/tile/WaterlilyTile.h"
#include "world/level/tile/WebTile.h"
#include "world/level/tile/WoodTile.h"
#include "world/level/tile/WorkbenchTile.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/ItemArmor.h"

class MineshaftPieces;
class MineshaftPieces__Corridor;
class MineshaftPieces__Crossing;
class MineshaftPieces__Room;
class MineshaftPieces__Stairs;
enum : int_t { CLS_MineshaftPieces = 1, CLS_MineshaftPieces__Corridor = 2, CLS_MineshaftPieces__Crossing = 3, CLS_MineshaftPieces__Room = 4, CLS_MineshaftPieces__Stairs = 5 };

class MineshaftPieces
{
public:
	static std::vector<WeighedTreasureItem> &LOOT_ENTRIES();
	static std::shared_ptr<StructurePiece> createPiece(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
	static std::shared_ptr<StructurePiece> generateNextPiece(std::shared_ptr<StructurePiece> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth);
};

class MineshaftPieces__Corridor : public StructurePiece
{
public:
	using Base = StructurePiece;
	bool hasRails;
	bool hasCobwebs;
	bool hasSpiderSpawner;
	int_t sections;
	MineshaftPieces__Corridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	static BoundingBox findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class MineshaftPieces__Crossing : public StructurePiece
{
public:
	using Base = StructurePiece;
	int_t direction;
	bool hasTwoFloors;
	MineshaftPieces__Crossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t direction);
	static BoundingBox findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class MineshaftPieces__Room : public StructurePiece
{
public:
	using Base = StructurePiece;
	std::vector<BoundingBox> entrances;
	MineshaftPieces__Room(int_t generationDepth, Random & random, int_t x, int_t z);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

class MineshaftPieces__Stairs : public StructurePiece
{
public:
	using Base = StructurePiece;
	MineshaftPieces__Stairs(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing);
	static BoundingBox findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing);
	virtual void addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random);
	virtual bool postProcess(Level & level, Random & random, const BoundingBox &chunkBB);
};

std::vector<WeighedTreasureItem> &MineshaftPieces::LOOT_ENTRIES()
{
	static std::vector<WeighedTreasureItem> value = std::vector<WeighedTreasureItem>{
      WeighedTreasureItem{Items::ingotIron->getShiftedIndex(), 0, 1, 5, 10},
      WeighedTreasureItem{Items::ingotGold->getShiftedIndex(), 0, 1, 3, 5},
      WeighedTreasureItem{Items::redstone->getShiftedIndex(), 0, 4, 9, 5},
      WeighedTreasureItem{Items::dyePowder->getShiftedIndex(), 4, 4, 9, 5},
      WeighedTreasureItem{Items::diamond->getShiftedIndex(), 0, 1, 2, 3},
      WeighedTreasureItem{Items::coal->getShiftedIndex(), 0, 3, 8, 10},
      WeighedTreasureItem{Items::bread->getShiftedIndex(), 0, 1, 3, 15},
      WeighedTreasureItem{Items::pickaxeIron->getShiftedIndex(), 0, 1, 1, 1},
      WeighedTreasureItem{Tile::rail.id, 0, 4, 8, 1},
      WeighedTreasureItem{Items::melonSeeds->getShiftedIndex(), 0, 2, 4, 10},
      WeighedTreasureItem{Items::pumpkinSeeds->getShiftedIndex(), 0, 2, 4, 10}
   };
	return value;
}

std::shared_ptr<StructurePiece> MineshaftPieces::createPiece(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      int_t var7 = random.nextInt(100);
      if (var7 >= 80) {
         BoundingBox var8 = MineshaftPieces__Crossing::findSize(pieces, random, x, y, z, facing);
         if (var8.valid) {
            return std::make_shared<MineshaftPieces__Crossing>(generationDepth, random, var8, facing);
         }
      } else if (var7 >= 70) {
         BoundingBox var9 = MineshaftPieces__Stairs::findSize(pieces, random, x, y, z, facing);
         if (var9.valid) {
            return std::make_shared<MineshaftPieces__Stairs>(generationDepth, random, var9, facing);
         }
      } else {
         BoundingBox var10 = MineshaftPieces__Corridor::findSize(pieces, random, x, y, z, facing);
         if (var10.valid) {
            return std::make_shared<MineshaftPieces__Corridor>(generationDepth, random, var10, facing);
         }
      }

      return nullptr;
   }

std::shared_ptr<StructurePiece> MineshaftPieces::generateNextPiece(std::shared_ptr<StructurePiece> startPiece, PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing, int_t generationDepth)
{
      if (generationDepth > 8) {
         return nullptr;
      } else if (std::abs(x - startPiece->getBoundingBox().x0) <= 80 && std::abs(z - startPiece->getBoundingBox().z0) <= 80) {
         std::shared_ptr<StructurePiece> var8 = createPiece(pieces, random, x, y, z, facing, generationDepth + 1);
         if (var8 != nullptr) {
            pieces.push_back(var8);
            var8->addChildren(startPiece, pieces, random);
         }

         return var8;
      } else {
         return nullptr;
      }
   }

MineshaftPieces__Corridor::MineshaftPieces__Corridor(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StructurePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
      this->hasRails = random.nextInt(3) == 0;
      this->hasCobwebs = !this->hasRails && random.nextInt(23) == 0;
      if (this->orientation != 2 && this->orientation != 0) {
         this->sections = bb.getXSpan() / 5;
      } else {
         this->sections = bb.getZSpan() / 5;
      }
   }

BoundingBox MineshaftPieces__Corridor::findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing)
{
      BoundingBox var6 = BoundingBox(x, y, z, x, y + 2, z);

      int_t var7;
      for (var7 = random.nextInt(3) + 2; var7 > 0; var7--) {
         int_t var8 = var7 * 5;
         switch (facing) {
            case 0:
               var6.x1 = x + 2;
               var6.z1 = z + (var8 - 1);
               break;
            case 1:
               var6.x0 = x - (var8 - 1);
               var6.z1 = z + 2;
               break;
            case 2:
               var6.x1 = x + 2;
               var6.z0 = z - (var8 - 1);
               break;
            case 3:
               var6.x1 = x + (var8 - 1);
               var6.z1 = z + 2;
         }

         if (StructurePiece::findCollisionPiece(pieces, var6) == nullptr) {
            break;
         }
      }

      return var7 > 0 ? var6 : BoundingBox::none();
   }

void MineshaftPieces__Corridor::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      int_t var4 = this->getGenDepth();
      int_t var5 = random.nextInt(4);
      switch (this->orientation) {
         case 0:
            if (var5 <= 1) {
               MineshaftPieces::generateNextPiece(
                  start, pieces, random, this->boundingBox.x0, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z1 + 1, this->orientation, var4
               );
            } else if (var5 == 2) {
               MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z1 - 3, 1, var4);
            } else {
               MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z1 - 3, 3, var4);
            }
            break;
         case 1:
            if (var5 <= 1) {
               MineshaftPieces::generateNextPiece(
                  start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z0, this->orientation, var4
               );
            } else if (var5 == 2) {
               MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z0 - 1, 2, var4);
            } else {
               MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z1 + 1, 0, var4);
            }
            break;
         case 2:
            if (var5 <= 1) {
               MineshaftPieces::generateNextPiece(
                  start, pieces, random, this->boundingBox.x0, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z0 - 1, this->orientation, var4
               );
            } else if (var5 == 2) {
               MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z0, 1, var4);
            } else {
               MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z0, 3, var4);
            }
            break;
         case 3:
            if (var5 <= 1) {
               MineshaftPieces::generateNextPiece(
                  start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z0, this->orientation, var4
               );
            } else if (var5 == 2) {
               MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 - 3, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z0 - 1, 2, var4);
            } else {
               MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 - 3, this->boundingBox.y0 - 1 + random.nextInt(3), this->boundingBox.z1 + 1, 0, var4);
            }
      }

      if (var4 < 8) {
         if (this->orientation != 2 && this->orientation != 0) {
            for (int_t var8 = this->boundingBox.x0 + 3; var8 + 3 <= this->boundingBox.x1; var8 += 5) {
               int_t var9 = random.nextInt(5);
               if (var9 == 0) {
                  MineshaftPieces::generateNextPiece(start, pieces, random, var8, this->boundingBox.y0, this->boundingBox.z0 - 1, 2, var4 + 1);
               } else if (var9 == 1) {
                  MineshaftPieces::generateNextPiece(start, pieces, random, var8, this->boundingBox.y0, this->boundingBox.z1 + 1, 0, var4 + 1);
               }
            }
         } else {
            for (int_t var6 = this->boundingBox.z0 + 3; var6 + 3 <= this->boundingBox.z1; var6 += 5) {
               int_t var7 = random.nextInt(5);
               if (var7 == 0) {
                  MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0, var6, 1, var4 + 1);
               } else if (var7 == 1) {
                  MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0, var6, 3, var4 + 1);
               }
            }
         }
      }
   }

bool MineshaftPieces__Corridor::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         int_t var8 = this->sections * 5 - 1;
         this->generateBox(level, chunkBB, 0, 0, 0, 2, 1, var8, 0, 0, false);
         this->generateMaybeBox(level, chunkBB, random, 0.8f, 0, 2, 0, 2, 2, var8, 0, 0, false);
         if (this->hasCobwebs) {
            this->generateMaybeBox(level, chunkBB, random, 0.6f, 0, 0, 0, 2, 1, var8, Tile::cobweb.id, 0, false);
         }

         for (int_t var9 = 0; var9 < this->sections; var9++) {
            int_t var10 = 2 + var9 * 5;
            this->generateBox(level, chunkBB, 0, 0, var10, 0, 1, var10, Tile::fence.id, 0, false);
            this->generateBox(level, chunkBB, 2, 0, var10, 2, 1, var10, Tile::fence.id, 0, false);
            if (random.nextInt(4) != 0) {
               this->generateBox(level, chunkBB, 0, 2, var10, 2, 2, var10, Tile::wood.id, 0, false);
            } else {
               this->generateBox(level, chunkBB, 0, 2, var10, 0, 2, var10, Tile::wood.id, 0, false);
               this->generateBox(level, chunkBB, 2, 2, var10, 2, 2, var10, Tile::wood.id, 0, false);
            }

            this->maybeGenerateBlock(level, chunkBB, random, 0.1f, 0, 2, var10 - 1, Tile::cobweb.id, 0);
            this->maybeGenerateBlock(level, chunkBB, random, 0.1f, 2, 2, var10 - 1, Tile::cobweb.id, 0);
            this->maybeGenerateBlock(level, chunkBB, random, 0.1f, 0, 2, var10 + 1, Tile::cobweb.id, 0);
            this->maybeGenerateBlock(level, chunkBB, random, 0.1f, 2, 2, var10 + 1, Tile::cobweb.id, 0);
            this->maybeGenerateBlock(level, chunkBB, random, 0.05f, 0, 2, var10 - 2, Tile::cobweb.id, 0);
            this->maybeGenerateBlock(level, chunkBB, random, 0.05f, 2, 2, var10 - 2, Tile::cobweb.id, 0);
            this->maybeGenerateBlock(level, chunkBB, random, 0.05f, 0, 2, var10 + 2, Tile::cobweb.id, 0);
            this->maybeGenerateBlock(level, chunkBB, random, 0.05f, 2, 2, var10 + 2, Tile::cobweb.id, 0);
            this->maybeGenerateBlock(level, chunkBB, random, 0.05f, 1, 2, var10 - 1, Tile::torch.id, 0);
            this->maybeGenerateBlock(level, chunkBB, random, 0.05f, 1, 2, var10 + 1, Tile::torch.id, 0);
            if (random.nextInt(100) == 0) {
               this->createChest(level, chunkBB, random, 2, 0, var10 - 1, MineshaftPieces::LOOT_ENTRIES(), 3 + random.nextInt(4));
            }

            if (random.nextInt(100) == 0) {
               this->createChest(level, chunkBB, random, 0, 0, var10 + 1, MineshaftPieces::LOOT_ENTRIES(), 3 + random.nextInt(4));
            }

            if (this->hasCobwebs && !this->hasSpiderSpawner) {
               int_t var11 = this->getWorldY(0);
               int_t var12 = var10 - 1 + random.nextInt(3);
               int_t var13 = this->getWorldX(1, var12);
               var12 = this->getWorldZ(1, var12);
               if (chunkBB.isInside(var13, var11, var12)) {
                  this->hasSpiderSpawner = true;
                  level.setTile(var13, var11, var12, Tile::mobSpawner.id);
                  std::shared_ptr<MobSpawnerTileEntity> var14 = std::dynamic_pointer_cast<MobSpawnerTileEntity>(level.getTileEntity(var13, var11, var12));
                  if (var14 != nullptr) {
                     var14->setEntityId(u"CaveSpider");
                  }
               }
            }
         }

         if (this->hasRails) {
            for (int_t var15 = 0; var15 <= var8; var15++) {
               int_t var16 = this->getBlock(level, 1, -1, var15, chunkBB);
               if (var16 > 0 && Tile::solid[var16]) {
                  this->maybeGenerateBlock(level, chunkBB, random, 0.7f, 1, 0, var15, Tile::rail.id, this->getOrientationData(Tile::rail.id, 0));
               }
            }
         }

         return true;
      }
   }

MineshaftPieces__Crossing::MineshaftPieces__Crossing(int_t generationDepth, Random & random, const BoundingBox & bb, int_t direction) : StructurePiece(generationDepth)
{
      this->direction = direction;
      this->boundingBox = bb;
      this->hasTwoFloors = bb.getYSpan() > 3;
   }

BoundingBox MineshaftPieces__Crossing::findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing)
{
      BoundingBox var6 = BoundingBox(x, y, z, x, y + 2, z);
      if (random.nextInt(4) == 0) {
         var6.y1 += 4;
      }

      switch (facing) {
         case 0:
            var6.x0 = x - 1;
            var6.x1 = x + 3;
            var6.z1 = z + 4;
            break;
         case 1:
            var6.x0 = x - 4;
            var6.z0 = z - 1;
            var6.z1 = z + 3;
            break;
         case 2:
            var6.x0 = x - 1;
            var6.x1 = x + 3;
            var6.z0 = z - 4;
            break;
         case 3:
            var6.x1 = x + 4;
            var6.z0 = z - 1;
            var6.z1 = z + 3;
      }

      return StructurePiece::findCollisionPiece(pieces, var6) != nullptr ? BoundingBox::none() : var6;
   }

void MineshaftPieces__Crossing::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      int_t var4 = this->getGenDepth();
      switch (this->direction) {
         case 0:
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y0, this->boundingBox.z1 + 1, 0, var4);
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0, this->boundingBox.z0 + 1, 1, var4);
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0, this->boundingBox.z0 + 1, 3, var4);
            break;
         case 1:
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y0, this->boundingBox.z0 - 1, 2, var4);
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y0, this->boundingBox.z1 + 1, 0, var4);
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0, this->boundingBox.z0 + 1, 1, var4);
            break;
         case 2:
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y0, this->boundingBox.z0 - 1, 2, var4);
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0, this->boundingBox.z0 + 1, 1, var4);
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0, this->boundingBox.z0 + 1, 3, var4);
            break;
         case 3:
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y0, this->boundingBox.z0 - 1, 2, var4);
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y0, this->boundingBox.z1 + 1, 0, var4);
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0, this->boundingBox.z0 + 1, 3, var4);
      }

      if (this->hasTwoFloors) {
         if (random.nextBoolean()) {
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y0 + 3 + 1, this->boundingBox.z0 - 1, 2, var4);
         }

         if (random.nextBoolean()) {
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 + 3 + 1, this->boundingBox.z0 + 1, 1, var4);
         }

         if (random.nextBoolean()) {
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 + 3 + 1, this->boundingBox.z0 + 1, 3, var4);
         }

         if (random.nextBoolean()) {
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 + 1, this->boundingBox.y0 + 3 + 1, this->boundingBox.z1 + 1, 0, var4);
         }
      }
   }

bool MineshaftPieces__Crossing::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         if (this->hasTwoFloors) {
            this->generateBox(
               level,
               chunkBB,
               this->boundingBox.x0 + 1,
               this->boundingBox.y0,
               this->boundingBox.z0,
               this->boundingBox.x1 - 1,
               this->boundingBox.y0 + 3 - 1,
               this->boundingBox.z1,
               0,
               0,
               false
            );
            this->generateBox(
               level,
               chunkBB,
               this->boundingBox.x0,
               this->boundingBox.y0,
               this->boundingBox.z0 + 1,
               this->boundingBox.x1,
               this->boundingBox.y0 + 3 - 1,
               this->boundingBox.z1 - 1,
               0,
               0,
               false
            );
            this->generateBox(
               level,
               chunkBB,
               this->boundingBox.x0 + 1,
               this->boundingBox.y1 - 2,
               this->boundingBox.z0,
               this->boundingBox.x1 - 1,
               this->boundingBox.y1,
               this->boundingBox.z1,
               0,
               0,
               false
            );
            this->generateBox(
               level,
               chunkBB,
               this->boundingBox.x0,
               this->boundingBox.y1 - 2,
               this->boundingBox.z0 + 1,
               this->boundingBox.x1,
               this->boundingBox.y1,
               this->boundingBox.z1 - 1,
               0,
               0,
               false
            );
            this->generateBox(
               level,
               chunkBB,
               this->boundingBox.x0 + 1,
               this->boundingBox.y0 + 3,
               this->boundingBox.z0 + 1,
               this->boundingBox.x1 - 1,
               this->boundingBox.y0 + 3,
               this->boundingBox.z1 - 1,
               0,
               0,
               false
            );
         } else {
            this->generateBox(
               level, chunkBB, this->boundingBox.x0 + 1, this->boundingBox.y0, this->boundingBox.z0, this->boundingBox.x1 - 1, this->boundingBox.y1, this->boundingBox.z1, 0, 0, false
            );
            this->generateBox(
               level, chunkBB, this->boundingBox.x0, this->boundingBox.y0, this->boundingBox.z0 + 1, this->boundingBox.x1, this->boundingBox.y1, this->boundingBox.z1 - 1, 0, 0, false
            );
         }

         this->generateBox(
            level,
            chunkBB,
            this->boundingBox.x0 + 1,
            this->boundingBox.y0,
            this->boundingBox.z0 + 1,
            this->boundingBox.x0 + 1,
            this->boundingBox.y1,
            this->boundingBox.z0 + 1,
            Tile::wood.id,
            0,
            false
         );
         this->generateBox(
            level,
            chunkBB,
            this->boundingBox.x0 + 1,
            this->boundingBox.y0,
            this->boundingBox.z1 - 1,
            this->boundingBox.x0 + 1,
            this->boundingBox.y1,
            this->boundingBox.z1 - 1,
            Tile::wood.id,
            0,
            false
         );
         this->generateBox(
            level,
            chunkBB,
            this->boundingBox.x1 - 1,
            this->boundingBox.y0,
            this->boundingBox.z0 + 1,
            this->boundingBox.x1 - 1,
            this->boundingBox.y1,
            this->boundingBox.z0 + 1,
            Tile::wood.id,
            0,
            false
         );
         this->generateBox(
            level,
            chunkBB,
            this->boundingBox.x1 - 1,
            this->boundingBox.y0,
            this->boundingBox.z1 - 1,
            this->boundingBox.x1 - 1,
            this->boundingBox.y1,
            this->boundingBox.z1 - 1,
            Tile::wood.id,
            0,
            false
         );
         return true;
      }
   }

MineshaftPieces__Room::MineshaftPieces__Room(int_t generationDepth, Random & random, int_t x, int_t z) : StructurePiece(generationDepth)
{
      this->boundingBox = BoundingBox(x, 50, z, x + 7 + random.nextInt(6), 54 + random.nextInt(6), z + 7 + random.nextInt(6));
   }

void MineshaftPieces__Room::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      int_t var4 = this->getGenDepth();
      int_t var6 = this->boundingBox.getYSpan() - 3 - 1;
      if (var6 <= 0) {
         var6 = 1;
      }

      int_t var5 = 0;

      while (var5 < this->boundingBox.getXSpan()) {
         var5 += random.nextInt(this->boundingBox.getXSpan());
         if (var5 + 3 > this->boundingBox.getXSpan()) {
            break;
         }

         std::shared_ptr<StructurePiece> var7 = MineshaftPieces::generateNextPiece(
            start, pieces, random, this->boundingBox.x0 + var5, this->boundingBox.y0 + random.nextInt(var6) + 1, this->boundingBox.z0 - 1, 2, var4
         );
         if (var7 != nullptr) {
            BoundingBox var8 = var7->getBoundingBox();
            this->entrances.push_back(BoundingBox(var8.x0, var8.y0, this->boundingBox.z0, var8.x1, var8.y1, this->boundingBox.z0 + 1));
         }

         var5 += 4;
      }

      var5 = 0;

      while (var5 < this->boundingBox.getXSpan()) {
         var5 += random.nextInt(this->boundingBox.getXSpan());
         if (var5 + 3 > this->boundingBox.getXSpan()) {
            break;
         }

         std::shared_ptr<StructurePiece> var16 = MineshaftPieces::generateNextPiece(
            start, pieces, random, this->boundingBox.x0 + var5, this->boundingBox.y0 + random.nextInt(var6) + 1, this->boundingBox.z1 + 1, 0, var4
         );
         if (var16 != nullptr) {
            BoundingBox var19 = var16->getBoundingBox();
            this->entrances.push_back(BoundingBox(var19.x0, var19.y0, this->boundingBox.z1 - 1, var19.x1, var19.y1, this->boundingBox.z1));
         }

         var5 += 4;
      }

      var5 = 0;

      while (var5 < this->boundingBox.getZSpan()) {
         var5 += random.nextInt(this->boundingBox.getZSpan());
         if (var5 + 3 > this->boundingBox.getZSpan()) {
            break;
         }

         std::shared_ptr<StructurePiece> var17 = MineshaftPieces::generateNextPiece(
            start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0 + random.nextInt(var6) + 1, this->boundingBox.z0 + var5, 1, var4
         );
         if (var17 != nullptr) {
            BoundingBox var20 = var17->getBoundingBox();
            this->entrances.push_back(BoundingBox(this->boundingBox.x0, var20.y0, var20.z0, this->boundingBox.x0 + 1, var20.y1, var20.z1));
         }

         var5 += 4;
      }

      var5 = 0;

      while (var5 < this->boundingBox.getZSpan()) {
         var5 += random.nextInt(this->boundingBox.getZSpan());
         if (var5 + 3 > this->boundingBox.getZSpan()) {
            break;
         }

         std::shared_ptr<StructurePiece> var18 = MineshaftPieces::generateNextPiece(
            start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0 + random.nextInt(var6) + 1, this->boundingBox.z0 + var5, 3, var4
         );
         if (var18 != nullptr) {
            BoundingBox var21 = var18->getBoundingBox();
            this->entrances.push_back(BoundingBox(this->boundingBox.x1 - 1, var21.y0, var21.z0, this->boundingBox.x1, var21.y1, var21.z1));
         }

         var5 += 4;
      }
   }

bool MineshaftPieces__Room::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(
            level, chunkBB, this->boundingBox.x0, this->boundingBox.y0, this->boundingBox.z0, this->boundingBox.x1, this->boundingBox.y0, this->boundingBox.z1, Tile::dirt.id, 0, true
         );
         this->generateBox(
            level,
            chunkBB,
            this->boundingBox.x0,
            this->boundingBox.y0 + 1,
            this->boundingBox.z0,
            this->boundingBox.x1,
            std::min(this->boundingBox.y0 + 3, this->boundingBox.y1),
            this->boundingBox.z1,
            0,
            0,
            false
         );

         for (auto var5 : this->entrances) {
            this->generateBox(level, chunkBB, var5.x0, var5.y1 - 2, var5.z0, var5.x1, var5.y1, var5.z1, 0, 0, false);
         }

         this->generateUpperHalfSphere(
            level, chunkBB, this->boundingBox.x0, this->boundingBox.y0 + 4, this->boundingBox.z0, this->boundingBox.x1, this->boundingBox.y1, this->boundingBox.z1, 0, false
         );
         return true;
      }
   }

MineshaftPieces__Stairs::MineshaftPieces__Stairs(int_t generationDepth, Random & random, const BoundingBox & bb, int_t facing) : StructurePiece(generationDepth)
{
      this->orientation = facing;
      this->boundingBox = bb;
   }

BoundingBox MineshaftPieces__Stairs::findSize(PieceList & pieces, Random & random, int_t x, int_t y, int_t z, int_t facing)
{
      BoundingBox var6 = BoundingBox(x, y - 5, z, x, y + 2, z);
      switch (facing) {
         case 0:
            var6.x1 = x + 2;
            var6.z1 = z + 8;
            break;
         case 1:
            var6.x0 = x - 8;
            var6.z1 = z + 2;
            break;
         case 2:
            var6.x1 = x + 2;
            var6.z0 = z - 8;
            break;
         case 3:
            var6.x1 = x + 8;
            var6.z1 = z + 2;
      }

      return StructurePiece::findCollisionPiece(pieces, var6) != nullptr ? BoundingBox::none() : var6;
   }

void MineshaftPieces__Stairs::addChildren(std::shared_ptr<StructurePiece> start, PieceList & pieces, Random & random)
{
      int_t var4 = this->getGenDepth();
      switch (this->orientation) {
         case 0:
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0, this->boundingBox.y0, this->boundingBox.z1 + 1, 0, var4);
            break;
         case 1:
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0 - 1, this->boundingBox.y0, this->boundingBox.z0, 1, var4);
            break;
         case 2:
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x0, this->boundingBox.y0, this->boundingBox.z0 - 1, 2, var4);
            break;
         case 3:
            MineshaftPieces::generateNextPiece(start, pieces, random, this->boundingBox.x1 + 1, this->boundingBox.y0, this->boundingBox.z0, 3, var4);
      }
   }

bool MineshaftPieces__Stairs::postProcess(Level & level, Random & random, const BoundingBox &chunkBB)
{
      if (this->edgesLiquid(level, chunkBB)) {
         return false;
      } else {
         this->generateBox(level, chunkBB, 0, 5, 0, 2, 7, 1, 0, 0, false);
         this->generateBox(level, chunkBB, 0, 0, 7, 2, 2, 8, 0, 0, false);

         for (int_t var4 = 0; var4 < 5; var4++) {
            this->generateBox(level, chunkBB, 0, 5 - var4 - (var4 < 4 ? 1 : 0), 2 + var4, 2, 7 - var4, 2 + var4, 0, 0, false);
         }

         return true;
      }
   }

class MineShaftStart : public StructureStart
{
public:
	MineShaftStart(Level &level, Random &random, int_t chunkX, int_t chunkZ)
	{
		auto room = std::make_shared<MineshaftPieces__Room>(0, random, (chunkX << 4) + 2, (chunkZ << 4) + 2);
		pieces.push_back(room);
		room->addChildren(room, pieces, random);
		calculateBoundingBox();
		moveBelowSeaLevel(level, random, 10);
	}
};

bool MineShaftFeature::isFeatureChunk(int_t x, int_t z)
{
	return random.nextInt(100) == 0 && random.nextInt(80) < std::max(std::abs(x), std::abs(z));
}

std::unique_ptr<StructureStart> MineShaftFeature::createStructureStart(int_t x, int_t z)
{
	return std::make_unique<MineShaftStart>(*level, random, x, z);
}
