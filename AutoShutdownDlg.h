
// AutoShutdownDlg.h: Headerdatei
//

#pragma once

// CAutoShutdownDlg-Dialogfeld
class CAutoShutdownDlg : public CDialogEx
{
// Konstruktion
public:
	CAutoShutdownDlg(CWnd* pParent = nullptr);	// Standardkonstruktor

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUTOSHUTDOWN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung


// Implementierung
protected:
  LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);

	HICON m_hIcon;
	POINT lastPoint;
	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;
  CSystemTray m_TrayIcon;
  CIniLib ini;
  int cdUser, cdCPU, cdNet;
  HBRUSH hbrUser, hbrCPU, hbrNet;
  CBrush *brGreen, *brRed;
  bool bFirstShow;
  CLog Log;

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton cbSystemRequired;
	CButton cbMouseActive;
	CEdit edCPU;
  CEdit edNetwork;
  int lastNetwork;
  CEdit edCdUser;
  CEdit edCdNetwork;
  CEdit edCdCPU;
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
//  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
//  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  CButton cbArmed;
  afx_msg void OnClickedCbarmed();
};
