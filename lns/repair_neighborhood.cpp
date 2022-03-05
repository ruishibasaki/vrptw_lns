
#include "neighborhoods.hpp"
#include <algorithm>

#include <cmath>
//=================================================================
//=================================================================
//=================================================================

void 
randomOrderBestPositionInsertion::repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin){
    unsigned int iters=0;
    unsigned int limitIters= fleetMin? LOOP_LIMIT: LOOP_LIMIT/1;
    while (!clntInsert.empty()){
        std::random_shuffle(clntInsert.begin(), clntInsert.end());
        repairInSequence( sol, clntInsert, (++iters <= limitIters), fleetMin);
    }
}

//=================================================================
//=================================================================
//=================================================================


void 
twoRegretInsertion::inner_loop(Solution& sol, std::vector<ClientPtr>& clntInsert, bool ejection, bool infeasAllowd){

    std::vector<positionCost> bestPosVec;
    std::vector<regretCell> bestKRegrets;
    bestKRegrets.resize(2);
    bestPosVec.reserve(data.clients.size()+1);
    std::vector<ClientPtr> newRoutes;
    while(!clntInsert.empty()){

        size_t nbToInsert = clntInsert.size();
        ClientPtr* clntSelected=0 ;
        std::list<Route>::iterator routeSelected;
        std::list<ClientPtr>::iterator posSelected;
        double maxDiff = -1e20;

        size_t i = 0;
        while( i < nbToInsert){
            
            for(size_t k = 0; k < 2; ++k){
                bestKRegrets[k].cost = 1e20;
                bestKRegrets[k].empty = true;
            }

            ClientPtr& client = clntInsert[i];            
            bool noRoute=true;  
            
            std::list<Route>::iterator itr = sol.routes.begin();
            for(; itr!=sol.routes.end();++itr){
                Route& route = *itr;            
                if(route.currentCapa + client.ptr->demand > data.capacitiesByVehiculeType[0]) continue;

                if(bestPositions(route, client,  bestPosVec)>0){
                    noRoute=false; 
                    size_t maxk = std::min(size_t(2),bestPosVec.size());
                    for(size_t k = 0; k < maxk; k++){
                        if(bestPosVec[k].cost < bestKRegrets.back().cost){
                            bestKRegrets.back().cost = bestPosVec[k].cost;
                            bestKRegrets.back().pos = bestPosVec[k].pos;
                            bestKRegrets.back().route = itr;
                            bestKRegrets.back().empty = false;
                            std::sort(bestKRegrets.begin(), bestKRegrets.end()); 
                        }else break;
                    }
                }
                bestPosVec.clear();
            }
            if(noRoute){
                client.ptr->noServe++;
                std::swap(client, clntInsert.back()); 
                if(ejection && ejectionSearch(sol, clntInsert.back(), clntInsert)){return;}
                if(infeasAllowd){
                    sol.requestBank.push_back(clntInsert.back());
                }else{
                    openRoute( sol, clntInsert.back()); 
                    sol.accountRoute(sol.routes.back());
                }
                clntInsert.pop_back(); 
                --nbToInsert;
                clntSelected=0;
                break;
            }else{
                double delta1 = bestKRegrets[0].cost - bestKRegrets[0].route->cost();
                double diff = -delta1;
                for(size_t k = 1; k < 2; k++){
                    if(!bestKRegrets[k].empty){
                        diff += bestKRegrets[k].cost - bestKRegrets[k].route->cost() ;
                    }else diff += 1e30;
                }
                if(maxDiff<diff){
                    maxDiff = diff;
                    clntSelected = &client;
                    posSelected =  bestKRegrets[0].pos;
                    routeSelected = bestKRegrets[0].route;
                }
                ++i; 
            }
        }
        if(clntSelected){
            sol.discountRoute(*routeSelected);
            insert(*routeSelected, clntSelected, posSelected);
            sol.accountRoute(*routeSelected);
            std::swap(*clntSelected, clntInsert.back());
            clntInsert.pop_back();
        }
        
    }
}

//=================================================================

void 
twoRegretInsertion::repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin){
    unsigned int iters=0;
    unsigned int limitIters= fleetMin? LOOP_LIMIT: LOOP_LIMIT/1;
    while (!clntInsert.empty()){
        inner_loop( sol, clntInsert, (++iters <= limitIters), fleetMin);
    }
}

//=================================================================
//=================================================================
//=================================================================

void 
sharpWindowOrderBestPositionInsertion::repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin){
    unsigned int iters=0;
    unsigned int limitIters= fleetMin? LOOP_LIMIT: LOOP_LIMIT/1;
    while (!clntInsert.empty()){
        std::sort(clntInsert.begin(), clntInsert.end(), ClientPtr::compareTimeWindow);
        repairInSequence(sol, clntInsert, (++iters <= limitIters), fleetMin);
    }
}

//=================================================================
//=================================================================
//=================================================================

void 
demandOrderBestPositionInsertion::repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin){
    unsigned int iters=0;
    unsigned int limitIters= fleetMin? LOOP_LIMIT: LOOP_LIMIT/1;
    while (!clntInsert.empty()){
        std::sort(clntInsert.begin(), clntInsert.end(), ClientPtr::compareDemand);
        repairInSequence(sol, clntInsert, (++iters <= limitIters), fleetMin);
    }
}


//=================================================================
//=================================================================
//=================================================================

void 
farestOrderBestPositionInsertion::repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin){
    unsigned int iters=0;
    unsigned int limitIters= fleetMin? LOOP_LIMIT: LOOP_LIMIT/1;
    while (!clntInsert.empty()){
        std::sort(clntInsert.begin(), clntInsert.end(), ClientPtr::compareFarest);
        repairInSequence(sol, clntInsert, (++iters <= limitIters), fleetMin);
    }
}

//=================================================================
//=================================================================
//=================================================================

void 
closestOrderBestPositionInsertion::repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin){
    unsigned int iters=0;
    unsigned int limitIters= fleetMin? LOOP_LIMIT: LOOP_LIMIT/1;
    while (!clntInsert.empty()){
        std::sort(clntInsert.begin(), clntInsert.end(), ClientPtr::compareClosest);
        repairInSequence(sol, clntInsert, (++iters <= limitIters), fleetMin);
    }
}

//=================================================================
//=================================================================
//=================================================================

void 
noServeOrderBestPositionInsertion::repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin){
    unsigned int iters=0;
    unsigned int limitIters= fleetMin? LOOP_LIMIT: LOOP_LIMIT/1;
    while (!clntInsert.empty()){
        std::sort(clntInsert.begin(), clntInsert.end(), ClientPtr::compareNoServe);
        repairInSequence(sol, clntInsert, (++iters <= limitIters), fleetMin);
    }
}