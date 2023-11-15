
#include <iostream>
#include "cache.h"

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


void Cache::print_stats() { 

   for(uint i = 0; i < num_sets_; i++) {
      for(uint j = 0; j < assoc_; j++) {
         num_invalidations_   += cache_[i][j]->get_num_invalidations();
         num_interventions_   += cache_[i][j]->get_num_interventions();
         num_busrdx_          += cache_[i][j]->get_num_busrdx();
         num_busupd_          += cache_[i][j]->get_num_busupd();    
         num_flushes_         += cache_[i][j]->get_num_flushes();
      }
   } 

   double miss_rate = (double) (num_read_misses_ + num_write_misses_) * 100 / (num_reads_ + num_writes_);
   ulong num_memory_transactions = num_read_misses_ + num_write_misses_ + num_write_backs_;

   BANNER("Simulation results (Cache %u)", id_);

   TRACE_STATS (1, "number of reads:",                   num_reads_);
   TRACE_STATS (2, "number of read misses:",             num_read_misses_);
   TRACE_STATS (3, "number of writes:",                  num_writes_);
   TRACE_STATS (4, "number of write misses:",            num_write_misses_);
   TRACE_STATSF(5, "total miss rate:",                   miss_rate);
   TRACE_STATS (6, "number of writebacks:",              num_write_backs_);
   TRACE_STATS (7, "number of memory transactions:",     num_memory_transactions);
   if (protocol_ == "MSI") {
   TRACE_STATS (8, "number of invalidations:",           num_invalidations_);
   TRACE_STATS (9, "number of flushes:",                 num_flushes_);
   TRACE_STATS (10, "number of BusRdX:",                 num_busrdx_);
   }
   else if (protocol_ == "Dragon") {
   TRACE_STATS (8, "number of interventions:",           num_interventions_);
   TRACE_STATS (9, "number of flushes:",                 num_flushes_);
   TRACE_STATS (10, "number of Bus Transactions (BusUpd):", num_busupd_);
   }
}
