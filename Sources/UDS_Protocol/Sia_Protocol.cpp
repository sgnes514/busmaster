#include "Sia_Protocol.h"
#define USAGE_EXPORT
#include "UDS_Extern.h"

CBaseDIL_CAN* g_pouDIL_CAN_Interface1;
DWORD g_dwClientID1 = 0;
UINT_PTR m_Timer;
extern int m_sendTimeOut;
CString addTime(char* msg_buf, int type);
void CB2CS(char* hex, CString &dst, int len);

void sendToTraceWin(char* buff, int type);
char rx_tx[2][5] =
{
    {"Tx:"},
    {"Rx:"},
};

ImmoTestMainWnd* omImmoTestMainWnd = NULL;
Sia_Protocol* omSiaPtr = NULL;
//Sia_16_Protocol* omWfs16Ptr = NULL;
//Sia_17_Protocol* omWfs17Ptr = NULL;


USAGEMODE HWND DIL_IMMOTEST_MainWnd(HWND hParent)
{
    //HRESULT hResult = S_OK;
    //DWORD hResult
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    INT_PTR nRet = -1;
    CWnd objParent;
    objParent.Attach(hParent);

    if(omImmoTestMainWnd == NULL)
    {
        omImmoTestMainWnd = new ImmoTestMainWnd(&objParent);
        omImmoTestMainWnd->Create(IDD_IMMOTEST);
    }

    omImmoTestMainWnd->ShowWindow(SW_SHOW);
    objParent.Detach();
    HWND tmp_hwnr = omImmoTestMainWnd->GetSafeHwnd();
    return tmp_hwnr;
}
USAGEMODE HRESULT EvaluteImmoMessage(STCAN_MSG  Mensage)
{
    omSiaPtr->checkEmsResp(Mensage);
    return 0;
}

Sia_Protocol::Sia_Protocol(void)
{
    mSia16Ptr = NULL;
    mSia17Ptr = NULL;
}


Sia_Protocol::~Sia_Protocol(void)
{
}

void Sia_Protocol::checkEmsResp(STCAN_MSG msg)
{
    chkMsgRsp res = NoRelMsg;

    if(siaStates != immoStateTimeOut)
    {
        if(mSia16Ptr != NULL && omImmoTestMainWnd->m_ImmoSS->GetCheck())
        {
            res = mSia16Ptr->checkRMSResp(msg);

            if(res != 3)
            {
                omImmoTestMainWnd->KillTimer(ID_TIMER_IMMO_RESP);

                if(res == 0)
                {
                    siaStates = immoStateWaiting;
                    m_Timer = omImmoTestMainWnd->SetTimer(ID_TIMER_IMMO_RESP, m_sendTimeOut, NULL);
                }

                // updateTraceInfo(omWfs16Ptr->m_curSendMsgID);
            }

            if(res == 1)
            {
                siaStates = immoStateOK;
            }
        }
        else if(mSia17Ptr != NULL && omImmoTestMainWnd->m_17_enAbleTest->GetCheck())
        {
            res = mSia17Ptr->checkRMSResp(msg);
        }
    }
}

void sendToTraceWin(char* buff, int type)
{
    omImmoTestMainWnd->m_immoTraceStr += addTime(buff, type);
}
void CB2CS(char* hex, CString &dst, int len)
{
    dst.Empty();
    int k(0);
    CString tmp;

    for(;;)
    {
        tmp.Empty();
        tmp.Format(_T("%2x"), hex[k]);
        tmp.Replace("FFFFFF", "");
        tmp.Replace("ffffff", "");

        if(++k <= len)
        {
            dst += tmp;
            dst += " ";
        }
        else
            break;
    }
}
CString addTime(char* msg_buf, int type)
{
    //CString buff, buff1;
    char buff1[150];
    SYSTEMTIME tmp_tm;
    GetSystemTime(&tmp_tm);
    //  CB2CS((char*)msg_buf, buff1, len);
    sprintf(buff1, "%-02d:%02d:%02d:%04d %s:%s \r\n", ((tmp_tm.wHour + 8) < 24) ? (tmp_tm.wHour + 8) : (tmp_tm.wHour - 16),
            tmp_tm.wMinute,
            tmp_tm.wSecond,
            tmp_tm.wMilliseconds,
            rx_tx[type],
            //(char*)(diasImmoMsgCfg[id]+11)
            msg_buf);
#if 0
    buff.Format("%-2d:%2d:%2d:%4d %s:%s ", ((tmp_tm.wHour + 8) < 24) ? (tmp_tm.wHour + 8) : (tmp_tm.wHour - 16),
                tmp_tm.wMinute,
                tmp_tm.wSecond,
                tmp_tm.wMilliseconds,
                rx_tx[type],
                //(char*)(diasImmoMsgCfg[id]+11)
                msg_buf
               );
#endif
    //buff += buff1;
    //buff += "\r\n";
    return CString(buff1);
}