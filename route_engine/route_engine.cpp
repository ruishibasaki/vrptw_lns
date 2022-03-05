
#include "route_engine.hpp"


//==============================================================
//==============================================================
//==============================================================

void
RouteEngine::toursToJson(std::vector<Tour> & tours){
    if(tours.empty()) return;

    size_t nbTours = tours.size();
    for(size_t t=0;t<nbTours;++t){
        VecPoints& tourPoints = tours[t].geolocs;
        ++depots[tourPoints[0]];
        size_t sz = tourPoints.size()-1;
        for(size_t i = 1;i<=sz;++i){
            ++clients[tourPoints[i]];
        }
    }


    std::map<Coordinates, int>::iterator it;
    size_t sz, count;
    std::ofstream of("result.json");
    of<<"{\"type\":\"coordinatesCollection\", \"coordinatesDepots\":[\n";
    count=0;
    sz = depots.size();
    for (it=depots.begin(); it!=depots.end(); ++it){
        of <<"["<<it->first.first <<","<< it->first.second <<"]";
        if(++count<sz) of<<",";
        of<<"\n";
    }
    of<<"], \"coordinatesClients\":[\n";
    count=0;
    sz = clients.size();
    for (it=clients.begin(); it!=clients.end(); ++it){
        of <<"["<<it->first.first <<","<< it->first.second <<"]";
        if(++count<sz) of<<",";
        of<<"\n";
    }
    of<<"], \"tours\":[\n";
    for(size_t t=0;t<nbTours;++t){
        of<<"{\"depot\" : ["<<tours[t].geolocs[0].first<<","<<tours[t].geolocs[0].second<<"],";
        of<<" \"clients\" : [";
        sz = tours[t].geolocs.size();
        for (size_t i =1; i<sz; ++i){
            of <<"["<<tours[t].geolocs[i].first<<","<<tours[t].geolocs[i].second<<"]";
            if(i<(sz-1)) of<<",\n";
        }
        of<<"], \"depotID\" : "<<tours[t].nodeIDs[0]<<",";
        of<<" \"clientsIDs\" : [";
        for (size_t i =1; i<sz; ++i){
            of <<tours[t].nodeIDs[i];
            if(i<(sz-1)) of<<",\n";
        }
        of<<"]}";
        if(t<(nbTours-1)) of<<",\n";
    }
    of<<"]}";
    of.close();
}