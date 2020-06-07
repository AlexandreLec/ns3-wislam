// Variables used for this implementation.
#include <ns3/core-module.h>

#include <ns3/network-module.h>

#include <ns3/config-store.h>

#include <ns3/buildings-module.h>
#include <ns3/buildings-helper.h>
#include <ns3/hybrid-buildings-propagation-loss-model.h>

#include <iostream>

// Class used.
using namespace std;
using namespace ns3;

// Variables defined 
double x_min = 0.0;
double x_max = 100.0;
double y_min = 0.0;
double y_max = 100.0;
double z_min = 0.0;
double z_max = 10.0;


// main function where we implement ours buildings and ours WI-FI elements/infrastructure.
int main ()
{

    // Set Building element (define properties) 
    Ptr<Building> b = CreateObject<Building> ();

        b->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
        b->SetBuildingType (Building::Office);
        b->SetExtWallsType (Building::ConcreteWithWindows);
        b->SetNFloors (1);
        b->SetNRoomsX (2);
        b->SetNRoomsY (2);

    // Variable used to simplify our work

    uint16_t BuildingType = b->GetBuildingType();
    uint16_t WallType = b->GetExtWallsType();

    // Print all the Values we defined.
    printf("\n");
    cout << "Dimensions : " << " Xmin :" << b->GetBoundaries().xMin << " Xmax :" << b->GetBoundaries().xMax << " Ymin :" << b->GetBoundaries().yMin << " Ymax :" << b->GetBoundaries().yMax << " Zmin :" << b->GetBoundaries().zMin << " Zmax :" << b->GetBoundaries().zMax; 
    printf("\n");

    switch (BuildingType)
    {
        case 0:
            cout << "Type of buildings : " << "Residential";
            printf("\n");
        break;

        case 1:
            cout << "Type of buildings : " << "Office";
            printf("\n");
        break;

        case 2:
            cout << "Type of buildings : " << "Commercial";
            printf("\n");
        break;

        default:
            cout << "Type of buildings : " << "No Type defined";
            printf("\n");
    }

    switch (WallType)
    {
        case 0:
            cout << "Type of wall : " << "Wood";
            printf("\n");
        break;

        case 1:
            cout << "Type of wall : " << "ConcreteWithWindows";
            printf("\n");
        break;

        case 2:
            cout << "Type of wall : " << "ConcreteWithoutWindows";
            printf("\n");
        break;

        case 3:
            cout << "Type of wall : " << "StoneBlocks";
            printf("\n");
        break;

        default:
            cout << "Type of wall : " << "No Type defined";
            printf("\n");
    }
    cout << "Number of floors : " << b->GetNFloors ();
    printf("\n");
    cout << "Number of rooms for X : " << b->GetNRoomsX ();
    printf("\n");
    cout << "Number of rooms for Y : " << b->GetNRoomsY ();
    printf("\n");
    cout << "Total number of rooms : " << b->GetNRoomsY () + b->GetNRoomsX ();
    printf("\n");

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
