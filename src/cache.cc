#include <stdlib.h>
#include <assert.h>
#include "cache.h"
#include "factory.h"

Cache::Cache(uint id, ulong size, ulong assoc, ulong block_size, std::string protocol)
: Port<bus_transaction_t>  ()
, id_          {id}
, size_        {size}
, assoc_       {assoc}
, block_size_  {block_size}
, protocol_    {protocol}
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
         cache_[i][j] = FACTORY_CREATE(protocol);
         cache_[i][j]->invalidate();
      }
   } 
}

Cache::~Cache() {
   for(uint i = 0; i < num_sets_; i++) {
      for(uint j = 0; j < assoc_; j++) {
         delete cache_[i][j];
      }
   } 
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
void Cache::Access(ulong addr, op_e op) {

   current_cycle_++;

   op_e operation = op;
   
   /* Update performance counters */
   if(operation == op_e::PrWr) {
      num_writes_++;
   } else if (operation == op_e::PrRd) {
      num_reads_++;
   }

   CacheBlock *block = find_block(addr);

   /* Miss */
   if(block == NULL) {

      if (operation == op_e::PrWr) {
         num_write_misses_++;
         operation = op_e::PrWrMiss;
      } else if (operation == op_e::PrRd) {
         num_read_misses_++;
         operation = op_e::PrRdMiss;
      }
      block = fill_block(addr); 
   }

   update_LRU(block);

   bus_transaction_t requesting_core_trans (id_, addr);

   /* Find out whether other caches have the block */
   Port<bus_transaction_t>::request(requesting_core_trans);

   /* A state transition could result in one or more bus signals */
   requesting_core_trans.bus_signals = block->next_state(operation, requesting_core_trans.copies_exist);

   /* Post the transaction on the bus */
   Port<bus_transaction_t>::send(requesting_core_trans);
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
   block->set_seq(current_cycle_);  
}

/******************************************************************/

/* Return an invalid block as LRU, if any, otherwise return LRU block */
CacheBlock * Cache::get_LRU(ulong addr) {

   ulong victim = assoc_;
   ulong min    = current_cycle_;
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

   if (victim->is_dirty()) {
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
      
   ulong tag = calc_tag(addr);   
   victim->set_tag(tag);
   return victim;
}

/******************************************************************/

/**
 * @brief A receiving core snoops a transaction from the bus
 * 
 * @param trans BusRd/BusRdx/BusUpd
 */
void Cache::receive(const bus_transaction_t &trans) {

   CacheBlock *block = find_block(trans.addr);

   /** 
    * The block in the receiving core is already INVALID
    * and there is no change to its state.
    */
   if (block == NULL) {
      return;
   }

   for (bus_signal_e requesting_core_signal : trans.bus_signals) {

      bus_transaction_t receiving_core_trans (id_, trans.addr);
      receiving_core_trans.bus_signals = block->next_state(requesting_core_signal);

      /* A flush results in a writeback */
      if (std::find (receiving_core_trans.bus_signals.begin(), receiving_core_trans.bus_signals.end(), bus_signal_e::Flush) != receiving_core_trans.bus_signals.end()) {
         num_write_backs_++;
      }
   }
}

/******************************************************************/

/**
 * @brief A receiving core checks whether it has a copy of a requested block
 * 
 */
void Cache::respond(bus_transaction_t &trans) {

   CacheBlock *block = find_block(trans.addr);

   if (block == NULL) {
      trans.copies_exist |= false;
   } else {
      trans.copies_exist |= true;
   }
}

/******************************************************************/
