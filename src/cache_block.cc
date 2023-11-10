#include <iostream>
#include "cache_block.h"

cacheBlock::cacheBlock(){}

ulong cacheBlock::get_tag() {
    return tag_;
}

ulong cacheBlock::get_state() {
    return state_;
}

ulong cacheBlock::get_seq() {
    return seq_;
}

void cacheBlock::set_tag(ulong tag) {
    tag_ = tag;
}

void cacheBlock::set_seq(ulong seq) {
    seq_ = seq;
}

void cacheBlock::invalidate() {
    tag_ = 0;
    state_ = INVALID;
}

bool cacheBlock::is_valid() {
    return (state_ != INVALID);
}

/******************************************************************/

/**
 * @brief Next state transition for a cache block on the REQUESTING core.
 * 
 * @param op: The operation (R/W) issued by the requesting core.
 * @return bus_signal_e: A bus signal that depends on the state transition.
 */
bus_signal_e cacheBlock::next_state(uchar op) {

   state_e next_state = INVALID;
   bus_signal_e bus_signal = NA;

   switch (state_) {
      case INVALID: 
         if (op == 'r') {
            next_state = CLEAN;
            bus_signal = BusRd;
         } else {
            next_state = MODIFIED;
            bus_signal = BusRdX;
         }
         break;
      
      case CLEAN:
         if (op == 'r') {
            next_state = CLEAN;
         } else {
            next_state = MODIFIED;
         }
         break;

      case MODIFIED:
         next_state = MODIFIED;
         break;

   }
   state_ = next_state;
   return bus_signal;
}

/******************************************************************/

/**
 * @brief Next state transition for a cache block on the RECEIVING core
 * 
 * @param signal: The bus signal snooped by the receiving core.
 * @return bus_signal_e: A bus signal that depends on the state transition.
 */
bus_signal_e cacheBlock::next_state(bus_signal_e signal) {

   state_e next_state = INVALID;
   bus_signal_e bus_signal = NA;
   
   switch(state_) {
      case INVALID:
         next_state = INVALID;
         break;

      case CLEAN:
         next_state = INVALID;
         break;

      case MODIFIED:
         /* FIXME: This shouldn't happen for the debug testcase */
         printf ("A block shouldn't be modified on the receiving core.\n");
         next_state = INVALID;
         bus_signal = Flush;
         break;
   }
   state_ = next_state;
   return bus_signal;
}

/******************************************************************/
