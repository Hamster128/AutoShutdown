
// AutoShutdown.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "AutoShutdown.h"
#include "AutoShutdownDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAutoShutdownApp

BEGIN_MESSAGE_MAP(CAutoShutdownApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
//  ON_COMMAND(ID_AUTOSHUTDOWN_EXIT, &CAutoShutdownApp::OnAutoshutdownExit)
ON_COMMAND(AFX_ID_PREVIEW_CLOSE, &CAutoShutdownApp::OnAfxIdPreviewClose)
END_MESSAGE_MAP()


// CAutoShutdownApp-Erstellung

CAutoShutdownApp::CAutoShutdownApp()
{
	// Neustart-Manager unterstützen
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: Hier Code zur Konstruktion einfügen
	// Alle wichtigen Initialisierungen in InitInstance positionieren
}


bool CheckAppIsAlreadyRunning()
{
  HANDLE m_hStartEvent = CreateEvent(NULL, TRUE, FALSE, "eventAutoShutdown");

  if (GetLastError() == ERROR_ALREADY_EXISTS)
  {
    CloseHandle(m_hStartEvent);
    return true;
  }

  return false;
}

// Das einzige CAutoShutdownApp-Objekt

CAutoShutdownApp theApp;

// CAutoShutdownApp-Initialisierung

BOOL CAutoShutdownApp::InitInstance()
{
  if (CheckAppIsAlreadyRunning())
    return FALSE;

	// InitCommonControlsEx() ist für Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder höher zum Aktivieren
	// von visuellen Stilen angibt.  Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Legen Sie dies fest, um alle allgemeinen Steuerelementklassen einzubeziehen,
	// die Sie in Ihrer Anwendung verwenden möchten.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Shell-Manager erstellen, falls das Dialogfeld
	// Shellbaumansicht- oder Shelllistenansicht-Steuerelemente enthält.
	CShellManager *pShellManager = new CShellManager;

	//Visuellen Manager "Windows Native" aktivieren, um Designs für MFC-Steuerelemente zu aktivieren
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standardinitialisierung
	// Wenn Sie diese Funktionen nicht verwenden und die Größe
	// der ausführbaren Datei verringern möchten, entfernen Sie
	// die nicht erforderlichen Initialisierungsroutinen.
	// Ändern Sie den Registrierungsschlüssel, unter dem Ihre Einstellungen gespeichert sind.
	// TODO: Ändern Sie diese Zeichenfolge entsprechend,
	// z.B. zum Namen Ihrer Firma oder Organisation.
	SetRegistryKey(_T("Vom lokalen Anwendungs-Assistenten generierte Anwendungen"));

	CAutoShutdownDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Fügen Sie hier Code ein, um das Schließen des
		//  Dialogfelds über "OK" zu steuern
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Fügen Sie hier Code ein, um das Schließen des
		//  Dialogfelds über "Abbrechen" zu steuern
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warnung: Fehler bei der Dialogfelderstellung, unerwartetes Beenden der Anwendung.\n");
		TRACE(traceAppMsg, 0, "Warnung: Wenn Sie MFC-Steuerelemente im Dialogfeld verwenden, ist #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS nicht möglich.\n");
	}

	// Den oben erstellten Shell-Manager löschen.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Da das Dialogfeld geschlossen wurde, FALSE zurückliefern, sodass wir die
	//  Anwendung verlassen, anstatt das Nachrichtensystem der Anwendung zu starten.
	return FALSE;
}

//void CAutoShutdownApp::OnAutoshutdownExit()
//{
//  m_pMainWnd->PostMessageA(WM_CLOSE, 0, NULL);
//}


void CAutoShutdownApp::OnAfxIdPreviewClose()
{
}
