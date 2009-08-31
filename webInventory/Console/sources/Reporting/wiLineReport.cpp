#include "wiLineReport.h"

wiLineReport::wiLineReport(wxString msg /*= wxT("")*/, int lvl /*= 0*/)
{
    text = msg;
    level = lvl;
    LoadTemplate(wxT("linereport.xml"));
}

wiLineReport::~wiLineReport()
{
    //dtor
}

void wiLineReport::WriteReport(wxRichTextCtrl& rt)
{
    wxString attrib;
    wxString style;
    long    iData;
    bool    isBold, isItalic, isUnder, isFontSize, isFontColor, isAlignment;

    rt.SetInsertionPointEnd();
    style = wxString::Format(wxT("level%d"), level);

    isAlignment = GetStyleAttr(style, wxT("align"), attrib, wxT("4"));
    if (!isAlignment) {
        isAlignment = GetStyleAttr(wxT("default"), wxT("align"), attrib, wxT("4"));
    }
    if (isAlignment) {
        attrib.ToLong(&iData);
        rt.BeginAlignment((wxTextAttrAlignment)iData);
    }

    isBold = GetStyleAttr(style, wxT("font_bold"), attrib, wxT(""));
    if (!isBold) {
        isBold = GetStyleAttr(wxT("default"), wxT("font_bold"), attrib, wxT(""));
    }
    if (isBold) {
        attrib.ToLong(&iData);
        isBold = (iData != 0);
        if (isBold) {
            rt.BeginBold();
        }
    }

    isItalic = GetStyleAttr(style, wxT("font_italic"), attrib, wxT(""));
    if (!isItalic) {
        isItalic = GetStyleAttr(wxT("default"), wxT("font_italic"), attrib, wxT(""));
    }
    if (isItalic) {
        attrib.ToLong(&iData);
        isItalic = (iData != 0);
        if (isItalic) {
            rt.BeginItalic();
        }
    }

    isUnder = GetStyleAttr(style, wxT("font_underline"), attrib, wxT(""));
    if (!isUnder) {
        isUnder = GetStyleAttr(wxT("default"), wxT("font_underline"), attrib, wxT(""));
    }
    if (isUnder) {
        attrib.ToLong(&iData);
        isUnder = (iData != 0);
        if (isUnder) {
            rt.BeginUnderline();
        }
    }

    isFontSize = GetStyleAttr(style, wxT("font_size"), attrib, wxT("10"));
    if (!isFontSize) {
        isFontSize = GetStyleAttr(wxT("default"), wxT("font_size"), attrib, wxT("10"));
    }
    if (isFontSize) {
        attrib.ToLong(&iData);
        rt.BeginFontSize(iData);
    }

    isFontColor = GetStyleAttr(style, wxT("font_color"), attrib, wxT("black"));
    if (!isFontColor) {
        isFontColor = GetStyleAttr(wxT("default"), wxT("font_color"), attrib, wxT("black"));
    }
    if (isFontColor) {
        rt.BeginTextColour(wxColor(attrib));
    }
    rt.WriteText(text);
    rt.WriteText(wxT("\n"));

    if (isFontColor) {
        rt.EndTextColour();
    }
    if (isFontSize) {
        rt.EndFontSize();
    }
    if (isUnder) {
        rt.EndUnderline();
    }
    if (isItalic) {
        rt.EndItalic();
    }
    if (isBold) {
        rt.EndBold();
    }
    if (isAlignment) {
        rt.EndAlignment();
    }
}
