// Sia_16_Protocol.cpp : 实现文件
//
#include <afxpriv.h>
#include "stdafx.h"
#include "UDS_Extern.h"
#include "UDS_Protocol.h"
#include "Sia_16_Protocol.h"


int ChangeToBcd(char iChTemp) ;
void prepareImmoResp(STCAN_MSG RxMsg);
VOID Utils_XTEADecrypt(USHORT v[], USHORT n, USHORT k[]);
VOID Utils_XTEAEncrypt(USHORT v[], USHORT n, USHORT k[]);


unsigned char m_diasImmoMsgList[10][8] =
{
    {0xB1, 0x68, 0x5C, 0x55, 0x8D, 0x6B, 0xDA, 0xA2},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x4E, 0xA7, 0x73, 0x3A, 0x92, 0x0E, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xCF, 0xCD, 0x73, 0xE3, 0x38, 0xE1, 0xC3, 0xB2},
    {0xB1, 0x68, 0x5C, 0x55, 0x8D, 0x6B, 0xDA, 0xA2},
    {0x2B, 0x02, 0xA4, 0xFD, 0xC6, 0xF8, 0x62, 0xC5},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7},
    {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7}
};
char m_wfs16_tips[][35] =
{
    {"Send learn SK msg"},
    {"Learn SK OK"},
    {"Send learn PIN msg"},
    {"Learn PIN OK,KL15 off 5 sec"},
    {"Send reset SK msg"},
    {"Reset SK OK"},
    {"Send reset EMS msg"},
    {"Reset EMS OK"},
    {"IMMO response"},
    {"EMS Challenge"}
};
const char diasImmoMsgCfg [10][40] =
{
    "ID_MESSAGE_IMMO_LRNSK_REQ",
    "ID_MESSAGE_ECM_LRNSK_RSP",
    "ID_MESSAGE_IMMO_LRNPIN_REQ",
    "ID_MESSAGE_ECM_LRNPIN_RSP",
    "ID_MESSAGE_IMMO_RPTSK_REQ",
    "ID_MESSAGE_ECM_RPTSK_RSP",
    "ID_MESSAGE_IMMO_RESET_REQ",
    "ID_MESSAGE_ECM_RESET_RSP",
    "ID_MESSAGE_IMMO_CHALLRSP",
    "ID_MESSAGE_ECM_CHALLENGE_REQ"

};

const char emsRspMsgTip[12][150] =
{
    "SERVICE  COMPLETED  SUCCESSFULLY",
    "GENERAL REJECT  Indicates that the requested action will not be taken but does not  provide any reason or explanation. ",
    "SERVICE  NOT  SUPPORTED   The requested service is not implemented in the ECM.  ",
    "BUSY-REPEAT  REQUEST   The ECM is currently busy.  ",
    "Virgin  EMS  WITHOUT  SK  Indicates that the EMS is a virgin one and has no SK. ",
    "Virgin  EMS  WITHOUT  PIN  Indicates that the EMS is a virgin one and has no PIN. ",
    "Programmed  EMS  WITH SK  Indicates that the EMS has already learned the SK.  ",
    "Programmed  EMS  WITH PIN  Indicates that the EMS has already learned the PIN. ",
    "Programmed  EMS  WITH WRONG SK  Indicates that the EMS has already learned the SK, but the SK is not  match the one in the SIM.   ",
    "Programmed  EMS  WITH WRONG PIN  Indicates that the EMS has already learned the PIN is not match the  one in the SIM.  ",
    "Check Sum error  Indicates that the check sum of SK or PIN is not correct.  ",
    "Invalid PIN  Indicates that the requested action will not be taken because the PIN  is wrong."
};

STCAN_MSG IMMOResponse = {0x51d, 0, 0, 0, 0};
UCHAR Chanllenge[8];
UCHAR Response[8];
USHORT ArrayTempU16[4];
USHORT SKArray[4] = {0xDA3B, 0x1511, 0x2233, 0xD7AB};
USHORT ArrayRDNPIN[4];


// Sia_16_Protocol

IMPLEMENT_DYNCREATE(Sia_16_Protocol, CWinApp)

Sia_16_Protocol::Sia_16_Protocol()
{
    //omWfs16Ptr = this;
    m_immoCfgHead = NULL;
    immoTxMsg = NULL;
    m_curSendMsgCnt = 0;
    m_curSendMsgID = 0;
}

Sia_16_Protocol::~Sia_16_Protocol()
{
    freeImmoCfgStr();

    if(immoTxMsg != NULL)
    {
        delete((immoTxMsg->m_psTxMsg));
        delete(immoTxMsg);
    }
}

BOOL Sia_16_Protocol::InitInstance()
{
    // TODO: 在此执行任意逐线程初始化
    return TRUE;
}

int Sia_16_Protocol::ExitInstance()
{
    // TODO: 在此执行任意逐线程清理
    return CWinApp::ExitInstance();
}

BEGIN_MESSAGE_MAP(Sia_16_Protocol, CWinApp)
END_MESSAGE_MAP()


int Sia_16_Protocol::LoadDiasCanCfg()
{
    xmlDocPtr doc;           //定义解析文档指针
    xmlNodePtr curNode;      //定义结点指针(你需要它为了在各个结点间移动)
    xmlChar *szKey;          //临时字符串变量
    char *szDocName;
    char cfgFileName[100];
    GetCurrentDirectory(100, cfgFileName);
    strcat(cfgFileName, "\\immo.xml");
    //char filename[] = "d:\\immo.xml";
    int i = 0;
    int res = 0, j = 0;
    doc = xmlReadFile(cfgFileName, "GB2312", XML_PARSE_RECOVER); //解析文件

    if(NULL == doc)
    {
        return -1;
    }

    curNode = xmlDocGetRootElement(doc); //确定文档根元素

    /*检查确认当前文档中包含内容*/
    if(NULL == curNode)
    {
        xmlFreeDoc(doc);
        return -1;
    }

    /*在这个例子中，我们需要确认文档是正确的类型。“immotest”是在这个示例中使用文档的根类型。*/
    if(xmlStrcmp(curNode->name, BAD_CAST "immotest"))
    {
        xmlFreeDoc(doc);
        return -1;
    }

    freeImmoCfgStr();
    curNode = curNode->xmlChildrenNode;
    curNode = curNode->next;
    xmlNodePtr propNodePtr = curNode;

    while(propNodePtr != NULL)
    {
        //取出节点中的内容
        curNode = propNodePtr;

        if(xmlStrcmp(curNode->name, (const xmlChar *)"canvers"))
        {
            xmlFree(curNode);
        }
        else
        {
            propNodePtr = curNode->next->next;
            immocfg* tmp = new immocfg;
            memset(tmp, 0, sizeof(immocfg));
            szKey = curNode->properties->children->content;
            CString s(szKey);
            tmp->Canvers.CString::CString();
            tmp->Canvers = s;//atoi((char *)szKey);
            curNode = curNode->children;
            curNode = curNode->next;
            i = 0;

            while(curNode != NULL && i <= 10)
            {
                if(0 == strcmp(diasImmoMsgCfg[i], (char *)(xmlChar *)curNode->name))
                {
                    CString tmp1((char *)(curNode->children->content));
                    res = 0;

                    for(j = 0; j < tmp1.GetLength(); j++)
                    {
                        res |= ChangeToBcd(tmp1.GetAt(j)) << ((tmp1.GetLength() - j - 1) * 4);
                    }

                    tmp->immoIdList[i] = res;
                }
                else
                {
                    free(tmp);
                    return -1;
                }

                //printf("Canvers: %s,%s", curNode->name,curNode->children->content);
                curNode = curNode->next->next;
                i++;
            }

            addImmoCfg(tmp);
        }
    }

    xmlFreeDoc(doc);
    return 0;
}

void Sia_16_Protocol::updateImmoId()
{
    int i = 0;
    immocfg* tmp = m_immoCfgHead;

    if(subTypeCount > 0)
    {
        int iPos = subTypeSel;

        while(i < iPos)
        {
            tmp = tmp->next;
            i++;
        }

        if(tmp != NULL)
        {
            memcpy(m_immoMsgId, tmp->immoIdList, 10 * sizeof(int));
        }
        else
        {
            memset(m_immoMsgId, 0, 10 * sizeof(int));
        }
    }
}

int ChangeToBcd(char iChTemp)               //转换单个字符
{
    int m;

    if(iChTemp >= '0' && iChTemp <= '9')
    {
        m = iChTemp - '0';
    }
    else if(iChTemp >= 'A' && iChTemp <= 'Z')
    {
        m = iChTemp - 'A' + 10;
    }
    else if(iChTemp >= 'a' && iChTemp <= 'z')
    {
        m = iChTemp - 'a' + 10;
    }

    return m;
}
void Sia_16_Protocol::initImmoMsg()
{
    int i = 0;

    if(immoTxMsg == NULL)
    {
        immoTxMsg = new msTXMSGDATA;
    }

    immoTxMsg->m_unCount = 10;
    immoTxMsg->m_psTxMsg = new STCAN_MSG[10];

    for(i = 0; i < 10; i++)
    {
        immoTxMsg->m_psTxMsg[i].m_ucChannel = 1;
        immoTxMsg->m_psTxMsg[i].m_ucDataLen = 8;
        immoTxMsg->m_psTxMsg[i].m_ucRTR = FALSE;
        immoTxMsg->m_psTxMsg[i].m_ucEXTENDED = FALSE;
        immoTxMsg->m_psTxMsg[i].m_unMsgID = m_immoMsgId[i];
        memcpy(immoTxMsg->m_psTxMsg[i].m_ucData, m_diasImmoMsgList[i], 8);
    }
}
void Sia_16_Protocol::addImmoCfg(immocfg* node)
{
    if(m_immoCfgHead == NULL)
    {
        m_immoCfgHead = node;
    }
    else
    {
        immocfg* tmp = m_immoCfgHead;

        while(tmp != NULL)
        {
            if(tmp->next == NULL)
            {
                tmp->next = node;
                return;
            }

            tmp->next = tmp;
        }
    }
}
void Sia_16_Protocol::freeImmoCfgStr()
{
    immocfg* tmp = m_immoCfgHead;
    immocfg* tmp1 = NULL;

    while(tmp != NULL)
    {
        tmp1 = tmp->next;
        delete(tmp);
        tmp = NULL;
        tmp = tmp1;
    }

    m_immoCfgHead = NULL;
}
void Sia_16_Protocol::updateCustomerCombox()
{
}
void Sia_16_Protocol::checkCfgValid()
{
}

int Sia_16_Protocol::sendImmomsg(int id)
{
    if(immoTxMsg != NULL)
    {
        if(immoTxMsg->m_psTxMsg[id].m_unMsgID != 0)
        {
            immoTxMsg->m_psTxMsg[id].m_unMsgID = m_immoMsgId[id];
            m_curSendMsgID = id;
            m_curSendMsgCnt++;
            int nReturn = g_pouDIL_CAN_Interface1->DILC_SendMsg(g_dwClientID1, immoTxMsg->m_psTxMsg[id]);
            sendToTraceWin(m_wfs16_tips[id], 0);
            // sendToTraceWin(m_wfs16_tips[id], 0);
        }

        return 0;
    }

    return 0;
}
/************************************************************************/
/*
 *CODE   TITLE   MEANING
00h  SERVICE  COMPLETED  SUCCESSFULLY  The requested service is implemented in the ECM.
10h  GENERAL REJECT  Indicates that the requested action will not be taken but does not  provide any reason or explanation.
11h   SERVICE  NOT  SUPPORTED   The requested service is not implemented in the ECM.
21h   BUSY-REPEAT  REQUEST   The ECM is currently busy.
31h  Virgin  EMS  WITHOUT  SK  Indicates that the EMS is a virgin one and has no SK.
32h  Virgin  EMS  WITHOUT  PIN  Indicates that the EMS is a virgin one and has no PIN.
41h  Programmed  EMS  WITH SK  Indicates that the EMS has already learned the SK.
42h  Programmed  EMS  WITH PIN  Indicates that the EMS has already learned the PIN.
51h  Programmed  EMS  WITH WRONG SK  Indicates that the EMS has already learned the SK, but the SK is not  match the one in the SIM.
52h  Programmed  EMS  WITH WRONG PIN  Indicates that the EMS has already learned the PIN is not match the  one in the SIM.
61h  Check Sum error  Indicates that the check sum of SK or PIN is not correct.
71h  Invalid PIN  Indicates that the requested action will not be taken because the PIN  is wrong.                                                                       */
/************************************************************************/
//#define EN_ERROR_REPORT
chkMsgRsp Sia_16_Protocol::checkRMSResp(STCAN_MSG Mensage)
{
    chkMsgRsp res = NoRelMsg;
    int i = 0;

    if(Mensage.m_unMsgID == m_immoMsgId[m_curSendMsgID + 1])
    {
        if(m_curSendMsgID == 4)
        {
            if(
                Mensage.m_ucData[0] == m_diasImmoMsgList[m_curSendMsgID + 1][0]
                && Mensage.m_ucData[1] == m_diasImmoMsgList[m_curSendMsgID + 1][1]
                && Mensage.m_ucData[2] == m_diasImmoMsgList[m_curSendMsgID + 1][2]
                && Mensage.m_ucData[3] == m_diasImmoMsgList[m_curSendMsgID + 1][3]
                && Mensage.m_ucData[4] == m_diasImmoMsgList[m_curSendMsgID + 1][4]
                && Mensage.m_ucData[5] == m_diasImmoMsgList[m_curSendMsgID + 1][5]
                && Mensage.m_ucData[6] == m_diasImmoMsgList[m_curSendMsgID + 1][6]
                && Mensage.m_ucData[7] == m_diasImmoMsgList[m_curSendMsgID + 1][7])
            {
                // sendToTraceWin("OK", 1);
                sendToTraceWin("OK", 1);
                sendImmomsg(m_curSendMsgID + 2);
                res = PosMsgRsp;
            }
        }
        else
        {
            int i = 0;

            switch(Mensage.m_ucData[0])
            {
                case 00:
                    i = 0;

                    if(m_curSendMsgID == INDEX_LEARN_SK || m_curSendMsgID == INDEX_RESET_SK)
                    {
                        res = PosMsgRspSendNext;
                    }
                    else
                    {
                        res = PosMsgRsp;
                    }

                    break;

                case 10:
                    i = 1;
                    break;

                case 11:
                    i = 2;
                    break;

                case 21:
                    i = 3;
                    break;

                case 31:
                    i = 4;
                    break;

                case 32:
                    i = 5;
                    break;

                case 41:
                    i = 6;
                    break;

                case 42:
                    i = 7;
                    break;

                case 51:
                    i = 8;
                    break;

                case 52:
                    i = 9;
                    break;

                case 61:
                    i = 10;
                    break;

                case 71:
                    i = 11;
                    break;
            }

            sendToTraceWin((char *)emsRspMsgTip[i], 1);
            //sendToTraceWin((char *)emsRspMsgTip[i], 1);

            if(Mensage.m_ucData[0] != 0)
            {
                //  USES_CONVERSION;
                //  LPSTR bufftmp = T2A(title);
                //::MessageBox(NULL, item,title,  MB_OK);
                res = NegMsgRsp;
            }

            if(res == 0)
            {
                sendImmomsg(m_curSendMsgID + 2);
            }
        }
    }

#if 0

    if(Mensage.m_unMsgID == m_immoMsgId[m_curSendMsgID + 1]
#ifdef EN_ERROR_REPORT
            && Mensage.m_ucData[0] == m_diasImmoMsgList[m_curSendMsgID + 1][0]
#endif
            && Mensage.m_ucData[1] == m_diasImmoMsgList[m_curSendMsgID + 1][1]
            && Mensage.m_ucData[2] == m_diasImmoMsgList[m_curSendMsgID + 1][2]
            && Mensage.m_ucData[3] == m_diasImmoMsgList[m_curSendMsgID + 1][3]
            && Mensage.m_ucData[4] == m_diasImmoMsgList[m_curSendMsgID + 1][4]
            && Mensage.m_ucData[5] == m_diasImmoMsgList[m_curSendMsgID + 1][5]
            && Mensage.m_ucData[6] == m_diasImmoMsgList[m_curSendMsgID + 1][6]
            && Mensage.m_ucData[7] == m_diasImmoMsgList[m_curSendMsgID + 1][7])
    {
#ifdef EN_ERROR_REPORT

        if(Mensage.m_ucData[0] == m_diasImmoMsgList[m_curSendMsgID + 1][0])
#endif
        {
            sendToTraceWin(m_wfs16_tips[m_curSendMsgID + 1], 1);

            if(m_curSendMsgID == INDEX_LEARN_SK || m_curSendMsgID == INDEX_RESET_SK)
            {
                sendImmomsg(m_curSendMsgID + 2);
                res = 0;
            }
            else
            {
                res = 1;
            }
        }

#ifdef EN_ERROR_REPORT
        else
        {
            CString title, item;

            switch(Mensage.m_ucData[0])
            {
                case 00:
                    title = "SERVICE  COMPLETED  SUCCESSFULLY";
                    item = "The requested service is implemented in the ECM. ";
                    break;

                case 10:
                    title = "GENERAL REJECT" ;
                    item = "Indicates that the requested action will not be taken but does not  provide any reason or explanation. ";
                    break;

                case 11:
                    title = " SERVICE  NOT  SUPPORTED";
                    item =   "The requested service is not implemented in the ECM.  ";
                    break;

                case 21:
                    title = " BUSY-REPEAT  REQUEST";
                    item =   "The ECM is currently busy.  ";
                    break;

                case 31:
                    title = "Virgin  EMS  WITHOUT  SK" ;
                    item = "Indicates that the EMS is a virgin one and has no SK. ";
                    break;

                case 32:
                    title = "Virgin  EMS  WITHOUT  PIN" ;
                    item = "Indicates that the EMS is a virgin one and has no PIN. ";
                    break;

                case 41:
                    title = "Programmed  EMS  WITH SK";
                    item =  "Indicates that the EMS has already learned the SK.  ";
                    break;

                case 42:
                    title = "Programmed  EMS  WITH PIN";
                    item =  "Indicates that the EMS has already learned the PIN. ";
                    break;

                case 51:
                    title = "Programmed  EMS  WITH WRONG SK" ;
                    item = "Indicates that the EMS has already learned the SK, but the SK is not  match the one in the SIM.  ";
                    break;

                case 52:
                    title = "Programmed  EMS  WITH WRONG PIN";
                    item =  "Indicates that the EMS has already learned the PIN is not match the  one in the SIM.  ";
                    break;

                case 61:
                    title = "Check Sum error" ;
                    item = "Indicates that the check sum of SK or PIN is not correct. " ;
                    break;

                case 71:
                    title = "Invalid PIN" ;
                    item = "Indicates that the requested action will not be taken because the PIN  is wrong. ";
                    break;
            }

            ::MessageBox(NULL, item, title,  MB_OK);
            res = 1;
        }

#endif
    }

#endif
    else if(Mensage.m_unMsgID == m_immoMsgId[INDEX_ECM_CHALLANGE]
            && enEcmChlRsp)
    {
        prepareImmoResp(Mensage);
        res = EmsChall;
    }
    else
        res = NoRelMsg;

    return res;
}
/* Start BUSMASTER generated function - prepareImmoResp */
void Sia_16_Protocol::prepareImmoResp(STCAN_MSG RxMsg)
{
    memcpy(Chanllenge, RxMsg.m_ucData, 8);
    memcpy(m_diasImmoMsgList[INDEX_ECM_CHALLANGE], RxMsg.m_ucData, 8);
    sendToTraceWin(m_wfs16_tips[INDEX_ECM_CHALLANGE], 1);
    ArrayTempU16[0] = (Chanllenge[0] << 8) | Chanllenge[1];
    ArrayTempU16[1] = (Chanllenge[2] << 8) | Chanllenge[3];
    ArrayTempU16[2] = (Chanllenge[4] << 8) | Chanllenge[5];
    ArrayTempU16[3] = (Chanllenge[6] << 8) | Chanllenge[7];
    Utils_XTEADecrypt(ArrayTempU16, 4, SKArray);

    if((ArrayTempU16[2] == 0xAABB) && (ArrayTempU16[3] == 0xCCDD))
    {
        ArrayRDNPIN[0] = ArrayTempU16[2];
        ArrayRDNPIN[1] = ArrayTempU16[3];
        ArrayRDNPIN[2] = ArrayTempU16[0];
        ArrayRDNPIN[3] = ArrayTempU16[1];
        Utils_XTEAEncrypt(ArrayRDNPIN, 4, SKArray);
        Response[0] = (ArrayRDNPIN[0] >> 8) & 0xFF;
        Response[1] = (ArrayRDNPIN[0]) & 0xFF;
        Response[2] = (ArrayRDNPIN[1] >> 8) & 0xFF;
        Response[3] = (ArrayRDNPIN[1]) & 0xFF;
        Response[4] = (ArrayRDNPIN[2] >> 8) & 0xFF;
        Response[5] = (ArrayRDNPIN[2]) & 0xFF;
        Response[6] = (ArrayRDNPIN[3] >> 8) & 0xFF;
        Response[7] = (ArrayRDNPIN[3]) & 0xFF;
        IMMOResponse.m_ucDataLen = 8;
        memcpy(IMMOResponse.m_ucData, Response, 8);
        memcpy(m_diasImmoMsgList[INDEX_ECM_CHALLANGE_RESP], Response, 8);
    }
    else
    {
        IMMOResponse.m_ucDataLen = 8;
        memset(IMMOResponse.m_ucData, 0xff, 8);
    }

    IMMOResponse.m_ucChannel = 1;                // First CAN channel
    IMMOResponse.m_ucEXTENDED = FALSE;           // Standard Message type
    IMMOResponse.m_ucRTR = FALSE;                // Not RTR type
    IMMOResponse.m_unMsgID = m_immoMsgId[INDEX_ECM_CHALLANGE_RESP];
    sendToTraceWin(m_wfs16_tips[INDEX_ECM_CHALLANGE_RESP], 0);
    g_pouDIL_CAN_Interface1->DILC_SendMsg(g_dwClientID1, IMMOResponse);
}
/* End BUSMASTER generated function - prepareImmoResp */


/* Start BUSMASTER generated function - Utils_XTEADecrypt */
VOID Utils_XTEADecrypt(USHORT v[], USHORT n, USHORT k[])
{
    static USHORT z , y, sum, e, x;
    static USHORT p, q;
    z = v[n - 1];
    y = v[0];
    q = 6 + 52 / n;
    sum = q * 0x79b9;

    while(sum != 0)
    {
        e = (sum >> 2) & (3);

        for(p = n - 1; p > 0; p--)
        {
            z = v[p - 1];
            x = v[p];
            x -= ((((z >> 5)) ^ ((y << 2))) + (((y >> 3)) ^ ((z << 4))) ^ (((sum) ^ (y))) + ((k[(p & 3) ^ (e)]) ^ (z)));
            v[p] = x;
            y = x;
        }

        z = v[n - 1];
        x = v[0];
        x -= ((((z >> 5)) ^ ((y << 2))) + (((y >> 3)) ^ ((z << 4))) ^ (((sum) ^ (y))) + ((k[(p & 3) ^ (e)]) ^ (z)));
        v[0] = x;
        y = x;
        sum -= 0x79b9;
    }
}
/* End BUSMASTER generated function - Utils_XTEADecrypt */

/* Start BUSMASTER generated function - Utils_XTEAEncrypt */
VOID Utils_XTEAEncrypt(USHORT v[], USHORT n, USHORT k[])
{
    static USHORT z, y, sum, e, x;
    static USHORT p, q;
    z = v[n - 1];
    y = v[0];
    sum = 0;
    e = 0;
    p = 0;
    q = 6 + 52 / n;

    while(q-- > 0)
    {
        sum += 0x79b9;
        e = (sum >> 2) & (3);

        for(p = 0; p < n - 1; p++)
        {
            y = v[p + 1];
            x = v[p];
            x += ((((z >> 5)) ^ ((y << 2))) + (((y >> 3)) ^ ((z << 4))) ^ (((sum) ^ (y))) + ((k[(p & 3) ^ (e)]) ^ (z)));
            v[p] = x;
            z = x;
        }

        y = v[0];
        x = v[n - 1];
        x += ((((z >> 5)) ^ ((y << 2))) + (((y >> 3)) ^ ((z << 4))) ^ (((sum) ^ (y))) + ((k[(p & 3) ^ (e)]) ^ (z)));
        v[n - 1] = x;
        z = x;
    }
}
/*@@end*/
// Sia_16_Protocol 消息处理程序
