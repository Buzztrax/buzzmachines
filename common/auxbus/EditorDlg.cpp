// EditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "auxbus.h"
#include "EditorDlg.h"
#include "internal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CAuxBus AuxBuses[NUM_BUSES];


/////////////////////////////////////////////////////////////////////////////
// CEditorDlg dialog


CEditorDlg::CEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditorDlg)
	DDX_Control(pDX, IDC_OUTPUT, m_Output);
	DDX_Control(pDX, IDC_INPUT, m_Input);
	DDX_Control(pDX, IDC_BUSEDIT, m_BusEdit);
	DDX_Control(pDX, IDC_SPIN1, m_BusSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CEditorDlg)
	ON_EN_CHANGE(IDC_BUSEDIT, OnChangeBusedit)
	ON_BN_CLICKED(IDC_INPUT, OnInput)
	ON_BN_CLICKED(IDC_OUTPUT, OnOutput)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorDlg message handlers

BOOL CEditorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_BusEdit.SetWindowText("1");
	m_BusSpin.SetRange(1, NUM_BUSES);
		

	if (AuxIn == NULL)
		m_Input.EnableWindow(FALSE);

	if (AuxOut == NULL)
		m_Output.EnableWindow(FALSE);

	SelBus = NULL;

	OnChangeBusedit();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditorDlg::OnChangeBusedit() 
{
	if (!IsWindow(m_BusEdit.m_hWnd))
		return;

	char buf[16];
	m_BusEdit.GetWindowText(buf, 16);

	int ch = atoi(buf);
	assert(ch >= 1 && ch <= NUM_BUSES);
	
	SelBus = AuxBuses + ch - 1;

	if (SelBus->InputUserName != NULL)
		m_Input.SetWindowText(SelBus->InputUserName);	
	else
		m_Input.SetWindowText("Set Input");

	if (SelBus->OutputUserName != NULL)
		m_Output.SetWindowText(SelBus->OutputUserName);	
	else
		m_Output.SetWindowText("Set Output");


}

void CEditorDlg::OnInput() 
{
	SelBus->SetInput(AuxUser, AuxCB, AuxName);
	
	
	*AuxIn = (SelBus - AuxBuses) + 1;

	OnChangeBusedit();
}

void CEditorDlg::OnOutput() 
{
	SelBus->SetOutput(AuxUser, AuxCB, AuxName);

	*AuxOut = (SelBus - AuxBuses) + 1;

	OnChangeBusedit();
}
