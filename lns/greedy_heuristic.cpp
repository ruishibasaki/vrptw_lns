
#include "greedy_heuristic.hpp"

#include <cmath> 
#include <algorithm> 
#include <cassert>

//=================================================================
//=================================================================
//=================================================================

void 
GreedyHeuristic::solve(Solution & sol){    

    const size_t nbClients = data.clients.size();
    double totalDemand=0; 
    for(size_t i = 0; i<nbClients;++i){
        totalDemand+=data.clients[i].demand; 
    }
    unsigned int nbRoutes = ceil(1.05*totalDemand/data.capacitiesByVehiculeType.back());
    nbRoutes = nbRoutes> nbClients? nbClients: nbRoutes;

    
    std::list<ClientPtr> farest;
    for(size_t i = 1; i<=nbClients;++i){
        farest.push_back(ClientPtr(data.distanceTable[i], i, &data.clients[i-1]));
    }
    farest.sort();
 
    double capa = data.capacitiesByVehiculeType[0];
                   
    while(!farest.empty()){
        
        
        Route& newRoute =  openRoute( sol, farest.back()); 
        
        farest.pop_back();
        bool closedRoute=false;
        
        while(!farest.empty() && !closedRoute){
            
            std::list<ClientPtr>::iterator minNeighbor;
            std::list<ClientPtr>::iterator minNeighborPos;
            std::list<ClientPtr>::iterator it = farest.begin();

            double minCost=1e20;
            bool incRoute=false;

            closedRoute=true;
            
            for(;it!=farest.end();++it){
                
                if(newRoute.currentCapa + it->ptr->demand > data.capacitiesByVehiculeType[0]) continue;
            
                std::list<ClientPtr>::iterator bestPos;
                double cost = bestPosition(newRoute, *it,  bestPos, true);
                if(cost>=0 && minCost>cost){
                    minCost = cost;
                    minNeighbor = it;
                    minNeighborPos = bestPos;
                    closedRoute=false;
                }
            }

            if(!closedRoute){
                insert(newRoute, &(*minNeighbor), minNeighborPos);
                farest.erase(minNeighbor);
            }
        }
        sol.accountRoute(sol.routes.back());
    }
}

