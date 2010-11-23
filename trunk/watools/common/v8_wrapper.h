#ifndef __V8_WRAPPER__H__
#define __V8_WRAPPER__H__

#include <string>
#include <sstream>

#include <boost/static_assert.hpp>

#include <v8.h>

#include <jsWrappers/jsGlobal.h>
#include <weLogger.h>

class js_dom_Node;
class js_dom_NodeList;
class js_html2_HTMLCollection;
class js_html2_HTMLDocument;
class js_html2_HTMLElement;

namespace v8_wrapper {

class iterator_dfs;
class tree_node;

typedef js_html2_HTMLDocument jsDocument;
typedef boost::shared_ptr<tree_node> tree_node_ptr;
typedef std::vector<tree_node_ptr> tree_node_list;

/**
 * This template class is data structure for javascript wrapped DOM.
 */
template <class T>
class dom_data {
public:
};

template <>
class dom_data <js_html2_HTMLDocument> {
public:
    dom_data():m_opened(false) {}
    std::ostringstream m_strbuf;
    bool m_opened;
    v8_wrapper::tree_node_ptr m_execution_point;
};

/**
 * Tree structure and iteration for DOM
 */
class tree_node : public boost::enable_shared_from_this<tree_node> {
public:
    tree_node():m_tag(HTML_TAG___UNKNOWN_TAG__) {}
    tree_node(const tree_node& copy):m_child_list(copy.m_child_list),m_tag(copy.m_tag),m_this(copy.m_this) {}

    virtual ~tree_node() {}

    /**
     * Creates DFS iterator for DOM node children
     * @return v8_wrapper::iterator_dfs
     */
    virtual iterator_dfs begin_dfs();

    /**
     * Creates end() of iterator element for compatibility with STL methods
     * @return v8_wrapper::iterator_dfs
     */
    virtual iterator_dfs end_dfs();

    /**
     * Prints list of attributes
     * @return const std::string
     */
    virtual const std::string get_fields() {
        return std::string();
    };

    /**
     * Prints tag from current node. Ex. <input  disabled="0"  name="lr"  readOnly="0"  type="hidden"  value="213" >
     * @return const std::string
     */
    const std::string get_dump() {
        std::ostringstream _out;
        _out << "<" << get_tag_name(m_tag) << " " << get_fields() << ">";
        return _out.str();
    };

    /**
     * Removes all children recursively and clear these parent pointers
     */
    void clear_children();

    tree_node_ptr m_parent;
    tree_node_list m_child_list;
    tree_node_list m_fields;
    HTML_TAG m_tag;
    v8::Persistent<v8::Object> m_this;
    webEngine::html_entity_ptr m_entity;
};

/**
 * Depth-first search iterator for node
 */
class iterator_dfs: public boost::iterator_facade <iterator_dfs, tree_node_ptr, boost::forward_traversal_tag> {
public:

    /**
     * Create iterator from a tree_node. This is equivalent for STL begin()
     * @param tree_node &
     */
    explicit iterator_dfs(tree_node&);

    /**
     * Create end() iterator.
     */
    iterator_dfs();

    /**
     * Returns curren deep level
     * @return size_t
     */
    size_t current_level() const {
        return m_stack.size();
    }

protected:
    friend class boost::iterator_core_access;
    virtual void increment();
    virtual tree_node_ptr & dereference() const;
    virtual bool equal(iterator_dfs const& other) const;
private:
    tree_node_ptr m_node;
    tree_node_list::iterator m_it;
    vector< tree_node_list::iterator > m_stack;
    bool m_end;
};

/**
 * Create JavaScript object for class T and put objToWrap into InternalField
 * @param tree_node * objToWrap
 * @return
 */
template<class T> v8::Handle<v8::Object> wrap_object(v8_wrapper::tree_node *objToWrap);

/**
 * Creates tree_node from entity
 * @param webEngine::html_entity_ptr objToWrap
 * @return v8_wrapper::tree_node_ptr
 */
v8_wrapper::tree_node_ptr wrap_entity(webEngine::html_entity_ptr objToWrap);

/**
 * Convert html_entity_ptr DOM to JS DOM. If parent not pecofied, will be created jsDocument. Returns parent node.
 * @param const webEngine::html_entity_ptr & dom
 * @param tree_node_ptr parent
 * @return v8_wrapper::tree_node_ptr
 */
v8_wrapper::tree_node_ptr wrap_dom(const webEngine::html_entity_ptr& dom, v8_wrapper::tree_node_ptr parent = v8_wrapper::tree_node_ptr() );

/**
 * Register all wrapped JS classes. This function automatically generated.
 * @param v8::Persistent<v8::ObjectTemplate> global
 */
void RegisterAll(v8::Persistent<v8::ObjectTemplate> global);

/**
 * Updates standard document lists: images, applets, forms, links, anchors
 * @param jsDocument & doc
 */
void update_document(jsDocument& doc);

/**
 * Check tree_node for attribute "name". Returns object or Undefined
 * @param tree_node * treenode
 * @param const std::string & name
 * @return v8::Handle<v8::Value>
 */
v8::Handle<v8::Value> check_object_name(v8_wrapper::tree_node* treenode, const std::string &name);

/**
 * Main wrapper class. This class implements JavaScript constraction and destruction of the wrapped objects.
 */
template <class T>
class Registrator : public virtual tree_node {

public:
    Registrator() {
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8_wrapper::Registrator<> 0x" << std::hex << (void*)this << " construct " << typeid(T).name() );
    }
    virtual ~Registrator() {
        if( !this->m_this.IsEmpty() ) {
#if 0// _DEBUG
            if( !this->m_this.IsWeak() ) {
                LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8_wrapper::Registrator<> 0x" << std::hex << (void*)this << " handle is not weak " << typeid(T).name() );
            } else if( !this->m_this.IsNearDeath()) {
                LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8_wrapper::Registrator<> 0x" << std::hex << (void*)this << " handle is not near death " << typeid(T).name() );
            }
#endif
            if( this->m_this->InternalFieldCount() > 0 ) {
                this->m_this->SetInternalField(0, v8::External::New(NULL));
            }
        }
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8_wrapper::Registrator<> 0x" << std::hex << (void*)this << " ~destruct " << typeid(T).name() );
    }

    /**
     * Returns JavaScript object template for creating new JavaScript objects
     */
    static v8::Persistent<v8::FunctionTemplate> GetTemplate();

    /**
     * This method allows to add custom handlers into python-generated GetTemplate()
     */
    inline static void AdditionalHandlersGetTemplate(v8::Local<v8::ObjectTemplate> instance, v8::Local<v8::ObjectTemplate> prototype) {}

    /**
     * This Constructor() must be calling from JavaScript to create new ojects.
     */
    static v8::Handle<v8::Value> Constructor(const v8::Arguments& args) {
        if (!args.IsConstructCall())
            return v8::ThrowException(v8::String::New("DOM object constructor cannot be called as a function."));
        T* obj = new T();
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8 JavaScript binded call 0x" << std::hex << (void*)obj << " constructor " << typeid(T).name() );
        obj->m_this = v8::Persistent<v8::Object>::New(wrap_object< T >(obj));
        //obj->m_this.MakeWeak( obj , Registrator< T >::Destructor);
        return obj->m_this;
    }

    /**
     * Destructor() for calling from V8 garbage collector
     */
    static void Destructor(v8::Persistent<v8::Value> object, void* ) {
        void* ptr = v8::Local<v8::External>::Cast(object->ToObject()->GetInternalField(0))->Value();
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8 JavaScript binded call 0x" << std::hex << (void*)ptr << " destructor " << typeid(T).name());
        object->ToObject()->SetInternalField(0, v8::External::New(NULL));
        if(ptr)
            delete static_cast< T *>(ptr);
    }
    dom_data<T> m_data;
};

template<class T>
typename v8::Handle<v8::Value> IndexedPropertyGetter_handler(uint32_t index, const v8::AccessorInfo& info);

template<class T>
typename v8::Handle<v8::Value> NamedPropertyGetter_handler(v8::Local<v8::String> property, const v8::AccessorInfo& info);

template<class T>
typename v8::Handle<v8::Integer> NamedPropertyQuery_handler(v8::Local<v8::String> property, const v8::AccessorInfo& info);

template<class T>
typename v8::Handle<v8::Array> NamedPropertyEnumerator_handler(const v8::AccessorInfo& info);

template<>
void v8_wrapper::Registrator<js_dom_NodeList>::AdditionalHandlersGetTemplate(v8::Local<v8::ObjectTemplate> instance, v8::Local<v8::ObjectTemplate> prototype)
{
    instance->SetIndexedPropertyHandler(IndexedPropertyGetter_handler<js_dom_NodeList>);
}

template<>
void v8_wrapper::Registrator<jsDocument>::AdditionalHandlersGetTemplate(v8::Local<v8::ObjectTemplate> instance, v8::Local<v8::ObjectTemplate> prototype)
{
    instance->SetNamedPropertyHandler(NamedPropertyGetter_handler<jsDocument>);
}

template<>
void v8_wrapper::Registrator<js_html2_HTMLCollection>::AdditionalHandlersGetTemplate(v8::Local<v8::ObjectTemplate> instance, v8::Local<v8::ObjectTemplate> prototype)
{
    instance->SetIndexedPropertyHandler(IndexedPropertyGetter_handler<js_html2_HTMLCollection>);
    instance->SetNamedPropertyHandler(NamedPropertyGetter_handler<js_html2_HTMLCollection>);
}

template<class T>
v8::Handle<v8::Value> getElementsByTagName(v8_wrapper::tree_node* root, const std::string& tagname)
{
    T* result = new T();
    result->m_this = v8::Persistent<v8::Object>::New(wrap_object< T >(result));

    HTML_TAG val_tag = find_tag_by_name(tagname);

    for(v8_wrapper::iterator_dfs it = root->begin_dfs(); it != root->end_dfs(); ++it) {
        if((*it) && (*it)->m_tag == val_tag)
            result->m_child_list.push_back(*it);
    }

    unsigned long *len = const_cast<unsigned long*>(&result->length);
    *len = result->m_child_list.size();
    return result->m_this;
}

template<class T> v8::Handle<v8::Object> wrap_object(v8_wrapper::tree_node *objToWrap)
{
    v8::HandleScope scope;
    v8::Local<v8::Object> _instance =  v8_wrapper::Registrator<T>::GetTemplate()->GetFunction()->NewInstance(); //
    _instance->SetInternalField(0, v8::External::New(objToWrap));
    return scope.Close(_instance);
}

// for custom attribute get/set

template<class J, int N>
class CustomAttribute {
public:
    static v8::Handle<v8::Value> static_get(v8::Local<v8::String> property, const v8::AccessorInfo &info) {
        return v8::Handle<v8::Value>();
    };

    static void static_set(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info) {};

    static const bool implemented = false;
};

template<>
class CustomAttribute<js_html2_HTMLElement, 7 /*innerHTML*/> {
public:
    static v8::Handle<v8::Value> static_get(v8::Local<v8::String> property, const v8::AccessorInfo &info);

    static void static_set(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

    static const bool implemented = true;
};

template<>
class CustomAttribute<js_html2_HTMLDocument, 12 /*innerHTML*/> {
public:
    static v8::Handle<v8::Value> static_get(v8::Local<v8::String> property, const v8::AccessorInfo &info);

    static void static_set(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

    static const bool implemented = true;
};

template<>
class CustomAttribute<js_dom_Node, 6 /*firstChild*/> {
public:
    static v8::Handle<v8::Value> static_get(v8::Local<v8::String> property, const v8::AccessorInfo &info);

    static void static_set(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

    static const bool implemented = true;
};



/////// casting c++ types from/to v8::Value

//////////////// From v8::Value

template <typename T>
inline T Get(const v8::Local<v8::Value>& val);

template <>
inline std::string Get(const v8::Local<v8::Value>& val)
{
    if(val->IsString()) {
        v8::String::Utf8Value utf8(val);
        return *utf8;
    }
    return std::string();
}

template <>
inline int Get(const v8::Local<v8::Value>& val)
{
    return val->Int32Value();
}

template <>
inline long Get(const v8::Local<v8::Value>& val)
{
    return val->Int32Value();
}

template <>
inline unsigned int Get(const v8::Local<v8::Value>& val)
{
    return val->Uint32Value();
}

template <>
inline bool Get(const v8::Local<v8::Value>& val)
{
    return val->BooleanValue();
}

template <>
inline unsigned long Get(const v8::Local<v8::Value>& val)
{
    return val->Uint32Value();
}

template <>
inline unsigned short int Get(const v8::Local<v8::Value>& val)
{
    return val->Uint32Value();
}

template <>
inline v8::Handle<v8::Value> Get(const v8::Local<v8::Value>& val)
{
    return v8::Handle<v8::Value>(val);
}

template <>
inline double Get(const v8::Local<v8::Value>& val)
{
    return val->NumberValue();
}


//////////////// to v8::Value

template <typename T>
inline v8::Handle<v8::Value> Set(const T& val);

template <>
inline v8::Handle<v8::Value> Set(const v8::Handle<v8::Value>& val)
{
    return val;
}

template <>
inline v8::Handle<v8::Value> Set(const bool& val)
{
    return v8::Handle<v8::Value>(v8::Boolean::New(val));
}

template <>
inline v8::Handle<v8::Value> Set(const long int& val)
{
    return v8::Handle<v8::Value>(v8::Int32::New(val));
}

template <>
inline v8::Handle<v8::Value> Set(const short int& val)
{
    return v8::Handle<v8::Value>(v8::Int32::New(val));
}

template <>
inline v8::Handle<v8::Value> Set(const short unsigned int& val)
{
    return v8::Handle<v8::Value>(v8::Uint32::New(val));
}

template <>
inline v8::Handle<v8::Value> Set(const long unsigned int& val)
{
    return v8::Handle<v8::Value>(v8::Uint32::New(val));
}

template <>
inline v8::Handle<v8::Value> Set(const double& val)
{
    return v8::Handle<v8::Value>(v8::Number::New(val));
}

template <>
inline v8::Handle<v8::Value> Set(const std::string& val)
{
    return v8::Handle<v8::Value>(v8::String::New(val.c_str()));
}

} //namespace v8_wrapper

#endif
