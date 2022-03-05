
#include "neighborhoods.hpp"

#include <cassert>
#include <algorithm>

//=================================================================
//=================================================================
//=================================================================

void 
randomRemovalNeighborhood::destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved){
    size_t nbRoutes = sol.routes.size();
    size_t nbRemove = rand()%(maxr-minr)+minr;

    while(nbRemove>0){
        size_t routeId = rand()%(nbRoutes);
        std::list<Route>::iterator itr = sol.routes.begin();
        std::advance(itr,routeId);
        Route & route = *itr;

        size_t clientRmv =  rand()%int(route.clients.size());
        std::list<ClientPtr>::iterator it = route.clients.begin();
        std::advance(it,clientRmv);
        

        if(!sol.feas() && route.clients.size()==1){--nbRemove; continue;}
        clntRemoved.push_back(*it);
        sol.discountRoute(route);
        if(!remove(route, it)){
            sol.accountRoute(route);
            clntRemoved.pop_back();
            continue;
        } 
        
        if(route.clients.empty()){
            sol.routes.erase(itr);
            --nbRoutes;  
        }else{
            sol.accountRoute(route);
        }
        --nbRemove;
    }
    
}


//=================================================================
//=================================================================
//=================================================================

void 
zoneRemovalNeighborhood::destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved){
    size_t nbRemove = rand()%(maxr-minr)+minr;
    size_t idClientRmv = randomRoutedClient( sol);
    regretCell centerRmv;
    std::vector<regretCell> clients;
    clients.reserve(base);
    
    std::list<Route>::iterator itr = sol.routes.begin();
    for(; itr!=sol.routes.end();++itr){
        std::list<ClientPtr>::iterator it = itr->clients.begin();
        for(;it!=itr->clients.end();++it){
            if(it->id == idClientRmv){
                centerRmv = {0, false, itr, it};
            }else if(data.distanceTable[idClientRmv*base+it->id]/data.maxDist < RADIUS){
                clients.push_back({-data.distanceTable[idClientRmv*base+it->id], false, itr, it});
            }
        }
    }
    std::random_shuffle(clients.begin(), clients.end()); 
    while(!clients.empty() && nbRemove>0){
        itr = clients.back().route;
        Route & route = *itr;
        std::list<ClientPtr>::iterator it = clients.back().pos;
        clients.pop_back();

        if(!sol.feas() && route.clients.size()==1){--nbRemove; continue;}
        clntRemoved.push_back(*it);
        sol.discountRoute(route);
        if(remove(route, it)){
            if(route.clients.empty()){
                sol.routes.erase(itr);
            }else{
                sol.accountRoute(route);
            }
            --nbRemove; 
        }else{
            sol.accountRoute(route);      
            clntRemoved.pop_back();  
        }
    }
    if(!sol.feas() && centerRmv.route->clients.size()==1){return;}
    clntRemoved.push_back(*centerRmv.pos);
    sol.discountRoute(*centerRmv.route);
    if(remove(*centerRmv.route, centerRmv.pos)){
        if(centerRmv.route->clients.empty()){
            sol.routes.erase(centerRmv.route);
        }else{
            sol.accountRoute(*centerRmv.route);
        }
    }else{
        sol.accountRoute(*centerRmv.route);
        clntRemoved.pop_back();
    }
}

//=================================================================
//=================================================================
//=================================================================

void 
timeRelatedRemovalNeighborhood::destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved){
    size_t nbRemove = rand()%(maxr-minr)+minr;
    size_t idClientRmv = randomRoutedClient( sol);
    regretCell centerRmv;
    std::vector<regretCell> clients;
    clients.reserve(base);
    
    std::list<Route>::iterator itr = sol.routes.begin();
    for(; itr!=sol.routes.end();++itr){
        std::list<ClientPtr>::iterator it = itr->clients.begin();
        for(;it!=itr->clients.end();++it){
            if(it->id == idClientRmv){
                centerRmv = {it->startTime, false, itr, it};
            }else if(data.distanceTable[idClientRmv*base+it->id]/data.maxDist < RADIUS){
                clients.push_back({it->startTime, false, itr, it});
            }
        }
    }
    for (size_t i = clients.size(); i--; ){
        clients[i].cost = -std::abs(clients[i].cost - centerRmv.cost);
    }
    std::sort(clients.begin(), clients.end()); 
    
    while(!clients.empty() && nbRemove>0){
        itr = clients.back().route;
        Route & route = *itr;
        std::list<ClientPtr>::iterator it = clients.back().pos;
        clients.pop_back();

        if(!sol.feas() && route.clients.size()==1){--nbRemove; continue;}
        clntRemoved.push_back(*it);
        sol.discountRoute(route);
        if(remove(route, it)){
            if(route.clients.empty()){
                sol.routes.erase(itr);
            }else{
                sol.accountRoute(route);
            }
            --nbRemove; 
        }else{
            sol.accountRoute(route);     
            clntRemoved.pop_back();   
        }
    }
    if(!sol.feas() && centerRmv.route->clients.size()==1){return;}
    clntRemoved.push_back(*centerRmv.pos);
    sol.discountRoute(*centerRmv.route);
    if(remove(*centerRmv.route, centerRmv.pos)){
        if(centerRmv.route->clients.empty()){
            sol.routes.erase(centerRmv.route);
        }else{
            sol.accountRoute(*centerRmv.route);
        }
    }else{
        sol.accountRoute(*centerRmv.route);
        clntRemoved.pop_back();
    }
}

//=================================================================
//=================================================================
//=================================================================

void
worstRemovalNeighborhood::destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved){
    size_t nbRoutes = sol.routes.size();
    size_t nbRemove = rand()%(maxr-minr)+minr;
    while(nbRemove>0){
        Route* rmvRoute;
        std::list<Route>::iterator rmvRouteId;
        std::list<ClientPtr>::iterator rmvClnt;
        double biggestDelta= 0;
        bool found =false;
        std::list<Route>::iterator itr = sol.routes.begin();
        for(; itr!=sol.routes.end();++itr){
            Route& route = *itr;
            double routeCost =  route.cost();
            if(!sol.feas() && route.clients.size()==1){ continue;}

            std::list<ClientPtr>::iterator it = route.clients.begin();
            for(;it!=route.clients.end();++it){
                double cost = computeRmvCost(route, it);
                double delta = routeCost - cost; 
                if(cost<0 || delta<0) continue;
                if(it->ptr->minCost > delta) it->ptr->minCost = delta;
                if(delta>biggestDelta){
                    biggestDelta = delta;
                    rmvClnt = it;
                    rmvRoute=&route;
                    rmvRouteId = itr;
                    found =true;
                }
            }
        }

        if(!found){--nbRemove; continue;}
        clntRemoved.push_back(*rmvClnt);
        sol.discountRoute(*rmvRoute);
        assert(remove(*rmvRoute, rmvClnt));
        
        if(rmvRoute->clients.empty()){
            sol.routes.erase(rmvRouteId);
            --nbRoutes;  
        }else{
            sol.accountRoute(*rmvRoute);
        }
        --nbRemove;
    }
}

//=================================================================
//=================================================================
//=================================================================

void
historicalWorstRemovalNeighborhood::destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved){
    size_t nbRoutes = sol.routes.size();
    size_t nbRemove = rand()%(maxr-minr)+minr;
    while(nbRemove>0){
        Route* rmvRoute;
        std::list<Route>::iterator rmvRouteId;
        std::list<ClientPtr>::iterator rmvClnt;
        double biggestDiff= 0;
        bool found =false;
        std::list<Route>::iterator itr = sol.routes.begin();
        for(; itr!=sol.routes.end();++itr){
            Route& route = *itr;
            double routeCost =  route.cost();
            if(!sol.feas() && route.clients.size()==1){ continue;}

            std::list<ClientPtr>::iterator it = route.clients.begin();
            for(;it!=route.clients.end();++it){
                double cost = computeRmvCost(route, it);
                double delta = routeCost - cost; 
                if(cost<0 || delta<0) continue;
                if(it->ptr->minCost > delta) it->ptr->minCost = delta;
                double diff = delta - it->ptr->minCost;
                if(diff>biggestDiff){
                    biggestDiff = diff;
                    rmvClnt = it;
                    rmvRoute=&route;
                    rmvRouteId = itr;
                    found =true;
                }
            }
        }
        if(!found){--nbRemove; continue;}
        clntRemoved.push_back(*rmvClnt);
        sol.discountRoute(*rmvRoute);
        assert(remove(*rmvRoute, rmvClnt));
        
        if(rmvRoute->clients.empty()){
            sol.routes.erase(rmvRouteId);
            --nbRoutes;  
        }else{
            sol.accountRoute(*rmvRoute);
        }
        --nbRemove;
    }
    
}


//=================================================================
//=================================================================
//=================================================================

void
routeRemovalNeighborhood::destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved){
    size_t nbRoutes = sol.routes.size();
    double minNoServeCost = 1e20;
    double slctNoServeCost = 1e20;
    bool selected=false;
    std::list<Route>::iterator minNoServeCostRoute;
    std::list<Route>::iterator selectedRoute;
    std::list<Route>::iterator itr = sol.routes.begin();

    for(; itr!=sol.routes.end();++itr){
        double noServeCost = itr->noServeCost(); 
        if(minNoServeCost>noServeCost){
            minNoServeCost = noServeCost;
            minNoServeCostRoute = itr;
        }
        if(slctNoServeCost>noServeCost && rand()%100 < 101){ 
            selectedRoute = itr;
            selected=true;
            slctNoServeCost = noServeCost;
        }
    }
    if(!selected)selectedRoute = minNoServeCostRoute;
    Route& rmvRoute = *selectedRoute ;
    size_t nbRemove = base; 
    if(!sol.feas()){
        if(rmvRoute.clients.size()==1)nbRemove=0;
        else{nbRemove = rand()%(rmvRoute.clients.size()-1);}
    } 

    bool frontOrEnd = rand()%2;
    while(!rmvRoute.clients.empty() && nbRemove>0){
        std::list<ClientPtr>::iterator client;
        if(frontOrEnd){
            client = rmvRoute.clients.end();--client;        
        }else{
            client = rmvRoute.clients.begin();    
        }
        clntRemoved.push_back(*client); 
        
        sol.discountRoute(rmvRoute);
        if(!remove(rmvRoute, client)){
            sol.accountRoute(rmvRoute);
            clntRemoved.pop_back();
            return;
        } 
        
        if(rmvRoute.clients.empty()){
            sol.routes.erase(selectedRoute);
            return;
        }else{
            sol.accountRoute(rmvRoute);
        }
        --nbRemove;
    }
}


//=================================================================
//=================================================================
//=================================================================

int
stringRemovalNeighborhood::forwardRemoval(size_t stringSize, regretCell& centerRmv, 
                                        Solution& sol, std::vector<ClientPtr>& clntRemoved){
    std::list<ClientPtr>::iterator bef;
    std::list<ClientPtr>::iterator aft; 
    std::list<ClientPtr>::iterator it = centerRmv.pos;
    std::list<Route>::iterator itr = centerRmv.route;
    Route & route = *itr;
    if(it==route.clients.end()) return -1;
    bef = it;
    aft = it; 
    if(bef != route.clients.begin())bef--;
    aft++;
    while(stringSize>0){
        clntRemoved.push_back(*it);
        sol.discountRoute(route);
        if(remove(route, it)){ 
            if(route.clients.empty()){
                sol.routes.erase(itr);
                return 0;
            }else{
                sol.accountRoute(route);
            }
            --stringSize;
        }else{
            sol.accountRoute(route);
            clntRemoved.pop_back();
            return -1;
        }
        if(aft==route.clients.end()){
            it = bef;
            if(bef!=route.clients.begin())bef--;
        }else{
            it = aft;
            aft++;
        }
    }
    if(it == route.clients.begin())centerRmv.pos = it;
    else if(aft==route.clients.end()) centerRmv.pos = it;
    else centerRmv.pos=bef;
    return 1;
}
//----------------------------------------------------------------

int
stringRemovalNeighborhood::backwardRemoval(size_t stringSize, regretCell& centerRmv,
                                        Solution& sol, std::vector<ClientPtr>& clntRemoved){
    std::list<ClientPtr>::iterator bef;
    std::list<ClientPtr>::iterator aft; 
    std::list<ClientPtr>::iterator it = centerRmv.pos;
    std::list<Route>::iterator itr = centerRmv.route;
    Route & route = *itr;
    if(it==route.clients.end()) return -1;
    bef = it;
    aft = it; 
    if(bef != route.clients.begin())bef--;
    aft++;
    
    while(stringSize>0){
        clntRemoved.push_back(*it);
        sol.discountRoute(route); 
        if(remove(route, it)){
            if(centerRmv.route->clients.empty()){
                sol.routes.erase(itr);
                return 0;
            }else{
                sol.accountRoute(route);
            }
            --stringSize;
        }else{
            sol.accountRoute(route);
            clntRemoved.pop_back();
            return -1;
        }
        if(aft==route.clients.begin()){
            aft++;
            it = route.clients.begin();
            bef=it;
        }else{
            it = bef;
            if(bef != route.clients.begin())bef--;
        }
    }
    if(aft == route.clients.end()) centerRmv.pos = it;
    else centerRmv.pos = aft;
    return 1;
}

//----------------------------------------------------------------

void
stringRemovalNeighborhood::split(size_t stringSize, regretCell& centerRmv,
                                        Solution& sol, std::vector<ClientPtr>& clntRemoved){
    
    std::list<Route>::iterator itr = centerRmv.route;
    Route & route = *itr;
    std::list<ClientPtr>::iterator end = route.clients.end(); end--;

    int splitSize = route.clients.size()-stringSize;
    if(splitSize<=0) return ;

    if(centerRmv.pos==end){
        size_t m = rand()%splitSize+1;
        for(;m--;) centerRmv.pos--; 
    }else if(centerRmv.pos==route.clients.begin()){
        size_t m = rand()%splitSize+1;
        for(;m--;) centerRmv.pos++;
    }else{
        unsigned int count = 0;
        std::list<ClientPtr>::iterator it = centerRmv.pos;
        for(++it;it!=route.clients.end();++it) ++count;
        if(count > (route.clients.size()-count)){
            splitSize = count-stringSize;
            if(splitSize<=0) return ;
            size_t m = rand()%splitSize+1; 
            for(;m--;) centerRmv.pos++;
        }else{
            splitSize = (route.clients.size()-count)-stringSize;
            if(splitSize<=0) return ;
            size_t m = rand()%splitSize+1;
            for(;m--;) centerRmv.pos--; 
        }
       
    }
    return ;
}
//----------------------------------------------------------------

void
stringRemovalNeighborhood::destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved){
    size_t nbRoutes = 0;
    size_t nbRemove = maxr;
    size_t idClientRmv = randomRoutedClient( sol);
    regretCell centerRmv;
    std::vector<regretCell> clients;
    clients.reserve(base);

    std::list<Route>::iterator itr = sol.routes.begin();
    for(; itr!=sol.routes.end();++itr){
        std::list<ClientPtr>::iterator it = itr->clients.begin();
        for(;it!=itr->clients.end();++it){
            if(it->id == idClientRmv){
                centerRmv = {static_cast<double>(nbRoutes), false, itr, it};
            }else if(data.distanceTable[idClientRmv*base+it->id]/data.maxDist < 0.1){
                clients.push_back({static_cast<double>(nbRoutes), false, itr, it});
            }
        }
        ++nbRoutes;
    }
    
    size_t nbStrings = rand()%(nbRoutes)+1;
    size_t targetStringSize = ceil(nbRemove/nbStrings);
    if(targetStringSize<2)targetStringSize = 2;
    size_t targetMax = (base/(2*nbRoutes));
    if(targetStringSize> targetMax)targetStringSize = targetMax;

    std::vector<bool> ruined(nbRoutes,false);
    std::random_shuffle(clients.begin(), clients.end()); 
    
    while(nbStrings>0){
        ruined[centerRmv.cost] = true;
        size_t stringSize = targetStringSize;
        if(rand()%(2)==1)stringSize+=rand()%(3);
        else{
            stringSize-=rand()%(3);
            if(stringSize<2)stringSize=targetStringSize;
        } 
        if(!sol.feas()) stringSize = std::min(stringSize, centerRmv.route->clients.size()-1);

        unsigned int stringType = rand()%(4);
        int res; 
        if(stringType==1){//foward
            res =forwardRemoval(stringSize,  centerRmv,  sol, clntRemoved);
        }else if(stringType==0){//backward
            res =backwardRemoval(stringSize,  centerRmv,   sol, clntRemoved);
        }else{
            size_t sizeForward = ceil(stringSize/2.0);
            size_t sizeBackward = floor(stringSize/2.0);
            
            res = forwardRemoval(sizeForward,  centerRmv,   sol, clntRemoved);
            if(res>0){
                if(stringType==3 && rand()%(100)>25){
                    split(stringSize, centerRmv,sol, clntRemoved);
                }
                res = backwardRemoval(sizeBackward,  centerRmv,   sol, clntRemoved);
            }
        }
        if(res<=0) return;
        --nbStrings;
        while (!clients.empty() && ruined[clients.back().cost]){
            clients.pop_back();
        }
        if(clients.empty())break;
        centerRmv = clients.back();
        clients.pop_back();
    }
}
