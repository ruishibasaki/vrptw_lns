
#include "operations.hpp"
#include <cassert>

#include <iostream>
#include <algorithm>

//=================================================================
//=================================================================
//=================================================================

double 
Operations::routeDuration(const Route& route){
    const ClientPtr& last = route.clients.back();
    double currentDuration =  last.finishTime + data.durationTable[last.id*base];
    currentDuration -= data.depotTimeWindow.first + std::min(route.fwdSlack0, last.aggWaitTime);
    return currentDuration;
}

//=================================================================

Route& 
Operations::openRoute(Solution& sol, ClientPtr& firstClient){
    sol.routes.push_back(Route());
    Route& newRoute =  sol.routes.back(); 

    double arrivalTime = (data.depotTimeWindow.first+data.durationTable[firstClient.id]);
    firstClient.startTime = std::max(arrivalTime,firstClient.ptr->timeWindow.first);
    firstClient.waitTime = std::max(0.0,firstClient.startTime - arrivalTime);
    firstClient.aggWaitTime = firstClient.waitTime;
    firstClient.finishTime = firstClient.startTime+firstClient.ptr->service;
    
    newRoute.clients.push_back(firstClient);
    fwdRecursion(newRoute, newRoute.clients.end());
    bwdRecursion(newRoute, newRoute.clients.begin());
    newRoute.currentServiceTime = data.durationTable[firstClient.id] + firstClient.ptr->service + data.durationTable[firstClient.id*base] ;
    newRoute.currentCapa = firstClient.ptr->demand;
    newRoute.currentDist = data.distanceTable[firstClient.id]+data.distanceTable[firstClient.id*base];
    newRoute.currentDuration = routeDuration(newRoute);
    return newRoute;
}

//=================================================================

double 
Operations::pushRoute(double push, double arrivalTime,
                    std::list<ClientPtr>::iterator& bef,
                    std::list<ClientPtr>::iterator& next, 
                    const std::list<ClientPtr>::iterator& end){
    while(1){
        next->waitTime = std::max(0.0, next->ptr->timeWindow.first- arrivalTime);
        next->aggWaitTime = next->waitTime + ((bef!=end) ? bef->aggWaitTime : 0.0);
        next->startTime += push;
        next->finishTime += push;
        bef = next;
        if(++next ==end) break;
        arrivalTime = bef->finishTime + data.durationTable[bef->id*base+next->id];
        push = std::max(arrivalTime, next->ptr->timeWindow.first) - next->startTime;
    }
    return push;
}

//=================================================================


double 
Operations::fwdRecursion(Route& route, const std::list<ClientPtr>::iterator & pos){
    std::list<ClientPtr>::iterator bef = pos; 
    double f0, f1;
    if(pos==route.clients.begin()){
        f0 = data.depotTimeWindow.second - data.depotTimeWindow.first;
        f1 = pos->ptr->timeWindow.second - pos->finishTime + pos->waitTime;
    }else{
        --bef;
        f0 = fwdRecursion(route,bef);
        if(pos==route.clients.end()){
            f1 = data.depotTimeWindow.second - (bef->finishTime + data.durationTable[bef->id*base]) + bef->aggWaitTime ;
            route.fwdSlack0 = std::min(f0,f1);
            return route.fwdSlack0;
        }else{
            f1 = pos->ptr->timeWindow.second - pos->finishTime + bef->aggWaitTime + pos->waitTime;
        }
    }
    pos->fwdSlack0 = std::min(f0,f1);
    return pos->fwdSlack0 ;
}

//=================================================================


double 
Operations::bwdRecursion(Route& route, const std::list<ClientPtr>::iterator & pos){
    if(pos==route.clients.end()) return 0.0;

    std::list<ClientPtr>::iterator next = pos; ++next;
    double f1, fn;
    if(next==route.clients.end()){
        f1 = pos->ptr->timeWindow.second - pos->finishTime;
        fn = data.depotTimeWindow.second - (pos->finishTime + data.durationTable[pos->id*base]);
        route.fwdSlack1 = fn;
        pos->bwdSlack1 = pos->finishTime - pos->ptr->timeWindow.first - pos->ptr->service;
    }else{
        fn = bwdRecursion(route,next) + next->waitTime;
        f1 = pos->ptr->timeWindow.second - pos->finishTime;
        double b0 = pos->finishTime - pos->ptr->timeWindow.first - pos->ptr->service;
        pos->bwdSlack1 = std::min(next->bwdSlack1, b0);
    }
    pos->fwdSlack1 = std::min(fn,f1);
    return pos->fwdSlack1 ;
}

//=================================================================


double 
Operations::concatenation(const Route& route, const ClientPtr& newClient, const std::list<ClientPtr>::iterator & pos){
    
    if(pos!=route.clients.end()){
        double f1;
        if(pos!=route.clients.begin()){
            std::list<ClientPtr>::const_iterator bef = pos; --bef;
            f1 = newClient.ptr->timeWindow.second - newClient.finishTime + newClient.aggWaitTime;
            f1 = std::min(bef->fwdSlack0 , f1);
        }else{
            f1 = newClient.ptr->timeWindow.second - newClient.finishTime + newClient.aggWaitTime;
            f1 = std::min((data.depotTimeWindow.second - data.depotTimeWindow.first) , f1);
        }

        double arrivalTime = newClient.finishTime + data.durationTable[newClient.id*base+pos->id];
        double f2 = pos->fwdSlack1 + newClient.aggWaitTime + (pos->finishTime - arrivalTime - pos->ptr->service);
        return std::min(f1, f2);
    }else{
        const ClientPtr& lastClient = route.clients.back();
        double f1 = newClient.ptr->timeWindow.second - newClient.finishTime + newClient.aggWaitTime;
        f1 = std::min(lastClient.fwdSlack0 , f1);
        return std::min(f1, (route.fwdSlack1+newClient.aggWaitTime));
    }
}

//=================================================================
//=================================================================
//=================================================================

double 
Operations::computeNewRouteDistance(const Route& route, ClientPtr& newClient,const std::list<ClientPtr>::iterator & pos){
    double cost = route.currentDist;
    std::list<ClientPtr>::const_iterator bef;

    if(pos==route.clients.begin()){
        if(!route.clients.empty()){
            cost -= data.distanceTable[pos->id];
            cost += data.distanceTable[newClient.id*base+pos->id];
        }else{
            cost += data.distanceTable[newClient.id*base];
        }
        cost += data.distanceTable[newClient.id];
        
        double arrivalTime = data.durationTable[newClient.id];
        newClient.startTime = std::max(arrivalTime,newClient.ptr->timeWindow.first);
        newClient.finishTime = newClient.startTime+newClient.ptr->service;
    }else if(pos==(route.clients.end())){
        bef = pos;
        bef--;
        cost -= data.distanceTable[bef->id*base];
        cost += data.distanceTable[bef->id*base+newClient.id];
        cost += data.distanceTable[newClient.id*base];

        double arrivalTime = bef->finishTime + data.durationTable[bef->id*base+newClient.id];
        newClient.startTime = std::max(arrivalTime, newClient.ptr->timeWindow.first) ;
        newClient.finishTime = newClient.startTime+newClient.ptr->service;

        double routeFinishTime = newClient.finishTime + data.durationTable[newClient.id*base];
        if(routeFinishTime > data.depotTimeWindow.second+ 1e-6){ return -1; }
    }else{
        bef = pos;
        bef--;
        cost -= data.distanceTable[bef->id*base+pos->id];
        cost += data.distanceTable[bef->id*base+newClient.id];
        cost += data.distanceTable[newClient.id*base+pos->id];

        double arrivalTime = bef->finishTime + data.durationTable[bef->id*base+newClient.id];
        newClient.startTime = std::max(arrivalTime, newClient.ptr->timeWindow.first) ;
        newClient.finishTime = newClient.startTime+newClient.ptr->service;
    }
    if(newClient.finishTime > newClient.ptr->timeWindow.second+ 1e-6){ return -1; }
    
    if(pos!=(route.clients.end())){
        double arrivalTime = newClient.finishTime + data.durationTable[newClient.id*base+pos->id];
        double startTime = std::max(arrivalTime, pos->ptr->timeWindow.first) ;
        if((arrivalTime - pos->startTime) > pos->fwdSlack1 + 1e-6){ return -1; }
    }

    return cost;
}

//=================================================================

double 
Operations::computeNewRouteService(const Route& route, const ClientPtr& newClient,const std::list<ClientPtr>::iterator & pos){
    double cost = route.currentServiceTime +newClient.ptr->service ;
    std::list<ClientPtr>::const_iterator bef;
    if(pos==route.clients.begin()){
        if(!route.clients.empty()){
            cost -= data.durationTable[pos->id];
            cost += data.durationTable[newClient.id*base+pos->id];
        }else{
            cost += data.durationTable[newClient.id*base];
        }
        cost += data.durationTable[newClient.id];
        
    }else if(pos==(route.clients.end())){
        cost -= data.durationTable[route.clients.back().id*base];
        cost += data.durationTable[route.clients.back().id*base+newClient.id];
        cost += data.durationTable[newClient.id*base];
    }else{
        bef = pos;
        bef--;
        cost -= data.durationTable[bef->id*base+pos->id];
        cost += data.durationTable[bef->id*base+newClient.id];
        cost += data.durationTable[newClient.id*base+pos->id];
    }
    return cost;
}

//=================================================================

double 
Operations::computeNewRouteDuration(const Route& route, ClientPtr& newClient,const std::list<ClientPtr>::iterator & pos){
    double duration=-1;
    const ClientPtr& lastClient = route.clients.back();
    if(pos!=route.clients.end()){
        
        if(pos==route.clients.begin()){
            double arrivalTime = data.durationTable[newClient.id];
            newClient.startTime = std::max(arrivalTime,newClient.ptr->timeWindow.first);
            newClient.finishTime = newClient.startTime+newClient.ptr->service;
            newClient.aggWaitTime = newClient.waitTime = newClient.startTime-arrivalTime;
        }else{
            std::list<ClientPtr>::const_iterator bef = pos;
            bef--;
            double arrivalTime = bef->finishTime + data.durationTable[bef->id*base+newClient.id];
            newClient.startTime = std::max(arrivalTime, newClient.ptr->timeWindow.first) ;
            newClient.finishTime = newClient.startTime+newClient.ptr->service;
            newClient.waitTime = newClient.startTime-arrivalTime;
            newClient.aggWaitTime = newClient.waitTime + bef->aggWaitTime;
        }
        
        if(newClient.finishTime > newClient.ptr->timeWindow.second) return -1;

        
        double arrivalTime = newClient.finishTime + data.durationTable[newClient.id*base+pos->id];
        double startTime = std::max(arrivalTime, pos->ptr->timeWindow.first) ;
        double push = arrivalTime - pos->startTime;
        if(push > pos->fwdSlack1) return -1;
        
        double totalWaitTime=newClient.aggWaitTime;
        double waitTime2 = lastClient.aggWaitTime - pos->aggWaitTime;

        
        if(push>=0){
            if(waitTime2>0){
                totalWaitTime += std::max(0.0 , waitTime2-push);
            }
        }else{
            if(waitTime2>0){
                totalWaitTime += std::max(0.0 , waitTime2-push);
            }else{
                totalWaitTime += std::max(0.0 , -push-pos->bwdSlack1);
            }
        }
        
        double f0 = concatenation( route, newClient, pos);
        duration = computeNewRouteService(route, newClient,pos)+totalWaitTime;
        duration -= std::min(f0,totalWaitTime);

    }else{
        double arrivalTime = lastClient.finishTime + data.durationTable[lastClient.id*base+newClient.id];
        newClient.startTime = std::max(arrivalTime, newClient.ptr->timeWindow.first);
        newClient.finishTime = newClient.startTime + newClient.ptr->service;
        if(newClient.finishTime > newClient.ptr->timeWindow.second)return -1;

        newClient.waitTime =  newClient.startTime - arrivalTime;
        newClient.aggWaitTime = newClient.waitTime  +lastClient.aggWaitTime;
        double routeFinishTime = newClient.finishTime + data.durationTable[newClient.id*base];
        if(routeFinishTime > data.depotTimeWindow.second) return -1;

        double f0 = concatenation( route, newClient, pos);
        duration = routeFinishTime - data.depotTimeWindow.first - std::min(f0, newClient.aggWaitTime);
        
    }
    

    return duration;
}

//=================================================================

double 
Operations::computeNewCost(const Route& route, ClientPtr& newClient,const std::list<ClientPtr>::iterator & pos){
    double rD, rT;
    if(OBJECTIF==1){
        rD = computeNewRouteDistance(route, newClient,pos);
        if(rD < 0) return -1;
        return rD;     
       
    }else{
        rT = computeNewRouteDuration(route, newClient,pos); 
        if(rT < 0) return -1;
        return rT;
    }
}
//=================================================================

double 
Operations::bestPosition(Route& route,  ClientPtr& newClient, std::list<ClientPtr>::iterator & bestPos, bool fleetMin){
    std::list<ClientPtr>::iterator it=route.clients.begin();
    double cost=-1;
    double min=1e20;
    bool noPos=true;
    for(;it!=route.clients.end();++it){
        if(fleetMin && (rand()%1000)/1000.0<0.01){ continue;}
        cost = computeNewCost(route, newClient, it);
        if(cost>=0 && min>cost){
            bestPos = it;
            min = cost;
            noPos=false;
        }
    }
    cost = computeNewCost(route, newClient, route.clients.end());
    if(cost>=0 && min>cost){
        bestPos = route.clients.end();
        min = cost;
        noPos=false;
    }
    if(noPos) return -1;
    return min;
}

//=================================================================

int 
Operations::bestPositions(Route& route,  ClientPtr& newClient, std::vector<positionCost> & bestPos){
    std::list<ClientPtr>::iterator it=route.clients.begin();
    double cost=-1;
    bool noPos=true;
    for(;it!=route.clients.end();++it){
        cost = computeNewCost(route, newClient, it); 
        if(cost>=0){
            bestPos.push_back({cost, it});
            noPos=false;
        }
    }
    
    cost = computeNewCost(route, newClient, route.clients.end());
    if(cost>=0){
        bestPos.push_back({cost, route.clients.end()});
        noPos=false;
    }

    if(noPos) return -1;
    std::sort(bestPos.begin(), bestPos.end()); 
    return 1;
}

//=================================================================

bool 
Operations::insert(Route& route, ClientPtr* newClient,  std::list<ClientPtr>::iterator & pos){

    double arrivalTime;

    std::list<ClientPtr>::iterator bef;
    std::list<ClientPtr>::iterator next;
    if(pos==route.clients.begin()){
        arrivalTime = data.durationTable[newClient->id];
        newClient->aggWaitTime = 0;
    }else{
        bef = pos;
        bef--;
        arrivalTime = bef->finishTime + data.durationTable[bef->id*base+newClient->id];
        newClient->aggWaitTime = bef->aggWaitTime;
    }
    newClient->startTime = std::max(arrivalTime,newClient->ptr->timeWindow.first);
    newClient->waitTime =  newClient->startTime-arrivalTime;
    newClient->aggWaitTime += newClient->waitTime ;
    newClient->finishTime = newClient->startTime+newClient->ptr->service;
    
    double delta = -route.cost();
    if(pos==route.clients.end()){
        route.currentCapa += newClient->ptr->demand;
        route.currentDist =  computeNewRouteDistance(route,  *newClient, pos);
        route.currentServiceTime =  computeNewRouteService(route,  *newClient, pos);
        route.clients.insert(pos, *newClient);
        fwdRecursion(route, route.clients.end());
        bwdRecursion(route, route.clients.begin());
        route.currentDuration = routeDuration(route);
        if(route.currentDist<0){ std::cout<<" new distancea "<<route.currentDist<<std::endl; abort();}
        delta += route.cost();
        if(newClient->ptr->minCost > delta) newClient->ptr->minCost = delta;
        return true;
    }
        
    route.currentCapa += newClient->ptr->demand;
    route.currentDist =  computeNewRouteDistance(route,  *newClient, pos);
    route.currentServiceTime =  computeNewRouteService(route,  *newClient, pos);
    bef = route.clients.insert(pos, *newClient);

    arrivalTime = newClient->finishTime + data.durationTable[newClient->id*base+pos->id];
    double push = std::max(arrivalTime, pos->ptr->timeWindow.first) - pos->startTime;
    next = pos;
    push = pushRoute(push,  arrivalTime, bef, next, route.clients.end());
    
    
    fwdRecursion(route, route.clients.end());
    bwdRecursion(route, route.clients.begin());
    route.currentDuration = routeDuration(route);
    if(route.currentDist<0){ std::cout<<" new distanceb "<<route.currentDist<<std::endl; abort();}
    delta += route.cost();
    if(newClient->ptr->minCost > delta) newClient->ptr->minCost = delta;
    return true;
}


//=================================================================
//=================================================================
//=================================================================

double 
Operations::computeRmvRouteDistance(const Route& route, const std::list<ClientPtr>::iterator & rmvPos){
    if(route.clients.size()==1){
        return 0;
    }
    double cost = route.currentDist;
    double arrivalTime;
    std::list<ClientPtr>::const_iterator bef;
    std::list<ClientPtr>::const_iterator aft;
    std::list<ClientPtr>::const_iterator end = route.clients.end(); end--;
    
    if(rmvPos==route.clients.begin()){
        aft = rmvPos;
        aft++;
        cost -= data.distanceTable[rmvPos->id]; 
        cost -= data.distanceTable[rmvPos->id*base+aft->id];
        cost += data.distanceTable[aft->id];
        arrivalTime = data.durationTable[aft->id];

        double push = std::max(arrivalTime, aft->ptr->timeWindow.first) - aft->startTime;
        if(push > aft->fwdSlack1 + 1e-6) return -1;
    }else if(rmvPos==end){
        bef = rmvPos;
        bef--;
        cost -= data.distanceTable[rmvPos->id*base];
        cost -= data.distanceTable[bef->id*base+rmvPos->id];
        cost += data.distanceTable[bef->id*base];
    }else{
        bef = rmvPos;
        bef--;
        aft = rmvPos;
        aft++;
        cost -= data.distanceTable[bef->id*base+rmvPos->id];
        cost -= data.distanceTable[rmvPos->id*base+aft->id];
        cost += data.distanceTable[bef->id*base+aft->id];
        arrivalTime = bef->finishTime +  data.durationTable[bef->id*base+aft->id];
        double push = std::max(arrivalTime, aft->ptr->timeWindow.first) - aft->startTime;
        if(push > aft->fwdSlack1+ 1e-6) return -1;
    }
    
    return  cost;
}

//=================================================================

double 
Operations::computeRmvRouteService(const Route& route, const std::list<ClientPtr>::iterator & rmvPos){
    if(route.clients.size()==1){
        return 0;
    }
    double cost = route.currentServiceTime - rmvPos->ptr->service;
    std::list<ClientPtr>::const_iterator bef;
    std::list<ClientPtr>::const_iterator aft;
    std::list<ClientPtr>::const_iterator end = route.clients.end(); end--;
    if(rmvPos==route.clients.begin()){
        aft = rmvPos;
        aft++;
        cost -= data.durationTable[rmvPos->id]; 
        cost -= data.durationTable[rmvPos->id*base+aft->id];
        cost += data.durationTable[aft->id];
    }else if(rmvPos==end){
        bef = rmvPos;
        bef--;
        cost -= data.durationTable[bef->id*base+rmvPos->id];
        cost -= data.durationTable[rmvPos->id*base];
        cost += data.durationTable[bef->id*base];
    }else{
        bef = rmvPos;
        bef--;
        aft = rmvPos;
        aft++;
        cost -= data.durationTable[bef->id*base+rmvPos->id];
        cost -= data.durationTable[rmvPos->id*base+aft->id];
        cost += data.durationTable[bef->id*base+aft->id];
    }
    return  cost;
}

//=================================================================

double 
Operations::computeRmvRouteDuration(const Route& route, const std::list<ClientPtr>::iterator & rmvPos){
    std::list<ClientPtr>::const_iterator end = route.clients.end(); end--;
    std::list<ClientPtr>::const_iterator bef;
    double duration=0;
    if(route.clients.size()==1){
        return 0;
    }else if(rmvPos!=end){
        std::list<ClientPtr>::const_iterator next = rmvPos; next++;
        double arrivalTime, totalWaitTime;
        double f1, f2;
        if(rmvPos==route.clients.begin()){
            arrivalTime = data.durationTable[next->id];
            totalWaitTime=0;
            f1 = (data.depotTimeWindow.second - data.depotTimeWindow.first);
            f2 = next->fwdSlack1 + (next->finishTime - arrivalTime - next->ptr->service);
        }else{
            bef = rmvPos; bef--;
            arrivalTime = bef->finishTime +  data.durationTable[bef->id*base+next->id];
            totalWaitTime=bef->aggWaitTime;
            f1 = bef->fwdSlack0;
            f2 = next->fwdSlack1 + bef->aggWaitTime + (next->finishTime - arrivalTime - next->ptr->service);
        }

        double push = std::max(arrivalTime, next->ptr->timeWindow.first) - next->startTime;
        if(push > next->fwdSlack1) return -1;

        double waitTime2 = next->aggWaitTime - next->aggWaitTime;
        if(push>=0){
            if(waitTime2>0){
                totalWaitTime += std::max(0.0 , waitTime2-push);
            }
        }else{
            if(waitTime2>0){
                totalWaitTime += std::max(0.0 , waitTime2-push);
            }else{
                totalWaitTime += std::max(0.0 , -push-next->bwdSlack1);
            }
        }
        double f0 = std::min(f1, f2);
        duration = computeRmvRouteService(route, rmvPos)+totalWaitTime;
        duration -= std::min(f0,totalWaitTime);
    }else{
        bef = end; --bef;
        duration=  bef->finishTime + data.durationTable[bef->id*base];
        duration -= data.depotTimeWindow.first + std::min(bef->fwdSlack0, bef->aggWaitTime);
    }
    
    return duration;
}


//=================================================================

double 
Operations::computeRmvCost(const Route& route, const std::list<ClientPtr>::iterator & rmvPos){
    double rD, rT;
    double cost;
   
    if(OBJECTIF==1){
        rD = computeRmvRouteDistance(route, rmvPos); 
        if(rD < 0) return -1;
        cost = rD;
    }else{
        rT = computeRmvRouteDuration(route, rmvPos); 
        if(rT < 0) return -1;
        cost = rT;
    }
    if(route.clients.size()==1) return cost/10.0;
    else return cost;
}

//=================================================================

bool 
Operations::remove(Route& route, std::list<ClientPtr>::iterator & rmvPos){
    std::list<ClientPtr>::iterator end = route.clients.end(); end--;
    std::list<ClientPtr>::iterator bef = route.clients.end();

    if(route.clients.size()==1){
        route.clear();
        return true;
    }else if(rmvPos!=end){
        std::list<ClientPtr>::iterator next = rmvPos; next++;
        double arrivalTime;
        if(rmvPos==route.clients.begin()){
            arrivalTime = data.durationTable[next->id];
        }else{
            bef = rmvPos; bef--;
            arrivalTime = bef->finishTime +  data.durationTable[bef->id*base+next->id];
        }

        double push = std::max(arrivalTime, next->ptr->timeWindow.first) - next->startTime;
        if(push > next->fwdSlack1 + 1e-8){std::cout<<"cannot remove, whoa "<<push<<" "<<next->fwdSlack1<<std::endl; return false;}
        push = pushRoute(push,  arrivalTime, bef, next, route.clients.end());
    }
    //std::cout<<rmvPos->id<<" ";
    route.currentCapa -= rmvPos->ptr->demand;
    route.currentDist = computeRmvRouteDistance(route, rmvPos);
    route.currentServiceTime =  computeRmvRouteService(route, rmvPos);
    route.clients.erase(rmvPos);
    fwdRecursion(route, route.clients.end());
    bwdRecursion(route, route.clients.begin());
    route.currentDuration = routeDuration(route);
    if(route.currentDist<0){ std::cout<<" new distance "<<route.currentDist<<std::endl; abort();}

    return true;
}

//=================================================================

void 
Operations::repairInSequence(Solution& sol, std::vector<ClientPtr>& clntInsert, bool ejection, bool fleetMin){
    while(!clntInsert.empty()){
        
        bool noRoute=true;  
        std::list<ClientPtr>::iterator minNeighborPos;
        ClientPtr& client = clntInsert.back(); //std::cout<<"insert "<<client.id<<" "<<client.ptr->noServe<<std::endl;
        double minCost=1e20;
        std::list<Route>::iterator bestRoute;
        std::list<Route>::iterator itr = sol.routes.begin();
        for(; itr!=sol.routes.end();++itr){
            Route& route = *itr;            
            if(route.currentCapa + client.ptr->demand > data.capacitiesByVehiculeType[0]) continue;
            
            std::list<ClientPtr>::iterator bestPos;
            double cost = bestPosition(route, client,  bestPos, fleetMin);
            if(cost>=0 && minCost>cost){
                minCost = cost;
                minNeighborPos = bestPos;
                noRoute=false;
                bestRoute = itr;
            }
        }
        if(noRoute){
            client.ptr->noServe++; 
            if(ejection && ejectionSearch(sol, client, clntInsert)){return;}
            if(fleetMin){ sol.requestBank.push_back(client); }
            else{
                openRoute( sol, client); 
                sol.accountRoute(sol.routes.back());
            }
        }else{
            sol.discountRoute(*bestRoute);
            insert(*bestRoute, &client, minNeighborPos);
            sol.accountRoute(*bestRoute);
        }
        clntInsert.pop_back();
    }
}

//=================================================================

unsigned int 
Operations::randomRoutedClient(const Solution& sol){
    size_t nbRoutes = sol.routes.size();
    std::list<Route>::const_iterator itr = sol.routes.begin();
    unsigned int rndRoute = rand()%nbRoutes;
    std::advance(itr, rndRoute);

    size_t nbRoutedClnts = itr->clients.size();
    unsigned int rndClnt = rand()%nbRoutedClnts;

    std::list<ClientPtr>::const_iterator it = itr->clients.begin();
    std::advance(it, rndClnt);
    return it->id;
}
