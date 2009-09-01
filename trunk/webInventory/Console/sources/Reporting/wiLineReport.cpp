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
    wxString style;

    rt.SetInsertionPointEnd();
    style = wxString::Format(wxT("level%d"), level);
    AddFormattedText(style, text + wxT("\n"), rt);
}
