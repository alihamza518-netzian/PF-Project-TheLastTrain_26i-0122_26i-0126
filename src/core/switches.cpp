#include "switches.h"
#include "util.h"

// Door lock mechanics, lifts, and cabinet verification without structures
bool unlockFloor3Door(
    const char inventory[MAX_INVENTORY_CAPACITY],
    int inventoryCount)
{
    if (inventory == nullptr ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }

    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == ENT_MASTER_KEY)
        {
            return true;
        }
    }

    return false;
}

bool canUseLift1(int fromFloor, int toFloor)
{
    return (fromFloor == 1 && toFloor == 2) ||
           (fromFloor == 2 && toFloor == 1);
}
bool canUseLift2(int fromFloor, int toFloor,
                 bool lift2Operational, int emergencyTripsLeft)
{
    const bool validFloors =
        (fromFloor == 2 || fromFloor == 4 || fromFloor == 5) &&
        (toFloor == 2 || toFloor == 4 || toFloor == 5);

    if (!validFloors || fromFloor == toFloor)
    {
        return false;
    }

    return lift2Operational || emergencyTripsLeft > 0;
}
bool installFuseControlPanel(
    char inventory[MAX_INVENTORY_CAPACITY],
    int& inventoryCount,
    bool objectives[NUM_OBJECTIVES],
    int& score)
{
    if (inventory == nullptr || objectives == nullptr ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }

    if (objectives[OBJ_FUSE_INSTALLED])
    {
        return false;
    }

    int fuseIndex = -1;

    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == ENT_FUSE)
        {
            fuseIndex = i;
            break;
        }
    }

    if (fuseIndex == -1)
    {
        return false;
    }

    for (int i = fuseIndex; i < inventoryCount - 1; i++)
    {
        inventory[i] = inventory[i + 1];
    }

    inventoryCount--;
    inventory[inventoryCount] = ENT_NONE;

    objectives[OBJ_FUSE_COLLECTED] = false;
    objectives[OBJ_FUSE_INSTALLED] = true;
    score += SCORE_INSTALL_FUSE;

    return true;
}
bool verifyCabinetCode(const char* enteredCode,
                       const bool objectives[NUM_OBJECTIVES])
{
    if (enteredCode == nullptr || objectives == nullptr)
    {
        return false;
    }

    return objectives[OBJ_CLUE_4] &&
           objectives[OBJ_CLUE_2] &&
           objectives[OBJ_CLUE_7] &&
           stringEquals(enteredCode, "427");
}