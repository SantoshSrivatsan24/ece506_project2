/*******************************************************
                          main.cc
********************************************************/

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"
#include "bus.h"

#define TRACE_CONFIG(s, d) \
   do { \
      printf("%-25s %lu\n", s, d); \
   } while(0)


int main(int argc, char *argv[]) {
    
    if(argv[1] == NULL){
         fprintf(stderr, "input format: ");
         fprintf(stderr, "./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
         exit(EXIT_FAILURE);
    }

    ulong cache_size        = atoi(argv[1]);
    ulong cache_assoc       = atoi(argv[2]);
    ulong blk_size          = atoi(argv[3]);
    ulong num_processors    = atoi(argv[4]);
    protocol_e protocol     = static_cast<protocol_e>(atoi(argv[5]));
    char *fname             = new char[20];
    fname                   = argv[6];

    FILE *trace = fopen (fname, "r");
    if(!trace) {   
        fprintf(stderr, "ERROR: Unable to open trace file %s\n", fname);
        exit(EXIT_FAILURE);
    }

    printf("===== 506 Personal information =====\n");
    printf("Name: Santosh Srivatsan\n");
    printf("UnityID: srsrivat\n");
    printf("ECE492 student? No\n");

    printf("===== 506 SMP Simulator configuration =====\n");
    TRACE_CONFIG("L1_SIZE:", cache_size);
    TRACE_CONFIG("L1_ASSOC:", cache_assoc);
    TRACE_CONFIG("L1_BLOCKSIZE:", blk_size);
    TRACE_CONFIG("NUMBER OF PROCESSORS:", num_processors);
    std::cout<<std::setw(25)<<std::left<<"COHERENCE PROTOCOL: "<< protocol<<'\n';
    printf("TRACE FILE: %s\n", fname);

    Bus *bus = new Bus();
    std::vector<Cache*> caches(num_processors);

    for(uint i = 0; i < num_processors; i++) {
        caches[i] = new Cache(i, cache_size, cache_assoc, blk_size, protocol);
        /* Two way communication between the cache and the bus */
        caches[i]->connect(bus);
        bus->connect(caches[i]);
    }

    ulong proc;
    op_e op;
    ulong addr;

    while(fscanf(trace, "%lu %c %lx", &proc, &op, &addr) != EOF) {
        caches[proc]->Access(addr, op);
    }
    fclose(trace);

    for (int i = 0; i < num_processors; i++) {
        caches[i]->print_stats();
    }

    return 0;
}
