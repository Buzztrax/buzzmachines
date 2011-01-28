#if !defined(AFX_EDITORDLG_H__D476D949_2725_11D3_9019_000000000000__INCLUDED_)
#define AFX_EDITORDLG_H__D476D949_2725_11D3_9019_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorDlg.h : header file
//

#include "resource.h"
#include "internal.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorDlg dialog

class CEditorDlg : public CDialog
{
// Construction
public:
	CEditorDlg(CWnd* pParent = NULL);   // standard constructor

	CAuxBus *SelBus;

	int *AuxIn;
	int *AuxOut;
	char const *AuxName;
	AB_DisconnectCallback AuxCB;
	void *AuxUser;

// Dialog Data
	//{{AFX_DATA(CEditorDlg)
	enum { IDD = IDD_EDITOR };
	CButton	m_Output;
	CButton	m_Input;
	CEdit	m_BusEdit;
	CSpinButtonCtrl	m_BusSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeBusedit();
	afx_msg void OnInput();
	afx_msg void OnOutput();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORDLG_H__D476D949_2725_11D3_9019_000000000000__INCLUDED_)
