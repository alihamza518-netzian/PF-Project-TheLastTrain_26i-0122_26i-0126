#include "logger.h"
#include "util.h"

// File persistence for savegame.txt and game_summary.txt using primitive variables
#include "logger.h"
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