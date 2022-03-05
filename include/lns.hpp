#pragma once
#include "neighborhoods.hpp"

#include <iostream> 

//=================================================================
//=================================================================
//=================================================================

class LNSMetaheuristic{
    const InstanceData & data;

    std::vector<repairNeighborhood*> repairers;
    std::vector<destroyNeighborhood*> destroyers;
    
    //----------------------------------------------------------------
    unsigned int nbRepaires;
    unsigned int nbDestroyers;
    
public:
    Solution  sol;
    Solution  bestSol;

    //----------------------------------------------------------------
    LNSMetaheuristic(const InstanceData & data_);
    ~LNSMetaheuristic();
    
    //----------------------------------------------------------------

    void changeRemoveInterval(double minr_, double maxr_);

    //----------------------------------------------------------------
    int compareSolutions(const Solution&  sol1, const Solution&  sol2);
    bool worseAcceptance(const Solution&  sol1, const Solution&  best, double rate, double & dev);
    bool infeasAcceptance(const Solution&  sol1, const Solution&  best);
    //----------------------------------------------------------------
    size_t getRepairerId(bool fleetMin, bool param);
    size_t getDestroyerId(bool param);
    //----------------------------------------------------------------
    void fleet_min();
    void solve();
    bool local(Solution & trial);
    //----------------------------------------------------------------
};