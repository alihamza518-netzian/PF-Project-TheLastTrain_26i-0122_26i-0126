
#include "level.h"
#include "util.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Adds an entity to a particular room
void addRoomEntity(
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR],
    int floor,
    int room,
    char symbol)
{
    if (floor < 0 || floor >= NUM_FLOORS ||
        room < 0 || room >= ROOMS_PER_FLOOR)
    {
        return;
    }

    if (entityCounts[floor][room] < MAX_ENTITIES_PER_ROOM)
    {
        roomEntities[floor][room][entityCounts[floor][room]] = symbol;
        entityCounts[floor][room]++;
    }
}


// Loads the building map
bool loadBuildingMap(
    const char* filename,
    char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    ifstream fin(filename);

    if (!fin)
    {
        cout << "ERROR! Could not open building file!" << endl;
        return false;
    }

    // STEP 1: Initialize entity counts and room entities
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            entityCounts[f][r] = 0;

            for (int e = 0; e < MAX_ENTITIES_PER_ROOM; e++)
            {
                roomEntities[f][r][e] = ENT_NONE;
            }
        }
    }

    // STEP 2: Initialize horizontal connections
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR - 1; r++)
        {
            horizontalConn[f][r] = CONN_NONE;
        }
    }

    // STEP 3: Read building file into a fixed-size array
    const int MAX_LINES = 100;

    string mapLines[MAX_LINES];
    int lineCount = 0;
    string line;

    while (getline(fin, line))
    {
        if (lineCount >= MAX_LINES)
        {
            cout << "ERROR! Building file has too many lines!" << endl;
            fin.close();
            return false;
        }

        mapLines[lineCount] = line;
        lineCount++;
    }

    // STEP 4: Find room-label rows
    bool floorFound[NUM_FLOORS] = {false};

    for (int i = 0; i < lineCount - 1; i++)
    {
        int floor = -1;
        int positions[ROOMS_PER_FLOOR];

        // Identify the floor using its first room label
        for (int f = 0; f < NUM_FLOORS; f++)
        {
            string label = "F" + to_string(f + 1) + "-R1";

            if (mapLines[i].find(label) != string::npos)
            {
                floor = f;
                break;
            }
        }

        // Skip lines without room labels
        if (floor == -1)
        {
            continue;
        }

        // Skip duplicate floor rows
        if (floorFound[floor])
        {
            continue;
        }

        // Find the positions of all four room labels
        bool allFound = true;

        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            string label = "F" + to_string(floor + 1)
                         + "-R" + to_string(r + 1);

            positions[r] = (int)mapLines[i].find(label);

            if (positions[r] == (int)string::npos)
            {
                allFound = false;
                break;
            }
        }

        if (!allFound)
        {
            cout << "ERROR! Incomplete room labels on F"
                 << floor + 1 << endl;

            fin.close();
            return false;
        }

        // STEP 5: Extract horizontal connections
        for (int r = 0; r < ROOMS_PER_FLOOR - 1; r++)
        {
            int start = positions[r] + 5;
            int end = positions[r + 1];

            for (int x = start; x < end; x++)
            {
                char symbol = mapLines[i][x];

                if (symbol == 'D' || symbol == 'W' || symbol == '#')
                {
                    horizontalConn[floor][r] = symbol;
                    break;
                }

                // L1 and L2 are lift labels, not locked doors
                if (symbol == 'L')
                {
                    if (x + 1 < end &&
                        (mapLines[i][x + 1] == '1' ||
                         mapLines[i][x + 1] == '2'))
                    {
                        continue;
                    }

                    horizontalConn[floor][r] = CONN_LOCKED_DOOR;
                    break;
                }
            }
        }

        // STEP 6: Read the entity row below the labels
        string entityLine = mapLines[i + 1];

        // Calculate approximate room-center positions
        int roomCenter[ROOMS_PER_FLOOR];

        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            roomCenter[r] = positions[r] + 5;
        }

        // STEP 7: Identify and store entities
        for (int x = 0; x < (int)entityLine.length(); x++)
        {
            char symbol = entityLine[x];
            bool isEntity = false;

            switch (symbol)
            {
                case '<':
                case ')':
                case '\\':
                case '4':
                case '*':
                case '~':
                case ';':
                case '(':
                case '!':
                case '%':
                case '$':
                case '2':
                case '7':
                case '^':
                case '@':
                case '&':
                    isEntity = true;
                    break;
            }

            if (!isEntity)
            {
                continue;
            }

            // Find the nearest room center
            int nearestRoom = 0;
            int smallestDistance = 100000;

            for (int r = 0; r < ROOMS_PER_FLOOR; r++)
            {
                int distance = x - roomCenter[r];

                if (distance < 0)
                {
                    distance = -distance;
                }

                if (distance < smallestDistance)
                {
                    smallestDistance = distance;
                    nearestRoom = r;
                }
            }

            // Check entity capacity before adding
            if (entityCounts[floor][nearestRoom] >= MAX_ENTITIES_PER_ROOM)
            {
                cout << "ERROR! Too many entities in F"
                     << floor + 1 << "-R" << nearestRoom + 1 << endl;

                fin.close();
                return false;
            }

            addRoomEntity(
                roomEntities,
                entityCounts,
                floor,
                nearestRoom,
                symbol);
        }

        floorFound[floor] = true;
    }

    // STEP 8: Check whether all five floors were found
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        if (!floorFound[f])
        {
            cout << "ERROR! Missing floor: F" << f + 1 << endl;
            fin.close();
            return false;
        }
    }

    fin.close();
    return true;
}


// Loads the initial game state
bool loadInitialState(
    const char* filename,
    int& playerFloor, int& playerRoom,
    int& energy, int& time, int& score,
    char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
    bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft)
{
    ifstream fin(filename);

    if (!fin)
    {
        cout << "ERROR! Could not open initial-state file!" << endl;
        return false;
    }

    // Set default values
    playerFloor = 1;
    playerRoom = 1;
    energy = INITIAL_ENERGY;
    time = INITIAL_TIME;
    score = INITIAL_SCORE;
    inventoryCount = 0;
    emergencyTripsLeft = 0;

    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
    {
        inventory[i] = ENT_NONE;
    }

    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        objectives[i] = false;
    }

    string line;

    while (getline(fin, line))
    {
        size_t pos = line.find(':');

        if (pos == string::npos)
        {
            continue;
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        // Remove leading and trailing whitespace from key
        size_t keyFirst = key.find_first_not_of(" \t\r");
        size_t keyLast = key.find_last_not_of(" \t\r");

        if (keyFirst == string::npos)
        {
            continue;
        }

        key = key.substr(keyFirst, keyLast - keyFirst + 1);

        // Remove leading and trailing whitespace from value
        size_t first = value.find_first_not_of(" \t\r");
        size_t last = value.find_last_not_of(" \t\r");

        if (first == string::npos)
        {
            value = "";
        }
        else
        {
            value = value.substr(first, last - first + 1);
        }

        // Load numeric values safely
        if (key == "INITIAL_ENERGY" ||
            key == "INITIAL_TIME" ||
            key == "INITIAL_SCORE")
        {
            try
            {
                size_t usedCharacters = 0;
                int number = stoi(value, &usedCharacters);

                if (usedCharacters != value.length())
                {
                    fin.close();
                    return false;
                }

                if (key == "INITIAL_ENERGY")
                    energy = number;

                else if (key == "INITIAL_TIME")
                    time = number;

                else if (key == "INITIAL_SCORE")
                    score = number;
            }
            catch (...)
            {
                fin.close();
                return false;
            }
        }

        // Load objective flags
        else if (key == "PASSENGER_RESCUED")
        {
            objectives[OBJ_PASSENGER_RESCUED] = (value == "YES");
        }

        else if (key == "CAT_RESCUED")
        {
            objectives[OBJ_CAT_SECURED] = (value == "YES");
        }

        else if (key == "CABINET_UNLOCKED")
        {
            objectives[OBJ_CABINET_UNLOCKED] = (value == "YES");
        }

        else if (key == "TRAIN_KEY_COLLECTED")
        {
            objectives[OBJ_TRAIN_KEY] = (value == "YES");
        }

        else if (key == "FUSE_INSTALLED")
        {
            objectives[OBJ_FUSE_INSTALLED] = (value == "YES");
        }

        else if (key == "LIFT_PASS_COLLECTED")
        {
            objectives[OBJ_LIFT_PASS] = (value == "YES");
        }

        else if (key == "TRAIN_PREPARED")
        {
            objectives[OBJ_TRAIN_PREPARED] = (value == "YES");
        }

        else if (key == "CLUES_COLLECTED")
        {
            objectives[OBJ_CLUE_4] = (value.find('4') != string::npos);
            objectives[OBJ_CLUE_2] = (value.find('2') != string::npos);
            objectives[OBJ_CLUE_7] = (value.find('7') != string::npos);
        }

        // Load starting room if it is explicitly provided
        else if (key == "START_ROOM" && value != "PLAYER_SELECTS")
        {
            if (value.length() != 5 ||
                value[0] != 'F' ||
                value[2] != '-' ||
                value[3] != 'R' ||
                value[1] < '1' ||
                value[1] > '5' ||
                value[4] < '1' ||
                value[4] > '4')
            {
                fin.close();
                return false;
            }

            playerFloor = value[1] - '0';
            playerRoom = value[4] - '0';
        }

        // PLAYER_AT_TRAIN is not stored because the supplied
        // state.h does not define a corresponding objective index.
    }

    fin.close();

    // Validate loaded values
    if (energy < 0 || energy > MAX_ENERGY ||
        time < 0 || time > MAX_TIME ||
        score < 0 ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY ||
        playerFloor < 1 || playerFloor > NUM_FLOORS ||
        playerRoom < 1 || playerRoom > ROOMS_PER_FLOOR)
    {
        return false;
    }

    return true;
}


// Validates the starting room
bool validateStartingRoom(
    int startFloor,
    int startRoom,
    const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])
{
    // Check floor boundaries
    if (startFloor < 1 || startFloor > NUM_FLOORS)
    {
        return false;
    }

    // Check room boundaries
    if (startRoom < 1 || startRoom > ROOMS_PER_FLOOR)
    {
        return false;
    }

    // Check that the connection array is available
    if (horizontalConn == nullptr)
    {
        return false;
    }

    return true;
}


// Returns the name of an entity
const char* getEntityName(char entitySymbol)
{
    switch (entitySymbol)
    {
        case ENT_MASTER_KEY:
            return "Master Key";

        case ENT_ENERGY_PACK:
            return "Energy Pack";

        case ENT_EMERGENCY_TOOLKIT:
            return "Emergency Toolkit";

        case ENT_CLUE_4:
            return "Clue 4";

        case ENT_FUSE:
            return "Fuse";

        case ENT_CAT:
            return "Cat";

        case ENT_CONTROL_PANEL:
            return "Control Panel";

        case ENT_LIFT_PASS:
            return "Lift Pass";

        case ENT_PASSENGER:
            return "Passenger";

        case ENT_MEDICAL_KIT:
            return "Medical Kit";

        case ENT_EMERGENCY_CARD:
            return "Emergency Card";

        case ENT_CLUE_2:
            return "Clue 2";

        case ENT_CLUE_7:
            return "Clue 7";

        case ENT_CABINET_TRAIN_KEY:
            return "Train Key";

        case ENT_CAT_CARRIER:
            return "Cat Carrier";

        case ENT_OBSERVATION_TOKEN:
            return "Observation Token";

        default:
            return "Unknown Entity";
    }
}
