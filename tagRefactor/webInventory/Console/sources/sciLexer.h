#ifndef SCILEXER_H_INCLUDED
#define SCILEXER_H_INCLUDED

typedef struct
{
	int		styleCode;
	wxChar	styleColor[10];
} DealLexerColors;

#define LEXER_LAST_RECORD	-1

DealLexerColors htmlLexerColors[] = {
	{wxSCI_H_DEFAULT,			wxT("#000000") },
	{wxSCI_H_TAG,				wxT("#000080") },
	{wxSCI_H_TAGUNKNOWN,		wxT("#000080") },
	{wxSCI_H_ATTRIBUTE,			wxT("#008080") },
	{wxSCI_H_ATTRIBUTEUNKNOWN,	wxT("#008080") },
	{wxSCI_H_NUMBER,			wxT("#c000c0") },
	{wxSCI_H_DOUBLESTRING,		wxT("#40c040") },
	{wxSCI_H_SINGLESTRING,		wxT("#40c040") },
	{wxSCI_H_OTHER,				wxT("#700000") },
	{wxSCI_H_COMMENT,			wxT("#c0c0c0") },
	{wxSCI_H_ENTITY,			wxT("#20a020") },
	{wxSCI_H_TAGEND,			wxT("#000080") },
	{wxSCI_H_XMLSTART,			wxT("#000080") },
	{wxSCI_H_XMLEND,			wxT("#000080") },
	{wxSCI_H_SCRIPT,			wxT("#700000") },
	{wxSCI_H_ASP,				wxT("#700000") },
	{wxSCI_H_ASPAT,				wxT("#700000") },
	{wxSCI_H_CDATA,				wxT("#800060") },
	{wxSCI_H_QUESTION,			wxT("#000080") },
	{wxSCI_H_VALUE,				wxT("#20a0a0") },

	{wxSCI_HJ_DEFAULT,			wxT("#b0b000") },
	{wxSCI_HJA_DEFAULT,			wxT("#b0b000") },
	{wxSCI_HJ_WORD,				wxT("#c0c000") },
	{wxSCI_HJ_KEYWORD,			wxT("#c0c000") },
	{wxSCI_HJA_KEYWORD,			wxT("#c0c000") },
	{wxSCI_HJ_NUMBER,			wxT("#c000c0") },
	{wxSCI_HJA_NUMBER,			wxT("#c000c0") },
	{wxSCI_HJ_DOUBLESTRING,		wxT("#40c040") },
	{wxSCI_HJA_DOUBLESTRING,	wxT("#40c040") },
	{wxSCI_HJ_SINGLESTRING,		wxT("#40c040") },
	{wxSCI_HJA_SINGLESTRING,	wxT("#40c040") },

	{LEXER_LAST_RECORD,			wxT("")	}
};

wxChar keywordsHTML[] = wxT("a abbr acronym address applet area b base basefont \
bdo big blockquote body br button caption center \
cite code col colgroup dd del dfn dir div dl dt em \
fieldset font form frame frameset h1 h2 h3 h4 h5 h6 \
head hr html i iframe img input ins isindex kbd label \
legend li link map menu meta noframes noscript \
object ol optgroup option p param pre q s samp \
script select small span strike strong style sub sup \
table tbody td textarea tfoot th thead title tr tt u ul \
var xml xmlns !doctype");

wxChar keywordsJavaScript[] = wxT("abstract boolean break byte case catch char class \
const continue debugger default delete do double else enum export extends \
final finally float for function goto if implements import in instanceof \
int interface long native new package private protected public \
return short static super switch synchronized this throw throws \
transient try typeof var void volatile while with");

#endif // SCILEXER_H_INCLUDED
