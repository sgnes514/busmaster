#pragma once
#include "resource.h"
#include "Utility/RadixEdit.h"
#include "DataTypes/UDS_DataTypes.h"
#include "DataTypes/MsgBufFSE.h"

// ImmoTest 对话框
typedef enum ITS
{
	immoStateIdel,
	immoStateWaiting,
#if 0
	immoStateLearnSKok,
	immoStateLearnPINok,
	immoStateResetSKok,
	immoStateResetEcuok,
	immoStateEcmChallengeOK,
#endif
	immoStateOK,
	immoStateCfgError,
	immoStateTimeOut
}immoTestStates;

#define  INDEX_LEARN_SK				0
#define  INDEX_LEARN_SK_RESP		INDEX_LEARN_SK+1
#define  INDEX_LEARN_PIN			INDEX_LEARN_SK_RESP+1
#define  INDEX_LEARN_PIN_RESP		INDEX_LEARN_PIN+1
#define  INDEX_RESET_SK				INDEX_LEARN_PIN_RESP+1
#define  INDEX_RESET_SK_RESP		INDEX_RESET_SK+1
#define  INDEX_RESET_ECU			INDEX_RESET_SK_RESP+1
#define  INDEX_REEST_ECU_RESP		INDEX_RESET_ECU+1
#define  INDEX_ECM_CHALLANGE_RESP		INDEX_REEST_ECU_RESP+1
#define  INDEX_ECM_CHALLANGE	INDEX_ECM_CHALLANGE_RESP+1



typedef struct IMMOCFG
{
	CString Canvers;
	int immoIdList[10];
	struct IMMOCFG* next;
}immocfg;

class ImmoTest : public CDialog
{
	DECLARE_DYNAMIC(ImmoTest)

public:
	ImmoTest(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~ImmoTest();
	CButton m_learnPin;
	CButton m_resetEcu;
	CRadixEdit m_immoTrace;
	CString m_immoTraceStr;
	CButton m_enImmoResp; 
	CButton m_ImmoSS;
	CComboBox m_customerType;
	immoTestStates m_testState;
	int m_immoMsgId[10];
	int m_curSendMsgCnt;
	int m_curSendMsgID;
	mPSTXSELMSGDATA immoTxMsg;
// 对话框数据
	enum { IDD = IDD_IMMOTEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	int LoadDiasCanCfg();
	void addImmoCfg(immocfg* node);
	void freeImmoCfgStr();
	void updateCustomerCombox();
	void updateImmoId();
	void checkCfgValid();
	int sendImmomsg(int id);
	void initImmoMsg();
	void checkEmsResp(STCAN_MSG  Mensage);
	void updateTraceInfo(int id);
	immocfg* m_immoCfgHead;
	afx_msg void OnBnClickedButtonlearnskpin();
	afx_msg void OnBnClickedButtonResetecu();
	afx_msg void OnCbnSelchangeComboCustomer();
	afx_msg void OnBnClickedCheckIss();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
