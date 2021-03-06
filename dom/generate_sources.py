import sys
import os
import time
import xml.dom.minidom

dom_dir_source = sys.argv[1] # ${DOM_DIRECTORY_SOURCE} 
dom_dir_tmp = sys.argv[2] # ${DOM_DIRECTORY_TMP}
dom_dir_implemented = sys.argv[3] # ${DOM_IMPLEMENTED_SOURCE} 
dom_dir_not_implemented = sys.argv[4] # ${DOM_NOT_IMPLEMENTED_SOURCE}

implemented_classes = []
for source in os.listdir(dom_dir_implemented):
    if source.endswith(".cpp"):
        implemented_classes.append(source[:-4])

idl_header = "html.h"
gccxml = xml.dom.minidom.parse(dom_dir_tmp + "/html.xml")
idl_header_id = None

nodes = {}

out_idl_source = open(dom_dir_source + "/html.cpp", 'w')
out_js_header  = open(dom_dir_source + "/html_js.h", 'w')
out_js_source  = open(dom_dir_source + "/html_js.cpp", 'w')
out_tags_header = open(dom_dir_source + "/html_tags_wrapper.h", 'w')

os.utime(os.path.dirname(sys.argv[0]) + "/CMakeLists.txt", None)

generated_files_head = """
/*
  DO NOT EDIT!
  This file has been generated by """ + os.path.basename(sys.argv[0]) + """ script.
  $Id$
*/
"""

timestamp_guard = "" #time.strftime("%Y_%m_%d__%H_%M_%S", time.localtime())

out_js_header.write("#ifndef __js_header_" + timestamp_guard +"__\n")
out_js_header.write("#define __js_header_" + timestamp_guard +"__\n")
out_js_header.write(generated_files_head)
out_js_header.write("""
#include <html.h>
#include <v8_wrapper.h>

""")

out_js_source.write(generated_files_head)
out_js_source.write("""
#include "precomp.h"

using namespace v8;

""")

out_idl_source.write(generated_files_head)
out_idl_source.write("""
#include "precomp.h"

""")

out_tags_header.write(generated_files_head)
out_tags_header.write("#ifndef __tags_header_" + timestamp_guard +"__\n")
out_tags_header.write("#define __tags_header_" + timestamp_guard +"__\n")
out_tags_header.write("""

template <class T, bool generated>
v8_wrapper::tree_node_ptr inline TreeNodeFromEntity(webEngine::html_entity_ptr objToWrap){
    return TreeNodeFromEntity<T, generated>(objToWrap, boost::shared_ptr<T>());
}

template <class T, bool generated>
v8_wrapper::tree_node_ptr TreeNodeFromEntity(webEngine::html_entity_ptr objToWrap, v8_wrapper::tree_node_ptr node)
{
    if(!node){
        node.reset(new T());
        node->m_this = v8::Persistent<v8::Object>::New(v8_wrapper::wrap_object< T >(node.get()));
        //node->m_this.MakeWeak( NULL , v8_wrapper::Registrator< T >::Destructor);
        node->m_tag = objToWrap->HtmlTag();
    }
    return node;
}

""")

def is_type_fundamental(node):
    result = False
    if node.tagName == "CvQualifiedType":
        node = nodes[node.getAttribute('type')]
    if node.tagName == "FundamentalType":
        result = True
    return result

def get_type_str(node):
    type_prefix = ""
    if node.tagName == "CvQualifiedType":
        if node.getAttribute("const") == "1":
            type_prefix = "const "
        node = nodes[node.getAttribute('type')]
    type_str = node.getAttribute('name')
    if node.getAttribute("context"):
        ctx = nodes[node.getAttribute("context")]
        if ctx.getAttribute('name') != "::":
            type_str = ctx.getAttribute('name') + "::" + type_str
    return type_prefix + type_str

def generate_class(class_node):
    c = class_node.getAttribute('demangled').split("::")
    c_js = "js_" + c[0] + "_" + c[1]
    c_name = class_node.getAttribute('demangled')

    out_virt_stub = None
    out_virt_stub_head = 0
    class_not_implemented = not c_js in implemented_classes

    parent_js = None

    for b in class_node.childNodes:
        if b.nodeType == b.ELEMENT_NODE and b.tagName == 'Base' and b.getAttribute('type') in nodes:
            b_type = nodes[b.getAttribute('type')]
            if b_type.getAttribute('file') == idl_header_id:
                bt = b_type.getAttribute('demangled').split("::")
                parent_js = "js_" + bt[0] + "_" + bt[1]
                break

    out_js_header.write("class " + c_js + " : public virtual " + c_name)
    if parent_js:
        out_js_header.write(", public " + parent_js)
    out_js_header.write(", public v8_wrapper::Registrator< " + c_js + " > {\n public: \n")

    #list methods
    for node in gccxml.getElementsByTagName('Method'):
        if node.getAttribute('context') == class_node.getAttribute('id'):
            method_name = node.getAttribute('name')
            out_js_header.write("static v8::Handle<v8::Value> static_" + method_name + "(const v8::Arguments& args);\n")

            out_js_source.write("v8::Handle<v8::Value> " + c_js + "::static_" + method_name + """(const v8::Arguments& args){
              HandleScope scope;
              Local<Object> self = args.This();\
              Handle<Value> retval;
              Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
              v8_wrapper::tree_node* ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
              LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), _T("v8 JavaScript binded call 0x") << std::hex << (size_t)ptr << _T(" method ") << std::string(__FUNCTION__) );
            """)
        
            num_args = 0
            arg_list = ""
            full_arg_list = ""
            method_result = ""
            arguments = node.getElementsByTagName('Argument')
            sorted(arguments, key=lambda cl_: int(cl_.getAttribute('line')))
            for a in arguments:
                argname = a.getAttribute('name')
                argtype = get_type_str(nodes[a.getAttribute("type")])
                out_js_source.write("    " + argtype + " val_" + argname + " = v8_wrapper::Get< " + argtype + " > ( args[" + str(num_args) + "] );\n")
                if arg_list == "":
                    arg_list = "val_" + argname
                    full_arg_list = argtype + " val_" + argname
                else: 
                    arg_list += ", val_" + argname
                    full_arg_list += ", " + argtype + " val_" + argname
                num_args += 1
              
            out_js_source.write( c_js + " * el = dynamic_cast<" + c_js + " *>(ptr);\n" )

            if nodes[node.getAttribute('returns')].getAttribute('name') == 'void':
                out_js_source.write("  el->" + method_name + "(" + arg_list + ");\n")
            else:
                out_js_source.write("  retval = v8_wrapper::Set( el->" + method_name + "(" + arg_list + ") );\n")
                if is_type_fundamental(nodes[node.getAttribute('returns')]):
                    method_result = "0"
                else:
                    method_result = get_type_str(nodes[node.getAttribute('returns')]) + "()"
            out_js_source.write("  return scope.Close(retval);\n")
            out_js_source.write("}\n\n")

            virt_method_result = get_type_str(nodes[node.getAttribute('returns')])
            virt_method =  virt_method_result + " " + c_js + "::" + method_name + "(" + full_arg_list + ")"
            out_js_header.write( "virtual " + virt_method_result + " " + method_name + "(" + full_arg_list + ");\n")
            if class_not_implemented:
                if not out_virt_stub:
                    out_virt_stub = open(dom_dir_not_implemented + "/" + c_js + ".cpp", 'w')
                if not out_virt_stub_head:
                   out_virt_stub.write(generated_files_head)
                   out_virt_stub.write("""
                       #include "precomp.h"
                       using namespace v8;

                       """)
                   out_virt_stub_head = 1
                if nodes[node.getAttribute('returns')].getAttribute('name') == 'void':
                    out_virt_stub.write( virt_method + " { " + c_name + "::" + method_name + "(" + arg_list + ");}\n")
                else:
                    out_virt_stub.write( virt_method + " { return " + c_name + "::" + method_name + "(" + arg_list + ");}\n")
            out_idl_source.write(get_type_str(nodes[node.getAttribute('returns')]) + " " + c_name + "::" + method_name + "(" + full_arg_list + ") " + 
            " { v8::ThrowException(v8::String::New((std::string(\"Method '\") +  __FUNCTION__ + \"' not implemented\").c_str())); return " + method_result + " ;}\n")
            

    field_list = ""

    out_tags_header.write("""
    template <>
    v8_wrapper::tree_node_ptr TreeNodeFromEntity< """ + c_js + """, true>(webEngine::html_entity_ptr objToWrap, v8_wrapper::tree_node_ptr node_ptr){
        if(!node_ptr){
            node_ptr.reset(new """ + c_js + """());
            node_ptr->m_this = v8::Persistent<v8::Object>::New(v8_wrapper::wrap_object< """ + c_js + """ >(node_ptr.get()));
            //node_ptr->m_this.MakeWeak( NULL , v8_wrapper::Registrator< """ + c_js + """ >::Destructor);
            node_ptr->m_tag = objToWrap->HtmlTag();
        }
        """ + c_js + """* node = dynamic_cast<""" + c_js + """*>(node_ptr.get());
    """)

    virtual_get_fields = """
    virtual const std::string get_fields() {
        std::ostringstream _out;
    """

    field_number = 0
    for field in gccxml.getElementsByTagName('Field'):
        if field.getAttribute('context') == class_node.getAttribute('id'):
            field_number += 1
            field_name = field.getAttribute('name')
            field_type = get_type_str(nodes[field.getAttribute("type")])
            if field_list == "":
                field_list += ": " + field_name + "()"
            else :
                field_list += ", " + field_name + "()"
            out_js_header.write("static v8::Handle<v8::Value> static_get_" + field_name + "(v8::Local<v8::String> property, const v8::AccessorInfo& info);\n")
            out_js_header.write("static void static_set_" + field_name + "(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);\n")
            out_js_source.write("""
                Handle<Value> """ + c_js + """::static_get_""" + field_name + """(Local<String> property, const AccessorInfo &info) {
                    if(v8_wrapper::CustomAttribute<""" + c_js + ", " + str(field_number) + """>::implemented) {
                        return v8_wrapper::CustomAttribute<""" + c_js + ", " + str(field_number) + """>::static_get(property, info);
                    }
                Local<Object> self = info.Holder();
                Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
                    v8_wrapper::tree_node* ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
                    LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), _T("v8 JavaScript binded call 0x") << std::hex << (size_t)ptr << _T(" getter ") << std::string(__FUNCTION__) );
                """ + field_type + " value = dynamic_cast<""" + c_js + "*>(ptr)->" + field_name + """;
                return v8_wrapper::Set(value);
              }

              void """ + c_js + """::static_set_""" + field_name + """(Local<String> property, Local<Value> value,
                             const AccessorInfo& info) {
                    if(v8_wrapper::CustomAttribute<""" + c_js + ", " + str(field_number) + """>::implemented) {
                        v8_wrapper::CustomAttribute<""" + c_js + ", " + str(field_number) + """>::static_set(property, value, info);
                        return;
                    }""")
            if nodes[field.getAttribute("type")].getAttribute("const") == "1":
                out_js_source.write("}\n")
            else:
                out_js_source.write("""
                Local<Object> self = info.Holder();
                Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
                            v8_wrapper::tree_node* ptr = static_cast<v8_wrapper::tree_node*>(wrap->Value());
                            LOG4CXX_TRACE(webEngine::iLogger::GetLogger(), _T("v8 JavaScript binded call 0x") << std::hex << (size_t)ptr << _T(" setter ") << std::string(__FUNCTION__) );
                dynamic_cast<""" + c_js + "*>(ptr)->" + field_name + """ = v8_wrapper::Get< """ + field_type + """ >(value);
              }
              """)
                if not field_type.startswith("v8::"):
                    out_tags_header.write("try{")
                    out_tags_header.write("webEngine::AttrMap::iterator attr = objToWrap->attr_list().find(\"" + field_name + "\");\n ")
                    out_tags_header.write("if(attr != objToWrap->attr_list().end()){")
                    out_tags_header.write("node->" + field_name + " = boost::lexical_cast< " + field_type + " > ( (*attr).second );}\n");
                    out_tags_header.write("}catch(boost::bad_lexical_cast &){LOG4CXX_ERROR(webEngine::iLogger::GetLogger(), " + 
                    "_T(\"Could not cast '\") <<  objToWrap->attr_list()[\"" + field_name + "\"] << _T(\"' to " + field_type + "\"));}\n")
            if field_type.find("v8::") == -1:
                if field_type.find("tring") != -1:
                    virtual_get_fields += " if(!" + field_name + ".empty())\n"
                elif field_type.find("bool") == -1:
                    virtual_get_fields += " if(" + field_name + " != 0)\n"
                virtual_get_fields += """_out << " """ + field_name + """=\\"" << """ + field_name + """ << "\\" ";\n"""
    if parent_js:
        virtual_get_fields += "_out << " + parent_js + "::get_fields();\n"

    virtual_get_fields += "return _out.str();}\n\n"
    out_js_header.write(virtual_get_fields)

    if out_virt_stub:
        out_js_header.write(c_js + "();\n")
        out_js_header.write("virtual ~" + c_js + "();\n")
        out_virt_stub.write("\n" + c_js + "::" + c_js + "(){}\n")
        out_virt_stub.write("\n" + c_js + "::~" + c_js + "(){}\n")
        out_virt_stub.close()
        out_virt_stub = None
    elif class_not_implemented:
        out_js_header.write(c_js + "(){}\n")
        out_js_header.write("virtual ~" + c_js + "(){}\n")
    else:
        out_js_header.write(c_js + "();\n")
        out_js_header.write("virtual ~" + c_js + "();\n")

    out_js_header.write(" };\n\n")
    
    out_idl_source.write( c_name + "::" + c[1] + "()" + field_list + "{}\n" )
    
    out_js_source.write("""
    namespace v8_wrapper{
    template <>
    v8::Persistent<v8::FunctionTemplate> Registrator< """ + c_js + """ >::GetTemplate() {
    static v8::Persistent<v8::FunctionTemplate> cachedTemplate;
    if (!cachedTemplate.IsEmpty())
        return cachedTemplate;

    v8::HandleScope scope;
    v8::Local<v8::FunctionTemplate> result = v8::FunctionTemplate::New();

    v8::Local<v8::ObjectTemplate> instance = result->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> proto = result->PrototypeTemplate();
    instance->SetInternalFieldCount(1);
    result->SetClassName(v8::String::New(\"""" + c_name + "\")); ")

    if parent_js:
        out_js_source.write("result->Inherit(v8_wrapper::Registrator< " + parent_js + " >::GetTemplate());\n")
        out_tags_header.write("TreeNodeFromEntity< " + parent_js + ", true >(objToWrap, node_ptr);\n")

    out_tags_header.write("""
        return node_ptr;
    }\n\n""")

    for node in gccxml.getElementsByTagName('Method'):
        if node.getAttribute('context') == class_node.getAttribute('id'):
            method_name = node.getAttribute('name')
            out_js_source.write("proto->Set(v8::String::New(\"" + method_name + "\"), v8::FunctionTemplate::New(" + c_js + "::static_" + method_name + "));")

    for field in gccxml.getElementsByTagName('Field'):
        if field.getAttribute('context') == class_node.getAttribute('id'):
            field_name = field.getAttribute('name')
            out_js_source.write("instance->SetAccessor(v8::String::New(\"" + field_name + "\"), " + c_js + "::static_get_" + field_name + ", " + c_js + "::static_set_" + field_name + ");\n")
    out_js_source.write("""
    v8_wrapper::Registrator< """ + c_js + """ >::AdditionalHandlersGetTemplate(instance, proto);
    cachedTemplate = v8::Persistent<v8::FunctionTemplate>::New(result);
    return cachedTemplate;
    }\n}\n\n""")
    
for node in gccxml.getElementsByTagName('Class'):
    nodes[node.getAttribute('id')] = node
for node in gccxml.getElementsByTagName('Typedef'):
    nodes[node.getAttribute('id')] = node
for node in gccxml.getElementsByTagName('Struct'):
    nodes[node.getAttribute('id')] = node
for node in gccxml.getElementsByTagName('FundamentalType'):
    nodes[node.getAttribute('id')] = node
for node in gccxml.getElementsByTagName('CvQualifiedType'):
    nodes[node.getAttribute('id')] = node
for node in gccxml.getElementsByTagName('Namespace'):
    nodes[node.getAttribute('id')] = node


for node in gccxml.getElementsByTagName('File'):
    if node.getAttribute('name').endswith(idl_header):
        idl_header_id = node.getAttribute('id')

classes = gccxml.getElementsByTagName('Class')
#classes.sort(key=lambda cl_: cl_.getAttribute('name'))
classes.sort(key=lambda cl_: int(cl_.getAttribute('line')))

print("Found " + str(len(classes)) + " classes")

#for node in classes:
#    if node.getAttribute('file') == idl_header_id:
#        c = node.getAttribute('demangled').split("::")
#        out_js_header.write("class js_" + c[0] + "_" + c[1] + ";\n")

for node in classes:
    if node.getAttribute('file') == idl_header_id:
        generate_class(node)

out_js_source.write("\n void v8_wrapper::RegisterAll(v8::Persistent<v8::ObjectTemplate> global) {\n")
for node in classes:
    if node.getAttribute('file') == idl_header_id:
        c = node.getAttribute('demangled').split("::")
        c_js = "js_" + c[0] + "_" + c[1]
        out_js_source.write("global->Set(String::New(\"" + c[1] + "\"), \nFunctionTemplate::New(v8_wrapper::Registrator< " + c_js + " >::Constructor));\n")
out_js_source.write("} \n")

out_js_header.write("#endif\n\n")
out_tags_header.write("#endif\n\n")

out_idl_source.close()
out_js_header.close()
out_js_source.close()
out_tags_header.close()
