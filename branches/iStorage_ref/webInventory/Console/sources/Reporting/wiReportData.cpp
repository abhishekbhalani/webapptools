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
 * @file      wiReportData.cpp
 * @brief     Defines wiLineReport class
 * @author    Andrew "Stinger" Abramov (stinger@lab18.net)
 * @date      06.08.2009
 **************************************************************/
#include "wiReportData.h"
#include <wx/sstream.h>
#include <wx/tokenzr.h>
#include <wx/log.h>
#include <wx/filename.h>

wiReportData::wiReportData()
{
    outputTemplate = NULL;
}

wiReportData::~wiReportData()
{
    if (outputTemplate != NULL) {
        delete outputTemplate;
    }
}

bool wiReportData::LoadTemplate(const wxString& fName)
{
    if (outputTemplate != NULL) {
        delete outputTemplate;
    }
    outputTemplate = new wxXmlDocument();
    return outputTemplate->Load(fName, wxT("UTF-8"));
}

bool wiReportData::TextTemplate(const wxString& text)
{
    wxStringInputStream strInput(text);

    if (outputTemplate != NULL) {
        delete outputTemplate;
    }
    outputTemplate = new wxXmlDocument();
    return outputTemplate->Load(strInput, wxT("UTF-8"));
}

void wiReportData::XmlTemplate(const wxXmlDocument& xml)
{
    if (outputTemplate != NULL) {
        delete outputTemplate;
    }
    outputTemplate = new wxXmlDocument(xml);
}

bool wiReportData::GetStyleAttr(const wxString& style, const wxString& attr, wxString& val, const wxString& def/* = wxT("")*/)
{
    bool retval = false;
    wxXmlNode *styles;
    wxXmlNode *attribute;

    val = def;
    if (outputTemplate != NULL && outputTemplate->GetRoot() != NULL) {
        styles = outputTemplate->GetRoot()->GetChildren();
        while (styles != NULL && !retval) {
            if (styles->GetType() == wxXML_ELEMENT_NODE && styles->GetName() == REPORT_STYLE_NODE) {
                wxString stName;
                if (styles->GetPropVal(REPORT_STYLE_NAME, &stName)) {
                    if (stName == style) {
                        attribute = styles->GetChildren();
                        while (attribute != NULL && !retval) {
                            if (attribute->GetType() == wxXML_ELEMENT_NODE && attribute->GetName().CmpNoCase(attr) == 0) {
                                val = attribute->GetNodeContent();
                                retval = true;
                            }
                            attribute = attribute->GetNext();
                        } // end of attribute search
                    }
                } // if requested style
            } // if style record

            styles = styles->GetNext();
        } // end of style search
    }
    return retval;
}

void wiReportData::AddStyledText(const wxString& style, const wxString& val, wxRichTextCtrl& rt)
{
    wxString attrib;
    long    iData;
    bool    isBold, isItalic, isUnder, isFontSize, isFontColor, isAlignment;

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

    rt.WriteText(val);

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

void wiReportData::AddFormattedText(const wxString& style, const wxString& val, wxRichTextCtrl& rt)
{
    bool retval = false;
    wxXmlNode *styles;
    wxXmlNode *attribute;
    wxString  format;

    if (outputTemplate != NULL && outputTemplate->GetRoot() != NULL) {
        styles = outputTemplate->GetRoot()->GetChildren();
        while (styles != NULL && !retval) {
            if (styles->GetType() == wxXML_ELEMENT_NODE && styles->GetName() == REPORT_STYLE_NODE) {
                wxString stName;
                if (styles->GetPropVal(REPORT_STYLE_NAME, &stName)) {
                    if (stName == style) {
                        attribute = styles->GetChildren();
                        while (attribute != NULL && !retval) {
                            if (attribute->GetType() == wxXML_ELEMENT_NODE && attribute->GetName().CmpNoCase(REPORT_STYLE_TEXT) == 0) {
                                format = attribute->GetNodeContent();
                                retval = true;
                            }
                            attribute = attribute->GetNext();
                        } // end of attribute search
                    }
                } // if requested style
            } // if style record

            styles = styles->GetNext();
        } // end of style search
    }
    if (!retval) {
        AddStyledText(style, val, rt);
    }
    else {
        // perform styled output
        wxString outText;
        wxString tok;
        wxString part;

        // replace format sequesces
        tok.Replace(wxT("\\r"), wxT(""), true);
        tok.Replace(wxT("\\n"), wxT("\n"), true);
        tok.Replace(wxT("\\t"), wxT("\t"), true);

        wxStringTokenizer tokenizer(format, wxT(" \t\r\n"), wxTOKEN_RET_EMPTY_ALL);
        outText.Empty();
        while (tokenizer.HasMoreTokens()) {
            tok = tokenizer.GetNextToken();
            if (tok.IsEmpty()) {
                outText += tokenizer.GetLastDelimiter();
            }
            else {
                AddStyledText(style, outText, rt);
                outText.Empty();
                // replace symbols to special marks
                tok.Replace(wxT("##"), REPORT_NUMBERS, true);
                tok.Replace(wxT("%%"), REPORT_PERCENTS, true);
                tok.Replace(wxT("#"), REPORT_TOKEN_MARK, true);
                // perfom substitution
                tok.Replace(REPORT_SUBST, val, true);
                // put back single symbols
                tok.Replace(REPORT_NUMBERS, wxT("#"), true);
                tok.Replace(REPORT_PERCENTS, wxT("%"), true);

                // process string by text and token names
                part = tok.BeforeFirst(REPORT_TOK_MARK);
                tok = tok.Mid(part.Len());
                if (!part.IsEmpty()) {
                    AddStyledText(style, part, rt);
                }
                while (tok[0] == REPORT_TOK_MARK) {
                    // part is now the token name
                    tok = tok.Mid(1);
                    part = tok.BeforeFirst(REPORT_TOK_MARK);
                    tok = tok.Mid(part.Len());
                    ProcessToken(style, part, rt);
                }
                if (! tok.IsEmpty()) {
                    // strange case, but...
                    AddStyledText(style, tok, rt);
                }
                outText += tokenizer.GetLastDelimiter();
            }
        }
    }
}

void wiReportData::ProcessToken(const wxString& style, const wxString& token, wxRichTextCtrl& rt)
{
    // backdrop processing
    // process special cases first
    // all unprocessed markers will be assumed as style links
    wxString tkName = token;
    wxString val;

    if (tkName.StartsWith(REPORT_IMAGE_TOKEN, &val)) {
        if (!val.IsEmpty()) {
            wxFileName flName(val);
            wxImage img;
            img.LoadFile(flName.GetFullPath());
            if(img.IsOk()) {
                rt.AddImage(img);
            }
            rt.SetInsertionPointEnd();
        }
        // en of img token processing
        return;
    }
    if (tkName.StartsWith(REPORT_CLEAR_TOKEN, &val)) {
        rt.Clear();
        return;
    }
    // unprocessed token - try to add style-formatter
    AddFormattedText(tkName, wxT(""), rt);
}
