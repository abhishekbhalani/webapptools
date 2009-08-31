#ifndef WIONELINEREPORT_H
#define WIONELINEREPORT_H

#include "wiReportData.h"


class wiLineReport : public wiReportData
{
    public:
        wiLineReport(wxString msg = wxT(""), int lvl = 0);
        virtual ~wiLineReport();

        virtual void  WriteReport(wxRichTextCtrl& rt);
    protected:
        wxString text;
        int level;
};

#endif // WIONELINEREPORT_H
