
#include "v8_wrapper.h"
#include <html_js.h>
#include <weStrings.h>

using namespace v8;

v8_wrapper::iterator_dfs::iterator_dfs(TreeNode& treenode)
    : m_node(treenode.shared_from_this()),
      m_it(m_node->m_child_list.begin()),
      m_end(m_it == m_node->m_child_list.end())
{}

v8_wrapper::iterator_dfs::iterator_dfs()
    : m_end(true)
{}

void v8_wrapper::iterator_dfs::increment()
{
    if(m_end)
        return;
    if( !(*m_it)->m_child_list.empty() ) {
        m_node = *m_it;
        m_stack.push_back( m_it );
        m_it = m_node->m_child_list.begin();
        return;
    }
    if(m_it != m_node->m_child_list.end())
        ++m_it;
    while(m_it == m_node->m_child_list.end()) {
        if(!m_stack.empty()) {
            m_it = m_stack.back();
            m_stack.pop_back();
            m_node = m_node->m_parent;
            ++m_it;
            continue;
        } else {
            m_end = true;
            return;
        }
    }
}

boost::shared_ptr<v8_wrapper::TreeNode> & v8_wrapper::iterator_dfs::dereference() const
{
    return *m_it;
}

bool v8_wrapper::iterator_dfs::equal(v8_wrapper::iterator_dfs const& other) const
{
    if(m_end != other.m_end)
        return false;
    if(m_end)
        return true;
    return m_it == other.m_it;
}

v8_wrapper::iterator_dfs v8_wrapper::TreeNode::begin_dfs()
{
    return v8_wrapper::iterator_dfs(*this);
}

v8_wrapper::iterator_dfs v8_wrapper::TreeNode::end_dfs()
{
    return v8_wrapper::iterator_dfs();
}

v8::Handle<v8::Value> v8_wrapper::check_object_name(v8_wrapper::TreeNode* treenode, const std::string &name)
{
    try {
        js_html2_HTMLFormElement * node = dynamic_cast<js_html2_HTMLFormElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLSelectElement * node = dynamic_cast<js_html2_HTMLSelectElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLInputElement * node = dynamic_cast<js_html2_HTMLInputElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLTextAreaElement * node = dynamic_cast<js_html2_HTMLTextAreaElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLButtonElement * node = dynamic_cast<js_html2_HTMLButtonElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLAnchorElement * node = dynamic_cast<js_html2_HTMLAnchorElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLImageElement * node = dynamic_cast<js_html2_HTMLImageElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLObjectElement * node = dynamic_cast<js_html2_HTMLObjectElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLParamElement * node = dynamic_cast<js_html2_HTMLParamElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLAppletElement * node = dynamic_cast<js_html2_HTMLAppletElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLMapElement * node = dynamic_cast<js_html2_HTMLMapElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLFrameElement * node = dynamic_cast<js_html2_HTMLFrameElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    try {
        js_html2_HTMLIFrameElement * node = dynamic_cast<js_html2_HTMLIFrameElement*>( treenode );
        if(node && node->name == name) return node->m_this;
    } catch(bad_cast&) {}

    return v8::Handle<v8::Value>();
}

v8::Handle<v8::Value> v8_wrapper::IndexedPropertyGetter_handler <js_dom_NodeList> (uint32_t index, const v8::AccessorInfo& info)
{
    HandleScope scope;
    Local<Object> self = info.Holder();
    Handle<Value> retval;
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    js_dom_NodeList * el = static_cast<js_dom_NodeList *>(ptr);
    retval = el->item(index);
    return scope.Close(retval);
}

v8::Handle<v8::Value> v8_wrapper::NamedPropertyGetter_handler <v8_wrapper::jsDocument> (v8::Local<v8::String> property, const v8::AccessorInfo& info)
{
    HandleScope scope;
    Local<Object> self = info.Holder();
    Handle<Value> retval = self->GetRealNamedProperty(property);
    if(retval.IsEmpty()) {
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        v8_wrapper::jsDocument * el = static_cast<v8_wrapper::jsDocument *>(ptr);
        std::string prop = v8_wrapper::Get<std::string>(property);
        for(v8_wrapper::iterator_dfs it = el->begin_dfs(); (it != el->end_dfs()) && retval.IsEmpty(); ++it) {
            if((*it))
                retval = v8_wrapper::check_object_name( (*it).get(), prop );
        }
    }
    return scope.Close(retval);
}

v8::Handle<v8::Value> v8_wrapper::IndexedPropertyGetter_handler <js_html2_HTMLCollection> (uint32_t index, const v8::AccessorInfo& info)
{
    HandleScope scope;
    Local<Object> self = info.Holder();
    Handle<Value> retval;
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    js_html2_HTMLCollection * el = static_cast<js_html2_HTMLCollection *>(ptr);
    retval = el->item(index);
    return scope.Close(retval);
}

v8::Handle<v8::Value> v8_wrapper::NamedPropertyGetter_handler <js_html2_HTMLCollection> (v8::Local<v8::String> property, const v8::AccessorInfo& info)
{
    HandleScope scope;
    Local<Object> self = info.Holder();
    Handle<Value> retval = self->GetRealNamedProperty(property);
    if(retval.IsEmpty()) {
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        js_html2_HTMLCollection * el = static_cast<js_html2_HTMLCollection *>(ptr);
        std::string prop = v8_wrapper::Get<std::string>(property);
        retval = el->namedItem(prop);
    }
    return scope.Close(retval);
}

void v8_wrapper::update_document(v8_wrapper::jsDocument& doc)
{
    js_html2_HTMLCollection* images = new js_html2_HTMLCollection();
    images->m_this = v8::Persistent<v8::Object>::New(wrap_object< js_html2_HTMLCollection >(images));

    js_html2_HTMLCollection* applets = new js_html2_HTMLCollection();
    applets->m_this = v8::Persistent<v8::Object>::New(wrap_object< js_html2_HTMLCollection >(applets));

    js_html2_HTMLCollection* forms = new js_html2_HTMLCollection();
    forms->m_this = v8::Persistent<v8::Object>::New(wrap_object< js_html2_HTMLCollection >(forms));

    js_html2_HTMLCollection* links = new js_html2_HTMLCollection();
    links->m_this = v8::Persistent<v8::Object>::New(wrap_object< js_html2_HTMLCollection >(links));

    js_html2_HTMLCollection* anchors = new js_html2_HTMLCollection();
    anchors->m_this = v8::Persistent<v8::Object>::New(wrap_object< js_html2_HTMLCollection >(anchors));

    for(v8_wrapper::iterator_dfs it = doc.begin_dfs(); it != doc.end_dfs(); ++it) {
        if((*it)) {
            switch((*it)->m_tag) {
            case HTML_TAG_form:
                forms->m_child_list.push_back(*it);
                break;
            case HTML_TAG_img:
                images->m_child_list.push_back(*it);
                break;
            case HTML_TAG_applet:
                applets->m_child_list.push_back(*it);
                break;
            case HTML_TAG_a: {
                webEngine::AttrMap &attrs = (*it)->m_entity->attr_list();
                if(attrs.find("href") != attrs.end())
                    links->m_child_list.push_back(*it);
                if(attrs.find("name") != attrs.end())
                    anchors->m_child_list.push_back(*it);
                break;
            }
            }
        }
    }

    const_cast<unsigned long&>(images->length) = images->m_child_list.size();
    const_cast<unsigned long&>(applets->length) = applets->m_child_list.size();
    const_cast<unsigned long&>(forms->length) = forms->m_child_list.size();
    const_cast<unsigned long&>(links->length) = links->m_child_list.size();
    const_cast<unsigned long&>(anchors->length) = anchors->m_child_list.size();

    const_cast<v8::Handle<v8::Value> &>(doc.images) = images->m_this;
    const_cast<v8::Handle<v8::Value> &>(doc.applets) = applets->m_this;
    const_cast<v8::Handle<v8::Value> &>(doc.forms) = forms->m_this;
    const_cast<v8::Handle<v8::Value> &>(doc.links) = links->m_this;
    const_cast<v8::Handle<v8::Value> &>(doc.anchors) = anchors->m_this;
}

boost::shared_ptr< v8_wrapper::TreeNode > v8_wrapper::wrap_dom(const webEngine::html_entity_ptr& dom, boost::shared_ptr<v8_wrapper::TreeNode> parent)
{
    boost::shared_ptr<v8_wrapper::TreeNode> root;
    if(!parent) {
        v8_wrapper::jsDocument* ptr = new jsDocument();
        ptr->m_this = v8::Persistent<v8::Object>::New(wrap_object<v8_wrapper::jsDocument>(ptr));
        root.reset(ptr);
    } else
        root = parent;

    boost::shared_ptr<v8_wrapper::TreeNode> tree_node(root);
    webEngine::html_entity_ptr current_level = dom;
    webEngine::entity_list::iterator it = current_level->Children().begin();
    typedef pair< webEngine::html_entity_ptr, webEngine::entity_list::iterator > level_t;
    vector< level_t > stack;

    boost::shared_ptr<v8_wrapper::TreeNode> node;

    while( true ) {
        if(it == current_level->Children().end()) {
            if(!stack.empty()) {
                level_t prev = stack.back();
                stack.pop_back();
                node = tree_node->m_parent;
                current_level = prev.first;
                it = prev.second;
                ++it;
                continue;
            } else {
                break;
            }
        } else {
            node = wrap_entity(boost::shared_dynamic_cast<webEngine::html_entity>(*it));
        }
        node->m_parent = tree_node;
        tree_node->m_child_list.push_back(node);
        if( !(*it)->Children().empty() ) {
            tree_node = node;
            stack.push_back( level_t(current_level, it) );
            current_level = boost::shared_dynamic_cast<webEngine::html_entity>(*it);
            it = current_level->Children().begin();
            continue;
        }
        ++it;
    }

    boost::shared_ptr<v8_wrapper::jsDocument> doc = boost::shared_dynamic_cast<v8_wrapper::jsDocument>(root);
    if(doc)
        v8_wrapper::update_document(*doc);

    return root;
}