#ifndef __BUS_H__
#define __BUS_H__

#include <vector>
#include "cache.h"
#include "port.h"

class Bus : public Port<bus_transaction_t> {

public:
    Bus();
    void receive(bus_transaction_t &trans) override;
};

#endif