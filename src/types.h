#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>

using ulong = unsigned long;
using uchar = unsigned char;
using uint = unsigned int;

// enum protocol_e : uint8_t {
//    MSI,
//    Dragon
// };

// std::ostream &operator<< (std::ostream &os, const protocol_e &p) {
//    switch(p) {
//       case protocol_e::MSI: return os << "MSI";
//       case protocol_e::Dragon: return os << "Dragon";
//    }
//    return os;
// }

// std::string to_string(const protocol_e &p) {
//    std::stringstream ss;
//    ss << p;
//    std::string str = ss.str();
//    return str;
// }

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


#endif