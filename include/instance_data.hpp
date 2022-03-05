#pragma once 

#include <utility>
#include <vector>
#include <string>
#include <list>

#include "alias.hpp"

#define OBJECTIF 1

//=================================================================
//=================================================================
//=================================================================

class Client{
public:
    Coordinates coordLoc;
    Coordinates coordNodeLoc;
    TimeWindow timeWindow;
    double demand;
    double service;
    double depotDist;
    mutable unsigned int noServe;
    mutable double minCost;
    size_t nodeID;
    Client(){noServe=0;minCost=1e30;}
};

//=================================================================

class ClientPtr{
public:
    const Client* ptr;
    
    unsigned int id;
    double startTime;
    double finishTime;
    double waitTime;
    double aggWaitTime;
    double fwdSlack0;
    double fwdSlack1;
    double bwdSlack1;
    
    //-------------------------------------------------------------
    ClientPtr();
    ClientPtr(unsigned int id_,const Client* clnt);
    ClientPtr(double startTime_,unsigned int id_,const Client* clnt);
    ClientPtr(const ClientPtr & copy);
    //-------------------------------------------------------------
    //-------------------------------------------------------------
    //-------------------------------------------------------------
    bool operator<(const ClientPtr & ptr_2 ) const;

    bool operator==(const ClientPtr & ptr_2 );

    static bool compareTimeWindow(const ClientPtr & ptr_1, const ClientPtr & ptr_2 );
    static bool compareDemand(const ClientPtr & ptr_1, const ClientPtr & ptr_2 );
    static bool compareInitialTime(const ClientPtr & ptr_1, const ClientPtr & ptr_2 );
    static bool compareFinalTime(const ClientPtr & ptr_1, const ClientPtr & ptr_2 );
    static bool compareNoServe(const ClientPtr & ptr_1, const ClientPtr & ptr_2 );
    static bool compareClosest(const ClientPtr & ptr_1, const ClientPtr & ptr_2 );
    static bool compareFarest(const ClientPtr & ptr_1, const ClientPtr & ptr_2 );
};

//=================================================================

class Route{
public:
    double currentCapa;
    double currentDist;
    double currentDuration;
    double currentServiceTime;
    double fwdSlack0;
    double fwdSlack1;
    std::list<ClientPtr> clients;
    //----------------------------------------------------------------
    Route(){currentCapa=currentDist=currentDuration=currentServiceTime=0;
            fwdSlack0=fwdSlack1=0;}
    //----------------------------------------------------------------
    void clear();
    //----------------------------------------------------------------
    double cost() const;
    double noServeCost();
};

//=================================================================

class Solution{
public:
    double totalDuration;
    double totalDistance;
    std::list<Route> routes;
    std::list<ClientPtr> requestBank;
    //----------------------------------------------------------------

    Solution(){totalDistance=totalDuration=0;}
    void clear();
    //----------------------------------------------------------------
    void discountRoute(const Route& route);
    void accountRoute(const Route& route);
    double cost()const;
    bool feas()const;
    void sendRequestBank(std::vector<ClientPtr>& clntInsert);
    unsigned int noServeRequestBank() const;
};

//=================================================================
//=================================================================
//=================================================================

class Tour{
public:
    VecPoints geolocs;
    std::vector<size_t> nodeIDs;
    std::vector<std::string> polylines;
    std::vector<std::string> macroPolylines;
};


//=================================================================

class InstanceData{

public:
    TimeWindow depotTimeWindow;
    Coordinates depotCoordLoc;
    Coordinates depotNodeCoordLoc;
    std::vector<Client> clients;
    std::vector<double > capacitiesByVehiculeType;
    std::vector<double > distanceTable;
    std::vector<double > durationTable;
    double maxDist;
    size_t depotNodeID;
    //-----------------------------------------------------------------
    //-----------------------------------------------------------------
    //-----------------------------------------------------------------
    InstanceData();

    //-----------------------------------------------------------------

    void reinitiate(unsigned int nbClients,
                    const std::vector<double >& capacitiesByType_);
};


//=================================================================

void readSolomonData( InstanceData & data, std::string in, unsigned int nbClientsPcnt=0);
void readData( InstanceData & data, std::string in);

void solution2Tours(const InstanceData & data, const Solution & sol, std::vector<Tour> & tours);

void readSolution(const InstanceData & data, std::string fsolution, Solution& sol);
