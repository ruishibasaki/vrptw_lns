#pragma once

#include "operations.hpp"
#include <cmath>
#include <iostream> 

#define SEED 123
//=================================================================
//=================================================================
//=================================================================

class destroyNeighborhood{
public:
    unsigned int maxr;
    unsigned int minr;
    virtual void destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved)=0;
    virtual ~destroyNeighborhood(){};
};

//=================================================================

class repairNeighborhood{
public:
    virtual void repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin)=0;
    virtual ~repairNeighborhood(){};
};


//=================================================================
//=================================================================
//=================================================================

class randomRemovalNeighborhood: public destroyNeighborhood, public Operations{
 
public:
    randomRemovalNeighborhood(const InstanceData & data_, double minr_, double maxr_): Operations(data_){ 
        srand(SEED);
        minr = ceil(minr_*data.clients.size());
        maxr = ceil(maxr_*data.clients.size());
    }

    void destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved);
};

//=================================================================

class zoneRemovalNeighborhood: public destroyNeighborhood, public Operations{
 
public:
    zoneRemovalNeighborhood(const InstanceData & data_, double minr_, double maxr_): Operations(data_){ 
        srand(SEED);
        minr = ceil(minr_*data.clients.size());
        maxr = ceil(maxr_*data.clients.size());
    }

    void destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved);
};


//=================================================================

class timeRelatedRemovalNeighborhood: public destroyNeighborhood, public Operations{
 
public:
    timeRelatedRemovalNeighborhood(const InstanceData & data_, double minr_, double maxr_): Operations(data_){ 
        srand(SEED);
        minr = ceil(minr_*data.clients.size());
        maxr = ceil(maxr_*data.clients.size());
    }

    void destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved);
};

//=================================================================

class worstRemovalNeighborhood: public destroyNeighborhood, public Operations{
 
public:
    worstRemovalNeighborhood(const InstanceData & data_, double minr_, double maxr_): Operations(data_){ 
        srand(SEED);
        minr = ceil(minr_*data.clients.size());
        maxr = ceil(maxr_*data.clients.size());
    }
    void destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved);
};

//=================================================================

class historicalWorstRemovalNeighborhood: public destroyNeighborhood, public Operations{
 
public:
    historicalWorstRemovalNeighborhood(const InstanceData & data_, double minr_, double maxr_): Operations(data_){ 
        srand(SEED);
        minr = ceil(minr_*data.clients.size());
        maxr = ceil(maxr_*data.clients.size());
    }
    void destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved);
};

//=================================================================

class routeRemovalNeighborhood: public destroyNeighborhood, public Operations{
 
public:
    routeRemovalNeighborhood(const InstanceData & data_, double minr_, double maxr_): Operations(data_){ 
        srand(SEED);
        minr = ceil(minr_*data.clients.size());
        maxr = ceil(maxr_*data.clients.size());
    }
    void destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved);
};

//=================================================================

class stringRemovalNeighborhood: public destroyNeighborhood, public Operations{
 
public:
    stringRemovalNeighborhood(const InstanceData & data_, double minr_, double maxr_): Operations(data_){ 
        srand(SEED);
        minr = ceil(minr_*data.clients.size());
        maxr = ceil(maxr_*data.clients.size());
    }
    void destroy(Solution& sol, std::vector<ClientPtr>& clntRemoved);

    int backwardRemoval(size_t stringSize, regretCell& centerRmv, 
                                        Solution& sol, std::vector<ClientPtr>& clntRemoved);
    
    int forwardRemoval(size_t stringSize, regretCell& centerRmv, 
                                        Solution& sol, std::vector<ClientPtr>& clntRemoved);
    
    void split(size_t stringSize, regretCell& centerRmv,
                                        Solution& sol, std::vector<ClientPtr>& clntRemoved);
};

//=================================================================
//=================================================================
//=================================================================

class randomOrderBestPositionInsertion: public repairNeighborhood, public Operations{
 
public:
    randomOrderBestPositionInsertion(const InstanceData & data_): Operations(data_){ srand(SEED);}

    void repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin);
};

//=================================================================

class twoRegretInsertion: public repairNeighborhood, public Operations{
 
public:
    twoRegretInsertion(const InstanceData & data_): Operations(data_){ srand(SEED);}
    void inner_loop(Solution& sol, std::vector<ClientPtr>& clntInsert, bool ejection, bool fleetMin);
    void repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin);
};

//=================================================================

class sharpWindowOrderBestPositionInsertion: public repairNeighborhood, public Operations{
 
public:
    sharpWindowOrderBestPositionInsertion(const InstanceData & data_): Operations(data_){ srand(SEED);}

    void repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin);
};

//=================================================================

class demandOrderBestPositionInsertion: public repairNeighborhood, public Operations{
 
public:
    demandOrderBestPositionInsertion(const InstanceData & data_): Operations(data_){ srand(SEED);}

    void repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin);
};

//=================================================================

class closestOrderBestPositionInsertion: public repairNeighborhood, public Operations{
 
public:
    closestOrderBestPositionInsertion(const InstanceData & data_): Operations(data_){ srand(SEED);}

    void repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin);
};

//=================================================================

class farestOrderBestPositionInsertion: public repairNeighborhood, public Operations{
 
public:
    farestOrderBestPositionInsertion(const InstanceData & data_): Operations(data_){ srand(SEED);}

    void repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin);
};

//=================================================================

class noServeOrderBestPositionInsertion: public repairNeighborhood, public Operations{
 
public:
    noServeOrderBestPositionInsertion(const InstanceData & data_): Operations(data_){ srand(SEED);}

    void repair(Solution& sol, std::vector<ClientPtr>& clntInsert, bool fleetMin);
};