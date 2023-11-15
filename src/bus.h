#ifndef __BUS_H__
#define __BUS_H__

#include <vector>
#include "cache.h"
#include "port.h"

class Bus : public Port<bus_transaction_t>{

public:
    Bus();
    void receive(const bus_transaction_t &trans) override;
    void respond(bus_transaction_t &trans) override;
};

#endif