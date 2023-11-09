#ifndef __BUS_H__
#define __BUS_H__

#include <vector>
#include "cache.h"
#include "port.h"

class Bus : public Port<bus_transaction_e>, public Imp<bus_transaction_e> {

private:


public:
    Bus();

    virtual void snoop(bus_transaction_e transaction);
};


#endif