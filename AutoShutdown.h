
// AutoShutdown.h: Hauptheaderdatei für die PROJECT_NAME-Anwendung
//

#pragma once

#ifndef __AFXWIN_H__
	#error "'stdafx.h' vor dieser Datei für PCH einschließen"
#endif

#include "resource.h"		// Hauptsymbole


// CAutoShutdownApp:
// Siehe AutoShutdown.cpp für die Implementierung dieser Klasse
//

class CAutoShutdownApp : public CWinApp
{
public:
	CAutoShutdownApp();

// Überschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()
//  afx_msg void OnAutoshutdownExit();
};

extern CAutoShutdownApp theApp;
