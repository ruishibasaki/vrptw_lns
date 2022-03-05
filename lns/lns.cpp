
#include "lns.hpp"

#include <iomanip>


//=================================================================
//=================================================================
//=================================================================
  
LNSMetaheuristic::LNSMetaheuristic(const InstanceData & data_): data(data_){
    repairers.reserve(8);
    destroyers.reserve(8);
    
    repairers.push_back(new demandOrderBestPositionInsertion(data_));
    repairers.push_back(new sharpWindowOrderBestPositionInsertion(data_));
    repairers.push_back(new noServeOrderBestPositionInsertion(data_));
    repairers.push_back(new randomOrderBestPositionInsertion(data_));
    repairers.push_back(new twoRegretInsertion(data_)); 
    

    destroyers.push_back(new routeRemovalNeighborhood(data_, 0.05, 0.1));
    destroyers.push_back(new stringRemovalNeighborhood(data_, 0.05, 0.1));
    destroyers.push_back(new timeRelatedRemovalNeighborhood(data_, 0.05, 0.1));
    destroyers.push_back(new randomRemovalNeighborhood(data_, 0.05, 0.1));
    destroyers.push_back(new zoneRemovalNeighborhood(data_, 0.05, 0.1));
    destroyers.push_back(new worstRemovalNeighborhood(data_, 0.05, 0.1));

    nbDestroyers = destroyers.size();
    nbRepaires = repairers.size();
}

//----------------------------------------------------------------

LNSMetaheuristic::~LNSMetaheuristic(){
    for (size_t i = repairers.size(); i--;){
        if(repairers[i]) delete repairers[i];
    }
    for (size_t i = destroyers.size(); i--;){
        if(destroyers[i]) delete destroyers[i];
    }
}

//=================================================================
//=================================================================
//=================================================================

void 
LNSMetaheuristic::solve(){
    std::vector<ClientPtr> clntRemoved;
    Solution trial;
    bool accepted = false;
    bool large = false;
    unsigned int iters=0;
    unsigned int bestIter=0;
    unsigned int noimprov=0;
    unsigned int largeIters=0;
    double rate=0.01;
    double dev;
    fleet_min();
    sol = bestSol;
    trial = sol;
    
    changeRemoveInterval(0.05, 0.1);
    while(++iters <= 500000){
        
        size_t destroyerId =  getDestroyerId(large);
        size_t repairerId = getRepairerId(false, large);
        
        destroyers[destroyerId]->destroy(trial, clntRemoved);
        if(clntRemoved.empty()){
            --iters;
            continue;
        } 
        repairers[repairerId]->repair(trial, clntRemoved, false);
        if(compareSolutions(trial, bestSol)==1){
            bestSol = trial;
            accepted = true;
            bestIter = iters; 
            // std::cout<<std::setprecision(10)<<"iters: "<<iters<<" ("<<destroyerId<<","<<repairerId<<")"
            // " nbRoute: "<<bestSol.routes.size()<<
            // " better cost dist "<<bestSol.totalDistance<<" duration "<<bestSol.totalDuration<<std::endl;
        }else{
            accepted = false; 
        } 

        if(accepted || worseAcceptance(trial, bestSol, rate, dev)  ){
            sol = trial;
        }else{   
            trial = sol;
        }
       
        if(accepted)noimprov=0;
        else if(dev>0.01) ++noimprov;
        if(noimprov>1000 && !large){
            changeRemoveInterval(0.15, 0.3);
            large=true;
            rate=0.05; 
        }else if(large){
            ++largeIters;
            if(largeIters>500 || accepted){
                large=false;
                largeIters=0;
                noimprov=0;
                changeRemoveInterval(0.05, 0.1);
                rate=0.01;
            }
        }
    }
    

} 

//=================================================================

bool 
LNSMetaheuristic::local(Solution & trial){
    std::vector<ClientPtr> clntRemoved;
    unsigned int iters=0; 
    changeRemoveInterval(0.01, 0.011);
    while(++iters <= 1000){
        
        trial.sendRequestBank(clntRemoved);
        repairers[getRepairerId(true,0)]->repair(trial, clntRemoved, true);    
    
        if(compareSolutions(trial, bestSol)==1){
            bestSol = trial;
            sol = trial;
            changeRemoveInterval(0.05, 0.1); 
            return true;
        }else trial = sol;
    }
    changeRemoveInterval(0.05, 0.1);
    return false;
} 

//=================================================================

void 
LNSMetaheuristic::fleet_min(){
    std::vector<ClientPtr> clntRemoved;
    Operations op(data);
    Solution trial;
    bool accepted = false;
    bool large = false;
    unsigned int iters=0;
    unsigned int largeIters=0;
    double rate=0.05;
    sol = bestSol;
    changeRemoveInterval(0.1, 0.3);
    
    destroyers[0]->destroy(sol, clntRemoved);
    repairers[1]->repair(sol, clntRemoved, true);
    trial = sol;

    while(++iters <= 200000){

        if(!trial.feas() && trial.requestBank.size()<=1){
            std::vector<ClientPtr> clntRemoved2;
            trial.sendRequestBank(clntRemoved2);
            op.ejectionSearch(trial, clntRemoved2.back(), clntRemoved2);
            trial.requestBank.insert(trial.requestBank.begin(), clntRemoved2.begin(), clntRemoved2.end());
        }
        destroyers[1]->destroy(trial, clntRemoved);
        trial.sendRequestBank(clntRemoved);
        repairers[rand()%2+1]->repair(trial, clntRemoved, true);    
        
        
    
        if(infeasAcceptance(trial, sol)==1){
            sol = trial;
        }else{
            trial = sol;
        }

        if(sol.feas()){
            largeIters=iters;
            bestSol = sol;
            destroyers[0]->destroy(sol, clntRemoved);
            repairers[1]->repair(sol, clntRemoved, true);
            trial = sol;
        }
    }
} 

//=================================================================
//=================================================================
//=================================================================

int 
LNSMetaheuristic::compareSolutions(const Solution&  sol1, const Solution&  sol2){
    if(!sol1.feas() && sol2.feas() ) return 2;
    else if(sol1.feas() && !sol2.feas() ) return 1;
    if(sol1.routes.size() > sol2.routes.size()) return 2;
    else if(sol1.routes.size() < sol2.routes.size()) return 1;
    else{
        double cost1 = sol1.cost();
        double cost2 = sol2.cost();
        if(cost1 > cost2) return 2;
        else if(cost1 < cost2) return 1;
        else return 0;
    }
}

//=================================================================

bool 
LNSMetaheuristic::worseAcceptance(const Solution&  sol1, const Solution&  best, double rate, double & dev){
    if(!sol1.feas()) return false;
    if(sol1.routes.size() > best.routes.size()) return false;
    double cost1 = sol1.cost();
    double costBest = best.cost();
    dev =(cost1 - costBest)/costBest;
    return (dev < rate);
}

//=================================================================

bool 
LNSMetaheuristic::infeasAcceptance(const Solution&  sol1, const Solution&  best){
    
    if(sol1.routes.size() > best.routes.size()){
        return false;
    }
    if(sol1.requestBank.size() < best.requestBank.size()){
        return true;
    }else if(sol1.noServeRequestBank() < best.noServeRequestBank()){
        return true;
    }
    else return false;
    
}

//=================================================================
//=================================================================
//=================================================================

void 
LNSMetaheuristic::changeRemoveInterval(double minr_, double maxr_){
    for (size_t i = destroyers.size(); i--;){
        destroyers[i]->minr = ceil(minr_*data.clients.size());
        destroyers[i]->maxr = ceil(maxr_*data.clients.size());
    }
}

//=================================================================
//=================================================================
//=================================================================

size_t 
LNSMetaheuristic::getDestroyerId(bool param){
    return rand()%(nbDestroyers-1)+1 ;
}

//=================================================================

size_t 
LNSMetaheuristic::getRepairerId(bool fleetMin, bool param){
    unsigned int repairerId = rand()%(100); //rand()%(5) ;
    if(repairerId<10){
        return 3;
    if(repairerId<50){
        return rand()%(2); 
    }
    }if(repairerId<100){
        return 4;
    }
}

//=================================================================
//=================================================================
//=================================================================
