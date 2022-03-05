#pragma once
#include "instance_data.hpp"

#define RADIUS 0.3
#define LOOP_LIMIT 10

//=================================================================
//=================================================================
//=================================================================

struct positionCost{
    double cost;
    std::list<ClientPtr>::iterator  pos;

    bool operator<(const positionCost & ptr_2 ) const{
        return cost < ptr_2.cost;
    }
};

//=================================================================

struct regretCell{
    double cost;
    bool empty;
    std::list<Route>::iterator route;
    std::list<ClientPtr>::iterator  pos;

    bool operator<(const regretCell & ptr_2 ) const{
        return cost < ptr_2.cost;
    }
};

//=================================================================

struct ejectionCell{
    double cost;
    std::list<Route>::iterator route;
    std::list<ClientPtr>::iterator  begin;
    std::list<ClientPtr>::iterator  end;
    ejectionCell(){cost=1e20;}

    bool operator<(const ejectionCell & ptr_2 ) const {
        return cost < ptr_2.cost;
    }
};

//=================================================================
//=================================================================
//=================================================================

class Operations{
public:

    const InstanceData & data;
    double base=0;

    Operations(const InstanceData & data_): data(data_){ base =  data.clients.size()+1;}
    virtual ~Operations(){}


    Route& openRoute(Solution& sol, ClientPtr& firstClient);
    double pushRoute(double push, double arrivalTime,
                    std::list<ClientPtr>::iterator& bef,
                    std::list<ClientPtr>::iterator& next, 
                    const std::list<ClientPtr>::iterator& end);
    
    double fwdRecursion(Route& route, const std::list<ClientPtr>::iterator & pos);
    double bwdRecursion(Route& route, const std::list<ClientPtr>::iterator & pos);
    double concatenation(const Route& route, const ClientPtr& newClient,const std::list<ClientPtr>::iterator & pos);

    double routeDuration(const Route& route);
    double computeNewRouteService(const Route& route, const ClientPtr& newClient,const std::list<ClientPtr>::iterator & pos);
    double computeNewRouteDistance(const Route& route,  ClientPtr& newClient,const std::list<ClientPtr>::iterator & pos);
    double computeNewRouteDuration(const Route& route, ClientPtr& newClient,const std::list<ClientPtr>::iterator & pos);
    double computeNewCost(const Route& route,  ClientPtr& newClient,const std::list<ClientPtr>::iterator & pos);
    double bestPosition(Route& route,  ClientPtr& newClient, std::list<ClientPtr>::iterator & bestPos, bool fleetMin);
    int bestPositions(Route& route,  ClientPtr& newClient, std::vector<positionCost> & bestPos);
    bool insert(Route& route, ClientPtr* newClient,  std::list<ClientPtr>::iterator & pos);

    double computeRmvRouteService(const Route& route, const std::list<ClientPtr>::iterator & rmvPos);
    double computeRmvRouteDistance(const Route& route, const std::list<ClientPtr>::iterator & rmvPos);
    double computeRmvRouteDuration(const Route& route, const std::list<ClientPtr>::iterator & rmvPos);
    double computeRmvCost(const Route& route, const std::list<ClientPtr>::iterator & rmvPos);

    bool remove(Route& route, std::list<ClientPtr>::iterator & pos);

    void repairInSequence(Solution& sol, std::vector<ClientPtr>& clntInsert, bool ejection, bool fleetMin);

    unsigned int randomRoutedClient(const Solution& sol);
    bool ejectionSearch(Solution& sol, ClientPtr newClient, std::vector<ClientPtr>& clntInsert);
    bool checkEjection(Route& route, ClientPtr& newClient,  std::list<ClientPtr>::iterator & beginRmv, 
                        std::list<ClientPtr>::iterator & endRmv);

};



//=================================================================
//=================================================================
//=================================================================
