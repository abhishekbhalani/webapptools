#ifndef WXSTATBAR_H
#define WXSTATBAR_H

#include <wx/statusbr.h>
#include <wx/statbmp.h>

// A custom status bar which contains controls, icons &c
class wiStatBar : public wxStatusBar
{
public:
    wiStatBar(wxWindow *parent);
    virtual ~wiStatBar();
    void OnSize(wxSizeEvent& event);

private:
    wxStaticBitmap *m_statbmp;
    DECLARE_EVENT_TABLE()
};

#endif // WXSTATBAR_H
