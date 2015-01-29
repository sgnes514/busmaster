// Sia_17_Protocol.cpp : 实现文件
//

#include "stdafx.h"
#include "UDS_Protocol.h"
#include "Sia_Protocol.h"

#include "UDS_Extern.h"

STCAN_MSG PEPS_3 = {0x110, 0, 0, 0, 0};
STCAN_MSG Resp = {0x38, 0, 0, 0, 0};
UCHAR smtRadm[8];
UCHAR smtSKArray[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
UCHAR ESKConst[16] = {0x41, 0x46, 0x5F, 0x45, 0x4D, 0x53, 0x42, 0x43, 0x4D, 0x50, 0x45, 0x50, 0x53, 0x54, 0x45, 0x4C};
UCHAR smtSK[16];
UCHAR ICUkey[8];




void Utils_IMMO_ad_calcKey();
void Utils_IMMO_af_calcKey();

unsigned char wfs17MsgList[][20] =
{
    {0x02, 0x10, 0x03},
    {0x03, 0x22, 0x02, 0x02},
    {0x02, 0x27, 0x03},
    {0x13, 0x2e, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
    {0x03, 0x22, 0x02, 0x03},
    {0x08, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};
char wfs17Tips[][50] =
{
    {"Request into extended Diagnostic Session"},
    {"Read EMS state"},
    {"Request security access"},
    {"Write SK"},
    {"Read SK"},
    {"EMS challenge"},
    {"IMMO response"},
    {"Failed"},
    {"OK"}
};
char wfs17SubType[2][MAX_NAME_LENGTH] =
{
    "GAC_AD",
    "GAC_AF"
};

// Sia_17_Protocol

IMPLEMENT_DYNCREATE(Sia_17_Protocol, CWinApp)

Sia_17_Protocol::Sia_17_Protocol()
{
    // omWfs17Ptr = this;
    m_wfs17TestStates = TestIdle;
    m_wfs17SubType = 0;
    m_wfs17UdsRespID = 0x7e8;
    m_wfs17UdsSendID = 0x7e0;
    m_wfs17ImmoRespID = 0x38;
    m_wfs17EmsChallangeID = 0x31;
    m_wfs17Ems4Msg.m_unMsgID = 0x373;
    // m_wfs17CanMsg = new msTXMSGDATA;
    //m_wfs17CanMsg.m_unCount = 1;
    // m_wfs17CanMsg.m_psTxMsg = new STCAN_MSG[1];
    m_wfs17CanMsg.m_unMsgID = m_wfs17UdsSendID;              // Message ID
    m_wfs17CanMsg.m_ucEXTENDED = FALSE;           // Standard Message type
    m_wfs17CanMsg.m_ucRTR = FALSE;                // Not RTR type
    m_wfs17CanMsg.m_ucChannel = 1;                // First CAN channel
}

Sia_17_Protocol::~Sia_17_Protocol()
{
}

BOOL Sia_17_Protocol::InitInstance()
{
    // TODO: 在此执行任意逐线程初始化
    return TRUE;
}

int Sia_17_Protocol::ExitInstance()
{
    // TODO: 在此执行任意逐线程清理
    return CWinApp::ExitInstance();
}
chkMsgRsp Sia_17_Protocol::checkRMSResp(STCAN_MSG msg)
{
    if(msg.m_unMsgID == m_wfs17UdsRespID)
    {
        if((msg.m_ucData[1] == 0x50) && (msg.m_ucData[2] == 0x03))
        {
            sendToTraceWin(wfs17Tips[WFS17_TIP_IIDEX_OK], 0);
            sendImmoMsg(WFS17_MSG_INDEX_READSTATE);
            m_wfs17TestStates = TestInExtMod;
        }
        else if((msg.m_ucData[1] == 0x62) && (msg.m_ucData[2] == 0x02)
                && (msg.m_ucData[3] == 0x02))
        {
            if((msg.m_ucData[4] == 0x02))
            {
                if(m_wfs17TestStates == TestSkLearned)
                {
                    //::MessageBox(NULL, _T(""), _T("ECU未初始化防盗"), MB_OK);
                }
                else
                {
                    sendToTraceWin(wfs17Tips[WFS17_TIP_IIDEX_OK], 0);
                    sendImmoMsg(WFS17_MSG_INDEX_SEC_ASS);
                }
            }
            else if(msg.m_ucData[4] = 0x01)
            {
                //m_wfs17TestStates = TestSKNotLearn;
                if(m_wfs17TestStates == TestSKNotLearn)
                {
                    sendToTraceWin(wfs17Tips[WFS17_TIP_IIDEX_FAILED], 0);
                    //sendImmoMsg(WFS17_MSG_INDEX_SEC_ASS);
                }
                else if(m_wfs17TestStates == TestSkLearned)
                {
                    sendToTraceWin(wfs17Tips[WFS17_TIP_IIDEX_OK], 0);
                    sendImmoMsg(WFS17_MSG_INDEX_SEC_ASS);
                    //::MessageBox(NULL, _T("请确认是否已经学习过SK"), _T("ECU状态错误"), MB_OK);
                }
            }
        }
        else if((msg.m_ucData[1] == 0x67) && (msg.m_ucData[2] == 0x03)
               )
        {
            if((msg.m_ucData[3] == 0x00) && (msg.m_ucData[4] == 0x00)
                    && (msg.m_ucData[5] == 0x00) && (msg.m_ucData[6] == 0x00))
            {
                sendToTraceWin(wfs17Tips[WFS17_TIP_IIDEX_OK], 0);

                if(m_wfs17TestStates == TestSKNotLearn)
                {
                    sendImmoMsg(WFS17_MSG_INDEX_WRI_SK);
                }
                else if(m_wfs17TestStates == TestSkLearned)
                {
                    sendImmoMsg(WFS17_MSG_INDEX_REA_SK);
                }
            }
            else
            {
                // ::MessageBox(NULL, _T("本程序不支持安全验证的防盗测试"), _T("注意"), MB_OK);
                sendToTraceWin(wfs17Tips[WFS17_TIP_IIDEX_FAILED], 0);
                m_wfs17TestStates = TestNotSup;
            }
        }
        else if(msg.m_ucData[0] = 0x30)
        {
            int j = 0;

            while(m_wfs17SendCount < m_wfs17SendMsgLen)
            {
                memset(m_wfs17CanMsg.m_ucData, 0, 8);
                int i = m_wfs17SendMsgLen - m_wfs17SendCount;
                i = (i <= 7) ? i : 7;
                m_wfs17CanMsg.m_ucData[0] = 0x21 + j;
                j++;
                memcpy(m_wfs17CanMsg.m_ucData + 1, m_wfs17SendMsgData + m_wfs17SendCount, i);
                m_wfs17CanMsg.m_unMsgID = m_wfs17UdsSendID;
                m_wfs17CanMsg.m_ucDataLen = 8;
                g_pouDIL_CAN_Interface1->DILC_SendMsg(g_dwClientID1, m_wfs17CanMsg);
                m_wfs17SendCount += i;
            }
        }
        else if(msg.m_ucData[1] == 0x6e
                && msg.m_ucData[2] == 0x02
                && msg.m_ucData[3] == 0x03)
        {
            m_wfs17TestStates = TestSkLearned;
            sendToTraceWin(wfs17Tips[WFS17_TIP_IIDEX_OK], 0);
            //  ::MessageBox(NULL, _T("防盗匹配完成，断开KL15 5秒钟后重新上电"), _T("注意"), MB_OK);
        }
        else if(msg.m_ucData[0] == 0x10
               ) //10 13 62 02 03
        {
            sendImmoMsg(WFS17_MSG_INDEX_FLW_REP);
#if 0

            if(msg.m_ucData[1] == 0x13
                    && msg.m_ucData[2] == 0x62
                    && msg.m_ucData[3] == 0x02
                    && msg.m_ucData[4] == 0x03)
            {
            }

#endif
        }
    }
    else if(msg.m_unMsgID == m_wfs17EmsChallangeID)
    {
        prepareImmoResp(msg);
    }
    else if(msg.m_unMsgID == m_wfs17Ems4Msg.m_unMsgID)
    {
        memcpy(m_wfs17Ems4Msg.m_sWhichBit.m_aucData, msg.m_ucData, 8);
    }

    return PosMsgRsp;
}
void Sia_17_Protocol::sendImmoMsg(int id)
{
    unsigned char buff[100];
    m_wfs17SendMsgLen = wfs17MsgList[id][0];
    sendToTraceWin(wfs17Tips[id], 0);

    if(m_wfs17SendMsgLen <= 7)
    {
        m_wfs17CanMsg.m_unMsgID = m_wfs17UdsSendID;
        m_wfs17CanMsg.m_ucDataLen = 8;
        memset(m_wfs17CanMsg.m_ucData, 0, sizeof(m_wfs17CanMsg.m_ucData));
        memcpy(m_wfs17CanMsg.m_ucData, wfs17MsgList[id], wfs17MsgList[id][0] + 1);
        g_pouDIL_CAN_Interface1->DILC_SendMsg(g_dwClientID1, m_wfs17CanMsg);
    }
    else
    {
        memset(m_wfs17SendMsgData, 0, sizeof(m_wfs17SendMsgData));
        m_wfs17CanMsg.m_ucData[0] = 0x10;
        memcpy(m_wfs17CanMsg.m_ucData + 1, wfs17MsgList[id], 7);
        memcpy(m_wfs17SendMsgData, wfs17MsgList[id], wfs17MsgList[id][0] + 1);
        m_wfs17CanMsg.m_unMsgID = m_wfs17UdsSendID;
        m_wfs17CanMsg.m_ucDataLen = 8;
        g_pouDIL_CAN_Interface1->DILC_SendMsg(g_dwClientID1, m_wfs17CanMsg);
        m_wfs17SendCount = 6;
    }
}
void Sia_17_Protocol::prepareImmoResp(STCAN_MSG msg)
{
    if(enEcmChlRsp == 0)
    {
        return;
    }

    memcpy(smtRadm, msg.m_ucData, 8);
    sendToTraceWin(wfs17Tips[WFS17_TIP_IIDEX_EMS_CHA], 1);

    if(m_wfs17SubType == 0)
    {
        Utils_IMMO_ad_calcKey();
    }
    else if(m_wfs17SubType == 1)
    {
        Utils_IMMO_af_calcKey();
    }

    m_wfs17CanMsg.m_unMsgID = m_wfs17ImmoRespID;
    m_wfs17CanMsg.m_ucDataLen = 8;
    m_wfs17CanMsg.m_ucData[0] = 0x0;
    m_wfs17CanMsg.m_ucData[1] = 0x0;
    memcpy(m_wfs17CanMsg.m_ucData + 2, ICUkey, 6);
    sendToTraceWin(wfs17Tips[WFS17_TIP_IIDEX_IMMO_REP], 0);
    g_pouDIL_CAN_Interface1->DILC_SendMsg(g_dwClientID1, m_wfs17CanMsg);
}
BEGIN_MESSAGE_MAP(Sia_17_Protocol, CWinApp)
END_MESSAGE_MAP()
/* Start BUSMASTER generated function - Utils_IMMO_calcKey */
void Utils_IMMO_ad_calcKey()
{
    /* TODO */
    static UCHAR num_rounds, i, j;
    static UINT  v[2], v0, v1;
    static UINT  k[4];
    static UINT  sum = 0, delta = 0x9e3779b9;

    //delta = 0x21B6A05D;
    for(i = 0; i < 4; i++)
    {
        k[i] = ((UINT)smtSKArray[4 * i + 3]) | (((UINT)smtSKArray[4 * i + 2]) << 8) | (((UINT)smtSKArray[4 * i + 1]) << 16) | (((UINT)smtSKArray[4 * i]) << 24);

        if(i < 2)
        {
            v[i] = ((UINT)smtRadm[4 * i + 3]) | (((UINT)smtRadm[4 * i + 2]) << 8) | (((UINT)smtRadm[4 * i + 1]) << 16) | (((UINT)smtRadm[4 * i]) << 24);
        }
    }

    num_rounds = 32;
    v0 = v[0];
    v1 = v[1];

    for(i = 0; i < num_rounds; i++)
    {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
    }

    v[0] = v0;
    v[1] = v1;
    j = 4;

    for(i = 0; i < 4; i++)
    {
        j--;
        ICUkey[i] = (UCHAR)(v[0] >> (8 * j));
        ICUkey[i + 4] = (UCHAR)(v[1] >> (8 * j));
    }

    /* header 315  */    return;
}/* End BUSMASTER generated function - Utils_IMMO_calcKey */
void Utils_IMMO_af_calcKey()
{
    /* TODO */
    static UCHAR num_rounds, i, j;
    static UINT  v[2], v0, v1;
    static UINT  k[4];
    static UINT  sum = 0, delta = 0x9e3779b9;

    //delta = 0x21B6A05D;
    for(i = 0; i < 16; i++)
    {
        smtSK[i] = smtSKArray[i] ^ ESKConst[i];
    }

    for(i = 0; i < 4; i++)
    {
        k[i] = ((UINT)smtSK[4 * i + 3]) | (((UINT)smtSK[4 * i + 2]) << 8) | (((UINT)smtSK[4 * i + 1]) << 16) | (((UINT)smtSK[4 * i]) << 24);

        if(i < 2)
        {
            v[i] = ((UINT)smtRadm[4 * i + 3]) | (((UINT)smtRadm[4 * i + 2]) << 8) | (((UINT)smtRadm[4 * i + 1]) << 16) | (((UINT)smtRadm[4 * i]) << 24);
        }
    }

    num_rounds = 32;
    v0 = v[0];
    v1 = v[1];

    for(i = 0; i < num_rounds; i++)
    {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
    }

    v[0] = v0;
    v[1] = v1;
    j = 4;

    for(i = 0; i < 4; i++)
    {
        j--;
        ICUkey[i] = (UCHAR)(v[0] >> (8 * j));
        ICUkey[i + 4] = (UCHAR)(v[1] >> (8 * j));
    }

    /* header 315  */    return;
}
// Sia_17_Protocol 消息处理程序
