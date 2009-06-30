/***************************************************************
 * Name:      wiApp.h
 * Purpose:   Defines Application Class
 * Author:    Andrew "Stinger" Abramov (stinger@lab18.net)
 * Created:   2008-10-15
 * Copyright: Andrew "Stinger" Abramov (https://igate.lab18.net)
 * License:
 **************************************************************/

#ifndef WXWEBINVENTAPP_H
#define WXWEBINVENTAPP_H

#include <wx/app.h>

class webInventApp : public wxApp
{
    protected:
        wxLocale m_locale;

    public:
        virtual bool OnInit();
};

#endif // WXWEBINVENTAPP_H
