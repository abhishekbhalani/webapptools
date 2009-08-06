#include "wiReportData.h"

wiSimpleReport::wiSimpleReport(wxString msg /*= wxT("")*/, int lvl /*= 0*/)
{
    text = msg;
    level = lvl;
}

wiSimpleReport::~wiSimpleReport()
{
    //dtor
}

void wiSimpleReport::WriteReport(wxRichTextCtrl& rt)
{
    rt.Clear();
    rt.BeginAlignment(wxTEXT_ALIGNMENT_CENTER);
    rt.BeginBold();
    rt.BeginItalic();
    rt.BeginFontSize(36);
    if (level == 1) {
        rt.BeginTextColour(*wxRED);
    }
    rt.WriteText(text);
    if (level > 0) {
        rt.EndTextColour();
    }
    rt.EndFontSize();
    rt.EndItalic();
    rt.EndBold();
    rt.EndAlignment();
}
