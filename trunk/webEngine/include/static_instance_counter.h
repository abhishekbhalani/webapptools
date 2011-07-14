#ifndef __static_instance_counter__H__
#define __static_instance_counter__H__

#include "weLogger.h"

#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>

template <class T>
class static_instance_counter {
public:
    static boost::uint32_t get_instance_counter() {
        return (boost::uint32_t)m_instance_count;
    }
#ifdef _DEBUG
    static static_instance_counter<T>* debug_get_instance(size_t i) {
        boost::mutex::scoped_lock lock(m_mutex);
        if(i < m_pointers.size())
            return m_pointers[i];
        return NULL;
    }
#endif
protected:
    static_instance_counter() {
        ++m_instance_count;
#ifdef _DEBUG
        boost::mutex::scoped_lock lock(m_mutex);
		LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), L"static_instance_counter " << (size_t) this << L" inc [" << m_instance_count << L"] " << std::string(typeid(T).name()) );
        m_pointers.push_back(this);
#endif
    }
    virtual ~static_instance_counter() {
        --m_instance_count;
#ifdef _DEBUG
        boost::mutex::scoped_lock lock(m_mutex);
		LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), L"static_instance_counter " << (size_t) this << L" dec [" << m_instance_count << L"] " << std::string(typeid(T).name()) );
        typename std::vector< static_instance_counter<T>* >::const_iterator it_ = std::find(m_pointers.begin(), m_pointers.end(), this);
        if(it_ != m_pointers.end())
            m_pointers.erase(it_);
#endif
    }
private:
    static volatile boost::uint32_t m_instance_count;
#ifdef _DEBUG
    static boost::mutex m_mutex;
    static std::vector< static_instance_counter<T>* > m_pointers;
#endif
};

template <class T> volatile boost::uint32_t static_instance_counter<T>::m_instance_count = 0;
#ifdef _DEBUG
template <class T> boost::mutex static_instance_counter<T>::m_mutex;
template <class T> std::vector< static_instance_counter<T>* > static_instance_counter<T>::m_pointers;
#endif



#endif

