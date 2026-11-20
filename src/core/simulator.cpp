#include "simulator.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"
#include<level.h>
#include<iostream>
using namespace std;
// Modular simulation loop using primitive variables
bool checkEndConditions(int energy, int time,const bool objectives[NUM_OBJECTIVES])
{
    if (energy <= 0 || time <= 0)
    {
        return true;
    }

    if (objectives[OBJ_MISSION_WON] ||  objectives[OBJ_MISSION_FAILED])
    {
        return true;
    }
    return false;
}

bool handleMoveCommand(int targetFloor, int targetRoom,
    int& currentFloor, int& currentRoom,
    int& energy, int& time,
    const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
    const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
    int& emergencyTripsLeft, bool lift2Operational)
{
    //check destination inside the building
    if (targetFloor < 1 || targetFloor > NUM_FLOORS || targetRoom < 1 || targetRoom > ROOMS_PER_FLOOR)
    {
        cout << "Invalid floor or room number."<<endl;
        return false;
    }

    // Stairs: F1-R1 to F2-R1, F2-R1 to F3-R1,F4-R1 to F5-R1
    bool usingStairs =
          (currentRoom == 1 && targetRoom == 1 &&
         ((currentFloor == 1 && targetFloor == 2) ||
          (currentFloor == 2 && targetFloor == 1) ||
          (currentFloor == 2 && targetFloor == 3) ||
          (currentFloor == 3 && targetFloor == 2) ||
          (currentFloor == 4 && targetFloor == 5) ||
          (currentFloor == 5 && targetFloor == 4)));
    if (usingStairs)
    {
        applyResourceDelta(energy, time,COST_MOVE_ORDINARY_ENERGY,COST_MOVE_ORDINARY_TIME);
        currentFloor = targetFloor;
        currentRoom = targetRoom;
        cout << "You used the stairs to Floor " << currentFloor<< ", Room " << currentRoom << ".\n";
        return true;
    }

    // Lift 1: crossed routes between Floors 1 and 2
    bool usingLift1 =
        (currentFloor == 1 && currentRoom == 2 &&
         targetFloor == 2 && targetRoom == 3) ||
        (currentFloor == 2 && currentRoom == 3 &&
         targetFloor == 1 && targetRoom == 2) ||
        (currentFloor == 2 && currentRoom == 2 &&
         targetFloor == 1 && targetRoom == 3) ||
        (currentFloor == 1 && currentRoom == 3 &&
         targetFloor == 2 && targetRoom == 2);
    if (usingLift1)
    {
        if (!canUseLift1(currentFloor, targetFloor))
        {
            cout << "Lift 1 cannot travel between those floors."<<endl;
            return false;
        }
    applyResourceDelta(energy, time,COST_MOVE_LIFT_ENERGY,COST_MOVE_LIFT_TIME);
        currentFloor = targetFloor;
        currentRoom = targetRoom;
        cout << "You used Lift 1 to Floor " << currentFloor<< ", Room " << currentRoom <<endl;
        return true;
    }
    // Lift 2: Room 4 on Floors 2, 4, and 5
    bool lift2FloorPair =
        (currentFloor == 2 || currentFloor == 4 || currentFloor == 5) &&
        (targetFloor == 2 || targetFloor == 4 || targetFloor == 5) &&
        currentFloor != targetFloor;

    if (currentRoom == 4 && targetRoom == 4 && lift2FloorPair)
    {
        if (!canUseLift2(currentFloor, targetFloor, lift2Operational, emergencyTripsLeft))
        {
            cout << "Lift 2 cannot be used right now."<<endl;
            return false;
        }
        // An emergency trip is used when Lift 2 is not operational
        if (!lift2Operational)
        {
            emergencyTripsLeft--;
        }
        applyResourceDelta(energy, time,  COST_MOVE_LIFT_ENERGY,COST_MOVE_LIFT_TIME);
        currentFloor = targetFloor;
        currentRoom = targetRoom;

        cout << "You used Lift 2 to Floor " << currentFloor
             << ", Room " << currentRoom << endl;
        return true;
    }
    // Check horizontal connections between adjacent rooms
    char connectionType = CONN_NONE;
    if (!isDirectlyConnected(currentFloor, currentRoom,targetFloor, targetRoom,horizontalConn, connectionType))
    {
        cout << "Those rooms are not directly connected.\n";
        return false;
    }
    // Check whether the connection can be crossed
    if (!canTraverse(targetFloor, targetRoom, connectionType,inventory, inventoryCount,emergencyTripsLeft, lift2Operational))
    {
        cout << "You cannot pass through that connection.\n";
        return false;
    }
    // Apply the movement cost
    if (connectionType == CONN_WINDOW)
    {
        applyResourceDelta(energy, time, COST_MOVE_WINDOW_ENERGY,COST_MOVE_WINDOW_TIME);
    }
    else
    {
        applyResourceDelta(energy, time,COST_MOVE_ORDINARY_ENERGY,COST_MOVE_ORDINARY_TIME);
    }
    currentFloor = targetFloor;
    currentRoom = targetRoom;
    cout << "You moved to Floor " << currentFloor
         << ", Room " << currentRoom << endl;
    return true;
}

bool handleInspectCommand(
    int currentFloor, int currentRoom,
    int& energy, int& time,
    const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCount,
    bool objectives[NUM_OBJECTIVES], int& score)
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS || currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR)
    {
        cout << "Invalid current floor or room.\n";
        return false;
    }
    if (entityCount < 0 || entityCount > MAX_ENTITIES_PER_ROOM)
    {
        cout << "Invalid entity count for this room.\n";
        return false;
    }
    applyResourceDelta(energy, time,
                       COST_INSPECT_ENERGY,
                       COST_INSPECT_TIME);

    renderRoomDetails(currentFloor, currentRoom,roomEntities, entityCount);
    // Inspection has no score or objective change in the current rules.
    (void)objectives;
    (void)score;
    return true;
}

bool handleCollectCommand(
    char itemSymbol, int currentFloor, int currentRoom,
    int& energy, int& time, int& score,
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int& entityCount,
    char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
    bool objectives[NUM_OBJECTIVES])
{
        if (itemSymbol == ENT_CABINET_TRAIN_KEY &&
        !objectives[OBJ_CABINET_UNLOCKED])
    {
        cout << "Unlock the cabinet before collecting the Train Key.\n";
        return false;
    }
    if (currentFloor < 1 || currentFloor > NUM_FLOORS || currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR)
    {
        cout << "Invalid current floor or room.\n";
        return false;
    }
    if (entityCount < 0 || entityCount > MAX_ENTITIES_PER_ROOM || inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        cout << "Invalid room or inventory data.\n";
        return false;
    }
    // These items can be picked up and placed in the inventory.
    bool canCollect = false;
    switch (itemSymbol)
    {
        case ENT_MASTER_KEY:
        case ENT_ENERGY_PACK:
        case ENT_EMERGENCY_TOOLKIT:
        case ENT_CLUE_4:
        case ENT_FUSE:
        case ENT_LIFT_PASS:
        case ENT_MEDICAL_KIT:
        case ENT_EMERGENCY_CARD:
        case ENT_CLUE_2:
        case ENT_CLUE_7:
        case ENT_CABINET_TRAIN_KEY:
        case ENT_CAT_CARRIER:
        case ENT_OBSERVATION_TOKEN:
            canCollect = true;
            break;
        default:
            cout << "That item cannot be collected.\n";
            return false;
    }
    // Find requested item in room.
    int itemIndex = -1;
    for (int i = 0; i < entityCount; i++)
    {
        if (roomEntities[currentFloor - 1][currentRoom - 1][i] == itemSymbol)
        {
            itemIndex = i;
            break;
        }
    }
    if (itemIndex == -1)
    {
        cout << "That item is not in this room.\n";
        return false;
    }
    if (!canCollect)
    {
        return false;
    }
    if (inventoryCount >= MAX_INVENTORY_CAPACITY)
    {
        cout << "Your inventory is full.\n";
        return false;
    }
    // Add item to inventory.
    inventory[inventoryCount] = itemSymbol;
    inventoryCount++;
    // Remove  item from  room by  entries left.
    for (int i = itemIndex; i < entityCount - 1; i++)
    {
        roomEntities[currentFloor - 1][currentRoom - 1][i] = roomEntities[currentFloor - 1][currentRoom - 1][i + 1];
    }
    entityCount--;
    roomEntities[currentFloor - 1][currentRoom - 1][entityCount] = ENT_NONE;
    // Apply pickup cost.
    applyResourceDelta(energy, time,
                       COST_ITEM_PICKUP_ENERGY,
                       COST_ITEM_PICKUP_TIME);

    // Update score for relevant items.
    switch (itemSymbol)
    {
        case ENT_MASTER_KEY:
            objectives[OBJ_MASTER_KEY] = true;
            score += SCORE_KEY_COLLECT;
            break;
        case ENT_CLUE_4:
            objectives[OBJ_CLUE_4] = true;
            score += SCORE_CLUE_FOUND;
            break;
        case ENT_CLUE_2:
            objectives[OBJ_CLUE_2] = true;
            score += SCORE_CLUE_FOUND;
            break;
        case ENT_CLUE_7:
            objectives[OBJ_CLUE_7] = true;
            score += SCORE_CLUE_FOUND;
            break;
        case ENT_FUSE:
            objectives[OBJ_FUSE_COLLECTED] = true;
            break;
        case ENT_LIFT_PASS:
            objectives[OBJ_LIFT_PASS] = true;
            score += SCORE_LIFT_PASS;
            break;
        case ENT_CABINET_TRAIN_KEY:
            objectives[OBJ_TRAIN_KEY] = true;
            score += SCORE_TRAIN_KEY;
            break;
    }
    cout << "Item collected: " << getEntityName(itemSymbol) <<endl;
    return true;
}

bool handleUseCommand(
    char itemSymbol, int currentFloor, int currentRoom,
    int& energy, int& time, int& score,
    char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
    bool objectives[NUM_OBJECTIVES])
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR)
    {
        cout << "Invalid current floor or room.\n";
        return false;
    }
    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        cout << "Invalid inventory data.\n";
        return false;
    }
    if (!hasItem(inventory, inventoryCount, itemSymbol))
    {
        cout << "You do not have that item.\n";
        return false;
    }
    if (itemSymbol == ENT_ENERGY_PACK ||
        itemSymbol == ENT_EMERGENCY_TOOLKIT ||
        itemSymbol == ENT_OBSERVATION_TOKEN)
    {
        int energyBonus = 0;
        if (itemSymbol == ENT_ENERGY_PACK)
            energyBonus = BONUS_ENERGY_PACK;
        else if (itemSymbol == ENT_EMERGENCY_TOOLKIT)
            energyBonus = BONUS_EMERGENCY_TOOLKIT;
        else
            energyBonus = BONUS_OBSERVATION_TOKEN;
        if (!removeItem(inventory, inventoryCount, itemSymbol))
        {
            cout << "Could not remove the item from inventory.\n";
            return false;
        }
        applyResourceDelta(
            energy, time,
            COST_ITEM_USE_ENERGY + energyBonus,
            COST_ITEM_USE_TIME);
        cout << "You used the item and restored energy.\n";
        return true;
    }
    // Use fuse at the control panel in F3-R4.
    if (itemSymbol == ENT_FUSE)
    {
        if (currentFloor != 3 || currentRoom != 4)
        {
            cout << "Use the fuse at the control panel in F3-R4.\n";
            return false;
        }
        if (!installFuseControlPanel(
                inventory, inventoryCount, objectives, score))
        {
            cout << "The fuse could not be installed.\n";
            return false;
        }
        applyResourceDelta(energy, time,
                           COST_ITEM_USE_ENERGY,
                           COST_ITEM_USE_TIME);
        cout << "The fuse was installed.\n";
        return true;
    }
    // Use the medical kit to rescue the passenger in F4-R1.
    if (itemSymbol == ENT_MEDICAL_KIT)
    {
        if (currentFloor != 4 || currentRoom != 1)
        {
            cout << "Use the medical kit in the passenger's room, F4-R1.\n";
            return false;
        }
        if (objectives[OBJ_PASSENGER_RESCUED])
        {
            cout << "The passenger has already been rescued.\n";
            return false;
        }
        if (!removeItem(inventory, inventoryCount, itemSymbol))
        {
            cout << "Could not remove the medical kit from inventory.\n";
            return false;
        }
        objectives[OBJ_PASSENGER_RESCUED] = true;
        score += SCORE_RESCUE_PASSENGER;
        applyResourceDelta(energy, time,
                           COST_ITEM_USE_ENERGY,
                           COST_ITEM_USE_TIME);
        cout << "The passenger has been rescued.\n";
        return true;
    }
    // Use the cat carrier in the cat's room, F3-R3.
    if (itemSymbol == ENT_CAT_CARRIER)
    {
        if (currentFloor != 3 || currentRoom != 3)
        {
            cout << "Use the cat carrier in the cat's room, F3-R3.\n";
            return false;
        }
        if (objectives[OBJ_CAT_SECURED])
        {
            cout << "The cat has already been secured.\n";
            return false;
        }
        if (!removeItem(inventory, inventoryCount, itemSymbol))
        {
            cout << "Could not remove the carrier from inventory.\n";
            return false;
        }
        objectives[OBJ_CAT_SECURED] = true;
        score += SCORE_SECURE_CAT;
        applyResourceDelta(energy, time,
                           COST_ITEM_USE_ENERGY,
                           COST_ITEM_USE_TIME);
        cout << "The cat has been secured.\n";
        return true;
    }
    cout << "That item has no direct use here.\n";
    return false;
}
bool handleUnlockCabinetCommand(
    const char* code, int currentFloor, int currentRoom,
    bool objectives[NUM_OBJECTIVES], int& score,
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int& entityCount)
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS ||currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR)
    {
        cout << "Invalid current floor or room.\n";
        return false;
    }
    // The cabinet is in F5-R1.
    if (currentFloor != 5 || currentRoom != 1)
    {
        cout << "The cabinet is in F5-R1.\n";
        return false;
    }
    if (objectives[OBJ_CABINET_UNLOCKED])
    {
        cout << "The cabinet is already unlocked.\n";
        return false;
    }
    if (entityCount < 0 || entityCount > MAX_ENTITIES_PER_ROOM)
    {
        cout << "Invalid room data.\n";
        return false;
    }
    // to  sure the cabinet/key marker is present in this room.
    bool cabinetFound = false;
    for (int i = 0; i < entityCount; i++)
    {
        if (roomEntities[currentFloor - 1][currentRoom - 1][i] == ENT_CABINET_TRAIN_KEY)
        {
            cabinetFound = true;
            break;
        }
    }
    if (!cabinetFound)
    {
        cout << "The cabinet is not here.\n";
        return false;
    }
    if (code == nullptr || !verifyCabinetCode(code, objectives))
    {
        cout << "The code is incorrect or you have not found all the clues.\n";
        return false;
    }
    objectives[OBJ_CABINET_UNLOCKED] = true;
    score += SCORE_CABINET_UNLOCK;
    cout << "The cabinet is unlocked. You can now collect the Train Key.\n";
    return true;
}

void runSimulation(
    int& playerFloor, int& playerRoom,
    int& energy, int& time, int& score,
    char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
    bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
    char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    bool quit = false;
    while (!quit && !checkEndConditions(energy, time, objectives))
    {
        renderDashboard(playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, emergencyTripsLeft);
        renderAsciiBuilding(playerFloor, playerRoom);
        cout<<endl;
        cout << "Choose an action:\n"
             << "1. Move\n"
             << "2. Inspect room\n"
             << "3. Collect item\n"
             << "4. Use item\n"
             << "5. Unlock cabinet\n"
             << "6. Show objectives\n"
             << "0. Quit\n"
             << "Choice: ";
        int choice;
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Please enter a number from the menu.\n";
            continue;
        }
        switch (choice)
        {
            case 1:
            {
                int targetFloor;
                int targetRoom;
                cout << "Enter target floor and room: ";
                if (!(cin >> targetFloor >> targetRoom))
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "Please enter two numbers.\n";
                    break;
                }
                handleMoveCommand(
                    targetFloor, targetRoom,
                    playerFloor, playerRoom,
                    energy, time,
                    horizontalConn,
                    inventory, inventoryCount,
                    emergencyTripsLeft,
                    objectives[OBJ_LIFT2_ACTIVE]);
                break;
            }
            case 2:
                handleInspectCommand(
                    playerFloor, playerRoom,
                    energy, time,
                    roomEntities,
                    entityCounts[playerFloor - 1][playerRoom - 1],
                    objectives, score);
                break;
            case 3:
            {
                char itemSymbol;
                cout << "Enter the item's symbol: ";
                if (!(cin >> itemSymbol))
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "Could not read the item symbol.\n";
                    break;
                }
                handleCollectCommand(
                    itemSymbol, playerFloor, playerRoom,
                    energy, time, score,
                    roomEntities,
                    entityCounts[playerFloor - 1][playerRoom - 1],
                    inventory, inventoryCount,
                    objectives);
                break;
            }
            case 4:
            {
                char itemSymbol;
                cout << "Enter the inventory item's symbol to use: ";
                if (!(cin >> itemSymbol))
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "Could not read the item symbol.\n";
                    break;
                }
                handleUseCommand(
                    itemSymbol, playerFloor, playerRoom,
                    energy, time, score,
                    inventory, inventoryCount,
                    objectives);
                break;
            }
            case 5:
            {
                char code[MAX_LINE_LEN];
                cout << "Enter the cabinet code: ";
                cin.width(MAX_LINE_LEN);
                if (!(cin >> code))
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "Could not read the code.\n";
                    break;
                }
                handleUnlockCabinetCommand(
                    code, playerFloor, playerRoom,
                    objectives, score,
                    roomEntities,
                    entityCounts[playerFloor - 1][playerRoom - 1]);
                break;
            }
            case 6:
                renderObjectivesChecklist(objectives);
                break;
            case 0:
                quit = true;
                break;
            default:
                cout << "That is not a menu option.\n";
                break;
        }
        if (energy <= 0 || time <= 0)
        {
            objectives[OBJ_MISSION_FAILED] = true;
        }
    }
    bool victory = objectives[OBJ_MISSION_WON];
    if (victory)
        cout << "\nMission complete!\n";
    else if (objectives[OBJ_MISSION_FAILED])
        cout << "\nMission failed.\n";
    else
        cout << "\nGame ended.\n";
    writeGameSummaryReport(
        "game_summary.txt",
        score, energy, time,
        objectives, victory);
}