#pragma once 


class Params{
public:
    unsigned int nbVehicleTypes;
    std::vector<double > capacitiesByType;

    unsigned int nbDepots;
    unsigned int nbClients;
    unsigned int nbClientsMin;
    unsigned int nbClientsMax;

    unsigned int minDemand;
    unsigned int maxDemand;
    std::vector<double > typeServices;
    std::vector<TimeWindow > timeWindows;
    std::vector<double> timeWindowsFreq;

    double seed;
    unsigned int distribution;
    std::vector<double> randParams;
    
    //-----------------------------------------------------------------
    Params(){
        seed=123;
        
        nbVehicleTypes= 1;
        capacitiesByType= {100};
        
        nbClientsMin =30;
        nbClientsMax = 200;
        nbClients = 30;
        nbDepots = 10; //change the number of depots to change the number of distances generated

        minDemand = 1;
        maxDemand = 80;
        
        typeServices = {5,10};
        
        timeWindows ={{6,9},{7,10},{7,12},{9,12},{12,14},{14,17},{16,20}};
        timeWindowsFreq = {0.15,0.25,0.15,0.15,0.1,0.1,0.1};

        randParams={2.0,5.33333};
    }

};
