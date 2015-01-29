#pragma once
#include "siaDef.h"
#include "UDSWnd_Defines.h"

extern unsigned char m_diasImmoMsgList[10][8];

typedef struct IMMOCFG
{
    CString Canvers;
    int immoIdList[10];
    struct IMMOCFG* next;
} immocfg;

#define  INDEX_LEARN_SK             0
#define  INDEX_LEARN_SK_RESP        INDEX_LEARN_SK+1
#define  INDEX_LEARN_PIN            INDEX_LEARN_SK_RESP+1
#define  INDEX_LEARN_PIN_RESP       INDEX_LEARN_PIN+1
#define  INDEX_RESET_SK             INDEX_LEARN_PIN_RESP+1
#define  INDEX_RESET_SK_RESP        INDEX_RESET_SK+1
#define  INDEX_RESET_ECU            INDEX_RESET_SK_RESP+1
#define  INDEX_REEST_ECU_RESP       INDEX_RESET_ECU+1
#define  INDEX_ECM_CHALLANGE_RESP       INDEX_REEST_ECU_RESP+1
#define  INDEX_ECM_CHALLANGE    INDEX_ECM_CHALLANGE_RESP+1

// Sia_16_Protocol

class Sia_16_Protocol : public CWinApp
{
    DECLARE_DYNCREATE(Sia_16_Protocol)

protected:



public:
    Sia_16_Protocol();           // 动态创建所使用的受保护的构造函数
    virtual ~Sia_16_Protocol();
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    int m_immoMsgId[10];
    int m_curSendMsgCnt;
    int m_curSendMsgID;
    int subTypeCount;
    int subTypeSel;
    bool enEcmChlRsp;
    mPSTXSELMSGDATA immoTxMsg;
    immocfg* m_immoCfgHead;
    int LoadDiasCanCfg();
    void addImmoCfg(immocfg* node);
    void freeImmoCfgStr();
    void updateCustomerCombox();
    void updateImmoId();
    void checkCfgValid();
    int sendImmomsg(int id);
    void initImmoMsg(void);
    chkMsgRsp checkRMSResp(STCAN_MSG Mensage);
    void prepareImmoResp(STCAN_MSG);
protected:
    DECLARE_MESSAGE_MAP()
};


