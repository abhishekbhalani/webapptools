
#include "v8_wrapper.h"
#include <html_js.h>
#include <weStrings.h>

#include <html_tags_wrapper.h>

using namespace v8;

v8_wrapper::iterator_dfs::iterator_dfs(v8_wrapper::tree_node& treenode)
    : m_node(treenode.shared_from_this()),
      m_it(m_node->m_child_list.begin()),
      m_end(m_it == m_node->m_child_list.end())
{}

v8_wrapper::iterator_dfs::iterator_dfs()
    : m_end(true)
{}

void v8_wrapper::iterator_dfs::increment()
{
#ifdef _DEBUG
    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), _T("iterator_dfs: ") << current_level() << (*m_it)->get_dump());
#endif
    if(!m_end) {
        if( !(*m_it)->m_child_list.empty() ) {
            m_node = *m_it;
            m_stack.push_back( m_it );
            m_it = m_node->m_child_list.begin();
        } else {
            if(m_it != m_node->m_child_list.end())
                ++m_it;
            while(m_it == m_node->m_child_list.end()) {
                if(!m_stack.empty()) {
                    if(m_node->m_parent) {
                        m_it = m_stack.back();
                        m_node = m_node->m_parent;
                        ++m_it;
                    }
                    m_stack.pop_back();
                    continue;
                } else {
                    m_end = true;
                    break;
                }
            }
        }
    }
}

v8_wrapper::tree_node_ptr & v8_wrapper::iterator_dfs::dereference() const
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

v8_wrapper::iterator_dfs v8_wrapper::tree_node::begin_dfs()
{
    return v8_wrapper::iterator_dfs(*this);
}

v8_wrapper::iterator_dfs v8_wrapper::tree_node::end_dfs()
{
    return v8_wrapper::iterator_dfs();
}

void v8_wrapper::tree_node::clear_children()
{
    v8_wrapper::tree_node_list tmp_list;
    for(v8_wrapper::iterator_dfs it = this->begin_dfs(); it != this->end_dfs(); ++it) {
        tmp_list.push_back(*it);
    }
    for(v8_wrapper::tree_node_list::iterator it = tmp_list.begin(); it != tmp_list.end(); ++it) {
        (*it)->m_parent.reset();
        (*it)->m_child_list.clear();
    }
    m_child_list.clear();
}


v8::Handle<v8::Value> v8_wrapper::check_object_name(v8_wrapper::tree_node* treenode, const std::string &name)
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
    v8_wrapper::tree_node* ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
    js_dom_NodeList * el = dynamic_cast<js_dom_NodeList *>(ptr);
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
        v8_wrapper::tree_node* ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
        v8_wrapper::jsDocument * el = dynamic_cast<v8_wrapper::jsDocument *>(ptr);
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
    v8_wrapper::tree_node* ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
    js_html2_HTMLCollection * el = dynamic_cast<js_html2_HTMLCollection *>(ptr);
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
        v8_wrapper::tree_node* ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
        js_html2_HTMLCollection * el = dynamic_cast<js_html2_HTMLCollection *>(ptr);
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

v8_wrapper::tree_node_ptr v8_wrapper::wrap_dom(const webEngine::html_entity_ptr& dom, v8_wrapper::tree_node_ptr parent)
{
    v8_wrapper::tree_node_ptr root;
    if(!parent) {
        v8_wrapper::jsDocument* ptr = new jsDocument();
        ptr->m_this = v8::Persistent<v8::Object>::New(wrap_object<v8_wrapper::jsDocument>(ptr));
        const_cast<Handle<Value>&>(ptr->documentElement) = ptr->m_this;
        root.reset(ptr);
    } else
        root = parent;

    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), _T("v8_wrapper::wrap_dom root 0x") << std::hex << (size_t)root.get() << _T(" starts with '") << dom->Name() << _T("'") );

    v8_wrapper::tree_node_ptr tree_node(root);
    webEngine::html_entity_ptr current_level = dom;
    webEngine::entity_list::iterator it = current_level->Children().begin();
    typedef pair< webEngine::html_entity_ptr, webEngine::entity_list::iterator > level_t;
    vector< level_t > stack;

    v8_wrapper::tree_node_ptr node;

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


v8_wrapper::tree_node_ptr v8_wrapper::wrap_entity( webEngine::html_entity_ptr objToWrap )
{
    v8_wrapper::tree_node_ptr node;
    HTML_TAG html_tag = objToWrap->HtmlTag();

    switch(html_tag) {
    case  HTML_TAG_a:
        node = TreeNodeFromEntity<js_html2_HTMLAnchorElement, true>(objToWrap);
        break;
    case  HTML_TAG_abbr :
    case  HTML_TAG_acronym :
    case  HTML_TAG_address :
    case  HTML_TAG_applet :
    case  HTML_TAG_area :
    case  HTML_TAG_b :
    case  HTML_TAG_base :
    case  HTML_TAG_basefont :
    case  HTML_TAG_bdo :
    case  HTML_TAG_big :
    case  HTML_TAG_blockquote :
        break;
    case  HTML_TAG_body :
        node = TreeNodeFromEntity<js_html2_HTMLBodyElement, true>(objToWrap);
        break;
    case  HTML_TAG_br :
        node = TreeNodeFromEntity<js_html2_HTMLBRElement, true>(objToWrap);
        break;
    case  HTML_TAG_button :
        node = TreeNodeFromEntity<js_html2_HTMLButtonElement, true>(objToWrap);
        break;
    case  HTML_TAG_caption :
        node = TreeNodeFromEntity<js_html2_HTMLTableCaptionElement, true>(objToWrap);
        break;
    case  HTML_TAG_center :
    case  HTML_TAG_cite :
    case  HTML_TAG_code :
    case  HTML_TAG_col :
    case  HTML_TAG_colgroup :
    case  HTML_TAG_dd :
    case  HTML_TAG_del :
    case  HTML_TAG_dfn :
    case  HTML_TAG_dir :
        break;
    case  HTML_TAG_div :
        node = TreeNodeFromEntity<js_html2_HTMLDivElement, true>(objToWrap);
        break;
    case  HTML_TAG_dl :
    case  HTML_TAG_dt :
    case  HTML_TAG_em :
    case  HTML_TAG_fieldset :
    case  HTML_TAG_font :
        break;
    case  HTML_TAG_form:
        node = TreeNodeFromEntity<js_html2_HTMLFormElement, true>(objToWrap);
        break;
    case  HTML_TAG_frame :
    case  HTML_TAG_frameset :
        break;
    case  HTML_TAG_h1 :
    case  HTML_TAG_h2 :
    case  HTML_TAG_h3 :
    case  HTML_TAG_h4 :
    case  HTML_TAG_h5 :
    case  HTML_TAG_h6 :
        node = TreeNodeFromEntity<js_html2_HTMLHeadingElement, true>(objToWrap);
        break;
    case  HTML_TAG_head :
        node = TreeNodeFromEntity<js_html2_HTMLHeadElement, true>(objToWrap);
        break;
    case  HTML_TAG_hr :
        node = TreeNodeFromEntity<js_html2_HTMLHRElement, true>(objToWrap);
        break;
    case  HTML_TAG_html :
        node = TreeNodeFromEntity<js_html2_HTMLHtmlElement, true>(objToWrap);
        break;
    case  HTML_TAG_i :
        break;
    case  HTML_TAG_iframe :
        node = TreeNodeFromEntity<js_html2_HTMLIFrameElement, true>(objToWrap);
        break;
    case  HTML_TAG_img :
        node = TreeNodeFromEntity<js_html2_HTMLImageElement, true>(objToWrap);
        break;
    case  HTML_TAG_input :
        node = TreeNodeFromEntity<js_html2_HTMLInputElement, true>(objToWrap);
        break;
    case  HTML_TAG_ins :
    case  HTML_TAG_isindex :
    case  HTML_TAG_kbd :
        break;
    case  HTML_TAG_label :
        node = TreeNodeFromEntity<js_html2_HTMLLabelElement, true>(objToWrap);
        break;
    case  HTML_TAG_legend :
        node = TreeNodeFromEntity<js_html2_HTMLLegendElement, true>(objToWrap);
        break;
    case  HTML_TAG_li :
        node = TreeNodeFromEntity<js_html2_HTMLLIElement, true>(objToWrap);
        break;
    case  HTML_TAG_link :
        node = TreeNodeFromEntity<js_html2_HTMLLinkElement, true>(objToWrap);
        break;
    case  HTML_TAG_map :
        node = TreeNodeFromEntity<js_html2_HTMLMapElement, true>(objToWrap);
        break;
    case  HTML_TAG_menu :
        node = TreeNodeFromEntity<js_html2_HTMLMenuElement, true>(objToWrap);
        break;
    case  HTML_TAG_meta :
        node = TreeNodeFromEntity<js_html2_HTMLMetaElement, true>(objToWrap);
        break;
    case  HTML_TAG_noframes :
    case  HTML_TAG_noscript :
        break;
    case  HTML_TAG_object :
        node = TreeNodeFromEntity<js_html2_HTMLObjectElement, true>(objToWrap);
        break;
    case  HTML_TAG_ol :
        node = TreeNodeFromEntity<js_html2_HTMLOListElement, true>(objToWrap);
        break;
    case  HTML_TAG_optgroup :
    case  HTML_TAG_option :
    case  HTML_TAG_p :
    case  HTML_TAG_param :
    case  HTML_TAG_pre :
    case  HTML_TAG_q :
    case  HTML_TAG_s :
    case  HTML_TAG_samp :
        break;
    case  HTML_TAG_script :
        node = TreeNodeFromEntity<js_html2_HTMLScriptElement, true>(objToWrap);
        break;
    case  HTML_TAG_select :
        node = TreeNodeFromEntity<js_html2_HTMLSelectElement, true>(objToWrap);
        break;
    case  HTML_TAG_small :
    case  HTML_TAG_span :
    case  HTML_TAG_strike :
    case  HTML_TAG_strong :
    case  HTML_TAG_style :
    case  HTML_TAG_sub :
    case  HTML_TAG_sup :
        break;
    case  HTML_TAG_table :
        node = TreeNodeFromEntity<js_html2_HTMLTableElement, true>(objToWrap);
        break;
    case  HTML_TAG_tbody :
    case  HTML_TAG_td :
    case  HTML_TAG_textarea :
    case  HTML_TAG_tfoot :
    case  HTML_TAG_th :
    case  HTML_TAG_thead :
        break;
    case  HTML_TAG_title :
        node = TreeNodeFromEntity<js_html2_HTMLTitleElement, true>(objToWrap);
        break;
    case  HTML_TAG_tr :
    case  HTML_TAG_tt :
    case  HTML_TAG_u :
    case  HTML_TAG_ul :
    case  HTML_TAG_var :
    default:
        node = TreeNodeFromEntity<js_html2_HTMLElement, true>(objToWrap);
        break;
    }

    if(!node)
        node = TreeNodeFromEntity<js_html2_HTMLElement, true>(objToWrap);

    node->m_entity = objToWrap;
    return node;
}


v8::Handle<v8::Value> v8_wrapper::CustomAttribute<js_html2_HTMLElement, 7 /*innerHTML*/>::static_get(v8::Local<v8::String> property, const v8::AccessorInfo &info)
{
    v8::ThrowException(v8::String::New("Method '" __FUNCTION__ "' not implemented"));
    return v8::Handle<v8::Value>();
}

void v8_wrapper::CustomAttribute<js_html2_HTMLElement, 7 /*innerHTML*/>::static_set(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    HandleScope scope;
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    v8_wrapper::tree_node* node_ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
    js_html2_HTMLElement* ptr = dynamic_cast<js_html2_HTMLElement*>(node_ptr);
	LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), _T("v8 JavaScript binded call 0x") << std::hex << (size_t)ptr << _T(" setter ") << std::string(__FUNCTION__));

    webEngine::html_entity_ptr wrapped(new webEngine::html_entity());
    wrapped->Parse("", webEngine::tag_scanner(webEngine::str_tag_stream(v8_wrapper::Get<html2::DOMString>(value).c_str())));
    ptr->clear_children();
    wrap_dom( wrapped,  ptr->shared_from_this());

    //v8::ThrowException(v8::String::New("Method '" __FUNCTION__ "' not implemented"));
}

v8::Handle<v8::Value> v8_wrapper::CustomAttribute<js_html2_HTMLDocument, 12 /*innerHTML*/>::static_get(v8::Local<v8::String> property, const v8::AccessorInfo &info)
{
    v8::ThrowException(v8::String::New("Method '" __FUNCTION__ "' not implemented"));
    return v8::Handle<v8::Value>();
}

void v8_wrapper::CustomAttribute<js_html2_HTMLDocument, 12 /*innerHTML*/>::static_set(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    v8::ThrowException(v8::String::New("Method '" __FUNCTION__ "' not implemented"));
}

v8::Handle<v8::Value> v8_wrapper::CustomAttribute<js_dom_Node, 6 /*firstChild*/>::static_get(v8::Local<v8::String> property, const v8::AccessorInfo &info)
{
    HandleScope scope;
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    v8_wrapper::tree_node* node_ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
    js_html2_HTMLElement* ptr = dynamic_cast<js_html2_HTMLElement*>(node_ptr);
	LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), _T("v8 JavaScript binded call 0x") << std::hex << (size_t)ptr << _T(" getter ") << std::string(__FUNCTION__));

    v8::Handle<v8::Value> result;
    if(!node_ptr->m_child_list.empty())
        result = (*(node_ptr->m_child_list.begin()))->m_this;
    return result;
}

void v8_wrapper::CustomAttribute<js_dom_Node, 6 /*firstChild*/>::static_set(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    v8::ThrowException(v8::String::New("Method '" __FUNCTION__ "' not implemented"));
}
