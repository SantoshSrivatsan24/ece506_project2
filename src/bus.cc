#include "bus.h"

Bus::Bus()
: Port<bus_transaction_t> ()
{}

/**
 * @brief Receive a bus transaction from a requesting core 
 * and forward it to all receiving cores
 * 
 * @param trans 
 */
void Bus::receive(bus_transaction_t &trans) {

    ulong requesting_core = trans.processor_id;
    for (ulong core = 0; core < Port<bus_transaction_t>::get_num_ports(); core++) {
        /* Forward the bus transaction to all receiving cores */
        if (core != requesting_core) {
            Port<bus_transaction_t>::send(core, trans);
        }
    }
}