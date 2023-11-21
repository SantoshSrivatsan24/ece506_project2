#ifndef __FACTORY_H__
#define __FACTORY_H__

#include <string>
#include <map>
#include "cache_block.h"
#include <iostream>

/**
 * @brief Code referenced from: https://www.codeproject.com/Articles/567242/AplusC-2b-2bplusObjectplusFactory
 */

/**
 * @brief callback_t is a pointer to a function that return CacheBlock* 
 * and takes in no arguments.
 * i.e., it is a pointer to a function that creates a new CacheBlock.
 */
// using callback_t = std::function<CacheBlock*(void)>;
using callback_t = CacheBlock* (*)();
using class_registry_t =  std::map<std::string, callback_t>;
using class_registry_it = class_registry_t::iterator;

class Factory {
private:
    class_registry_t class_registry;

public:
    void register_class (const std::string &name, callback_t callback);
    CacheBlock* create(const std::string &name);

    static Factory* get_instance();
};

class Registry {
public:
    Registry (const std::string &name, callback_t callback);
};

#define FACTORY_CREATE(NAME) \
    Factory::get_instance()->create(NAME);

#define FACTORY_REGISTER(NAME, TYPE) \
    static Registry registry(NAME, \
        [](void)->CacheBlock* {return new TYPE();}); /* This is the callback function */


#endif /* __FACTORY_H__ */