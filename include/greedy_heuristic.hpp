#pragma once

#include "instance_data.hpp"
#include "operations.hpp"

#include <list>
#include <string>
#include <iostream> 




//=================================================================

class GreedyHeuristic: public Operations{
public:

    GreedyHeuristic(const InstanceData & data_):Operations(data_){}
    
    //-------------------------------------------------------------

    void solve(Solution & sol);
};