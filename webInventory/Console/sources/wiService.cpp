#include "wiService.h"

wiService::wiService( wxWindow* parent )
:
Service( parent )
{
    m_pVulners = new VulnDB( m_fnbService ); //, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_fnbService->AddPage( m_pVulners, _("Vulnerabilities"), true );
    m_pReports = new RepDB( m_fnbService ); //, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_fnbService->AddPage( m_pReports, _("Reports Templates"), false );
    m_pScripts = new Scripts( m_fnbService ); //, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_fnbService->AddPage( m_pScripts, _("Scripts"), false );
}
