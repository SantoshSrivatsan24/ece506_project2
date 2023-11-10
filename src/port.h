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

    /* Broadcast the transaction onto every port */
    void send(T &trans) {
        /* Iterate over every port implementation connected to this port */
        for (Port<T> *port : ports_) {
            /* A transaction sent over one port is received by the other ports */
            port->receive(trans);
        }
    }

    /* Send the transcation on a specific port */
    void send(uint32_t port_id, T &trans) {
        ports_[port_id]->receive(trans);
    }

    /* The receiver decides how it wants to handle the transaction */
    virtual void receive (T &trans) = 0;
};


#endif /* __PORT_H__ */