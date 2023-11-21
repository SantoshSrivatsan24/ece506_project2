#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>
#include "types.h"
#include "port.h"
#include "cache_block.h"  

/**
 * The Cache extends Port<bus_transaction_t> in order to send and receive bus transactions
*/
class Cache : public Port<bus_transaction_t>{
private:
   /* Data structure to model a cache */
   using set_t = std::vector<CacheBlock*>;
   using cache_t = std::vector<set_t>;
   cache_t cache_; 

   uint id_;
   ulong current_cycle_;  

   /* Cache configuration */
   ulong size_, assoc_, block_size_, num_sets_{0}, num_index_bits_{0}, num_block_offset_bits_{0}, tag_mask_{0}, num_blocks_{0};

   std::string protocol_;

   /* Performance counters */
   ulong num_reads_{0}, num_read_misses_{0}, num_writes_{0}, num_write_misses_{0}, num_write_backs_{0};

   /* Coherence counters */
   ulong num_invalidations_{0}, num_interventions_{0}, num_busrdx_{0}, num_busupd_{0}, num_flushes_{0};

   ulong get_num_invalidations() const;
   ulong get_num_interventions() const;
   ulong get_num_busrdx() const;
   ulong get_num_busupd() const;
   ulong get_num_flushes() const;

   ulong calc_tag(ulong addr);
   ulong calc_index(ulong addr);
   ulong calc_addr_for_tag(ulong tag);

   CacheBlock *find_block_to_replace(ulong addr);
   CacheBlock *fill_block(ulong addr);
   CacheBlock *find_block(ulong addr);
   CacheBlock *get_LRU(ulong);
   void update_LRU(CacheBlock *);

   void receive(const bus_transaction_t &trans) override;
   void respond(bus_transaction_t &trans) override;
   
public:
     
    Cache(uint id, ulong size, ulong assoc, ulong block_size, protocol_e protocol);
   ~Cache();
   
   void Access(ulong addr, op_e op);
   void print_stats();
};

#endif
