/*******************************************************
                          cache.cc
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
using namespace std;

Cache::Cache(int size,int assoc, int block_size)
: Port<bus_transaction_e>  ()
, Imp<bus_transaction_e> ()
, size_ ((ulong) size)
, assoc_ ((ulong) assoc)
, block_size_ ((ulong) block_size)
, tag_mask_(0)
, num_reads_(0)
, num_read_misses_(0)
, num_writes_(0)
, num_write_misses_(0)
, num_write_backs_(0)
{
   num_sets_               = size_ / (block_size_ * assoc_);
   num_blocks_             = size_ / block_size_;
   num_index_bits_         = log2(num_sets_);
   num_block_offset_bits_  = log2(block_size_);
   tag_mask_               = (1 << num_index_bits_) - 1;
 
   
   /* Create a two dimentional cache, sized as cache[sets][assoc] */ 
   cache = new cacheLine*[num_sets_];
   for(ulong i = 0; i < num_sets_; i++) {

      cache[i] = new cacheLine[assoc];
      for(ulong j = 0; j < assoc_; j++) {
         cache[i][j].invalidate();
      }
   }      
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
void Cache::Access(ulong addr,uchar op)
{
   currentCycle++;/*per cache global counter to maintain LRU order 
                    among cache ways, updated on every cache access*/
         
   if(op == 'w') num_writes_++;
   else          num_reads_++;
   
   cacheLine * line = findLine(addr);
   /* Miss */
   if(line == NULL) {
      if(op == 'w') num_write_misses_++;
      else num_read_misses_++;

      cacheLine *newline = fillLine(addr);
      if(op == 'w') newline->set_state(MODIFIED);    
      
   }
   else {
      /* Hit. Update LRU and dirty flags */
      update_LRU(line);
      if(op == 'w') line->set_state(MODIFIED);
   }

   /**
    * Post a transaction onto the bus just to see if what I've done works
   */
   bus_transaction_e transaction = BusRd;
   Port<bus_transaction_e>::post(transaction);
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr) {

   ulong pos = assoc_;
   ulong tag = calc_tag(addr);
   ulong i   = calc_index(addr);
  
   for(ulong j = 0; j < assoc_; j++){
      cacheLine &block = cache[i][j];
      if(block.is_valid() && block.get_tag() == tag) {
         pos = j; 
         return &cache[i][j];
      }
   }
   return NULL;
}

/* Upgrade LRU line to be MRU line */
void Cache::update_LRU(cacheLine *line) {
   line->set_seq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr) {

   ulong victim = assoc_;
   ulong min    = currentCycle;
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

/* Find a victim, move it to MRU position */
cacheLine *Cache::findLineToReplace(ulong addr) {
   cacheLine * victim = getLRU(addr);
   update_LRU(victim);
  
   return (victim);
}

/* Allocate a new line */
cacheLine *Cache::fillLine(ulong addr) { 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   
   if(victim->get_state() == MODIFIED) {
      write_back(addr);
   }
      
   tag = calc_tag(addr);   
   victim->set_tag(tag);
   victim->set_state(CLEAN);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::snoop(bus_transaction_e transaction) {
   printf ("Snooping bus transaction\n");
}

void Cache::print_stats()
{ 
   printf("===== Simulation results      =====\n");
   /****print out the rest of statistics here.****/
   /****follow the ouput file format**************/
   printf("01. number of reads: %lu\n", num_reads_);
   printf("02. number of read misses: %lu\n", num_read_misses_);
   printf("03. number of writes: %lu\n", num_writes_);
   printf("04. number of write misses: %lu\n", num_write_misses_);
   printf("05. total miss rate: \n");
   printf("06. number of writebacks: %lu\n", num_write_backs_);
   printf("07. number of memory transactions: \n");
   printf("08. number of invalidations: \n");
   printf("09. number of flushes: \n");
   printf("10. number of BusRdX: \n");
}
