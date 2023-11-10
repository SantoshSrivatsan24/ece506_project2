#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

using ulong = unsigned long;
using uchar = unsigned char;
using uint = unsigned int;

enum state_e : uint8_t {
   INVALID  = 0,
   CLEAN    = 1,
   MODIFIED = 2,
};

enum bus_signal_e : uint8_t {
   NA, /* No bus signal*/
   BusRd,
   BusRdX,
   Flush
};

struct bus_transaction_t {
   ulong        processor_id;  /* ID of the requesting core */
   ulong        addr;
   bus_signal_e bus_signal;
};


#endif