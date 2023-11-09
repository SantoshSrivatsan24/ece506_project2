#ifndef __PORT_H__
#define __PORT_H__

#include <vector>

template <typename T> class Imp;


template <typename T>
class Port {

private:
    /**
     * A port can have multiple implementations.
     * This models a one to many connection
    */
    std::vector<Imp<T> *> port_imp_;

public:
    Port() {}

    /* Connect the port to its implementation */
    void connect(Imp<T> *imp) {
        port_imp_.push_back(imp);
    }

    /* Write a transaction onto the port */
    void post(T transaction) {
        /* Iterate over every port implementation connected to this port */
        for (Imp<T> * imp : port_imp_) {
            /* The port's implementation `snoops` the transaction that the port writes */
            imp->snoop(transaction);
        }
    }
};

template <typename T>
class Imp {

public:
    Imp() {}

    virtual void snoop(T transaction) = 0;
};

#endif /* __PORT_H__ */