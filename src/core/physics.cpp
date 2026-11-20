
#include "physics.h"
#include "util.h"
#include<iostream>
#include <cstdlib>
using namespace std;

// Movement validation and resource adjustments without structures

bool isDirectlyConnected(
    int fromF, int fromR, int toF, int toR,
    const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
    char& outConnType)
{
    // Initialize output connection
    outConnType = CONN_NONE;

    // Check room boundaries
    if (fromF < 1 || fromF > NUM_FLOORS ||
        toF < 1 || toF > NUM_FLOORS ||
        fromR < 1 || fromR > ROOMS_PER_FLOOR ||
        toR < 1 || toR > ROOMS_PER_FLOOR)
    {
        return false;
    }

    // Check connection array
    if (horizontalConn == nullptr)
    {
        return false;
    }

    // Only horizontal connections are handled here
    if (fromF != toF || std::abs(fromR - toR) != 1)
    {
        return false;
    }

    // Get connection between adjacent rooms
    outConnType = horizontalConn[fromF - 1]
                                  [(fromR < toR ? fromR : toR) - 1];

    // Reject walls and empty connections
    if (outConnType == CONN_NONE ||
        outConnType == CONN_SOLID_WALL)
    {
        outConnType = CONN_NONE;
        return false;
    }

    return true;
}
