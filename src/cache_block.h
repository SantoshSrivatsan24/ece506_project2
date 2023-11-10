#ifndef __CACHE_BLOCK_H__
#define __CACHE_BLOCK_H__

#include "types.h"

class cacheBlock {

private:
   ulong tag_{0};
   state_e state_{INVALID};
   ulong seq_; 

public:
   cacheBlock();
   ulong get_tag();
   ulong get_state();
   ulong get_seq();
   void set_seq(ulong Seq);
   void set_tag(ulong a);
   void invalidate();
   bool is_valid();

   /**
    * For the requesting core, the next state depends on the operation (r/w)
    * and the current state
   */
   bus_signal_e next_state(uchar op);

   /**
    * For the receiving core, the next state depends on the bus signal (BusRd/BusRdX)
    * and the current state
   */
   bus_signal_e next_state(bus_signal_e signal);
};

#endif