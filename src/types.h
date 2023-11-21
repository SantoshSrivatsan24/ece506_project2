#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>

using ulong = unsigned long;
using uchar = unsigned char;
using uint = unsigned int;

enum protocol_e : uint8_t {
   MSI,
   Dragon
};

enum class op_e : char {
   PrRd = 'r',
   PrWr = 'w',
   PrRdMiss = 'm',
   PrWrMiss = 'n'
};

enum class state_e : uint8_t {
   INVALID  = 0,
   CLEAN,
   MODIFIED,
   EXCLUSIVE,
   SHARED_CLEAN,
   SHARED_MODIFIED 
};

enum class bus_signal_e : uint8_t {
   BusRd,
   BusRdX,
   BusUpd,
   Flush,
   Update,
};

std::ostream &operator<< (std::ostream &os, const protocol_e &p);
std::ostream &operator<< (std::ostream &os, const op_e &o);
std::ostream &operator<< (std::ostream &os, const state_e &s);
std::ostream &operator<< (std::ostream &os, const bus_signal_e &s);

using bus_signal_t = std::vector<bus_signal_e>;

struct bus_transaction_t {
   bus_transaction_t()
   : processor_id{0}
   , addr{0}
   , copies_exist{false}
   {}

   bus_transaction_t (ulong id_, ulong addr_)
   : processor_id (id_)
   , addr(addr_) 
   , copies_exist{false}
   {}

   ulong        processor_id;  /* ID of the requesting core */
   ulong        addr;
   bus_signal_t bus_signals;
   bool         copies_exist;
};

#define FATAL(msg) \
   do { \
      std::cout<<__FILE__<<"::"<<__LINE__<<msg<<'\n'; \
      exit(EXIT_FAILURE); \
   } while(0)

#endif