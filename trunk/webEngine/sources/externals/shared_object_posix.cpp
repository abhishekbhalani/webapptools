#include <iostream>
#include <boost/utility.hpp>

#include <dlfcn.h>

#include "shared_object.hpp"

namespace dyn {
class shared_object::impl_t : boost::noncopyable {
public:
    explicit impl_t(const char *soname);
    ~impl_t();
    void *get_symbol(const char *symbol) const;
    void *get_handle(void) const;
private:
    void *handle;
};

shared_object::impl_t::impl_t(const char *soname)
    : handle(0)
{
    handle = dlopen(soname, RTLD_NOW);
    if(!handle) {
        const char *error = dlerror();
        throw shared_object::exception(error ? error : "");
    }
}

shared_object::impl_t::~impl_t()
{
    if(dlclose(handle)) {
        std::cerr << "can't close shared object" << std::endl;
    }
}

void* shared_object::impl_t::get_handle( void ) const
{
    return handle;
}

void* shared_object::impl_t::get_symbol(const char *symbol) const
{
    void *sym = dlsym(handle, symbol);
    if(!sym) {
        if(!sym) {
            const char *error = dlerror();
            throw shared_object::exception(error ? error : "");
        }
    }

    return sym;
}

shared_object::shared_object(const char *soname)
    : impl(new impl_t(soname))
{
}

shared_object::~shared_object() {}

void *shared_object::get_symbol(const char *symbol) const
{
    return impl->get_symbol(symbol);
}

//     void *shared_object::get_handle(void) const
//     {
//         return impl->get_handle();
//     };
}

