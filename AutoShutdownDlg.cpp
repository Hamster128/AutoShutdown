
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
  //  DDX_Control(pDX, IDC_CBMOUSEACTIVE, cbMouseActive);
  DDX_Control(pDX, IDC_EDCPU, edCPU);
  DDX_Control(pDX, IDC_EDNETWORK, edNetwork);
  DDX_Control(pDX, IDC_EDCDUSER, edCdUser);
  DDX_Control(pDX, IDC_EDCDNET, edCdNetwork);
  DDX_Control(pDX, IDC_EDCDCPU, edCdCPU);
  DDX_Control(pDX, IDC_CBARMED, cbArmed);
  DDX_Control(pDX, IDC_EDCDSYSTEM, edCdSystem);
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
ON_BN_CLICKED(IDC_CBSYSTEM_REQUIRED, &CAutoShutdownDlg::OnBnClickedCbsystemRequired)
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

  brGreen = new CBrush(RGB(60, 240, 60));
  brOrange = new CBrush(RGB(255, 160, 0));
  brRed = new CBrush(RGB(240, 40, 40));

  GetCursorPos(&lastPoint);

  Menu = GetMenu();

  // load config
  if (!ini.Open("AutoShutdown.ini"))
  {
    AfxMessageBox(ini.LastMessage+"\nAutoShutdown.ini", MB_OK);
    exit(0);
  }

  cdUser   = ini.EntryDouble("general", "user_idle_mins", 5) * 60;
  cdCPU    = ini.EntryDouble("general", "cpu_idle_mins", 5) * 60;
  cdNet    = ini.EntryDouble("general", "net_idle_mins", 5) * 60;
  cdSystem = ini.EntryDouble("general", "system_idle_mins", 1) * 60;

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
bool CAutoShutdownDlg::userIsActive()
{
  LASTINPUTINFO info;
  info.cbSize = sizeof(info);

  GetLastInputInfo(&info);

  return (GetTickCount() - info.dwTime) < 1500;
}

//---------------------------------------------------------------------------------------
double CAutoShutdownDlg::Median(vector<int> scores)
{
  size_t size = scores.size();

  if (size == 0)
  {
    return 0;  // Undefined, really.
  }
  else
  {
    sort(scores.begin(), scores.end());
    if (size % 2 == 0)
    {
      return (scores[size / 2 - 1] + scores[size / 2]) / 2;
    }
    else
    {
      return scores[size / 2];
    }
  }
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

	// check keyboard / mouse
  bool bUserIsActive = userIsActive();

	// check cpu usage
	PDH_FMT_COUNTERVALUE cpuUsage;

	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &cpuUsage);

  cpuData.push_back(cpuUsage.doubleValue);

  if(cpuData.size() > ini.EntryInt("general", "cpu_median_size", 10))
    cpuData.erase(cpuData.begin());

  cpuUsage.doubleValue = Median(cpuData);

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

  netData.push_back(networkDiff);

  if (netData.size() > ini.EntryInt("general", "net_median_size", 10))
    netData.erase(netData.begin());

  networkDiff = Median(netData);

  csTxt.Format("%d", networkDiff);
  edNetwork.SetWindowText(csTxt);

  // check thresholds and timeouts
  hbrUser = *brRed;
  if (!bUserIsActive)
  {
    hbrUser = *brOrange;

    if (cdUser)
      cdUser--;
    
    if (!cdUser)
      hbrUser = *brGreen;
  }
  else
    cdUser = ini.EntryDouble("general", "user_idle_mins", 5) * 60;

  csTxt.Format("%d", cdUser);
  edCdUser.SetWindowText(csTxt);


  hbrSystem = *brRed;
  if (!bSystemRequired)
  {
    hbrSystem = *brOrange;

    if (cdSystem)
      cdSystem--;

    if (!cdSystem)
      hbrSystem = *brGreen;
  }
  else
    cdSystem = ini.EntryDouble("general", "system_idle_mins", 1) * 60;

  csTxt.Format("%d", cdSystem);
  edCdSystem.SetWindowText(csTxt);


  hbrCPU = *brRed;
  if (cpuUsage.doubleValue < ini.EntryInt("general", "cpu_threshold", 40))
  {
    hbrCPU = *brOrange;

    if (cdCPU)
      cdCPU--;

    if (!cdCPU)
      hbrCPU = *brGreen;
  }
  else
    cdCPU = ini.EntryDouble("general", "cpu_idle_mins", 5) * 60;

  csTxt.Format("%d", cdCPU);
  edCdCPU.SetWindowText(csTxt);


  hbrNet = *brRed;
  if (networkDiff < ini.EntryInt("general", "net_threshold", 20))
  {
    hbrNet = *brOrange;

    if (cdNet)
      cdNet--;

    if (!cdNet)
      hbrNet = *brGreen;
  }
  else
    cdNet = ini.EntryDouble("general", "net_idle_mins", 5) * 60;

  csTxt.Format("%d", cdNet);
  edCdNetwork.SetWindowText(csTxt);

  // check total idle
  if (cdUser || cdCPU || cdNet || cdSystem)
    return;

  // execute sleep
  cdUser   = ini.EntryDouble("general", "user_idle_mins", 5) * 60;
  cdCPU    = ini.EntryDouble("general", "cpu_idle_mins", 5) * 60;
  cdNet    = ini.EntryDouble("general", "net_idle_mins", 5) * 60;
  cdSystem = ini.EntryDouble("general", "system_idle_mins", 1) * 60;

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

  if ((CTLCOLOR_EDIT == nCtlColor) && (IDC_EDCDSYSTEM == pWnd->GetDlgCtrlID()))
    return hbrSystem;

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


void CAutoShutdownDlg::OnBnClickedCbsystemRequired()
{
  // TODO: Add your control notification handler code here
}


void CAutoShutdownDlg::OnEnChangeEdcduser2()
{
  // TODO:  If this is a RICHEDIT control, the control will not
  // send this notification unless you override the CDialogEx::OnInitDialog()
  // function and call CRichEditCtrl().SetEventMask()
  // with the ENM_CHANGE flag ORed into the mask.

  // TODO:  Add your control notification handler code here
}
