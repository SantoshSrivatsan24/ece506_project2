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
                    fprintf(stderr, "%s::%d: Encountered invalid operation '%c'.\n", __FILE__, __LINE__ , (char)op);
                    exit (EXIT_FAILURE);
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
                    fprintf(stderr, "%s::%d: Encountered invalid operation '%c'.\n", __FILE__, __LINE__ , (char)op);
                    exit (EXIT_FAILURE);
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
                    fprintf(stderr, "%s::%d: Encountered invalid operation '%c'.\n", __FILE__, __LINE__ , (char)op);
                    exit (EXIT_FAILURE); 
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
                    fprintf(stderr, "%s::%d: Encountered invalid operation '%c'.\n", __FILE__, __LINE__ , (char)op);
                    exit (EXIT_FAILURE);  
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
                    fprintf(stderr, "%s::%d: Encountered invalid operation '%c'.\n", __FILE__, __LINE__ , (char)op);
                    exit (EXIT_FAILURE);  
                }
                break;

            default: 
                fprintf (stderr, "ERROR:: Encountered unknown state %hhu for the Dragon protocol.\n", state_);
                exit (EXIT_FAILURE);
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
                if (signal == bus_signal_e::BusRd) {
                    next_state = state_e::SHARED_MODIFIED;
                    bus_signals = {bus_signal_e::Flush};
                    num_interventions_++;
                    num_flushes_++;
                }
                else {
                    /* The requesting core doesn't have the block to issue a BusUpd */
                    fprintf (stderr, "%s::%d:: Encountered invalid signal %hhu.\n", __FILE__, __LINE__, signal);
                    exit(EXIT_FAILURE);
                }
                break;

            case state_e::EXCLUSIVE:
                if (signal == bus_signal_e::BusRd) {
                    next_state = state_e::SHARED_CLEAN;
                    num_interventions_++;
                }
                else {
                     /* The requesting core doesn't have the block to issue a BusUpd */
                    fprintf (stderr, "%s::%d:: Encountered invalid signal %hhu.\n", __FILE__, __LINE__, signal);
                    exit(EXIT_FAILURE);
                }
                break;

            case state_e::SHARED_CLEAN:
                if (signal == bus_signal_e::BusRd) {
                    next_state = state_e::SHARED_CLEAN;
                }
                else if (signal == bus_signal_e::BusUpd) {
                    next_state = state_e::SHARED_CLEAN;
                    bus_signals = {bus_signal_e::Update};
                }
                else if (signal == bus_signal_e::Flush) {
                    next_state = state_e::SHARED_CLEAN;
                }
                else {
                    fprintf (stderr, "%s::%d:: Encountered invalid signal %hhu.\n", __FILE__, __LINE__, signal);
                    exit(EXIT_FAILURE);
                }
                break;

            case state_e::SHARED_MODIFIED:
                if (signal == bus_signal_e::BusRd) {
                    next_state = state_e::SHARED_MODIFIED;
                    bus_signals = {bus_signal_e::Flush};
                    num_flushes_++;
                }
                else if (signal == bus_signal_e::BusUpd) {
                    next_state = state_e::SHARED_CLEAN;
                    bus_signals = {bus_signal_e::Update};
                }
                else if (signal == bus_signal_e::Flush) {
                    next_state = state_e::SHARED_CLEAN;
                }
                else {
                    fprintf (stderr, "%s::%d:: Encountered invalid signal %hhu.\n", __FILE__, __LINE__, signal);
                    exit(EXIT_FAILURE);
                }
                break;

            default:
                fprintf (stderr, "ERROR:: Encountered unknown state %hhu for the Dragon protocol.\n", state_);
                exit (EXIT_FAILURE);
        }
        state_ = next_state;
        return bus_signals;
    }
};

FACTORY_REGISTER("Dragon", CacheBlockDragon);
