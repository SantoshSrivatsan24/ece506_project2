#include <stdlib.h>
#include <assert.h>
#include "cache.h"


Cache::Cache(ulong id, ulong size, ulong assoc, ulong block_size)
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
   
   /* Create a two dimensional cache, sized as cache[sets][assoc] */ 
   cache = new cacheBlock*[num_sets_];
   for(ulong i = 0; i < num_sets_; i++) {

      cache[i] = new cacheBlock[assoc];
      for(ulong j = 0; j < assoc_; j++) {
         cache[i][j].invalidate();
      }
   }      
}

Cache::~Cache() {
   delete[] cache;
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
   
   cacheBlock * block = find_block(addr);
   bus_signal_e bus_signal;

   /* Miss */
   if(block == NULL) {
      if(op == 'w')  num_write_misses_++;
      else           num_read_misses_++;

      cacheBlock *new_block = fill_block(addr);
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

cacheBlock * Cache::find_block(ulong addr) {

   ulong tag = calc_tag(addr);
   ulong i   = calc_index(addr);
  
   for(ulong j = 0; j < assoc_; j++){
      cacheBlock &block = cache[i][j];
      if(block.is_valid() && block.get_tag() == tag) {
         return &cache[i][j];
      }
   }
   return NULL;
}

/******************************************************************/

/* Upgrade LRU block to be MRU block */
void Cache::update_LRU(cacheBlock *block) {
   block->set_seq(current_cycle);  
}

/******************************************************************/

/* Return an invalid block as LRU, if any, otherwise return LRU block */
cacheBlock * Cache::get_LRU(ulong addr) {

   ulong victim = assoc_;
   ulong min    = current_cycle;
   ulong i      = calc_index(addr);
   
   for(ulong j = 0; j < assoc_; j++) {
      if(cache[i][j].is_valid() == 0) { 
         return &(cache[i][j]); 
      }   
   }

   for(ulong j = 0; j < assoc_; j++) {
      if(cache[i][j].get_seq() <= min) { 
         victim = j; 
         min = cache[i][j].get_seq();}
   } 

   assert(victim != assoc_);
   
   return &(cache[i][victim]);
}

/******************************************************************/

/* Find a victim, move it to MRU position */
cacheBlock *Cache::find_block_to_replace(ulong addr) {

   cacheBlock * victim = get_LRU(addr);
   update_LRU(victim);
   return (victim);
}

/******************************************************************/

/* Allocate a new block */
cacheBlock *Cache::fill_block(ulong addr) { 
   ulong tag;
  
   cacheBlock *victim = find_block_to_replace(addr);
   assert(victim != 0);
   
   if(victim->get_state() == MODIFIED) {
      num_write_backs_++;
   }
      
   tag = calc_tag(addr);   
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


   cacheBlock *block = find_block(trans.addr);

   /** 
    * The block in the receiving core is already INVALID
    * and there is no change to its state.
    */
   if (block == NULL) {
      return;
   }

   num_invalidations_++;
   bus_signal_e bus_signal = block->next_state(trans.bus_signal);

   if (bus_signal == Flush) num_flushes_++;
}

/******************************************************************/

void Cache::print_stats() { 
   printf("===== Simulation results      =====\n");
   printf("01. number of reads: %lu\n",            num_reads_);
   printf("02. number of read misses: %lu\n",      num_read_misses_);
   printf("03. number of writes: %lu\n",           num_writes_);
   printf("04. number of write misses: %lu\n",     num_write_misses_);
   printf("05. total miss rate: \n");
   printf("06. number of writebacks: %lu\n",       num_write_backs_);
   printf("07. number of memory transactions: \n");
   printf("08. number of invalidations: %lu\n",    num_invalidations_);
   printf("09. number of flushes: %lu\n",          num_flushes_);
   printf("10. number of BusRdX: %lu\n",           num_busrdx_);
}

/******************************************************************/
