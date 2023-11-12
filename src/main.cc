/*******************************************************
                          main.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"
#include "bus.h"

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
    char *protocol          = new char[20];
    protocol                = argv[5];
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
    printf("ECE 492 student? No\n");

    printf("===== Simulator configuration =====\n");
    printf("L1_SIZE: %lu\n", cache_size);
    printf("L1_ASSOC: %lu\n", cache_assoc);
    printf("L1_BLOCKSIZE: %lu\n", blk_size);
    printf("NUMBER OF PROCESSORS: %lu\n", num_processors);
    printf("COHERENCE PROTOCOL: %s\n", protocol); /* FIXME: */
    printf("TRACE FILE: %s\n", fname);

    Bus *bus = new Bus();
    std::vector<Cache*> caches(num_processors);

    for(uint i = 0; i < num_processors; i++) {
        caches[i] = new Cache(i, cache_size, cache_assoc, blk_size);
        /* Two way communication between the cache and the bus */
        caches[i]->connect(bus);
        bus->connect(caches[i]);
    }

    ulong proc;
    char op;
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
