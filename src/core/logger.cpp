#include "logger.h"
#include "util.h"

// File persistence for savegame.txt and game_summary.txt using primitive variables

#include <fstream>

using namespace std;
void writeGameSummaryReport(
    const char* filename,
    int score,
    int remainingEnergy,
    int remainingTime,
    const bool objectives[NUM_OBJECTIVES],
    bool victory)
{
    if (filename==nullptr || objectives==nullptr)
    {
        return;
    }
    ofstream fout(filename);
    if (!fout)
    {
        return;
    }

    fout<<"THE LAST TRAIN - GAME SUMMARY\n";
    fout<<"==============================\n";
    fout<<"Result: "<<(victory ? "Victory" : "Game ended")<<"\n";
    fout<<"Score: "<<score<<"\n";
    fout<<"Energy remaining: "<<remainingEnergy<<"\n";
    fout<<"Time remaining: "<<remainingTime<<"\n\n";
    fout<<"Objectives:\n";
    for (int i=0; i<NUM_OBJECTIVES; i++)
    {
        fout<<"Objective "<<i+1<<": "
            <<(objectives[i] ? "Complete" : "Incomplete")
            <<"\n";
    }
    fout.close();
}


bool saveGameStateToFile(
    const char* filename,
    int playerFloor,int playerRoom,
    int energy,int time,int score,
    const char inventory[MAX_INVENTORY_CAPACITY],
    int inventoryCount,
    const bool objectives[NUM_OBJECTIVES],
    int emergencyTripsLeft,
    const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    if (filename==nullptr || inventory==nullptr || objectives==nullptr || roomEntities==nullptr || entityCounts==nullptr || inventoryCount<0 || inventoryCount>MAX_INVENTORY_CAPACITY)
    {
        return false;
    }
    ofstream saveFile(filename);
    if (!saveFile)
    {
        return false;
    }
    saveFile<<playerFloor<<'\n';
    saveFile<<playerRoom<<'\n';
    saveFile<<energy<<'\n';
    saveFile<<time<<'\n';
    saveFile<<score<<'\n';
    saveFile<<inventoryCount<<'\n';
    saveFile<<emergencyTripsLeft<<'\n';
    for (int i=0; i<MAX_INVENTORY_CAPACITY; i++)
    {
        saveFile<<static_cast<int>(inventory[i])<<' ';
    }
    saveFile<<'\n';
    for (int i=0; i<NUM_OBJECTIVES; i++)
    {
        saveFile<<objectives[i]<<' ';
    }
    saveFile<<'\n';
    for (int floor=0; floor<NUM_FLOORS; floor++)
    {
        for (int room=0; room<ROOMS_PER_FLOOR; room++)
        {
            if (entityCounts[floor][room]<0 || entityCounts[floor][room]>MAX_ENTITIES_PER_ROOM)
            {
                return false;
            }
            saveFile<<entityCounts[floor][room]<<' ';
            for (int item=0; item<MAX_ENTITIES_PER_ROOM; item++)
            {
                saveFile<<static_cast<int>(roomEntities[floor][room][item])
                        <<' ';
            }
            saveFile<<'\n';
        }
    }
    return static_cast<bool>(saveFile);
}


bool loadGameStateFromFile(
    const char* filename,
    int& playerFloor,int& playerRoom,
    int& energy,int& time,int& score,
    char inventory[MAX_INVENTORY_CAPACITY],
    int& inventoryCount,
    bool objectives[NUM_OBJECTIVES],
    int& emergencyTripsLeft,
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    if (filename==nullptr || inventory==nullptr || objectives==nullptr || roomEntities==nullptr || entityCounts==nullptr)
    {
        return false;
    }
    ifstream loadFile(filename);
    if (!loadFile)
    {
        return false;
    }
    if (!(loadFile>>playerFloor>>playerRoom>>energy>>time>>score>>inventoryCount>>emergencyTripsLeft))
    {
        return false;
    }
    if (playerFloor<1 || playerFloor>NUM_FLOORS ||
        playerRoom<1 || playerRoom>ROOMS_PER_FLOOR ||
        energy<0 || energy>MAX_ENERGY || time<0 || time>MAX_TIME ||
        score<0 || inventoryCount<0 || inventoryCount>MAX_INVENTORY_CAPACITY || emergencyTripsLeft<0)
    {
        return false;
    }
    for (int i=0; i<MAX_INVENTORY_CAPACITY; i++)
    {
        int savedItem;
        if (!(loadFile>>savedItem) || savedItem<0 || savedItem>255)
        {
            return false;
        }
        inventory[i]=static_cast<char>(savedItem);
    }
    for (int i=0; i<NUM_OBJECTIVES; i++)
    {
        int savedObjective;
        if (!(loadFile>>savedObjective) || (savedObjective!=0 && savedObjective!=1))
        {
            return false;
        }
        objectives[i]=(savedObjective==1);
    }
    for (int floor=0; floor<NUM_FLOORS; floor++)
    {
        for (int room=0; room<ROOMS_PER_FLOOR; room++)
        {
            if (!(loadFile>>entityCounts[floor][room]) || entityCounts[floor][room]<0 || entityCounts[floor][room]>MAX_ENTITIES_PER_ROOM)
            {
                return false;
            }
            for (int item=0; item<MAX_ENTITIES_PER_ROOM; item++)
            {
                int savedItem;
                if (!(loadFile>>savedItem) || savedItem<0 || savedItem>255)
                {
                    return false;
                }
                roomEntities[floor][room][item]=static_cast<char>(savedItem);
            }
        }
    }
    return static_cast<bool>(loadFile);
}