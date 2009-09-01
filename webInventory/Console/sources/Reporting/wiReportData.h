/*
    webInventory is the web-application audit programm
    Copyright (C) 2009 Andrew Abramov stinger911@gmail.com

    This file is part of webInventory

    webInventory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webInventory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webInventory.  If not, see <http://www.gnu.org/licenses/>.
*/
/***************************************************************
 * @file      wiReportData.h
 * @brief     Declaration of the Report classes
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      06.08.2009
 **************************************************************/
#ifndef WIREPORTDATA_H
#define WIREPORTDATA_H

#include <wx/richtext/richtextctrl.h>
#include <wx/xml/xml.h>

#define REPORT_STYLE_NODE   wxT("style")
#define REPORT_STYLE_NAME   wxT("name")
#define REPORT_STYLE_TEXT   wxT("text")
#define REPORT_TOKEN_MARK   wxT("\x02")
#define REPORT_TOK_MARK     wxT('\x02')
#define REPORT_PERCENTS     wxT("\x03")
#define REPORT_NUMBERS      wxT("\x04")
#define REPORT_SUBST        wxT("%s")

#define REPORT_IMAGE_TOKEN  wxT("img=")
#define REPORT_CLEAR_TOKEN  wxT("cls")

class wiReportData
{
    public:
        wiReportData();
        virtual ~wiReportData();

        virtual void WriteReport(wxRichTextCtrl& rt) = 0;

        virtual bool LoadTemplate(const wxString& fName);
        virtual bool TextTemplate(const wxString& text);
        virtual void XmlTemplate(const wxXmlDocument& xml);

        bool GetStyleAttr(const wxString& style, const wxString& attr, wxString& val, const wxString& def = wxT(""));
        void AddFormattedText(const wxString& style, const wxString& val, wxRichTextCtrl& rt);
        void AddStyledText(const wxString& style, const wxString& val, wxRichTextCtrl& rt);
        virtual void ProcessToken(const wxString& style, const wxString& token, wxRichTextCtrl& rt);

    protected:
        wxXmlDocument* outputTemplate;
};

#endif // WIREPORTDATA_H
