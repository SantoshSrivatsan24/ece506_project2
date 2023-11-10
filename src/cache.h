#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>
#include "types.h"
#include "port.h"
#include "cache_block.h"  

/**
 * The Cache extends Port in order to send and receive
 * bus transactions
*/
class Cache : public Port<bus_transaction_t> {
private:
   /* Data structure to model a cache */
   cacheBlock **cache;
   ulong id_;
   ulong current_cycle;  

   /* Cache configuration */
   ulong size_, assoc_, block_size_, num_sets_{0}, num_index_bits_{0}, num_block_offset_bits_{0}, tag_mask_{0}, num_blocks_{0};

   /* Performance counters */
   ulong num_reads_{0}, num_read_misses_{0}, num_writes_{0}, num_write_misses_{0}, num_write_backs_{0};

   /* Coherence counters */
   ulong num_invalidations_{0}, num_busrdx_{0}, num_flushes_{0}; 


   ulong calc_tag(ulong addr);
   ulong calc_index(ulong addr);
   ulong calc_addr_for_tag(ulong tag);

   cacheBlock *find_block_to_replace(ulong addr);
   cacheBlock *fill_block(ulong addr);
   cacheBlock *find_block(ulong addr);
   cacheBlock *get_LRU(ulong);
   void update_LRU(cacheBlock *);

   void post(bus_transaction_t &trans);
   void receive(bus_transaction_t &trans) override;
   
public:
     
    Cache(ulong id, ulong size, ulong assoc, ulong block_size);
   ~Cache();
   
   void Access(ulong,uchar);
   void print_stats();
};

#endif
