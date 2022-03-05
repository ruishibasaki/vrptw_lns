#include "greedy_heuristic.hpp"
#include "lns.hpp"
#include "route_engine.hpp"


#include <iostream> 
#include <time.h>


//=================================================================
//=================================================================
//=================================================================

int main(int argc, const char *argv[]){   
    
    std::vector<Tour>  tours;
    for(size_t i=1;i<=1;++i){

        std::string instance(argv[1]);

        InstanceData data;
        readSolomonData( data, instance, 100);
        
        GreedyHeuristic constructer(data);
        LNSMetaheuristic lns(data); 

        clock_t t_u;
        t_u = clock();

        constructer.solve(lns.bestSol);
        lns.solve();

        solution2Tours(data, lns.bestSol, tours);
        std::cout<<" cputime: "<<double( clock() - t_u ) / double( CLOCKS_PER_SEC )<<std::endl;
    }
    
    return EXIT_SUCCESS;
}


//=================================================================
//=================================================================
//=================================================================
