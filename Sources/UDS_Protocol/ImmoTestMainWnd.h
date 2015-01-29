#pragma once
#include "resource.h"
#include "Utility/RadixEdit.h"
#include "DataTypes/UDS_DataTypes.h"
#include "DataTypes/MsgBufFSE.h"
#include <libxml/parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include "DIL_Interface/BaseDIL_CAN.h"
#include "DIL_Interface/DIL_Interface_extern.h"
#include <fstream>
#include <iosfwd>
#include "Sia_16_Protocol.h"
#include "Sia_17_Protocol.h"

// ImmoTest 对话框


extern  CBaseDIL_CAN* g_pouDIL_CAN_Interface1;
extern DWORD g_dwClientID1;




class ImmoTestMainWnd : public CDialog
{
    DECLARE_DYNAMIC(ImmoTestMainWnd)

public:
    ImmoTestMainWnd(CWnd* pParent = NULL);   // 标准构造函数
    virtual ~ImmoTestMainWnd();
    //wfs 16
    CComboBox m_immoType;

    CButton* m_learnPin;
    CButton* m_resetEcu;
    CRadixEdit m_immoTrace;
    CString m_immoTraceStr;
    //CString m_immoTraceStrOld;
    CButton m_enImmoResp;
    CButton* m_ImmoSS;
    CComboBox* m_customerType;
    immoTestStates m_testState;

    //wfs 17
    CButton* m_17_startBut;
    CString     m_17_emsReleasedStr;
    CButton*     m_17_enAbleTest;
    CRadixEdit*  m_17_emsReleased;
    CComboBox*   m_17_subType;
// 对话框数据
    enum { IDD = IDD_IMMOTEST };
    void updateCustomerCombox(void);
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    void checkEmsResp(STCAN_MSG  Mensage);
    void updateTraceInfo(int id);
    afx_msg void OnBnClickedButtonlearnskpin();
    afx_msg void OnBnClickedButtonResetecu();
    afx_msg void OnCbnSelchangeComboCustomer();
    afx_msg void OnBnClickedCheckIss();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedCheckWfs17Enable();
    afx_msg void OnBnClickedButtonWfs17StartTest();
    afx_msg void OnCbnSelchangeComboWfs17Subtype();
    afx_msg void OnEnChangeEditWfs17Emsreleased();
    afx_msg void OnBnClickedButtonClearTrace();
    afx_msg void doDestoryWin();

    void creatWidgets(void);
    void showWfs16Widgets(bool);
    void showWfs17Widgets(bool);
    afx_msg void OnCbnSelchangeComboWfsType();
};
