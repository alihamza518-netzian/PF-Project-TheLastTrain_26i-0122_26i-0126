#include "viewer_iface.h"
#include "level.h"
#include <iostream>

using namespace std;

void renderDashboard(
    int playerFloor,int playerRoom,
    int energy,int time,int score,
    const char inventory[MAX_INVENTORY_CAPACITY],
    int inventoryCount,
    int emergencyTripsLeft)
{
    cout<<"\n+--------------------------------------+\n";
    cout<<"|           THE LAST TRAIN             |\n";
    cout<<"+--------------------------------------+\n";
    cout<<"| Location       : Floor "<<playerFloor
        <<", Room "<<playerRoom<<'\n';
    cout<<"| Energy         : "<<energy<<" / "<<MAX_ENERGY<<'\n';
    cout<<"| Time           : "<<time<<" / "<<MAX_TIME<<'\n';
    cout<<"| Score          : "<<score<<'\n';
    cout<<"| Lift 2 trips   : "<<emergencyTripsLeft<<'\n';
    cout<<"+--------------------------------------+\n";
    cout<<"| Inventory                              |\n";

    if (inventoryCount==0)
    {
        cout<<"|   (empty)                              |\n";
    }
    else
    {
        for (int i=0;i<inventoryCount;i++)
        {
            cout<<"|   "<<getEntityName(inventory[i])<<'\n';
        }
    }

    cout<<"+======================================+\n";
}

void renderRoomDetails(
    int floor,int room,
    const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCount)
{
    cout<<"\n+======================================+\n";
    cout<<"| Room: Floor "<<floor<<", Room "<<room<<'\n';
    cout<<"+======================================+\n";

    if (entityCount==0)
    {
        cout<<"|   No items or objects found.          |\n";
    }
    else
    {
        for (int i=0;i<entityCount;i++)
        {
            cout<<"|   - "
                <<getEntityName(roomEntities[floor-1][room-1][i])
                <<'\n';
        }
    }

    cout<<"+--------------------------------------+\n";
}

void renderAsciiBuilding(int playerFloor,int playerRoom)
{
    cout<<"\nBUILDING MAP  (P = your location)\n";

    for (int floor=NUM_FLOORS;floor>=1;floor--)
    {
        cout<<"Floor "<<floor<<"  ";

        for (int room=1;room<=ROOMS_PER_FLOOR;room++)
        {
            if(floor==playerFloor&&room==playerRoom)
            {
                cout<<"[ P ]";
            }
            else
            {
                cout<<"[ R"<<room<<" ]";
            }
        }

        cout<<'\n';
    }

    cout<<"             Rooms:  1     2     3     4\n";
}

void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES])
{
    const char* objectiveNames[NUM_OBJECTIVES]=
    {
        "Master Key collected",
        "Clue 4 found",
        "Clue 2 found",
        "Clue 7 found",
        "Cabinet unlocked",
        "Train Key collected",
        "Fuse collected",
        "Fuse installed",
        "Lift Pass collected",
        "Lift 2 active",
        "Passenger rescued",
        "Cat Carrier collected",
        "Cat secured",
        "Train prepared",
        "Mission won",
        "Mission failed"
    };

    cout<<"\n+--------------------------------------+\n";
    cout<<"|           MISSION OBJECTIVES         |\n";
    cout<<"+--------------------------------------+\n";

    for (int i=0;i<NUM_OBJECTIVES;i++)
    {
        if (objectives[i])
        {
            cout<<"| [x] "<<objectiveNames[i]<<'\n';
        }
        else
        {
            cout<<"| [ ] "<<objectiveNames[i]<<'\n';
        }
    }

    cout<<"+======================================+\n";
}
