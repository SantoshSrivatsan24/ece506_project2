/**
 *  -   This class implements a Bus that a cache connects to.
 *  -   For the requesting core, a signal is posted onto the bus depending 
 *      on the state of the cacheline/block
 *  -   For the receiving core, there is always a bus signal unless the block is
 *      in the invalid state.
 * -    I should use enum classes to construct the state diagram for the modified MSI
 *      protocol.
 * -    I can use inheritance to connect a Cache to the Bus
 *      https://stackoverflow.com/questions/30823958/communication-between-objects-in-c
*/

#include "bus.h"

Bus::Bus()
: Port<bus_transaction_e> ()
, Imp<bus_transaction_e> ()
{}


void Bus::snoop(bus_transaction_e transaction) {
    printf ("Received bus transaction: %d!\n", transaction);

    /**
     * TODO: The bus should post every transaction that it snoops
     * It posts a transaction on it's port.
     * That port should be connected to the port implementations of every cache
    */
}