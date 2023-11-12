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
   state_e state_{INVALID};

public:
   CacheBlock()               {}
   ulong get_tag() const      { return tag_;}
   uint  get_state() const    { return state_;}
   ulong get_seq() const      { return seq_; }
   void set_seq(ulong seq)    { seq_ = seq; }
   void set_tag(ulong tag)    { tag_ = tag; }
   void invalidate()          { state_ = INVALID;}
   bool is_valid()            { return (state_ != INVALID);}

   /**
    * For the requesting core, the next state depends on the operation (r/w)
    * and the current state
   */
   virtual bus_signal_e next_state(uchar op) = 0;

   /**
    * For the receiving core, the next state depends on the bus signal (BusRd/BusRdX/BusUpd/etc)
    * and the current state
   */
   virtual bus_signal_e next_state(bus_signal_e signal) = 0;
};


#endif