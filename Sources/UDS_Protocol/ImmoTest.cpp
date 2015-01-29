// ImmoTest.cpp : 实现文件
//

#pragma once
#include "StdAfx.h"
#define USAGE_EXPORT
#include "UDS_Protocol.h"
#include "ImmoTest.h"
//#include "afxdialogex.h"
#include "UDS_Extern.h"
#include "resource.h"
#include <libxml/parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include "DIL_Interface/BaseDIL_CAN.h"
#include "DIL_Interface/DIL_Interface_extern.h"
#include <fstream>
#include <iosfwd>

unsigned char m_immoMsgList[10][8]={
	{0xB1,0x68,0x5C,0x55,0x8D,0x6B,0xDA,0xA2},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x4E,0xA7,0x73,0x3A,0x92,0x0E,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0xCF,0xCD,0x73,0xE3,0x38,0xE1,0xC3,0xB2},
	{0xB1,0x68,0x5C,0x55,0x8D,0x6B,0xDA,0xA2},
	{0x2B,0x02,0xA4,0xFD,0xC6,0xF8,0x62,0xC5},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7},
	{0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7}
};
const char immoMsgCfg [10][40]={
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
char rx_tx[2][5]={
	{"Tx:"},
	{"Rx:"},
};
STCAN_MSG IMMOResponse = {0x51d,0,0,0,0};
UCHAR Chanllenge[8];
UCHAR Response[8];
USHORT ArrayTempU16[4];
USHORT SKArray[4] = {0xDA3B,0x1511,0x2233,0xD7AB};
USHORT ArrayRDNPIN[4];


UINT_PTR m_Timer;

int m_sendTimeOut = 150;
#define DEBUG_ON
// ImmoTest 对话框

IMPLEMENT_DYNAMIC(ImmoTest, CDialog)



	static CBaseDIL_CAN* g_pouDIL_CAN_Interface1;
static DWORD g_dwClientID = 0;
int ChangeToBcd(char iChTemp) ;
void prepareImmoResp(STCAN_MSG RxMsg);
VOID Utils_XTEADecrypt(USHORT v[],USHORT n,USHORT k[]);
VOID Utils_XTEAEncrypt(USHORT v[],USHORT n,USHORT k[]);
void CB2CS(char* hex,CString &dst, int len);

ImmoTest::ImmoTest(CWnd* pParent /*=NULL*/)
	: CDialog(ImmoTest::IDD, pParent)
{
}

ImmoTest::~ImmoTest()
{
	freeImmoCfgStr();
}

void ImmoTest::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTONLEARNSKPIN, m_learnPin);
	DDX_Control(pDX, IDC_IMMOTEST_TRACE, m_immoTrace);
	DDX_Control(pDX, IDC_BUTTON_RESETECU, m_resetEcu);
	DDX_Text(pDX,IDC_IMMOTEST_TRACE,m_immoTraceStr);
	DDX_Control(pDX, IDC_COMBO_CUSTOMER, m_customerType);
	DDX_Control(pDX, IDC_CHECK_ENIMMORESP, m_enImmoResp);
	DDX_Control(pDX, IDC_CHECK_ISS, m_ImmoSS);
}


BEGIN_MESSAGE_MAP(ImmoTest, CDialog)
	ON_BN_CLICKED(IDC_BUTTONLEARNSKPIN, OnBnClickedButtonlearnskpin)
	ON_BN_CLICKED(IDC_BUTTON_RESETECU, OnBnClickedButtonResetecu)
	ON_CBN_SELCHANGE(IDC_COMBO_CUSTOMER, OnCbnSelchangeComboCustomer)
	ON_BN_CLICKED(IDC_CHECK_ISS, OnBnClickedCheckIss)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// ImmoTest 消息处理程序
USAGEMODE HWND DIL_IMMOTEST_MainWnd(HWND hParent)
{
	//HRESULT hResult = S_OK;
	//DWORD hResult
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	INT_PTR nRet = -1;
	CWnd objParent;
	objParent.Attach(hParent);
	if (omImmoTestWnd == NULL)
	{
		omImmoTestWnd = new ImmoTest(&objParent);
		omImmoTestWnd->Create(IDD_IMMOTEST);
	}
	omImmoTestWnd->ShowWindow(SW_SHOW);
	objParent.Detach();

	HWND tmp_hwnr = omImmoTestWnd->GetSafeHwnd();
	return tmp_hwnr;
}
USAGEMODE HRESULT EvaluteImmoMessage(STCAN_MSG  Mensage)
{
	omImmoTestWnd->checkEmsResp(Mensage);
	return 0;
}
BOOL ImmoTest::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO:  在此添加额外的初始化
	m_immoCfgHead = NULL;
	immoTxMsg = NULL;
	m_testState = immoStateIdel;
	m_curSendMsgCnt = 0;
	m_curSendMsgID = 0;
	/* Get CAN DIL interface */
	DIL_GetInterface(CAN, (void**)&g_pouDIL_CAN_Interface1);
	g_pouDIL_CAN_Interface1->DILC_RegisterClient(TRUE, g_dwClientID, _("CAN_MONITOR"));
	LoadDiasCanCfg();
	updateCustomerCombox();
	updateImmoId();
	initImmoMsg();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


BOOL ImmoTest::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	return CDialog::PreTranslateMessage(pMsg);
}

int ImmoTest::LoadDiasCanCfg()
{

	xmlDocPtr doc;           //定义解析文档指针
	xmlNodePtr curNode;      //定义结点指针(你需要它为了在各个结点间移动)
	xmlChar *szKey;          //临时字符串变量
	char *szDocName;
	char filename[] = "d:\\immo.xml";
	int i=0;
	int res = 0, j=0;

	doc = xmlReadFile(filename,"GB2312",XML_PARSE_RECOVER); //解析文件
	if (NULL == doc)
	{
		return -1;
	}
	curNode = xmlDocGetRootElement(doc); //确定文档根元素
	/*检查确认当前文档中包含内容*/
	if (NULL == curNode)
	{
		xmlFreeDoc(doc);
		return -1;
	}
	/*在这个例子中，我们需要确认文档是正确的类型。“immotest”是在这个示例中使用文档的根类型。*/
	if (xmlStrcmp(curNode->name, BAD_CAST "immotest"))
	{
		xmlFreeDoc(doc);
		return -1;
	}
	curNode = curNode->xmlChildrenNode;
	curNode = curNode->next;
	xmlNodePtr propNodePtr = curNode;
	while(propNodePtr != NULL)
	{
		//取出节点中的内容
		curNode = propNodePtr;
		if (xmlStrcmp(curNode->name, (const xmlChar *)"canvers"))
		{
			xmlFree(curNode);
		}
		else
		{
			propNodePtr = curNode->next->next;
			immocfg* tmp = new immocfg;
			memset(tmp,0,sizeof(immocfg));
			szKey = curNode->properties->children->content;
			CString s(szKey);
			tmp->Canvers.CString::CString();
			tmp->Canvers = s;//atoi((char *)szKey);
			curNode = curNode->children;
			curNode = curNode->next;
			i = 0;
			while (curNode != NULL && i <= 10)
			{
				if (0 == strcmp(immoMsgCfg[i],(char *)(xmlChar *)curNode->name))
				{
					CString tmp1((char *)(curNode->children->content));
					res =0;
					for (j=0;j<tmp1.GetLength();j++)
					{
						res |= ChangeToBcd(tmp1.GetAt(j)) << ((tmp1.GetLength() - j -1)*4);
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
int ChangeToBcd(char iChTemp)               //转换单个字符
{
	int m;
	if( iChTemp >= '0' && iChTemp<='9' )
	{
		m = iChTemp - '0';
	}
	else if( iChTemp>='A' && iChTemp<='Z' )
	{
		m=iChTemp-'A'+10;
	}
	else if( iChTemp>='a' && iChTemp<='z' )
	{
		m=iChTemp-'a'+10;
	}
	return m;
}
void ImmoTest::initImmoMsg()
{

	int i = 0;
	if (immoTxMsg == NULL)
	{
		immoTxMsg = new msTXMSGDATA;
	}
	immoTxMsg->m_unCount = 10;
	immoTxMsg->m_psTxMsg = new STCAN_MSG[10];
	for (i=0;i<10;i++)
	{
		immoTxMsg->m_psTxMsg[i].m_ucChannel = 1;
		immoTxMsg->m_psTxMsg[i].m_ucDataLen = 8;
		immoTxMsg->m_psTxMsg[i].m_ucRTR = FALSE;
		immoTxMsg->m_psTxMsg[i].m_ucEXTENDED= FALSE;
		immoTxMsg->m_psTxMsg[i].m_unMsgID = m_immoMsgId[i];
		memcpy(immoTxMsg->m_psTxMsg[i].m_ucData,m_immoMsgList[i],8);
	}



}
void ImmoTest::addImmoCfg(immocfg* node)
{
	if (m_immoCfgHead == NULL)
	{
		m_immoCfgHead = node;
	} 
	else
	{
		immocfg* tmp = m_immoCfgHead;
		while (tmp != NULL)
		{
			if (tmp->next == NULL)
			{
				tmp->next = node;
				return;
			}
			tmp->next = tmp;
		}
	}
}
void ImmoTest::freeImmoCfgStr()
{
	immocfg* tmp = m_immoCfgHead;
	immocfg* tmp1 = NULL;
	while(tmp != NULL)
	{
		tmp1 = tmp->next;
		free(tmp);
		tmp = tmp1;
	}
}
void ImmoTest::updateCustomerCombox()
{
	immocfg* tmp = m_immoCfgHead;
	while (tmp != NULL)
	{
		//CString tmp_s;
		//tmp_s.Format("%d",tmp->Canvers);
		m_customerType.AddString((LPCTSTR)tmp->Canvers);
		tmp = tmp->next;
	}
	if (m_customerType.GetCount() > 0)
	{
		m_customerType.SetCurSel(0);
	}

}
void ImmoTest::checkCfgValid()
{

}
void ImmoTest::OnBnClickedButtonlearnskpin()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_testState == immoStateIdel)
	{
		sendImmomsg(INDEX_LEARN_SK);
		//m_learnPin.EnableWindow(FALSE);
		
	} 
}


void ImmoTest::OnBnClickedButtonResetecu()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_testState == immoStateIdel || m_testState == immoStateOK)
	{
		sendImmomsg(INDEX_RESET_SK);
		//m_resetEcu.EnableWindow(FALSE);

	} 
}
int ImmoTest::sendImmomsg(int id)
{

	if (immoTxMsg != NULL)
	{
		if( immoTxMsg->m_psTxMsg[id].m_unMsgID != 0 )
		{
			m_curSendMsgID = id;
			m_curSendMsgCnt++;
			int nReturn = g_pouDIL_CAN_Interface1->DILC_SendMsg(g_dwClientID, immoTxMsg->m_psTxMsg[id]);
			m_testState = immoStateWaiting;
			m_Timer = SetTimer(ID_TIMER_IMMO_RESP, m_sendTimeOut, NULL);
			updateTraceInfo(id);
		}
		return 0;
	}

	return 0;

}
void ImmoTest::updateTraceInfo(int id)
{

	CString buff,buff1;
	SYSTEMTIME tmp_tm;
	GetSystemTime(&tmp_tm);

	CB2CS((char*)m_immoMsgList[id], buff1,8);
	buff.Format("%-2d:%2d:%2d:%4d %s 0X%x {",((tmp_tm.wHour+8) < 24)?(tmp_tm.wHour+8):(tmp_tm.wHour-16),
										tmp_tm.wMinute,
										tmp_tm.wSecond,
										tmp_tm.wMilliseconds,
										rx_tx[id%2],
										//(char*)(immoMsgCfg[id]+11)
										m_immoMsgId[id]
										);
	buff += buff1;
	m_immoTraceStr = m_immoTraceStr + buff + "}\r\n";
	UpdateData(FALSE);

}
void CB2CS(char* hex,CString &dst, int len)
{
	dst.Empty();
	int k(0);
	CString tmp;
	for(;;)
	{
		tmp.Empty();
		tmp.Format(_T("%2x"),hex[k]);
		tmp.Replace("FFFFFF","");
		tmp.Replace("ffffff","");
		if(++k<=len)
		{
			dst += tmp;
			dst += " ";
		}
		else
			break;
	}
}
void ImmoTest::OnCbnSelchangeComboCustomer()
{
	// TODO: 在此添加控件通知处理程序代码
	updateImmoId();
}
void ImmoTest::updateImmoId()
{
	int i = 0;
	immocfg* tmp = m_immoCfgHead;
	if (m_customerType.GetCount() > 0)
	{
		int iPos= m_customerType.GetCurSel();
		while (i++ != iPos)
		{
			tmp = tmp->next;
		}
		if (tmp != NULL)
		{
			memcpy(m_immoMsgId,tmp->immoIdList,10*sizeof(int));

		} 
		else
		{
			memset(m_immoMsgId,0,10*sizeof(int));
		}

	}

}


void ImmoTest::OnBnClickedCheckIss()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_ImmoSS.GetCheck())
	{
		m_learnPin.EnableWindow(TRUE);
		m_resetEcu.EnableWindow(TRUE);
		m_customerType.EnableWindow(FALSE);
		m_testState = immoStateIdel;
		m_curSendMsgCnt	= 0;
		m_curSendMsgID = 0;
	}
	else
	{
		m_learnPin.EnableWindow(FALSE);
		m_resetEcu.EnableWindow(FALSE);
		m_customerType.EnableWindow(TRUE);
	}
}


void ImmoTest::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == ID_TIMER_IMMO_RESP)
	{
		if (m_curSendMsgCnt < 14)
		{
			sendImmomsg(m_curSendMsgID);
		} 
		else
		{
			m_testState = immoStateCfgError;
			KillTimer(ID_TIMER_IMMO_RESP);
		}

	}
	CDialog::OnTimer(nIDEvent);
}
void ImmoTest::checkEmsResp(STCAN_MSG Mensage)
{
	int i =0;
	if (m_testState != immoStateTimeOut)
	{
		if (Mensage.m_unMsgID == m_immoMsgId[m_curSendMsgID+1]
		&& Mensage.m_ucData[i] == m_immoMsgList[m_curSendMsgID+1][i++]
		&& Mensage.m_ucData[i] == m_immoMsgList[m_curSendMsgID+1][i++]
		&& Mensage.m_ucData[i] == m_immoMsgList[m_curSendMsgID][i++]
		&& Mensage.m_ucData[i] == m_immoMsgList[m_curSendMsgID+1][i++]
		&& Mensage.m_ucData[i] == m_immoMsgList[m_curSendMsgID+1][i++]
		&& Mensage.m_ucData[i] == m_immoMsgList[m_curSendMsgID][i++]
		&& Mensage.m_ucData[i] == m_immoMsgList[m_curSendMsgID+1][i++]
		&& Mensage.m_ucData[i] == m_immoMsgList[m_curSendMsgID+1][i++])
		{
			KillTimer(ID_TIMER_IMMO_RESP);
			if (m_curSendMsgID != 2 && m_curSendMsgID != 6)
			{
				sendImmomsg(m_curSendMsgID+2);
			}
			else
			{
				m_testState == immoStateOK;
			}
			
		}
	} 
	else if(Mensage.m_unMsgID == m_immoMsgId[INDEX_ECM_CHALLANGE]
					&& m_enImmoResp.GetCheck())
	{
		prepareImmoResp(Mensage);
	}

}


/* Start BUSMASTER generated function - prepareImmoResp */
void prepareImmoResp(STCAN_MSG RxMsg)
{
	memcpy(Chanllenge,RxMsg.m_ucData,8);
	memcpy(m_immoMsgList[INDEX_ECM_CHALLANGE],RxMsg.m_ucData,8);
	ArrayTempU16[0] = (Chanllenge[0] << 8) | Chanllenge[1];
	ArrayTempU16[1] = (Chanllenge[2] << 8) | Chanllenge[3];
	ArrayTempU16[2] = (Chanllenge[4] << 8) | Chanllenge[5];
	ArrayTempU16[3] = (Chanllenge[6] << 8) | Chanllenge[7];

	Utils_XTEADecrypt (ArrayTempU16, 4, SKArray);

	if ( (ArrayTempU16[2] == 0xAABB) && (ArrayTempU16[3] == 0xCCDD))
	{

		ArrayRDNPIN[0] = ArrayTempU16[2];
		ArrayRDNPIN[1] = ArrayTempU16[3];
		ArrayRDNPIN[2] = ArrayTempU16[0];
		ArrayRDNPIN[3] = ArrayTempU16[1];

		Utils_XTEAEncrypt (ArrayRDNPIN, 4, SKArray);

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
		memcpy(m_immoMsgList[INDEX_ECM_CHALLANGE_RESP], Response, 8);
	}
	else
	{
		IMMOResponse.m_ucDataLen = 8;
		memset(IMMOResponse.m_ucData,0xff,8);
	}
	IMMOResponse.m_ucChannel = 1;                // First CAN channel
	IMMOResponse.m_ucEXTENDED = FALSE;           // Standard Message type
	IMMOResponse.m_ucRTR = FALSE;                // Not RTR type
	IMMOResponse.m_unMsgID =omImmoTestWnd->m_immoMsgId[INDEX_ECM_CHALLANGE_RESP];
	g_pouDIL_CAN_Interface1->DILC_SendMsg(INDEX_ECM_CHALLANGE_RESP, IMMOResponse);
}
/* End BUSMASTER generated function - prepareImmoResp */


/* Start BUSMASTER generated function - Utils_XTEADecrypt */
VOID Utils_XTEADecrypt(USHORT v[],USHORT n,USHORT k[])
{
	static USHORT z , y, sum, e, x;

	static USHORT p, q;


	z = v[n - 1];
	y = v[0];
	q = 6 + 52 / n;
	sum = q * 0x79b9;

	while (sum != 0)
	{
		e = (sum >> 2) & (3);
		for (p = n - 1; p > 0; p--)
		{
			z = v[p - 1];
			x = v[p];
			x -= ((((z >> 5)) ^ ((y << 2))) + (((y >> 3)) ^ ((z << 4))) ^ (((sum) ^ (y))) + ((k[(p & 3) ^ (e)]) ^ (z)) );
			v[p] = x;
			y = x;
		}
		z = v[n - 1];
		x = v[0];
		x -= ((((z >> 5)) ^ ((y << 2))) + (((y >> 3)) ^ ((z << 4))) ^ (((sum) ^ (y))) + ((k[(p & 3) ^ (e)]) ^ (z)) );
		v[0] = x;
		y = x;
		sum -= 0x79b9;
	}
}
/* End BUSMASTER generated function - Utils_XTEADecrypt */

/* Start BUSMASTER generated function - Utils_XTEAEncrypt */
VOID Utils_XTEAEncrypt(USHORT v[],USHORT n,USHORT k[])
{
	static USHORT z, y, sum, e, x;

	static USHORT p, q;


	z = v[n - 1];
	y = v[0];
	sum = 0;
	e = 0;
	p = 0;
	q = 6 + 52 / n;
	while (q-- > 0)
	{
		sum += 0x79b9;
		e = (sum >> 2) & (3);
		for (p = 0; p < n - 1; p++)
		{
			y = v[p + 1];
			x = v[p];
			x += ((((z >> 5)) ^ ((y << 2))) + (((y >> 3)) ^ ((z << 4))) ^ (((sum) ^ (y))) + ((k[(p & 3) ^ (e)]) ^ (z)) );
			v[p] = x;
			z = x;
		}
		y = v[0];
		x = v[n - 1];
		x += ((((z >> 5)) ^ ((y << 2))) + (((y >> 3)) ^ ((z << 4))) ^ (((sum) ^ (y))) + ((k[(p & 3) ^ (e)]) ^ (z)) );
		v[n - 1] = x;
		z = x;
	}
}
/*@@end*/
