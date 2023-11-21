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

    ~CacheBlockMSI(){}

    bool is_dirty() const override {
        return (state_ == state_e::MODIFIED);
    }

    /**
     * @brief Next state transition for a cache block on the REQUESTING core.
     * 
     * @param op: The operation (R/W) issued by the requesting core.
     * @param copies_exist: Not used for the MSI protocol.
     * @return bus_signal_e: A bus signal that results from the state transition.
     */
    bus_signal_t next_state(op_e op, bool copies_exist) override {
        state_e next_state = state_e::INVALID;
        bus_signal_t bus_signals;

        switch (state_) {
            case state_e::INVALID: 
                if (op == op_e::PrRdMiss) {
                    next_state = state_e::CLEAN;
                    bus_signals = {bus_signal_e::BusRd};
                } 
                else if (op == op_e::PrWrMiss) {
                    next_state = state_e::MODIFIED;
                    bus_signals = {bus_signal_e::BusRdX};
                    num_busrdx_++;
                }
                else {
                    FATAL("Encountered invalid operation " << op);
                }
                break;
            
            case state_e::CLEAN:
                if (op == op_e::PrRd) {
                    next_state = state_e::CLEAN;
                } 
                else if (op == op_e::PrWr) {
                    next_state = state_e::MODIFIED;
                }
                else {
                    FATAL("Encountered invalid operation " << op);
                }
                break;

            case state_e::MODIFIED:
                if (op == op_e::PrRd) {
                    next_state = state_e::MODIFIED;
                }
                else if (op == op_e::PrWr) {
                    next_state = state_e::MODIFIED;
                }
                else {
                    FATAL("Encountered invalid operation " << op);  
                }
                break;

            default:
                FATAL("Encountered unknown state for the " << state_ << " protocol.");

        }
        state_ = next_state;
        return bus_signals;
    }

    /**
     * @brief Next state transition for a cache block on the RECEIVING core
     * 
     * @param signal: The bus signal snooped by the receiving core.
     * @return bus_signal_e: A bus signal that results from the state transition.
     */
    bus_signal_t next_state(bus_signal_e signal) override {
        state_e next_state = state_e::INVALID;
        bus_signal_t bus_signals;

        switch(state_) {

            case state_e::CLEAN:
                next_state = state_e::INVALID;
                num_invalidations_++;
                break;

            case state_e::MODIFIED:
                next_state = state_e::INVALID;
                bus_signals = {bus_signal_e::Flush};
                num_invalidations_++;
                num_flushes_++;
                break;

            default:
                FATAL("Encountered unknown state for the " << state_ << " protocol.");
        }
        state_ = next_state;
        return bus_signals;
    }
};

FACTORY_REGISTER("MSI", CacheBlockMSI);











