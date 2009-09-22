#ifndef __wiPluginSettings__
#define __wiPluginSettings__

#include <wx/xml/xml.h>
#include "wiGuiData.h"

#define CT_STRING   wxT("string")
#define CT_INT      wxT("int")
#define CT_FLOAT    wxT("float")
#define CT_ENUM     wxT("enum")
#define CT_BOOL     wxT("bool")

/** Implementing PluginSettings */
class wiPluginSettings : public PluginSettings
{
protected:
    // functions
    void ProcessOption(wxXmlNode* node, const wxString& catId);
    void ProcessCategory(wxXmlNode* node, const wxString& catId);
    wxPGProperty* FindProperty(const wxString& name);

    // members
    wxString defaultSpace;
    bool defSpaceUsed;
    int defCount;
    wxXmlDocument structure;

public:
	/** Constructor */
	wiPluginSettings( wxWindow* parent, const wxString& description );

	void RebuildView();
	void FillValues(wxXmlNode& xmlData);
	void ComposeValues(wxXmlNode& xmlData);
	static wxXmlNode* SaveTaskOption (wxXmlNode *root, const wxString& name, const wxString& type, const wxString& value);
};

#endif // __wiPluginSettings__
