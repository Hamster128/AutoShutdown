
// AutoShutdownDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AutoShutdown.h"
#include "AutoShutdownDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "PowrProf.lib")
#pragma comment(lib, "Pdh.lib")
#pragma comment(lib, "Iphlpapi.lib")

#define WM_ICON_NOTIFY  WM_APP+10

CString commonDocs__;

//---------------------------------------------------------------------------------------
// CAutoShutdownDlg-Dialogfeld
CAutoShutdownDlg::CAutoShutdownDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_AUTOSHUTDOWN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//---------------------------------------------------------------------------------------
void CAutoShutdownDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_CBSYSTEM_REQUIRED, cbSystemRequired);
  DDX_Control(pDX, IDC_CBMOUSEACTIVE, cbMouseActive);
  DDX_Control(pDX, IDC_EDCPU, edCPU);
  DDX_Control(pDX, IDC_EDNETWORK, edNetwork);
  DDX_Control(pDX, IDC_EDCDUSER, edCdUser);
  DDX_Control(pDX, IDC_EDCDNET, edCdNetwork);
  DDX_Control(pDX, IDC_EDCDCPU, edCdCPU);
  DDX_Control(pDX, IDC_CBARMED, cbArmed);
}

//---------------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CAutoShutdownDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
  ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
  ON_WM_SIZE()
  ON_WM_CTLCOLOR()
//  ON_WM_SHOWWINDOW()
ON_BN_CLICKED(IDC_CBARMED, &CAutoShutdownDlg::OnClickedCbarmed)
ON_COMMAND(ID_AUTOSHUTDOWN_ARMED, &CAutoShutdownDlg::OnAutoshutdownArmed)
ON_COMMAND(ID_AUTOSHUTDOWN_EXIT32774, &CAutoShutdownDlg::OnAutoshutdownExit32774)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------------------
// CAutoShutdownDlg-Meldungshandler
BOOL CAutoShutdownDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen.  Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	// TODO: Hier zusätzliche Initialisierung einfügen
  lastNetwork = 0;
  bFirstShow = true;

  char path[MAX_PATH];
  SHGetSpecialFolderPath(NULL, path, CSIDL_COMMON_DOCUMENTS, true);
  commonDocs__ = path;

  Log.Open(commonDocs__ + "\\AutoShutdown.log");
  Log.Format("Start");

  cbArmed.SetCheck(true);

  HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));  // Icon to use
  m_TrayIcon.Create(this, WM_ICON_NOTIFY, "AutoShutdown", hIcon, IDR_MENU1);

	// ini cpu performance counter
	PdhOpenQuery(NULL, NULL, &cpuQuery);
	// You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU values with PdhGetFormattedCounterArray()
	PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhCollectQueryData(cpuQuery);

  brGreen = new CBrush(RGB(128, 255, 128));
  brRed = new CBrush(RGB(250, 128, 128));

  GetCursorPos(&lastPoint);

  Menu = GetMenu();

  // load config
  if (!ini.Open("AutoShutdown.ini"))
  {
    AfxMessageBox(ini.LastMessage+"\nAutoShutdown.ini", MB_OK);
    exit(0);
  }

  cdUser = ini.EntryInt("general", "user_idle_mins", 5) * 60;
  cdCPU  = ini.EntryInt("general", "cpu_idle_mins", 5) * 60;
  cdNet  = ini.EntryInt("general", "net_idle_mins", 5) * 60;

	// start timer
	SetTimer(0, 1, NULL);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

//---------------------------------------------------------------------------------------
LRESULT CAutoShutdownDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
  // Delegate all the work back to the default implementation in CSystemTray.
  return m_TrayIcon.OnTrayNotification(wParam, lParam);
}

//---------------------------------------------------------------------------------------
// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie
//  den nachstehenden Code, um das Symbol zu zeichnen.  Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.
void CAutoShutdownDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//---------------------------------------------------------------------------------------
// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CAutoShutdownDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//---------------------------------------------------------------------------------------
void CAutoShutdownDlg::OnTimer(UINT_PTR nIDEvent)
{
  if (bFirstShow)
  {
    KillTimer(0);
    bFirstShow = false;
    ShowWindow(SW_HIDE);
    SetTimer(0, 1000, NULL);
  }

	// check system required
	bool bSystemRequired = false;

	ULONG systemRequired;
	CallNtPowerInformation(SystemExecutionState, NULL, 0, &systemRequired, sizeof(systemRequired));

	if (systemRequired & ES_SYSTEM_REQUIRED)
		bSystemRequired = true;
		
	cbSystemRequired.SetCheck(bSystemRequired);

	// check mouse movements
	bool bMouseActive = false;
	POINT actPoint;

  if (GetCursorPos(&actPoint))
  {
    if (actPoint.x != lastPoint.x || actPoint.y != lastPoint.y)
    {
      lastPoint = actPoint;
      bMouseActive = true;
    }
  }

	cbMouseActive.SetCheck(bMouseActive);

	// check cpu usage
	PDH_FMT_COUNTERVALUE cpuUsage;

	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &cpuUsage);

	CString csTxt;
  csTxt.Format("%.0f", cpuUsage.doubleValue);
	edCPU.SetWindowText(csTxt);

	// check network usage
  int network = 0;

	PMIB_IF_TABLE2 pIfTable;

	GetIfTable2(&pIfTable);
	for (int i = 0; i < pIfTable->NumEntries; i++)
	{
    MIB_IF_ROW2 *pIfRow = (MIB_IF_ROW2*)&pIfTable->Table[i];

		if (pIfRow->Type != IF_TYPE_ETHERNET_CSMACD)
			continue;

    if (pIfRow->MediaType != NdisMedium802_3)
      continue;

    if (!pIfRow->InterfaceAndOperStatusFlags.HardwareInterface)
      continue;

    network += pIfRow->OutOctets / 1024;
    network += pIfRow->InOctets / 1024;
	}
  FreeMibTable(pIfTable);

  int networkDiff;

  if (lastNetwork)
    networkDiff = network - lastNetwork;
  else
    networkDiff = 0;

  lastNetwork = network;

  csTxt.Format("%d", networkDiff);
  edNetwork.SetWindowText(csTxt);

  // check thresholds and timeouts
  hbrUser = *brRed;
  if (!bMouseActive)
  {
    if(cdUser)
      cdUser--;
    else
      hbrUser = *brGreen;
  }
  else
    cdUser = ini.EntryInt("general", "user_idle_mins", 5) * 60;

  csTxt.Format("%d", cdUser);
  edCdUser.SetWindowText(csTxt);


  hbrCPU = *brRed;
  if (cpuUsage.doubleValue < ini.EntryInt("general", "cpu_threshold", 40))
  {
    if (cdCPU)
      cdCPU--;
    else
      hbrCPU = *brGreen;
  }
  else
    cdCPU = ini.EntryInt("general", "cpu_idle_mins", 5) * 60;

  csTxt.Format("%d", cdCPU);
  edCdCPU.SetWindowText(csTxt);


  hbrNet = *brRed;
  if (networkDiff < ini.EntryInt("general", "net_threshold", 20))
  {
    if (cdNet)
      cdNet--;
    else
      hbrNet = *brGreen;
  }
  else
    cdNet = ini.EntryInt("general", "net_idle_mins", 5) * 60;

  csTxt.Format("%d", cdNet);
  edCdNetwork.SetWindowText(csTxt);

  // check total idle
  if (cdUser || cdCPU || cdNet || bSystemRequired)
    return;

  // execute sleep
  cdUser = ini.EntryInt("general", "user_idle_mins", 5) * 60;
  cdCPU = ini.EntryInt("general", "cpu_idle_mins", 5) * 60;
  cdNet = ini.EntryInt("general", "net_idle_mins", 5) * 60;

  if (!cbArmed.GetCheck())
    return;

  Log.Format("Go to sleep!");

  SetSuspendState(FALSE, TRUE, FALSE);
}

//---------------------------------------------------------------------------------------
void CAutoShutdownDlg::OnSize(UINT nType, int cx, int cy)
{
  CDialogEx::OnSize(nType, cx, cy);

  if (nType == SIZE_MINIMIZED)
  {
    ShowWindow(SW_HIDE);
  }
}

//---------------------------------------------------------------------------------------
HBRUSH CAutoShutdownDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

  // TODO:  Change any attributes of the DC here
  // TODO:  Return a different brush if the default is not desired

  if ((CTLCOLOR_EDIT == nCtlColor) && (IDC_EDCDUSER == pWnd->GetDlgCtrlID()))
    return hbrUser;

  if ((CTLCOLOR_EDIT == nCtlColor) && (IDC_EDCDCPU == pWnd->GetDlgCtrlID()))
    return hbrCPU;

  if ((CTLCOLOR_EDIT == nCtlColor) && (IDC_EDCDNET == pWnd->GetDlgCtrlID()))
    return hbrNet;

  return hbr;
}


//---------------------------------------------------------------------------------------
void CAutoShutdownDlg::OnClickedCbarmed()
{
  if (cbArmed.GetCheck()) m_TrayIcon.SetIcon(IDR_MAINFRAME);
  else                    m_TrayIcon.SetIcon(IDI_OFF);
  CustomizeMenu();
}

//---------------------------------------------------------------------------------------
void CAutoShutdownDlg::OnAutoshutdownArmed()
{
  if (cbArmed.GetCheck())
  {
    cbArmed.SetCheck(0);
    m_TrayIcon.SetIcon(IDI_OFF);
  }
  else
  {
    cbArmed.SetCheck(1);
    m_TrayIcon.SetIcon(IDR_MAINFRAME);
  }

  CustomizeMenu();
}

//---------------------------------------------------------------------------------------
void CAutoShutdownDlg::OnAutoshutdownExit32774()
{
  PostMessage(WM_CLOSE, 0, NULL);
}

//---------------------------------------------------------------------------------------
void CAutoShutdownDlg::CustomizeMenu()
{
  if (cbArmed.GetCheck())
    m_TrayIcon.pMenu->CheckMenuItem(ID_AUTOSHUTDOWN_ARMED, MF_CHECKED | MF_BYCOMMAND);
  else
    m_TrayIcon.pMenu->CheckMenuItem(ID_AUTOSHUTDOWN_ARMED, MF_UNCHECKED | MF_BYCOMMAND);
}
