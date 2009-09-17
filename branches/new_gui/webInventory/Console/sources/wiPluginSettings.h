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

    // members
    wxString defaultSpace;
    bool defSpaceUsed;
    int defCount;
    wxXmlDocument structure;

public:
	/** Constructor */
	wiPluginSettings( wxWindow* parent, const wxString& description );

	void RebuildView();
	void FillValues(wxXmlDocument& xmlData);
	wxXmlDocument* ComposeValues();
};

#endif // __wiPluginSettings__
