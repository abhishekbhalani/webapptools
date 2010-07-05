#ifndef SHARED_OBJECT_H
#define SHARED_OBJECT_H

#include <stdexcept>
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

/**
	\page shared_object Shared Objects

	Libdyn is a C++ utility for loading dynamically linked libraries. It
	is a wrapper to platform specific dll loading functions, UNIX's dlopen,
	Windows' LoadLibrary and SDL's SDL_LoadObject.

	Libraries are loaded by creating an instance of the dyn::shared_object
	class. A library is unloaded when the shared_object is destroyed.

	\code

	try
	{    
		dyn::shared_object so("./libhello.so");
	} catch(dyn::shared_object::exception const &e)
	{
		std::cerr << "Can't load dynamic library: " << e.what();
	}

	\endcode
	
	Symbols (functions, etc) are loaded from dynamic libraries using
	shared_object::get_symbol.
	
	\code
	try
	{    
		void (*ptr)() = 0;
		so.get_symbol("hello", ptr);
		ptr();
	} catch(dyn::shared_object::exception const &e)
	{
		std::cerr << "Can't load symbol for dynamic library: " << e.what();
	}
	\endcode
	
	See your platform documentation for details on library search paths and
	name resolution.
	
*/

/// Dynamically linked libraries
namespace dyn
{
	/// A shared object
	class shared_object : boost::noncopyable
	{
		public:
			/// Exception
			struct exception : public std::runtime_error
			{
				/// Create a new exception
				/**
					@param msg the exception message
				*/
				explicit exception(const char *msg)
					: std::runtime_error(msg)
				{
				}
			};

			/// Constructor
			/**
				@param soname the filename of the dynamically linked library
				@throws exception
			*/
			explicit shared_object(const char *soname);
			
			/// Destructor
			/**
				@note If an error occurs, a message is printed to std::cerr.
			*/
			~shared_object();
			
			/// Get a symbol from the dynamically linked library
			/**
				C++ forbids casting from void pointers to function pointers. A union is
				used to create a workaround for the casting.

				@param symbol the name of the symbol
				@param ptr a reference to the symbol pointer to store
				@throws exception
				@note Strong exception safety: The value of ptr is untouched if an exception is thrown
			*/
			template <typename T>
			void get_symbol(const char* symbol, T *& ptr) const
			{
				union { void *from; T* to; };
				from = get_symbol(symbol);
				ptr = to;
			}

            void* get_handle( void ) const { return (void*)this; };
			
		private:
			class impl_t;
			boost::scoped_ptr<impl_t> impl;

			void *get_symbol(const char *symbol) const;
	};
}

#endif

