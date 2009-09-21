#include <wx/intl.h>
#include <wx/mstream.h>
#include <wx/log.h>
#include "wiPluginSettings.h"

wiPluginSettings::wiPluginSettings( wxWindow* parent, const wxString& description ) :
    PluginSettings( parent )
{
    wxCharBuffer buff = description.utf8_str();
    int buffLen = strlen(buff.data());
    wxMemoryInputStream xmlStream(buff.data(), buffLen);

    m_props->RegisterAdditionalEditors();
    wxLogNull noLog;
    if(structure.Load(xmlStream)) {
        RebuildView();
    }
    else {
        m_props->Append( new wxPropertyCategory(_("Plugin doesn't provide settings"), wxT("generic")));
        m_props->CollapseAll();
        m_props->ExpandAll();
    }
    m_props->SetValidationFailureBehavior( wxPG_VFB_BEEP | wxPG_VFB_MARK_CELL | wxPG_VFB_SHOW_MESSAGE );
}

void wiPluginSettings::RebuildView()
{
    wxString currCatId;
    wxString label;

    m_props->Clear();

    wxXmlNode *root = structure.GetRoot();
    if (root && root->GetName().CmpNoCase(wxT("plugin")) == 0) {
        if ( root->GetPropVal(wxT("name"), &defaultSpace) ) {
            defaultSpace = wxString::Format(_("%s plugin settings"), defaultSpace.c_str());
        }
        else {
            defaultSpace = _("Generic settings");
        }
        wxXmlNode *chld = root->GetChildren();
        currCatId = wxT("generic");
        defCount = 0;
        defSpaceUsed = false;

        while (chld != NULL) {
            if (chld->GetName().CmpNoCase(wxT("category")) == 0) {
                if ( chld->GetPropVal(wxT("name"), &label) ) {
                    currCatId = label;
                }
                else {
                    currCatId = wxString::Format(wxT("category%04d"), defCount++);
                }
                if ( ! chld->GetPropVal(wxT("label"), &label) ) {
                    label = currCatId;
                }
                label = wxGetTranslation(label.c_str());
                m_props->Append( new wxPropertyCategory(label, currCatId));
                // set flag if category marked for generic settings
                if (currCatId == wxT("generic") ) {
                    defSpaceUsed = true;
                }
                wxXmlNode* nested = chld->GetChildren();
                ProcessCategory(nested, currCatId);
            }
            if (chld->GetName().CmpNoCase(wxT("option")) == 0) {
                ProcessOption(chld, currCatId);
            }

            chld = chld->GetNext();
        }
    }
    else {
        m_props->Append( new wxPropertyCategory(_("Plugin doesn't provide settings"), wxT("generic")));
    }

    m_props->CollapseAll();
    m_props->ExpandAll();
}

void wiPluginSettings::ProcessCategory(wxXmlNode* node, const wxString& catId)
{
    wxString currCatId = catId;
    wxString label;

    while (node != NULL) {
        if (node->GetName().CmpNoCase(wxT("category")) == 0) {
            if ( node->GetPropVal(wxT("name"), &label) ) {
                currCatId = label;
            }
            else {
                currCatId = wxString::Format(wxT("category%04d"), defCount++);
            }
            if ( ! node->GetPropVal(wxT("label"), &label) ) {
                label = currCatId;
            }
            label = wxGetTranslation(label.c_str());
            // set flag if category marked for generic settings
            if (currCatId == wxT("generic") ) {
                defSpaceUsed = true;
                m_props->Append( new wxPropertyCategory(label, currCatId));
            }
            else {
                m_props->Insert( catId, -1, new wxPropertyCategory(label, currCatId));
            }
            wxXmlNode* nested = node->GetChildren();
            ProcessCategory(nested, currCatId);
        }
        if (node->GetName().CmpNoCase(wxT("option")) == 0) {
            ProcessOption(node, catId);
        }

        node = node->GetNext();
    }
}

void wiPluginSettings::ProcessOption(wxXmlNode* node, const wxString& catId)
{
    wxString label;
    wxString name;
    wxString category;
    wxString defValue;
    wxString controlType;
    wxPGProperty* pid;

    if ( ! node->GetPropVal(wxT("name"), &name) || name.IsEmpty() ) {
        name = wxString::Format(wxT("/AutoOption %d"), defCount++);
    }

    if ( ! node->GetPropVal(wxT("label"), &label) ) {
        label = name;
    }
    label = wxGetTranslation(label.c_str());

    if ( ! node->GetPropVal(wxT("category"), &category) ) {
        category = catId;
        if (catId == wxT("generic") && defSpaceUsed == false) {
            m_props->Append( new wxPropertyCategory(defaultSpace, wxT("generic")));
            defSpaceUsed = true;
        }
    }
    defValue = node->GetNodeContent();
    // get type and others
    if ( ! node->GetPropVal(wxT("control"), &controlType) ) {
        controlType = CT_STRING;
    }

    // add to grid
    if (controlType == CT_ENUM) {
        wxArrayString labels;
        wxString elem;
        int pos;

        pos = defValue.Find(wxT(';'));
        while (pos != wxNOT_FOUND) {
            elem = defValue.Mid(0, pos);
            labels.Add(elem);
            if ( pos < defValue.Len() ) {
                defValue = defValue.Mid(pos + 1);
            }
            else {
                defValue.Empty();
            }
            pos = defValue.Find(wxT(';'));
        }
        if ( !defValue.IsEmpty()) {
            labels.Add(defValue);
        }
        m_props->Insert( category, -1, new wxEnumProperty(label, name, labels) );
    }
    else if (controlType == CT_INT) {
        wxString attr;
        int useSpin;
        wxVariant vval;
        long vmin, vmax;

        vval = defValue;
        pid = m_props->Insert( category, -1, new wxIntProperty(label, name, vval) );
        useSpin = 0;
        if ( node->GetPropVal(wxT("min"), &attr) ) {
            attr.ToLong(&vmin);
            useSpin++;
        }
        if ( node->GetPropVal(wxT("max"), &attr) ) {
            attr.ToLong(&vmax);
            useSpin++;
        }
        if (useSpin == 2) {
            m_props->SetPropertyEditor(pid, wxPG_EDITOR(SpinCtrl));
            m_props->SetPropertyAttribute(pid, wxT("Min"), vmin);
            m_props->SetPropertyAttribute(pid, wxT("Max"), vmax);
//            m_props->SetPropertyAttribute( name, wxT("Wrap"), true );
        }
    }
    else if (controlType == CT_BOOL) {
        bool val;
        long num;
        wxString attr;

        defValue.ToLong(&num);
        val = false;
        if (defValue.CmpNoCase(wxT("true")) == 0 ||
                defValue.CmpNoCase(wxT("yes")) == 0 ||
                num != 0) {
            val = true;
        }
        pid = m_props->Insert( category, -1, new wxBoolProperty(label, name, val) );
        if ( node->GetPropVal(wxT("chkbox"), &attr) ) {
            m_props->SetPropertyAttribute(pid, wxT("UseCheckbox"), true);
        }

    }
    else if (controlType == CT_FLOAT) {
        double val;
        defValue.ToDouble(&val);
        pid = m_props->Insert( category, -1, new wxFloatProperty(label, name, val) );
    }
    else {
        if (defValue.StartsWith(wxT("<composed>")) ) {
            defValue = wxT("<composed>");
        }
        m_props->Insert( category, -1, new wxStringProperty(label, name, defValue) );
    }

    wxXmlNode* nested = node->GetChildren();
    while (nested) {
        if (nested->GetName().CmpNoCase(wxT("option")) == 0) {
            ProcessOption(nested, name);
        }
        nested = nested->GetNext();
    }
}

void wiPluginSettings::FillValues(wxXmlDocument& xmlData)
{
}

wxXmlDocument* wiPluginSettings::ComposeValues()
{
    return NULL;
}
