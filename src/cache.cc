#include <stdlib.h>
#include <assert.h>
#include "cache.h"
// #include "cache_block_msi.h"
#include "factory.h"

#define BANNER(s, ...) \
   do { \
      printf("============ "); \
      printf(s, ## __VA_ARGS__); \
      printf(" ============\n"); \
   } while(0)

#define TRACE_STATS(i, s, d) \
   do { \
      printf("%02d. %-40s %lu\n", i, s, d); \
   } while(0)

#define TRACE_STATSF(i, s, d) \
   do { \
      printf("%02d. %-40s %.2lf%%\n", i, s, d); \
   } while(0)

Cache::Cache(uint id, ulong size, ulong assoc, ulong block_size)
: Port<bus_transaction_t>  ()
, id_          {id}
, size_        {size}
, assoc_       {assoc}
, block_size_  {block_size}
{
   num_sets_               = size_ / (block_size_ * assoc_);
   num_blocks_             = size_ / block_size_;
   num_index_bits_         = log2(num_sets_);
   num_block_offset_bits_  = log2(block_size_);
   tag_mask_               = (1 << num_index_bits_) - 1;

   cache_.resize(num_sets_);
   for (uint i = 0; i < num_sets_; i++){
      cache_[i].resize(assoc_);
   }
   
   for(uint i = 0; i < num_sets_; i++) {
      for(uint j = 0; j < assoc_; j++) {
         // cache_[i][j] = new CacheBlock();

         cache_[i][j] = Factory::get_instance()->create("MSI");
         cache_[i][j]->invalidate();
      }
   } 
}

Cache::~Cache() {
   
}

ulong Cache::calc_tag(ulong addr) {
   return (addr >> num_block_offset_bits_);
}

ulong Cache::calc_index(ulong addr) {
   return ((addr >> num_block_offset_bits_) & tag_mask_);
}

ulong Cache::calc_addr_for_tag(ulong tag) {
   return (tag << (num_block_offset_bits_));
}

/******************************************************************/

/**
 * @brief Entry point to the memory hierarchy
 * 
 * @param addr 
 * @param op R/W
 */
void Cache::Access(ulong addr,uchar op) {
   /**
    * Per cache global counter to maintain LRU order
    * among cache ways.
    * Updated on every cache access.
    */
   current_cycle++;
         
   if(op == 'w') num_writes_++;
   else          num_reads_++;
   
   CacheBlock *block = find_block(addr);
   bus_signal_e bus_signal;

   /* Miss */
   if(block == NULL) {
      if(op == 'w')  num_write_misses_++;
      else           num_read_misses_++;

      CacheBlock *new_block = fill_block(addr);
      /* A state transition could result in a bus signal */ 
      bus_signal = new_block->next_state(op);   
   }

   /* Hit. Update LRU and dirty flags */
   else {
      update_LRU(block);
      /* A state transition could result in a bus signal */
      bus_signal = block->next_state(op);
   }

   /* Construct a bus transaction */
   bus_transaction_t trans = {id_, addr, bus_signal};

   /* Post the transaction on the bus */
   post(trans);
}

/******************************************************************/

CacheBlock* Cache::find_block(ulong addr) {

   ulong tag = calc_tag(addr);
   ulong i   = calc_index(addr);
  
   for(ulong j = 0; j < assoc_; j++){
      if(cache_[i][j]->is_valid() && cache_[i][j]->get_tag() == tag) {
         return cache_[i][j];
      }
   }
   return NULL;
}

/******************************************************************/

/* Upgrade LRU block to be MRU block */
void Cache::update_LRU(CacheBlock *block) {
   block->set_seq(current_cycle);  
}

/******************************************************************/

/* Return an invalid block as LRU, if any, otherwise return LRU block */
CacheBlock * Cache::get_LRU(ulong addr) {

   ulong victim = assoc_;
   ulong min    = current_cycle;
   ulong i      = calc_index(addr);
   
   for(ulong j = 0; j < assoc_; j++) {
      if(cache_[i][j]->is_valid() == 0) { 
         return cache_[i][j]; 
      }   
   }

   for(ulong j = 0; j < assoc_; j++) {
      if(cache_[i][j]->get_seq() <= min) { 
         victim = j; 
         min = cache_[i][j]->get_seq();}
   } 

   assert(victim != assoc_);
   
   return cache_[i][victim];
}

/******************************************************************/

/* Evict a victim block from the cache */
CacheBlock *Cache::find_block_to_replace(ulong addr) {

   CacheBlock *victim = get_LRU(addr);

   if (victim->get_state() == MODIFIED) {
      num_write_backs_++;
   }

   if (victim->is_valid()) {
      victim->invalidate();
   }

   return (victim);
}

/******************************************************************/

/* Allocate a new block */
CacheBlock *Cache::fill_block(ulong addr) { 
  
   CacheBlock *victim = find_block_to_replace(addr);
   assert(victim != 0);

   update_LRU(victim);
      
   ulong tag = calc_tag(addr);   
   victim->set_tag(tag);
   return victim;
}

/******************************************************************/

/**
 * @brief The requesting core posts a transaction onto the bus
 * Wrapper function for Port<bus_transaction_t>::send()
 * 
 * @param trans BusRd/BusRdX
 */
void Cache::post(bus_transaction_t &trans) {

   switch (trans.bus_signal) {

      case BusRd:  
         Port<bus_transaction_t>::send(trans);
         return; 

      case BusRdX: 
         num_busrdx_++;
         Port<bus_transaction_t>::send(trans);
         return;

      default:  
         return;
   }
}

/******************************************************************/

/**
 * @brief A receiving core snoops a transaction from the bus
 * 
 * @param trans BusRd/BusRdx
 */
void Cache::receive(bus_transaction_t &trans) {


   CacheBlock *block = find_block(trans.addr);

   /** 
    * The block in the receiving core is already INVALID
    * and there is no change to its state.
    */
   if (block == NULL) {
      return;
   }

   num_invalidations_++;
   bus_signal_e bus_signal = block->next_state(trans.bus_signal);

   /* If a receiving core sees a flush*/
   if (bus_signal == Flush) {
      num_flushes_++;
      num_write_backs_++; /* A flush results in a writeback */
   }
}

/******************************************************************/

void Cache::print_stats() { 

   double miss_rate = (double) (num_read_misses_ + num_write_misses_) * 100 / (num_reads_ + num_writes_);
   ulong num_memory_transactions = num_read_misses_ + num_write_misses_ + num_write_backs_;

   BANNER("Simulation results (Cache %u)", id_);

   TRACE_STATS (1, "number of reads:",                 num_reads_);
   TRACE_STATS (2, "number of read misses:",           num_read_misses_);
   TRACE_STATS (3, "number of writes:",                num_writes_);
   TRACE_STATS (4, "number of write misses:",          num_write_misses_);
   TRACE_STATSF(5, "total miss rate:",                 miss_rate);
   TRACE_STATS (6, "number of writebacks:",            num_write_backs_);
   TRACE_STATS (7, "number of memory transactions:",   num_memory_transactions);
   TRACE_STATS (8, "number of invalidations:",         num_invalidations_);
   TRACE_STATS (9, "number of flushes:",               num_flushes_);
   TRACE_STATS (10, "number of BusRdX:",               num_busrdx_);
}

/******************************************************************/
