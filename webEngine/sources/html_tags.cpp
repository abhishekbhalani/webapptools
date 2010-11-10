
#include "html_tags.h"
#include <boost/algorithm/string/case_conv.hpp>

namespace {
#define TO_STR(unused,data,elem) BOOST_PP_STRINGIZE(elem) ,
const char * HTML_TAGS_LIST_strs[]= { BOOST_PP_SEQ_FOR_EACH(TO_STR,~,HTML_TAGS_LIST) };
#undef TO_STR
}

const char* get_tag_name(HTML_TAG html_tag)
{
    return HTML_TAGS_LIST_strs[html_tag];
}

HTML_TAG find_tag_by_name(const std::string& html_tag_name)
{
    HTML_TAG result = HTML_TAG___UNKNOWN_TAG__;
    std::string tag_name = boost::to_lower_copy(html_tag_name);
    for(size_t i = 0; i < HTML_TAG___UNKNOWN_TAG__; ++i) {
        if(tag_name == HTML_TAGS_LIST_strs[i]) {
            result = (HTML_TAG)i;
        }
    }
    return result;
}
