#pragma once
//#include "UDS_Protocol.h"
#include <afxwin.h>
#include "siaDef.h"

#define  MAX_NAME_LENGTH    10
extern char wfs17SubType[2][MAX_NAME_LENGTH];
// Sia_17_Protocol


typedef unsigned long dword;
#ifndef MESSAGENAME_EMS_4
#define MESSAGENAME_EMS_4
typedef union
{
    unsigned char      m_aucData[64];
    unsigned short int m_auwData[4];
    unsigned long  int m_aulData[2];

    struct
    {
        unsigned int EMS_MilSt        :  1 ;
        unsigned int EMS_EPCSt        :  1 ;
        unsigned int EMS_EngSt        :  1 ;
        unsigned int EMS_EMSReleased  :  2 ;
        unsigned int EMS_StartStopSt  :  2 ;
        unsigned int EMS_FuelLevelVD  :  1 ;
        unsigned int EMS_FuelLevel    :  8 ;
        unsigned int                  :  8 ;
        unsigned int EMS_FuelPulse    : 16 ;
        unsigned int EMS_BattSOC      :  8 ;
        unsigned int EMS_EngCrankSt   :  1 ;
        unsigned int                  :  5 ;
        unsigned int EMS_StartStopSwitchSt :  1 ;
        unsigned int                  : 17 ;
    };
} EMS_4_;
typedef struct
{
    unsigned int  m_unMsgID       ;
    unsigned char m_ucEXTENDED    ;
    unsigned char m_ucRTR         ;
    unsigned char m_ucDLC         ;
    unsigned char m_ucChannel   ;
    EMS_4_        m_sWhichBit     ;

    UINT m_ulTimeStamp;
    bool             m_bCANFD        ;

} EMS_4;
#endif /* MESSAGENAME_EMS_4 */
typedef enum
{
    TestIdle,
    TestInExtMod,
    TestSKNotLearn,
    TestSkLearned,
    TestNeedInit,
    TestNotSup
} wfs17TestStates;

#define     WFS17_MSG_INDEX_EXTMOD      0
#define     WFS17_MSG_INDEX_READSTATE   WFS17_MSG_INDEX_EXTMOD+1
#define     WFS17_MSG_INDEX_SEC_ASS     WFS17_MSG_INDEX_READSTATE+1
#define     WFS17_MSG_INDEX_WRI_SK      WFS17_MSG_INDEX_SEC_ASS+1
#define     WFS17_MSG_INDEX_REA_SK      WFS17_MSG_INDEX_WRI_SK+1
#define     WFS17_MSG_INDEX_FLW_REP     WFS17_MSG_INDEX_REA_SK+1


#define     WFS17_TIP_IIDEX_REQ_EXT     1
#define     WFS17_TIP_IIDEX_READ_STA    WFS17_TIP_IIDEX_REQ_EXT+1
#define     WFS17_TIP_IIDEX_SEC_ACC     WFS17_TIP_IIDEX_READ_STA+1
#define     WFS17_TIP_IIDEX_WRI_SK      WFS17_TIP_IIDEX_SEC_ACC+1
#define     WFS17_TIP_IIDEX_READ_SK     WFS17_TIP_IIDEX_WRI_SK+1
#define     WFS17_TIP_IIDEX_EMS_CHA          WFS17_TIP_IIDEX_READ_SK+1
#define     WFS17_TIP_IIDEX_IMMO_REP      WFS17_TIP_IIDEX_EMS_CHA+1
#define     WFS17_TIP_IIDEX_OK          WFS17_TIP_IIDEX_IMMO_REP+1
#define     WFS17_TIP_IIDEX_FAILED      WFS17_TIP_IIDEX_OK+1

class Sia_17_Protocol : public CWinApp
{
    DECLARE_DYNCREATE(Sia_17_Protocol)

protected:
    // 动态创建所使用的受保护的构造函数


public:
    Sia_17_Protocol();
    virtual ~Sia_17_Protocol();
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    chkMsgRsp checkRMSResp(STCAN_MSG msg);
    void prepareImmoResp(STCAN_MSG msg);
    void sendImmoMsg(int id);

    EMS_4 m_wfs17Ems4Msg;
    int m_wfs17SubType;
    int m_wfs17UdsRespID;
    int m_wfs17UdsSendID;
    int m_wfs17ImmoRespID;
    int m_wfs17EmsChallangeID;
    int m_wfs17SendMsgLen;
    int m_wfs17SendCount;
    bool enEcmChlRsp;
    unsigned char m_wfs17SendMsgData[100];
    unsigned short m_wfs17Uds22Id;
    unsigned short m_wfs17Uds23Id;
    STCAN_MSG m_wfs17CanMsg;
    wfs17TestStates m_wfs17TestStates;
protected:
    DECLARE_MESSAGE_MAP()
};


