#ifndef __PORT_H__
#define __PORT_H__

#include <vector>

/**
 * Facilitate two way communication between objects.
 * Ports are connected to other ports
*/
template <typename T>
class Port {

private:
    /* A port can be connected to multiple other ports */
    std::vector<Port<T> *> ports_;

public:
    Port() {}

    /* Connect the port to another port */
    void connect(Port<T> *port) {
        ports_.push_back(port);
    }

    int get_num_ports() {
        return ports_.size();
    }

    /* Send a transaction over the first connected port */
    void send(const T &trans) {
        ports_[0]->receive(trans);
    }

    /* Send a transcation over a specific port */
    void send(uint32_t port_id, const T &trans) {
        ports_[port_id]->receive(trans);
    }

    /* Request a transaction from the first connected port */
    void request(T &trans) {
        ports_[0]->respond(trans);
    }

    /* Request a transaction from a specific port */
    void request(uint32_t port_id, T &trans) {
        ports_[port_id]->respond(trans);
    }

    /* The receiver decides how it wants to handle the sent transaction */
    virtual void receive (const T &trans) = 0;

    /* The receiver decides how it wants to handle the requested transaction */
    virtual void respond (T &trans) = 0;
};


#endif /* __PORT_H__ */