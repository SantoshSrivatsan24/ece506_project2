#include <assert.h>
#include "cache_block.h"
#include "factory.h"

/**
 * @brief Implement a state machine for the Dragon protocol
 */
class CacheBlockDragon : public CacheBlock {

public:
    CacheBlockDragon()
    :CacheBlock() 
    {}

    ~CacheBlockDragon(){}

    bool is_dirty() const override {
        return (state_ == state_e::MODIFIED || state_ == state_e::SHARED_MODIFIED);
    }

    /**
     * @brief Next state transition for a cache block on the REQUESTING core.
     * 
     * @param op: The operation (R/W) issued by the requesting core.
     * @param copies_exist: Whether other caches have a copy of the block.
     * @return bus_signal_e: A bus signal that results from the state transition.
     */
    bus_signal_t next_state(op_e op, bool copies_exist) override {
        state_e next_state = state_e::INVALID;
        bus_signal_t bus_signals;

        switch(state_) {

            case state_e::INVALID:
                if (op == op_e::PrRdMiss && !copies_exist) {
                    next_state = state_e::EXCLUSIVE;
                    bus_signals = {bus_signal_e::BusRd};
                } 
                else if (op == op_e::PrRdMiss && copies_exist) {
                    next_state = state_e::SHARED_CLEAN;
                    bus_signals = {bus_signal_e::BusRd};
                }
                else if (op == op_e::PrWrMiss && !copies_exist) {
                    next_state = state_e::MODIFIED;
                    bus_signals = {bus_signal_e::BusRd};
                }
                else if (op == op_e::PrWrMiss && copies_exist) {
                    next_state = state_e::SHARED_MODIFIED;
                    bus_signals = {bus_signal_e::BusRd, bus_signal_e::BusUpd};
                    num_busupd_++;
                }
                else {
                    FATAL("Encountered invalid operation " << op);
                }
                break;

            case state_e::MODIFIED:
                assert(!copies_exist);
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

            case state_e::EXCLUSIVE:
                assert(!copies_exist);
                if (op == op_e::PrRd) {
                    next_state = state_e::EXCLUSIVE;
                }
                else if (op == op_e::PrWr) {
                    next_state = state_e::MODIFIED;
                } 
                else {
                    FATAL("Encountered invalid operation " << op);
                }
                break;

            case state_e::SHARED_CLEAN:
                if (op == op_e::PrRd) {
                    next_state = state_e::SHARED_CLEAN;
                }
                else if (op == op_e::PrWr && !copies_exist) {
                    next_state = state_e::MODIFIED;
                    bus_signals = {bus_signal_e::BusUpd};
                    num_busupd_++;
                }
                else if (op == op_e::PrWr && copies_exist) {
                    next_state = state_e::SHARED_MODIFIED;
                    bus_signals = {bus_signal_e::BusUpd};
                    num_busupd_++;
                }
                else {
                    FATAL("Encountered invalid operation " << op);
                }
                break;

            case state_e::SHARED_MODIFIED:
                if (op == op_e::PrRd) {
                    next_state = state_e::SHARED_MODIFIED;
                }
                else if (op == op_e::PrWr && !copies_exist) {
                    next_state = state_e::MODIFIED;
                    bus_signals = {bus_signal_e::BusUpd};
                    num_busupd_++;
                }
                else if (op == op_e::PrWr && copies_exist) {
                    next_state = state_e::SHARED_MODIFIED;
                    bus_signals = {bus_signal_e::BusUpd};
                    num_busupd_++;
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
            case state_e::MODIFIED:
                switch(signal) {
                    case bus_signal_e::BusRd    : next_state = state_e::SHARED_MODIFIED;
                                                  bus_signals = {bus_signal_e::Flush};
                                                  num_interventions_++;
                                                  num_flushes_++;
                                                  break;

                    default                     : FATAL("Encountered invalid signal " << signal);
                }
                break;

            
            case state_e::EXCLUSIVE:
                switch(signal) {
                    case bus_signal_e::BusRd    : next_state = state_e::SHARED_CLEAN;
                                                  num_interventions_++;
                                                  break;

                    default                     : FATAL("Encountered invalid signal " << signal);
                }
                break;


            case state_e::SHARED_CLEAN:
                switch(signal) {
                    case bus_signal_e::BusRd    : next_state = state_e::SHARED_CLEAN; 
                                                  break;

                    case bus_signal_e::BusUpd   : next_state = state_e::SHARED_CLEAN; 
                                                  bus_signals = {bus_signal_e::Update}; 
                                                  break;

                    case bus_signal_e::Flush    : next_state = state_e::SHARED_CLEAN; 
                                                  break;

                    case bus_signal_e::Update   : next_state = state_e::SHARED_CLEAN; 
                                                  break;

                    default                     :  FATAL("Encountered invalid signal " << signal);;
                }
                break;


            case state_e::SHARED_MODIFIED:
                switch(signal) {
                    case bus_signal_e::BusRd    : next_state = state_e::SHARED_MODIFIED; 
                                                  bus_signals = {bus_signal_e::Flush};
                                                  num_flushes_++;
                                                  break;

                    case bus_signal_e::BusUpd   : next_state = state_e::SHARED_CLEAN;
                                                  bus_signals = {bus_signal_e::Update};
                                                  break;

                    case bus_signal_e::Flush    : next_state = state_e::SHARED_CLEAN;
                                                  break;

                    case bus_signal_e::Update   : next_state = state_e::SHARED_CLEAN;
                                                  break;

                    default                     :  FATAL("Encountered invalid signal " << signal);
                }
                break;


            default:
                FATAL ("Encountered unknown state " << state_ << " for the Dragon protocol.");
        }
        state_ = next_state;
        return bus_signals;
    }
};

FACTORY_REGISTER("Dragon", CacheBlockDragon);
