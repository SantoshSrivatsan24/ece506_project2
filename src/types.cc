#include "types.h"

std::ostream &operator<< (std::ostream &os, const protocol_e &p) {
    switch(p) {
        case protocol_e::MSI      : return os << "MSI";
        case protocol_e::Dragon   : return os << "Dragon";
    }
   return os;
}

std::ostream &operator<< (std::ostream &os, const op_e &o) {
    switch(o) {
        case op_e::PrRd       : return os << "PrRd";
        case op_e::PrWr       : return os << "PrWr";
        case op_e::PrRdMiss   : return os << "PrRdMiss";
        case op_e::PrWrMiss   : return os << "PrWrMiss";
    }
    return os;
}

std::ostream &operator<< (std::ostream &os, const state_e &s) {
    switch(s) {
        case state_e::INVALID           : return os << "INVALID";
        case state_e::CLEAN             : return os << "CLEAN";
        case state_e::MODIFIED          : return os << "MODIFIED";
        case state_e::EXCLUSIVE         : return os << "EXCLUSIVE";
        case state_e::SHARED_CLEAN      : return os << "SHARED_CLEAN";
        case state_e::SHARED_MODIFIED   : return os << "SHARED_MODIFIED";
   }
   return os;
}


std::ostream &operator<< (std::ostream &os, const bus_signal_e &s) {
    switch(s) {
        case bus_signal_e::BusRd    : return os << "BusRd";
        case bus_signal_e::BusRdX   : return os << "BusRdX";
        case bus_signal_e::BusUpd   : return os << "BusUpd";
        case bus_signal_e::Flush    : return os << "Flush";
        case bus_signal_e::Update   : return os << "Update";
   }
   return os;
}