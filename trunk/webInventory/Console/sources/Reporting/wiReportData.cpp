#include "wiReportData.h"
#include <wx/sstream.h>
#include <wx/log.h>

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
                                wxLogMessage(wxT("Found %s = %s"), attr.c_str(), val.c_str());
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
