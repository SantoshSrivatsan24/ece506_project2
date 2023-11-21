#ifndef __CACHE_BLOCK_H__
#define __CACHE_BLOCK_H__

#include "types.h"

/**
 * @brief Abstract class. Derived classes define state transitions
 * based on the protocol that they implement.
 */
class CacheBlock {

private:
   ulong tag_{0};
   ulong seq_{0}; 

protected:
   state_e state_{state_e::INVALID};

   /* Coherence counters */
   ulong num_invalidations_{0}, num_interventions_{0}, num_busrdx_{0}, num_busupd_{0}, num_flushes_{0};

public:
   CacheBlock()                  {}
   virtual ~CacheBlock()         = default;
   ulong get_tag() const         { return tag_;}
   state_e get_state() const     { return state_;}
   ulong get_seq() const         { return seq_; }
   void set_seq(ulong seq)       { seq_ = seq; }
   void set_tag(ulong tag)       { tag_ = tag; }
   void invalidate()             { state_ = state_e::INVALID; }
   bool is_valid()               { return (state_ != state_e::INVALID);}
   virtual bool is_dirty() const = 0;
   
   ulong get_num_invalidations() const { return num_invalidations_; }
   ulong get_num_interventions() const { return num_interventions_; }
   ulong get_num_busrdx()        const { return num_busrdx_; }
   ulong get_num_busupd()        const { return num_busupd_; }
   ulong get_num_flushes()       const { return num_flushes_; }

   /**
    * For the requesting core, the next state depends on:
    * 1. The operation (PrRd/PrWr/PrRdMiss/PrWrMiss)
    * 2. Whether other caches have the block (copies_exist)
    * 3. The current state
   */
   virtual bus_signal_t next_state(op_e op, bool copies_exist) = 0;

   /**
    * For the receiving core, the next state depends on:
    * 1. The bus signal (BusRd/BusRdX/BusUpd/etc)
    * 2. The current state
   */
   virtual bus_signal_t next_state(bus_signal_e signal) = 0;
};


#endif