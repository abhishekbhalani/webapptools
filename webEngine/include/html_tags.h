
#ifndef __HTML_TAGS__H__
#define __HTML_TAGS__H__

#include <string>
#include <boost/preprocessor.hpp>

#define HTML_TAGS_LIST \
(a)(abbr)(acronym)(address)(applet)(area) \
(b)(base)(basefont)(bdo)(big)(blockquote)(body)(br)(button) \
(caption)(center)(cite)(code)(col)(colgroup) \
(dd)(del)(dfn)(dir)(div)(dl)(dt) \
(em) \
(fieldset)(font)(form)(frame)(frameset) \
(h1)(h2)(h3)(h4)(h5)(h6)(head)(hr)(html) \
(i)(iframe)(img)(input)(ins)(isindex) \
(kbd) \
(label)(legend)(li)(link) \
(map)(menu)(meta) \
(noframes)(noscript) \
(object)(ol)(optgroup)(option) \
(p)(param)(pre) \
(q) \
(s)(samp)(script)(select)(small)(span)(strike)(strong)(style)(sub)(sup) \
(table)(tbody)(td)(textarea) \
(tfoot)(th)(thead)(title)(tr)(tt) \
(u)(ul) \
(var) \
(__UNKNOWN_TAG__)

#define __HTML_TAG_TO_NAME(unused,data,elem) BOOST_PP_CAT(data, elem) ,

/**
 * List of html tags
 */
enum HTML_TAG {
    BOOST_PP_SEQ_FOR_EACH(__HTML_TAG_TO_NAME,HTML_TAG_,HTML_TAGS_LIST)
};

/**
 * Get tag name by tag enum
 * @param HTML_TAG html_tag
 * @return const char*
 */
const char* get_tag_name(HTML_TAG html_tag);

/**
 * Get enum by tag name
 * @param const std::string & html_tag_name
 * @return HTML_TAG
 */
HTML_TAG find_tag_by_name(const std::string& html_tag_name);









#endif