// ImmoTest.cpp : 实现文件
//

#pragma once
#include "StdAfx.h"

#include "UDS_Protocol.h"
#include "ImmoTestMainWnd.h"
//#include "afxdialogex.h"
#include "UDS_Extern.h"
#include "resource.h"
#include "afxpriv.h"
#include "Sia_Protocol.h"


extern Sia_Protocol* omSiaPtr;
extern UINT_PTR m_Timer;

void getCanDilInterface();
int m_sendTimeOut = 150;
#define DEBUG_ON
// ImmoTestMainWnd 对话框

IMPLEMENT_DYNAMIC(ImmoTestMainWnd, CDialog)



ImmoTestMainWnd::ImmoTestMainWnd(CWnd* pParent /*=NULL*/)
    : CDialog(ImmoTestMainWnd::IDD, pParent)
{
}

ImmoTestMainWnd::~ImmoTestMainWnd()
{
}

void ImmoTestMainWnd::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //DDX_Control(pDX, IDC_BUTTON_WFS16_LEARNSKPIN, m_learnPin);
    DDX_Control(pDX, IDC_IMMOTEST_TRACE, m_immoTrace);
    //DDX_Control(pDX, IDC_BUTTON_WFS16_RESETECU, m_resetEcu);
    //DDX_Text(pDX, IDC_IMMOTEST_TRACE, m_immoTraceStr);
    //DDX_Control(pDX, IDC_COMBO_WFS16_SUBTYPE, m_customerType);
    DDX_Control(pDX, IDC_CHECK_ENIMMORESP, m_enImmoResp);
    DDX_Control(pDX, IDC_COMBO_WFS_TYPE, m_immoType);
    //DDX_Control(pDX, IDC_CHECK_WFS16_ENABLE, m_ImmoSS);
    //DDX_Control(pDX, IDC_COMBO_WFS17_SUBTYPE, m_17_subType);
    //DDX_Control(pDX, IDC_BUTTON_WFS17_START_TEST, m_17_startBut);
    //DDX_Control(pDX, IDC_EDIT_WFS17_EMSRELEASED, m_17_emsReleased);
    //DDX_Text(pDX, IDC_EDIT_WFS17_EMSRELEASED, m_17_emsReleasedStr);
    //DDX_Control(pDX, IDC_CHECK_WFS17_ENABLE, m_17_enAbleTest);
}


BEGIN_MESSAGE_MAP(ImmoTestMainWnd, CDialog)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_TRACE, OnBnClickedButtonClearTrace)
    ///ON_MESSAGE_VOID(WM_CLOSE, doDestoryWin)
    ON_CBN_SELCHANGE(IDC_COMBO_WFS_TYPE, OnCbnSelchangeComboWfsType)
    ON_BN_CLICKED(IDC_BUTTON_WFS16_PINSK, OnBnClickedButtonlearnskpin)
    ON_BN_CLICKED(IDC_BUTTON_WFS16_RESETECU, OnBnClickedButtonResetecu)
    ON_CBN_SELCHANGE(IDC_COMBO_WFS16_SUBTYPE, OnCbnSelchangeComboCustomer)
    ON_BN_CLICKED(IDC_CHECK_WFS16_ENABLE, OnBnClickedCheckIss)




    ON_BN_CLICKED(IDC_CHECK_WFS17_ENABLE, OnBnClickedCheckWfs17Enable)
    ON_BN_CLICKED(IDC_BUTTON_WFS17_START, OnBnClickedButtonWfs17StartTest)
    ON_CBN_SELCHANGE(IDC_COMBO_WFS17_SUBTYPE, OnCbnSelchangeComboWfs17Subtype)
    ON_EN_CHANGE(IDC_EDIT_WFS17_EMSRELEASED, OnEnChangeEditWfs17Emsreleased)
END_MESSAGE_MAP()


// ImmoTestMainWnd 消息处理程序

void ImmoTestMainWnd::doDestoryWin()
{
}
BOOL ImmoTestMainWnd::OnInitDialog()
{
    CDialog::OnInitDialog();
    // TODO:  在此添加额外的初始化
    creatWidgets();
    /* Get CAN DIL interface */
    getCanDilInterface();

    if(omSiaPtr == NULL)
    {
        omSiaPtr = new Sia_Protocol();

        if(omSiaPtr->mSia16Ptr == NULL)
        {
            omSiaPtr->mSia16Ptr = new Sia_16_Protocol();
            omSiaPtr->mSia16Ptr->LoadDiasCanCfg();
            updateCustomerCombox();
            omSiaPtr->mSia16Ptr->updateImmoId();
            omSiaPtr->mSia16Ptr->initImmoMsg();

            if(m_customerType->GetCount() > 0)
            {
                m_customerType->SetCurSel(0);
            }
        }

        if(omSiaPtr->mSia17Ptr == NULL)
        {
            omSiaPtr->mSia17Ptr = new Sia_17_Protocol();
            m_17_subType->AddString(wfs17SubType[0]);
            m_17_subType->AddString(wfs17SubType[1]);
            m_17_subType->SetCurSel(0);
            m_17_startBut->EnableWindow(FALSE);
        }
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}
void ImmoTestMainWnd::creatWidgets(void)
{
    m_immoType.AddString("GAC IMMO");
    m_immoType.AddString("DIAS CAN");
    m_immoType.SetCurSel(0);
    m_testState = immoStateIdel;
    m_immoTraceStr.Empty();
    //m_immoTraceStrOld.Empty();
    int x_start = 20, y_start = 100, width = 80, height_com = 60, height = 20, distance = 20;
    DWORD dwStyle_button = WS_CHILD  | BS_PUSHBUTTON | WS_VISIBLE;
    DWORD dwStyle_combox = CBS_DROPDOWN | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE | WS_CHILD; // WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST;
    DWORD dwStyle_checkbox = WS_CHILD | BS_AUTOCHECKBOX | WS_VISIBLE;
    m_customerType = new CComboBox;
    m_customerType->Create(dwStyle_combox, CRect(x_start, y_start, x_start + width, y_start + height_com), this, IDC_COMBO_WFS16_SUBTYPE);
    m_customerType->SetFont(m_enImmoResp.GetFont());
    m_learnPin = new CButton;
    x_start = x_start + width + distance;
    m_learnPin->Create(_T("Learn PIN/SK"), dwStyle_button, CRect(x_start, y_start, x_start + width, y_start + height), this, IDC_BUTTON_WFS16_PINSK);
    m_learnPin->SetFont(m_enImmoResp.GetFont());
    m_resetEcu = new CButton;
    x_start = x_start + width + distance;
    m_resetEcu->Create(_T("Reset SK"), dwStyle_button,  CRect(x_start, y_start, x_start + width, y_start + height), this, IDC_BUTTON_WFS16_RESET_SK);
    m_resetEcu->SetFont(m_enImmoResp.GetFont());
    m_ImmoSS = new CButton;
    x_start = x_start + width + distance;
    m_ImmoSS->Create(_T("ON/OFF"), dwStyle_checkbox,  CRect(x_start, y_start, x_start + width, y_start + height), this, IDC_CHECK_WFS16_ENABLE);
    m_ImmoSS->SetFont(m_enImmoResp.GetFont());
    x_start = 20, y_start = 100, width = 80, height_com = 60, height = 20, distance = 20;
    dwStyle_button = WS_CHILD  | BS_PUSHBUTTON ;
    dwStyle_combox = CBS_DROPDOWN | CBS_SORT | WS_VSCROLL | WS_TABSTOP  | WS_CHILD; // WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST;
    dwStyle_checkbox = WS_CHILD | BS_AUTOCHECKBOX ;
    DWORD dwStyle_edit = ES_MULTILINE | ES_WANTRETURN ;
    m_17_subType = new CComboBox;
    m_17_subType->Create(dwStyle_combox, CRect(x_start, y_start, x_start + width, y_start + height_com), this, IDC_COMBO_WFS17_SUBTYPE);
    m_customerType->SetFont(m_enImmoResp.GetFont());
    x_start = x_start + width + distance;
    m_17_startBut = new CButton;
    m_17_startBut->Create(_T("Start"), dwStyle_button, CRect(x_start, y_start, x_start + width, y_start + height), this, IDC_BUTTON_WFS17_START);
    m_learnPin->SetFont(m_enImmoResp.GetFont());
    x_start = x_start + width + distance;
    m_17_emsReleased = new CRadixEdit;
    m_17_emsReleased->Create(dwStyle_edit, CRect(x_start, y_start, x_start + width, y_start + height), this, IDC_EDIT_WFS17_EMSRELEASED);
    m_17_emsReleased->SetFont(m_enImmoResp.GetFont());
    m_17_emsReleased->EnableWindow(FALSE);
    x_start = x_start + width + distance;
    m_17_enAbleTest = new CButton;
    m_17_enAbleTest->Create(_T("ON/OFF"), dwStyle_checkbox, CRect(x_start, y_start, x_start + width, y_start + height), this, IDC_CHECK_WFS17_ENABLE);
    m_17_enAbleTest->SetFont(m_enImmoResp.GetFont());
}
void getCanDilInterface()
{
    DIL_GetInterface(CAN, (void**)&g_pouDIL_CAN_Interface1);
    g_pouDIL_CAN_Interface1->DILC_RegisterClient(TRUE, g_dwClientID1, _("CAN_MONITOR"));
}
BOOL ImmoTestMainWnd::PreTranslateMessage(MSG* pMsg)
{
    // TODO: 在此添加专用代码和/或调用基类
    return CDialog::PreTranslateMessage(pMsg);
}

void ImmoTestMainWnd::OnBnClickedButtonlearnskpin()
{
    // TODO: 在此添加控件通知处理程序代码
    if(m_testState == immoStateIdel  || m_testState == immoStateOK)
    {
        omSiaPtr->mSia16Ptr->sendImmomsg(INDEX_LEARN_SK);
        m_testState = immoStateWaiting;
        m_Timer = SetTimer(ID_TIMER_IMMO_RESP, m_sendTimeOut, NULL);
        //updateTraceInfo(INDEX_LEARN_SK);
        //m_learnPin.EnableWindow(FALSE);
    }
}


void ImmoTestMainWnd::OnBnClickedButtonResetecu()
{
    // TODO: 在此添加控件通知处理程序代码
    if(m_testState == immoStateIdel || m_testState == immoStateOK)
    {
        omSiaPtr->mSia16Ptr->sendImmomsg(INDEX_RESET_SK);
        m_testState = immoStateWaiting;
        m_Timer = SetTimer(ID_TIMER_IMMO_RESP, m_sendTimeOut, NULL);
        //updateTraceInfo(INDEX_RESET_SK);
        //m_resetEcu.EnableWindow(FALSE);
    }
}

void ImmoTestMainWnd::updateTraceInfo(int id)
{
#if 0
    CString buff, buff1;
    SYSTEMTIME tmp_tm;
    GetSystemTime(&tmp_tm);
    CB2CS((char*)m_diasImmoMsgList[id], buff1, 8);
    buff.Format("%-2d:%2d:%2d:%4d %s 0X%x {", ((tmp_tm.wHour + 8) < 24) ? (tmp_tm.wHour + 8) : (tmp_tm.wHour - 16),
                tmp_tm.wMinute,
                tmp_tm.wSecond,
                tmp_tm.wMilliseconds,
                //rx_tx[id % 2],
                //(char*)(diasImmoMsgCfg[id]+11)
                omSiaPtr->mSia16Ptr->m_immoMsgId[id]
               );
    buff += buff1;
    m_immoTraceStr = m_immoTraceStr + buff + "\r\n";
    UpdateData(FALSE);
#endif
}
void ImmoTestMainWnd::OnCbnSelchangeComboCustomer()
{
    // TODO: 在此添加控件通知处理程序代码
    omSiaPtr->mSia16Ptr->LoadDiasCanCfg();
    updateCustomerCombox();
    omSiaPtr->mSia16Ptr->updateImmoId();
}
void ImmoTestMainWnd::updateCustomerCombox()
{
    int i;
    immocfg* tmp = omSiaPtr->mSia16Ptr->m_immoCfgHead;
    i = m_customerType->GetCurSel();
    m_customerType->ResetContent();

    while(tmp != NULL)
    {
        //CString tmp_s;
        //tmp_s.Format("%d",tmp->Canvers);
        m_customerType->AddString((LPCTSTR)tmp->Canvers);
        tmp = tmp->next;
    }

    m_customerType->SetCurSel(i);
    omSiaPtr->mSia16Ptr->subTypeCount = m_customerType->GetCount();
    omSiaPtr->mSia16Ptr->subTypeSel = m_customerType->GetCurSel();
}
void ImmoTestMainWnd::OnBnClickedCheckIss()
{
    // TODO: 在此添加控件通知处理程序代码
    if(m_ImmoSS->GetCheck())
    {
        m_learnPin->EnableWindow(TRUE);
        m_resetEcu->EnableWindow(TRUE);
        m_customerType->EnableWindow(FALSE);
        m_testState = immoStateIdel;

        if(omSiaPtr->mSia16Ptr == NULL)
        {
            omSiaPtr->mSia16Ptr = new Sia_16_Protocol();
        }

        omSiaPtr->mSia16Ptr->m_curSendMsgCnt = 0;
        omSiaPtr->mSia16Ptr->m_curSendMsgID = 0;
        //m_17_enAbleTest->EnableWindow(FALSE);
        m_Timer = SetTimer(IDC_EDIT_WFS17_EMSRELEASED, m_sendTimeOut, NULL);
    }
    else
    {
        m_learnPin->EnableWindow(FALSE);
        m_resetEcu->EnableWindow(FALSE);
        m_customerType->EnableWindow(TRUE);
        //    m_17_enAbleTest->EnableWindow(TRUE);
        KillTimer(IDC_EDIT_WFS17_EMSRELEASED);
    }
}


void ImmoTestMainWnd::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if(nIDEvent == ID_TIMER_IMMO_RESP)
    {
        if(omSiaPtr->mSia16Ptr->m_curSendMsgCnt < 14)
        {
            omSiaPtr->mSia16Ptr->sendImmomsg(omSiaPtr->mSia16Ptr->m_curSendMsgID);
            m_testState = immoStateWaiting;
            m_Timer = SetTimer(ID_TIMER_IMMO_RESP, m_sendTimeOut, NULL);
            //updateTraceInfo(omSiaPtr->mSia16Ptr->m_curSendMsgID);
        }
        else
        {
            m_testState = immoStateTimeOut;
            KillTimer(ID_TIMER_IMMO_RESP);
        }
    }
    else if(nIDEvent == IDC_EDIT_WFS17_EMSRELEASED)
    {
        if(m_immoTraceStr.GetLength() > 0)
        {
            m_17_emsReleasedStr.Format("%d", omSiaPtr->mSia17Ptr->m_wfs17Ems4Msg.m_sWhichBit.EMS_EMSReleased);
            // m_immoTraceStrOld = m_immoTraceStr;
            m_immoTrace.SetSel(-1, -1);
            m_immoTrace.ReplaceSel(m_immoTraceStr);
            m_immoTraceStr.Empty();
            UpdateData(TRUE);
        }
    }

    CDialog::OnTimer(nIDEvent);
}
void ImmoTestMainWnd::checkEmsResp(STCAN_MSG Mensage)
{
}





void ImmoTestMainWnd::OnBnClickedCheckWfs17Enable()
{
    // TODO: 在此添加控件通知处理程序代码
    if(m_17_enAbleTest->GetCheck())
    {
        m_17_subType->EnableWindow(FALSE);
        m_17_startBut->EnableWindow(TRUE);

        if(omSiaPtr->mSia17Ptr == NULL)
        {
            omSiaPtr->mSia17Ptr = new Sia_17_Protocol();
        }

        m_testState = immoStateIdel;
        m_Timer = SetTimer(IDC_EDIT_WFS17_EMSRELEASED, m_sendTimeOut, NULL);
        // m_ImmoSS->EnableWindow(FALSE);
    }
    else
    {
        m_17_subType->EnableWindow(TRUE);
        m_17_startBut->EnableWindow(FALSE);
        KillTimer(ID_TIMER_IMMO_RESP);
        //m_ImmoSS->EnableWindow(TRUE);
    }

    m_testState = immoStateIdel;
}


void ImmoTestMainWnd::OnBnClickedButtonWfs17StartTest()
{
    // TODO: 在此添加控件通知处理程序代码
    if(m_testState == immoStateIdel || m_testState == immoStateOK)
    {
        omSiaPtr->mSia17Ptr->sendImmoMsg(WFS17_MSG_INDEX_EXTMOD);
    }
}


void ImmoTestMainWnd::OnCbnSelchangeComboWfs17Subtype()
{
    // TODO: 在此添加控件通知处理程序代码
    omSiaPtr->mSia17Ptr->m_wfs17SubType = m_17_subType->GetCurSel();
}


void ImmoTestMainWnd::OnEnChangeEditWfs17Emsreleased()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。
    // TODO:  在此添加控件通知处理程序代码
}


void ImmoTestMainWnd::OnBnClickedButtonClearTrace()
{
    // TODO: 在此添加控件通知处理程序代码
    m_immoTrace.SetSel(0, -1);
    m_immoTrace.Clear();
    m_immoTraceStr.Empty();
    UpdateData(true);
}


void ImmoTestMainWnd::OnCbnSelchangeComboWfsType()
{
    // TODO: 在此添加控件通知处理程序代码
    if(m_immoType.GetCurSel() == 0)
    {
        showWfs17Widgets(false);
        showWfs16Widgets(true);
    }
    else if(m_immoType.GetCurSel() == 1)
    {
        showWfs16Widgets(false);
        showWfs17Widgets(true);
    }
}
void ImmoTestMainWnd::showWfs16Widgets(bool b_show)
{
    m_learnPin->ShowWindow(b_show);
    m_resetEcu->ShowWindow(b_show);
    m_ImmoSS->ShowWindow(b_show);
    m_customerType->ShowWindow(b_show);
}
void ImmoTestMainWnd::showWfs17Widgets(bool b_show)
{
    m_17_startBut->ShowWindow(b_show);
    m_17_enAbleTest->ShowWindow(b_show);
    m_17_emsReleased->ShowWindow(b_show);
    m_17_subType->ShowWindow(b_show);
}