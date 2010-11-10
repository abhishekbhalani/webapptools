#ifndef __V8_WRAPPER__H__
#define __V8_WRAPPER__H__

#include <v8/v8.h>
#include <weHtmlEntity.h>
#include <html_tags.h>
#include <string>
#include <sstream>
#include <boost/static_assert.hpp>
#include <jsWrappers/jsGlobal.h>
#include <weLogger.h>

class js_html2_HTMLDocument;
class js_dom_NodeList;
class js_html2_HTMLCollection;

namespace v8_wrapper {

typedef js_html2_HTMLDocument jsDocument;


//////////////// From v8::Value

template <typename T>
inline T Get(const v8::Local<v8::Value>& val)
{
    BOOST_STATIC_ASSERT(0);
}

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
inline v8::Handle<v8::Value> Set(const T& val)
{
    BOOST_STATIC_ASSERT(0);
}

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

template <class T>
class DomData {
public:
};

template <>
class DomData <js_html2_HTMLDocument> {
public:
    DomData():m_opened(false) {}
    std::ostringstream m_strbuf;
    bool m_opened;
    boost::shared_ptr<v8_wrapper::TreeNode> m_execution_point;
};

/////////////// Tree Node
class iterator_dfs;

class TreeNode : public boost::enable_shared_from_this<TreeNode> {
public:
    TreeNode():m_tag(HTML_TAG___UNKNOWN_TAG__) {}
    TreeNode(const TreeNode& copy):m_child_list(copy.m_child_list),m_tag(copy.m_tag),m_this(copy.m_this) {}

    virtual ~TreeNode() {}

    virtual iterator_dfs begin_dfs();
    virtual iterator_dfs end_dfs();

    //private:
    boost::shared_ptr<TreeNode> m_parent;
    std::vector< boost::shared_ptr<TreeNode> > m_child_list;
    HTML_TAG m_tag;
    v8::Persistent<v8::Object> m_this;
    webEngine::html_entity_ptr m_entity;
};

class iterator_dfs: public boost::iterator_facade <iterator_dfs, boost::shared_ptr<TreeNode>, boost::forward_traversal_tag> {
public:
    explicit iterator_dfs(TreeNode&);
    iterator_dfs();
    size_t current_level() const {
        return m_stack.size();
    }
protected:
    friend class boost::iterator_core_access;
    virtual void increment();
    virtual boost::shared_ptr<TreeNode> & dereference() const;
    virtual bool equal(iterator_dfs const& other) const;
private:
    boost::shared_ptr<v8_wrapper::TreeNode> m_node;
    std::vector< boost::shared_ptr<TreeNode> >::iterator m_it;
    vector< std::vector< boost::shared_ptr<TreeNode> >::iterator > m_stack;
    bool m_end;
};

/////////////// Registrator
template <class T>
class Registrator : public virtual TreeNode {

public:
    Registrator() {
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8_wrapper::Registrator<> 0x" << std::hex << (void*)this << " construct " << typeid(T).name() );
    }
    virtual ~Registrator() {
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8_wrapper::Registrator<> 0x" << std::hex << (void*)this << " ~destruct " << typeid(T).name() );
    }
    static v8::Persistent<v8::FunctionTemplate> GetTemplate();

    inline static void AdditionalHandlersGetTemplate(v8::Local<v8::ObjectTemplate> instance, v8::Local<v8::ObjectTemplate> prototype) {}

    static v8::Handle<v8::Value> Constructor(const v8::Arguments& args) {
        if (!args.IsConstructCall())
            return v8::ThrowException(v8::String::New("DOM object constructor cannot be called as a function."));
        T* obj = new T();
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8 JavaScript binded call 0x" << std::hex << (void*)obj << " constructor " << typeid(T).name() );
        obj->m_this = v8::Persistent<v8::Object>::New(wrap_object< T >(obj));
        obj->m_this.MakeWeak( NULL , Registrator< T >::Destructor);
        return obj->m_this;
    }

    static void Destructor(v8::Persistent<v8::Value> object, void* parameter) {
        //TODO: check and fix
        void* ptr = Local<External>::Cast(object->ToObject()->GetInternalField(0))->Value();
        LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), "v8 JavaScript binded call 0x" << std::hex << (void*)ptr << " destructor " << typeid(T).name() );
        object->ToObject()->SetInternalField(0, v8::External::New(NULL));
        if(parameter)
            delete static_cast< T *>(parameter);
    }
    DomData<T> m_data;
};

template<class T>
typename v8::Handle<v8::Value> IndexedPropertyGetter_handler(uint32_t index, const v8::AccessorInfo& info);

template<class T>
typename v8::Handle<v8::Value> NamedPropertyGetter_handler(v8::Local<v8::String> property, const v8::AccessorInfo& info);

template<class T>
typename v8::Handle<v8::Integer> NamedPropertyQuery_handler(v8::Local<v8::String> property, const v8::AccessorInfo& info);

template<class T>
typename v8::Handle<v8::Array> NamedPropertyEnumerator_handler(const v8::AccessorInfo& info);

void v8_wrapper::Registrator<js_dom_NodeList>::AdditionalHandlersGetTemplate(v8::Local<v8::ObjectTemplate> instance, v8::Local<v8::ObjectTemplate> prototype)
{
    instance->SetIndexedPropertyHandler(IndexedPropertyGetter_handler<js_dom_NodeList>);
}

void v8_wrapper::Registrator<jsDocument>::AdditionalHandlersGetTemplate(v8::Local<v8::ObjectTemplate> instance, v8::Local<v8::ObjectTemplate> prototype)
{
    instance->SetNamedPropertyHandler(NamedPropertyGetter_handler<jsDocument>);
}

void v8_wrapper::Registrator<js_html2_HTMLCollection>::AdditionalHandlersGetTemplate(v8::Local<v8::ObjectTemplate> instance, v8::Local<v8::ObjectTemplate> prototype)
{
    instance->SetIndexedPropertyHandler(IndexedPropertyGetter_handler<js_html2_HTMLCollection>);
    instance->SetNamedPropertyHandler(NamedPropertyGetter_handler<js_html2_HTMLCollection>);
}

void RegisterAll(v8::Persistent<v8::ObjectTemplate> global);

boost::shared_ptr< v8_wrapper::TreeNode > wrap_dom(const webEngine::html_entity_ptr& dom, boost::shared_ptr<v8_wrapper::TreeNode> parent = boost::shared_ptr<v8_wrapper::TreeNode>() );

template<class T>
v8::Handle<v8::Value> getElementsByTagName(v8_wrapper::TreeNode* root, const std::string& tagname)
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

void update_document(jsDocument& doc);

v8::Handle<v8::Value> check_object_name(v8_wrapper::TreeNode* treenode, const std::string &name);

}

#endif
