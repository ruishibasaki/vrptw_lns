#pragma once 

#include "instance_data.hpp"
#include <map>
#include <fstream>


//=====================================================================
class RouteEngine{
    std::map<Coordinates, int> depots;
    std::map<Coordinates, int> clients;
    

public:
    RouteEngine(){}
    void toursToJson(std::vector<Tour> & tours);
};



//=================================================================
//=================================================================
//=================================================================

