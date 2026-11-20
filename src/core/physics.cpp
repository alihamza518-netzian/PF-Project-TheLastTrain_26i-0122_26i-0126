
#include "physics.h"
#include<switches.h>
#include "util.h"
#include<iostream>
#include <cstdlib>
using namespace std;

// Movement validation and resource adjustments without structures
bool isDirectlyConnected(int fromF, int fromR, int toF, int toR,const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],char& outConnType)
    {
    outConnType = CONN_NONE;
    if (fromF < 1 || fromF > NUM_FLOORS || toF < 1 || toF > NUM_FLOORS || fromR < 1 || fromR > ROOMS_PER_FLOOR || toR < 1 || toR > ROOMS_PER_FLOOR)
    {
        return false;
    }

    if (horizontalConn == nullptr)
    {
        return false;
    }

    //horizontal connections handled here
    if (fromF != toF || std::abs(fromR - toR) != 1)
    {
        return false;
    }

    //connection between rooms
    outConnType = horizontalConn[fromF - 1][(fromR < toR ? fromR : toR) - 1];
    // Reject walls 
    if (outConnType == CONN_NONE ||outConnType == CONN_SOLID_WALL)
    {
        outConnType = CONN_NONE;
        return false;
    }

    return true;
}
void applyResourceDelta(
    int& energy, int& time,
    int deltaEnergy, int deltaTime)
{
    energy += deltaEnergy;
    time += deltaTime;

    if (energy < 0)
        energy = 0;
    else if (energy > MAX_ENERGY)
        energy = MAX_ENERGY;

    if (time < 0)
        time = 0;
    else if (time > MAX_TIME)
        time = MAX_TIME;
}
bool canTraverse(
    int targetFloor, int targetRoom, char connType,
    const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
    int emergencyTripsLeft, bool lift2Operational)
{
    // Check that the destination is inside the building.
    if (targetFloor < 1 || targetFloor > NUM_FLOORS ||
        targetRoom < 1 || targetRoom > ROOMS_PER_FLOOR)
    {
        return false;
    }

    if (inventory == nullptr ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }

    // Stairs and lifts are checked separately in handleMoveCommand().
    (void)emergencyTripsLeft;
    (void)lift2Operational;

    switch (connType)
    {
        case CONN_DOOR:
        case CONN_WINDOW:
            return true;

        case CONN_LOCKED_DOOR:
            return unlockFloor3Door(inventory, inventoryCount);

        case CONN_NONE:
        case CONN_SOLID_WALL:
        default:
            return false;
    }
}
void applyScoreDelta(int& score, int deltaScore)
{
    score += deltaScore;

    if (score < 0)
    {
        score = 0;
    }
}

bool hasItem(
    const char inventory[MAX_INVENTORY_CAPACITY],
    int inventoryCount,
    char itemSymbol)
{
    if (inventory == nullptr ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }

    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
        {
            return true;
        }
    }

    return false;
}
bool addItem(
    char inventory[MAX_INVENTORY_CAPACITY],
    int& inventoryCount,
    char itemSymbol)
{
    if (inventory == nullptr ||
        inventoryCount < 0 ||
        inventoryCount >= MAX_INVENTORY_CAPACITY ||
        itemSymbol == ENT_NONE)
    {
        return false;
    }

    inventory[inventoryCount] = itemSymbol;
    inventoryCount++;

    return true;
}
bool removeItem(
    char inventory[MAX_INVENTORY_CAPACITY],
    int& inventoryCount,
    char itemSymbol)
{
    if (inventory == nullptr ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY ||
        itemSymbol == ENT_NONE)
    {
        return false;
    }

    int itemIndex = -1;

    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
        {
            itemIndex = i;
            break;
        }
    }

    if (itemIndex == -1)
    {
        return false;
    }

    for (int i = itemIndex; i < inventoryCount - 1; i++)
    {
        inventory[i] = inventory[i + 1];
    }

    inventoryCount--;
    inventory[inventoryCount] = ENT_NONE;

    return true;
}