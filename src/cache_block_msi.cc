#include "cache_block.h"
#include "factory.h"

/**
 * @brief Implement a state machine for a modified version
 * of the MSI protocol
 */
class CacheBlockMSI : public CacheBlock {

public:
    CacheBlockMSI()
    :CacheBlock() 
    {}

    /**
     * @brief Next state transition for a cache block on the REQUESTING core.
     * 
     * @param op: The operation (R/W) issued by the requesting core.
     * @return bus_signal_e: A bus signal that depends on the state transition.
     */
    bus_signal_e next_state(uchar op) override {
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

    /**
     * @brief Next state transition for a cache block on the RECEIVING core
     * 
     * @param signal: The bus signal snooped by the receiving core.
     * @return bus_signal_e: A bus signal that depends on the state transition.
     */
    bus_signal_e next_state(bus_signal_e signal) override {
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
                next_state = INVALID;
                bus_signal = Flush;
                break;
        }
        state_ = next_state;
        return bus_signal;
    }
};

REGISTER_CLASS("MSI", CacheBlockMSI);











