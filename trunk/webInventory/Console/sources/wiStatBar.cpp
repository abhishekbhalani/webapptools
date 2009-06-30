#include "wiStatBar.h"
#include "../images/tree_no.xpm"
#include "../images/tree_unk.xpm"
#include "../images/tree_yes.xpm"

BEGIN_EVENT_TABLE(wiStatBar, wxStatusBar)
    EVT_SIZE(wiStatBar::OnSize)
END_EVENT_TABLE()

wiStatBar::wiStatBar(wxWindow *parent) : wxStatusBar(parent, wxID_ANY)
{
    static const int widths[4] = { 16,  100, 200, -1 };
    SetFieldsCount(4);
    SetStatusWidths(4, widths);
    m_statbmp = new wxStaticBitmap(this, wxID_ANY, wxIcon(tree_unk));
    SetMinHeight(16);
}

wiStatBar::~wiStatBar()
{
}

void wiStatBar::OnSize(wxSizeEvent& event)
{
    wxRect rect;

    GetFieldRect(0, rect);
    wxSize size = m_statbmp->GetSize();

    m_statbmp->Move(rect.x + (rect.width - size.x) / 2,
        rect.y + (rect.height - size.y) / 2);

    event.Skip();
}

