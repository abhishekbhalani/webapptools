#include <iostream>
#include <string>
#include <boost/utility.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "shared_object.hpp"

namespace dyn
{
	class shared_object::impl_t : boost::noncopyable
	{
		public:
			explicit impl_t(const char *soname);
			~impl_t();
			void *get_symbol(const char *symbol) const;
            void *get_handle(void) const;
		private:
			HMODULE handle;
	};

	namespace
	{
		std::string getErrorMsg()
		{
			DWORD err = GetLastError();

			LPVOID msgBuf;
			FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR) &msgBuf,
				0, NULL );

			try
			{
				std::string msg(reinterpret_cast<const char *>(msgBuf));
				LocalFree(msgBuf);
				return msg;
			} catch(...)
			{
				LocalFree(msgBuf);
				throw;
			}
		}
	}

	shared_object::impl_t::impl_t(const char *soname)
		: handle(0)
	{
		handle = LoadLibraryA(soname);
		if(!handle)
		{
			throw shared_object::exception(getErrorMsg().c_str());
		}
	}

	shared_object::impl_t::~impl_t()
	{
		if(!FreeLibrary(handle))
		{
			std::string str;
			try
			{
				str = getErrorMsg();
			} catch(...)
			{
				return;
			}

			std::cerr << str << std::endl;
		}
	}

    void* shared_object::impl_t::get_handle( void ) const
    {
        return (void*)handle;
    }

	void* shared_object::impl_t::get_symbol(const char *symbol) const
	{
		void *ptr = GetProcAddress(handle, symbol);

		if(!ptr)
		{
			throw shared_object::exception(getErrorMsg().c_str());
		}

		return ptr;
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

