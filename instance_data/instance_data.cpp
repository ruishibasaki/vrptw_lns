
#include "instance_data.hpp"


#include <fstream> 
#include <sstream> 
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>

#define EPSILON 1e-4
//=================================================================
//=================================================================
//=================================================================

ClientPtr::ClientPtr(unsigned int id_,const Client* clnt):id(id_),ptr(clnt),
    startTime(0),finishTime(0), waitTime(0),
    aggWaitTime(0), fwdSlack1(0),fwdSlack0(0), bwdSlack1(0){}

//-------------------------------------------------------------
ClientPtr::ClientPtr():
    startTime(0),id(0),ptr(nullptr),finishTime(0), waitTime(0),
    aggWaitTime(0), fwdSlack1(0),fwdSlack0(0), bwdSlack1(0){}
//-------------------------------------------------------------
ClientPtr::ClientPtr(double startTime_,unsigned int id_,const Client* clnt):
    id(id_),ptr(clnt){
        startTime = std::max(startTime_, clnt->timeWindow.first);
        waitTime= aggWaitTime =(startTime - startTime_);
        finishTime = startTime + clnt->service;
        if(finishTime>clnt->timeWindow.second){
            std::cout<<"infeasible solution:  "<<id_<<" "<<finishTime<<" "<<clnt->timeWindow.second<<std::endl;
            abort();
        }
        fwdSlack0 = bwdSlack1 = fwdSlack1 = 0;
    }
//-------------------------------------------------------------
ClientPtr::ClientPtr(const ClientPtr & copy):
    startTime(copy.startTime),id(copy.id),ptr(copy.ptr), waitTime(copy.waitTime),
    finishTime(copy.finishTime), aggWaitTime(copy.aggWaitTime),
    fwdSlack1(copy.fwdSlack1), fwdSlack0(copy.fwdSlack0), bwdSlack1(copy.bwdSlack1){}

//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
bool 
ClientPtr::operator<(const ClientPtr & ptr_2 ) const{
    return (startTime < ptr_2.startTime);
}
//-------------------------------------------------------------
bool 
ClientPtr::operator==(const ClientPtr & ptr_2 ){
    return (id == ptr_2.id);
}
//-------------------------------------------------------------
bool 
ClientPtr::compareTimeWindow(const ClientPtr & ptr_1, const ClientPtr & ptr_2 ){
    double d1 = (ptr_1.ptr->timeWindow.second-ptr_1.ptr->timeWindow.first);
    double d2 = (ptr_2.ptr->timeWindow.second-ptr_2.ptr->timeWindow.first);
    if ( d1 > d2)
        return true;
    else if(d1 == d2)
        return (ptr_1.ptr->demand < ptr_2.ptr->demand);
    return false;
}

//-------------------------------------------------------------
bool 
ClientPtr::compareDemand(const ClientPtr & ptr_1, const ClientPtr & ptr_2 ){
    return (ptr_1.ptr->demand < ptr_2.ptr->demand);
}

//-------------------------------------------------------------
bool 
ClientPtr::compareInitialTime(const ClientPtr & ptr_1, const ClientPtr & ptr_2 ){
    return (ptr_1.ptr->timeWindow.first < ptr_2.ptr->timeWindow.first);
}
//-------------------------------------------------------------
bool 
ClientPtr::compareFinalTime(const ClientPtr & ptr_1, const ClientPtr & ptr_2 ){
    return (ptr_1.ptr->timeWindow.second > ptr_2.ptr->timeWindow.second);
}
//-------------------------------------------------------------
bool 
ClientPtr::compareNoServe(const ClientPtr & ptr_1, const ClientPtr & ptr_2 ){
    if(ptr_1.ptr->noServe < ptr_2.ptr->noServe) return true;
    else if(ptr_1.ptr->noServe == ptr_2.ptr->noServe){
        return compareTimeWindow( ptr_1,  ptr_2 ); 
    }else return false;
}

//-------------------------------------------------------------

bool 
ClientPtr::compareClosest(const ClientPtr & ptr_1, const ClientPtr & ptr_2 ){
    return (ptr_1.ptr->depotDist > ptr_2.ptr->depotDist);
}

//-------------------------------------------------------------

bool 
ClientPtr::compareFarest(const ClientPtr & ptr_1, const ClientPtr & ptr_2 ){
    return (ptr_1.ptr->depotDist < ptr_2.ptr->depotDist);
}
//=================================================================
//=================================================================
//=================================================================
void
Route::clear(){
    currentCapa = currentDist= 0;
    currentDuration = currentServiceTime = 0;
    fwdSlack0=fwdSlack1=0;
    clients.clear();
}

//----------------------------------------------------------------

double 
Route::cost() const{
    if(OBJECTIF==1)
        return currentDist;
    else return currentDuration;
}

//----------------------------------------------------------------

double 
Route::noServeCost(){
    double cost = 0;
    std::list<ClientPtr>::iterator it =  clients.begin();
    for ( ; it != clients.end(); it++){
        cost+=it->ptr->noServe;
    }
    return cost;
}
 

//=================================================================
//=================================================================
//=================================================================

void 
Solution::discountRoute(const Route& route){
    totalDuration -= route.currentDuration  ;
    totalDistance -= route.currentDist  ;
}

//----------------------------------------------------------------

void 
Solution::accountRoute(const Route& route){
    totalDuration += route.currentDuration  ;
    totalDistance += route.currentDist  ;
}

//----------------------------------------------------------------
double 
Solution::cost() const{
    if(OBJECTIF==1)
        return totalDistance;
    else return totalDuration;
}
//----------------------------------------------------------------
void 
Solution::clear(){
    totalDistance = totalDuration=0;
    routes.clear(); requestBank.clear();
}

//----------------------------------------------------------------

bool 
Solution::feas()const{
    return requestBank.empty();
}

//----------------------------------------------------------------
void
Solution::sendRequestBank(std::vector<ClientPtr>& clntInsert){
    while(!requestBank.empty()){
        clntInsert.push_back(requestBank.back());
        requestBank.pop_back();
    }
}

//----------------------------------------------------------------
unsigned int
Solution::noServeRequestBank() const{
    std::list<ClientPtr>::const_iterator it= requestBank.begin();
    unsigned int count=0;
    while(it!=requestBank.end()){
        count += it->ptr->noServe;
        ++it;
    }
    return count;
}

//=================================================================
//=================================================================
//=================================================================

InstanceData::InstanceData(){
    depotTimeWindow = {0,22*3600};
    depotCoordLoc = {0,0};
}
    
//----------------------------------------------------------------

void 
InstanceData::reinitiate(unsigned int nbClients,
                const std::vector<double >& capacitiesByType_){
    depotTimeWindow = {0,22*3600};
    depotCoordLoc = {0,0};
    clients.clear();
    clients.resize(nbClients);
    capacitiesByVehiculeType = capacitiesByType_;
    distanceTable.clear();
    durationTable.clear();
}



//=================================================================
//=================================================================
//=================================================================
//=================================================================


void readData( InstanceData & data, std::string in){
    std::ifstream file(in);
    assert(file.is_open());
    std::string s;
    std::istringstream ss;
    getline(file,s);
    ss.str(s);
    ss>>data.depotCoordLoc.first>>data.depotCoordLoc.second;
    ss>>data.depotNodeCoordLoc.first>>data.depotNodeCoordLoc.second>>data.depotNodeID;
    ss.clear();

    unsigned int nbVehicleTypes;
    getline(file,s);
    ss.str(s);
    ss>>nbVehicleTypes;
    ss.clear();

    getline(file,s);
    ss.str(s);
    data.capacitiesByVehiculeType.resize(nbVehicleTypes);
    for(size_t i = 0; i<nbVehicleTypes;++i)
        ss>>data.capacitiesByVehiculeType[i];
    ss.clear();

    getline(file,s);
    ss.str(s);
    unsigned int nbClients;
    ss>>nbClients;
    ss.clear();

    data.clients.resize(nbClients);
    for(size_t i = 0; i<nbClients;++i){
        getline(file,s);
        ss.str(s);
        ss>>data.clients[i].demand>>data.clients[i].service>>data.clients[i].timeWindow.first>>data.clients[i].timeWindow.second;
        ss>>data.clients[i].coordLoc.first>>data.clients[i].coordLoc.second;
        
        data.clients[i].timeWindow.first*=3600;
        data.clients[i].timeWindow.second*=3600;
        data.clients[i].service*=60;
        ss.clear();
    }

    data.durationTable.resize((nbClients+1)*(nbClients+1));
    for(size_t i = 0; i<=nbClients;++i){
        getline(file,s);
        ss.str(s);
        for(size_t j = 0; j<=nbClients;++j){
            ss>>data.durationTable[i*(nbClients+1)+j];
        }
        ss.clear();
    }
    data.maxDist=0;
    data.distanceTable.resize((nbClients+1)*(nbClients+1));
    for(size_t i = 0; i<=nbClients;++i){
        getline(file,s);
        ss.str(s);
        for(size_t j = 0; j<=nbClients;++j){
            ss>>data.distanceTable[i*(nbClients+1)+j];
            if(data.maxDist<data.distanceTable[i*(nbClients+1)+j]) 
                data.maxDist=data.distanceTable[i*(nbClients+1)+j];
        }
        ss.clear();
    }
    for(size_t i = 0; i<nbClients;++i)
        data.clients[i].depotDist = data.distanceTable[i+1];
    file.close();
    
}


//=================================================================
//=================================================================
//=================================================================
//=================================================================


void readSolomonData( InstanceData & data, std::string in, unsigned int nbClientsPcnt){
    std::ifstream file(in);
    assert(file.is_open());
    std::string s;
    std::istringstream ss;
    for(size_t i = 0; i<4;++i)
        getline(file,s);
    getline(file,s);
    ss.str(s);
    unsigned int nbVehicules;
    ss>>nbVehicules;
    data.capacitiesByVehiculeType.resize(1);
    ss>>data.capacitiesByVehiculeType[0];
    ss.clear();

    std::cout<<in<<" "<<nbClientsPcnt<<" ";
    for(size_t i = 0; i<4;++i)
        getline(file,s);
    
    unsigned int idPoint;
    double aux;
    getline(file,s);
    ss.str(s);
    
    ss>>idPoint>>data.depotCoordLoc.first>>data.depotCoordLoc.second;
    ss>>aux>>data.depotTimeWindow.first>>data.depotTimeWindow.second;
    ss.clear();

    unsigned int nbClients=0;
    while(getline(file,s) && nbClientsPcnt>nbClients ){
        ss.str(s);
        data.clients.push_back(Client());
        ss>>idPoint>>data.clients.back().coordLoc.first>>data.clients.back().coordLoc.second;
        ss>>data.clients.back().demand>>data.clients.back().timeWindow.first>>data.clients.back().timeWindow.second;
        ss>>data.clients.back().service;
        data.clients.back().timeWindow.second+=data.clients.back().service;
        ss.clear();
        ++nbClients; 
    }
    data.maxDist=0;
    data.durationTable.resize((nbClients+1)*(nbClients+1));
    data.distanceTable.resize((nbClients+1)*(nbClients+1));
    for(size_t i = 0; i<=nbClients;++i){
        for(size_t j = 0; j<=nbClients;++j){
            double p1x,p2x =0;
            double p1y,p2y =0;
            if(i==j){
                data.durationTable[i*(nbClients+1)+j]=0;
                data.distanceTable[i*(nbClients+1)+j] = 0;
                continue;
            }else if(i==0){
                p1x = data.depotCoordLoc.first;
                p1y = data.depotCoordLoc.second;
                p2x = data.clients[j-1].coordLoc.first;
                p2y = data.clients[j-1].coordLoc.second;
            }else if(j==0){
                p1x = data.clients[i-1].coordLoc.first;
                p1y = data.clients[i-1].coordLoc.second;
                p2x = data.depotCoordLoc.first;
                p2y = data.depotCoordLoc.second;
            }else{
                p1x = data.clients[i-1].coordLoc.first;
                p1y = data.clients[i-1].coordLoc.second;
                p2x = data.clients[j-1].coordLoc.first;
                p2y = data.clients[j-1].coordLoc.second;
            }
            double x_sq = pow((p2x - p1x),2.0);
            double y_sq = pow((p2y - p1y),2.0); 
            data.durationTable[i*(nbClients+1)+j]=std::sqrt(x_sq + y_sq);
            data.distanceTable[i*(nbClients+1)+j] = data.durationTable[i*(nbClients+1)+j];
            if(data.maxDist<data.distanceTable[i*(nbClients+1)+j]) 
                data.maxDist=data.distanceTable[i*(nbClients+1)+j];
        }
    }

    file.close();
    
}

//=================================================================
//=================================================================
//=================================================================


void 
solution2Tours(const InstanceData & data, const Solution & sol, std::vector<Tour> & tours){
    double base = data.clients.size()+1;
    double totalWait =0;
    std::list<Route>::const_iterator itr = sol.routes.begin();
    unsigned int i=0;
    double totatdist =0;
    double totalclients =0;
    std::vector<bool> valclients(data.clients.size(), false);
    for(; itr!=sol.routes.end();++itr){
        const Route& route = *itr;
        tours.push_back(Tour());
        Tour& tour = tours.back();
        tour.geolocs.push_back(data.depotNodeCoordLoc);
        tour.nodeIDs.push_back(data.depotNodeID);
        std::list<ClientPtr>::const_iterator it = route.clients.begin();
        std::list<ClientPtr>::const_iterator bef = it;
        //std::cout<<"============= route "<<++i<<" =============="<<std::endl;
        // std::endl<<"nb clients: "<<route.clients.size()<<std::endl;
        //std::cout<<std::endl<<"Route "<<++i<<": ";

        double push = std::min(route.fwdSlack0, route.clients.back().aggWaitTime);
        double arrival = push+data.durationTable[it->id]; totatdist+=data.distanceTable[it->id];
        double finish;
        while(1){
            tour.geolocs.push_back(it->ptr->coordNodeLoc);
            tour.nodeIDs.push_back(it->ptr->nodeID);
            double start = std::max(arrival,it->ptr->timeWindow.first);
            double wait = start - arrival;
            finish = start + it->ptr->service;
            //std::cout<<it->id<<" ";
            ++totalclients;if(valclients[it->id-1]){std::cout<<"client visited twice"<<std::endl;}else valclients[it->id-1]=true;
            // std::cout<<"client "<<it->id<<" window ["<<it->ptr->timeWindow.first<<","<<it->ptr->timeWindow.second
            // <<"] start "<<start<<" wait "<<wait<<" finish "<<finish<<" ("<<it->finishTime<<")"<<
            // " info: fwd0 "<<it->fwdSlack0<<" fwd1 "<<it->fwdSlack1<<" bwd1 "<<it->bwdSlack1<<" aggT "<<it->aggWaitTime<<std::endl;
            if((start+EPSILON)<it->ptr->timeWindow.first || finish>(it->ptr->timeWindow.second+EPSILON)){
                std::cout<<"invalide "<<std::endl;
                abort();
            } 
            totalWait += wait;
            bef = it;
            if(++it ==route.clients.end()) break;
            arrival = finish + data.durationTable[bef->id*base+it->id];
            totatdist+=data.distanceTable[bef->id*base+it->id];
        }
        totatdist+=data.distanceTable[bef->id*base];
        if(finish+data.durationTable[bef->id*base] >data.depotTimeWindow.second) abort();
        

        // std::cout<<"start "<<push<<" finish "<<finish+data.durationTable[bef->id*base]<<std::endl;
        // std::cout<<"distance "<<route.currentDist<<std::endl;
        // std::cout<<"duration "<<route.currentDuration<<" ( service "<<route.currentServiceTime<<")"<<std::endl;
        // std::cout<<"load "<<route.currentCapa<<std::endl;
        
    }
    if(totalclients<data.clients.size()){std::cout<<"not all clients"<<std::endl;}
    //std::cout<<std::endl<<" Cost "<<sol.totalDistance<<std::endl;
    std::cout<<"Solution: dist "<<sol.totalDistance<<" ("<<totatdist<<")"<<" time "<<sol.totalDuration<<" nroutes "<<sol.routes.size()<<" total wait: "<<totalWait;

}


//=================================================================
//=================================================================
//=================================================================

void 
readSolution(const InstanceData & data, std::string fsolution, Solution& sol){
    double base = data.clients.size()+1;
    
    sol.totalDuration = 0;
    sol.totalDistance = 0;

    std::ifstream file(fsolution);
    std::string s;
    std::istringstream ss;

    getline(file,s);
    ss.str(s);
    ss>>s;
    unsigned int id;
    unsigned int pred;
    double predWait = 0;
    while(s.find("Route") != std::string::npos){
        ss>>s;
        sol.routes.push_back(Route());
        Route& route = sol.routes.back();

        pred=0;
        predWait = 0;
        while (!ss.eof()) {
            ss>>s;
            if(s.empty()){break;}
            id = std::stoi( s );

            route.currentCapa += data.clients[id-1].demand;
            route.currentDist += data.distanceTable[pred*base+id];
            route.currentServiceTime += data.durationTable[pred*base+id] + data.clients[id-1].service;
            double startTime_ =  route.clients.empty() ? 0.0 : route.clients.back().finishTime;
            startTime_ += data.durationTable[pred*base+id];
            route.clients.push_back(ClientPtr(startTime_, id, &data.clients[id-1]));
            route.clients.back().aggWaitTime += predWait;
            
            if(route.currentCapa>data.capacitiesByVehiculeType[0]){
                std::cout<<"infeasible solution:  "<<std::endl;
                abort();
            }
            predWait = route.clients.back().aggWaitTime;
            pred=id;
            s.clear();
        }
        ClientPtr & finalClient = route.clients.back();
        route.currentDuration =0;
        route.currentDist += data.distanceTable[finalClient.id*base];
        route.currentServiceTime += data.durationTable[finalClient.id*base];
        ss.clear();
        getline(file,s);
        ss.str(s);
        ss>>s;
    }
    double cost;
    ss>>cost;
    std::cout<<cost<<std::endl;
    

}
