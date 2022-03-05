#include "operations.hpp"
#include <iostream>
#include <algorithm>

//=================================================================
//=================================================================
//=================================================================

bool
Operations::ejectionSearch(Solution& sol, ClientPtr newClient, std::vector<ClientPtr>& clntInsert){ 
    size_t nbRemove = 3;
    std::vector<regretCell> clients;
    clients.reserve(base);
    double windowDepot = data.depotTimeWindow.second-data.depotTimeWindow.first;
    double vehiCapa = data.capacitiesByVehiculeType[0];
    std::list<Route>::iterator itr = sol.routes.begin();
    for(; itr!=sol.routes.end();++itr){
        std::list<ClientPtr>::iterator it = itr->clients.begin();

        for(;it!=itr->clients.end();++it){
            if((it->startTime + 1e-4 < newClient.ptr->timeWindow.second) &&
                (it->finishTime > newClient.ptr->timeWindow.first )){
                clients.push_back({data.distanceTable[it->id*base+newClient.id], false, itr, it});
            }
        }
    }
    std::sort(clients.begin(), clients.end()); 
    ejectionCell minCostEjection;
    bool foundEjection =false;
    for (size_t i = 0; i < clients.size(); i++){
        std::list<Route>::iterator route = clients[i].route;
        std::list<ClientPtr>::iterator pos = clients[i].pos;
        std::list<ClientPtr>::iterator bef = clients[i].pos;
        double capacity = route->currentCapa + newClient.ptr->demand; 
        double maxCost = 0;
        for (size_t n = 0; n < nbRemove; n++){
            double tW = bef->ptr->timeWindow.second-bef->ptr->timeWindow.first;
            double cost = bef->ptr->noServe/1000 + windowDepot/tW;
            capacity -= bef->ptr->demand;
            maxCost += cost*(n+1);
            if(capacity<=vehiCapa){
                if( checkEjection(*route, newClient,  bef, pos)){
                    if( minCostEjection.cost >= maxCost){
                        minCostEjection.cost = maxCost;
                        minCostEjection.route = route;
                        minCostEjection.begin =bef;
                        minCostEjection.end = pos;
                        foundEjection =true;
                        nbRemove=2;
                    }
                }
            }
            if(bef != route->clients.begin())bef--;
            else break;
            
        }
    }
    if(foundEjection){
        clntInsert.pop_back();
        minCostEjection.end++;
        sol.discountRoute(*minCostEjection.route);
        std::list<ClientPtr>::iterator pos; 
        while(minCostEjection.begin != minCostEjection.end){
            pos = minCostEjection.begin;
            minCostEjection.begin++;
            clntInsert.push_back(*pos);
            if(!remove(*minCostEjection.route, pos)){
                std::cout<<"ERROR WITH EJECTION SEARCH"<<std::endl; abort();      
            }
            
        }
        insert(*minCostEjection.route, &newClient, minCostEjection.end);
        sol.accountRoute(*minCostEjection.route);
        return true;
    }
    return false;
}

//=================================================================

bool 
Operations::checkEjection(Route& route, ClientPtr& newClient,  std::list<ClientPtr>::iterator & beginRmv, 
                        std::list<ClientPtr>::iterator & endRmv){
    
    double arrivalTime;
    std::list<ClientPtr>::iterator bef;
    std::list<ClientPtr>::iterator aft=endRmv; aft++;
    if(beginRmv == route.clients.begin()){
        arrivalTime = data.durationTable[newClient.id];
    }else{
        bef = beginRmv; bef--;
        arrivalTime = bef->finishTime + data.durationTable[bef->id*base+newClient.id];
    }
    newClient.startTime = std::max(newClient.ptr->timeWindow.first, arrivalTime);
    newClient.finishTime = newClient.startTime + newClient.ptr->service;
    if(newClient.finishTime > newClient.ptr->timeWindow.second + 1e-6) return false;

    if(aft == route.clients.end()){
        arrivalTime = newClient.finishTime + data.durationTable[newClient.id*base];
        if(arrivalTime<=data.depotTimeWindow.second + 1e-6) return true;
        else return false;
    }else{
        arrivalTime = newClient.finishTime + data.durationTable[newClient.id*base+aft->id];
    }
    double push = arrivalTime - aft->startTime; 
    if(push > aft->fwdSlack1) return false;
    else return true;
}