#include "factory.h"

/**
 * @brief 
 * 
 * @param name The name of the derived class
 * @param callback The callback function used to create an object from the derived class
 */
void Factory::register_class (const std::string &name, callback_t callback) {
    class_registry[name] = callback;
}

 /**
 * @brief Create a derived object from the base class `CacheBlock`
 * 
 * @param name: A string that determines the derived object that will be created
 * @return std::shared_ptr<CacheBlock> 
 */
CacheBlock* Factory::create(const std::string &name) {

    class_registry_it entry = class_registry.find(name);
    if (entry != class_registry.end()) {
        /* Call the callback function. This creates a derived object */
        return (entry->second)();
    }
    fprintf (stderr, "ERROR: Type %s not registered with the object factory.\n", name.c_str());
    exit (EXIT_FAILURE);
}

Factory* Factory::get_instance() {
    static Factory factory;
    return &factory;
}

/**
 * 
 * @brief Register a class with the object factory
 * We would like to call Factory::register_object(name, callback)
 * globally, but we can't.
 * So we do it in the constructor of this class.
 */
Registry::Registry(const std::string &name, callback_t callback) {
    Factory::get_instance()->register_class(name, callback);
}
