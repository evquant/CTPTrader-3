//���������룺SHFE/DCE/CZCE/CFFEX(������/����/֣��/�н���)



/**********************************************************************
��Ŀ����: CTP�ڻ�����ϵͳ(�콢��)
����޸ģ�20150508
�������ߣ���ӯ���������Ŷӣ��ٷ�QQȺ��202367118


**********************************************************************/

#include <afx.h>
#include "ctp.h"
#include "mdspi.h"
#include "traderspi.h"
#include "strategy.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <iostream>
#include <sstream>
#include "Logger.h"
#include "StructFunction.h"

using namespace std;
using namespace CPlusPlusLogging;

int strategyVolumeTarget;

string exePath = "";

string strategykdbscript;

double par1, par2, par3, par4, par5, par6;

int strategyOrderType1, strategyOrderType2, strategyOrderType3, strategyOrderType4, strategyOrderType5, strategyOrderType6;

string strategyPairLeg1Symbol, strategyPairLeg2Symbol, strategyPairLeg3Symbol;

int m_quote_kdbPort, kdbPort;

int requestId=0;//������

HANDLE g_hEvent;//���߳��õ��ľ��

Strategy* g_strategy;//������ָ��

CtpTraderSpi* g_pUserSpi_tradeAll;//ȫ�ֵ�TD�ص�����������˻������������õ�

//�˻���������
DWORD WINAPI ThreadProc(LPVOID lpParameter);

int main(int argc, const char* argv[])
{
	//LOG_ALWAYS("<=============================== START OF PROGRAM CTP TRADER ===============================>");
	string strategyAccountParampath = argv[1];
	//string strategyAccountParampath = "5004_AccountParam_CHENLIANGQING_DONGZHENG_Quote.ini";
	g_hEvent=CreateEvent(NULL, true, false, NULL); 

	//--------------��ȡ�����ļ�����ȡ�˻���Ϣ����������ַ�����׵ĺ�Լ����--------------
	ReadMessage readMessage;
	memset(&readMessage, 0, sizeof(readMessage));
	SetMessage(readMessage, &exePath, &m_quote_kdbPort, &kdbPort, &strategyVolumeTarget, &strategykdbscript, &par1, &par2, &par3, &par4, &par5, &par6, &strategyOrderType1, &strategyOrderType2, &strategyOrderType3, &strategyOrderType4, &strategyOrderType5, &strategyOrderType6, strategyAccountParampath, &strategyPairLeg1Symbol, &strategyPairLeg2Symbol, &strategyPairLeg3Symbol);

	SetConsoleTitle(("CTP" + (strategyAccountParampath)).c_str());

	
	

	//--------------��ʼ������UserApi����������APIʵ��----------------------------------
	CThostFtdcMdApi* pUserApi_md = CThostFtdcMdApi::CreateFtdcMdApi(mdflowPath.c_str());
	CtpMdSpi* pUserSpi_md = new CtpMdSpi(pUserApi_md);//�����ص����������MdSpi
	pUserApi_md->RegisterSpi(pUserSpi_md);// �ص�����ע��ӿ���
	pUserApi_md->RegisterFront(readMessage.m_mdFront);// ע������ǰ�õ�ַ	
	pUserSpi_md->setAccount(readMessage.m_appId, readMessage.m_userId, readMessage.m_passwd);//���͹�˾��ţ��û���������
	pUserSpi_md->setInstId(readMessage.m_read_contract);//MD���趩������ĺ�Լ�������Խ��׵ĺ�Լ



	//--------------��ʼ������UserApi����������APIʵ��----------------------------------
	CThostFtdcTraderApi* pUserApi_trade = CThostFtdcTraderApi::CreateFtdcTraderApi(tdflowPath.c_str());
	CtpTraderSpi* pUserSpi_trade = new CtpTraderSpi(pUserApi_trade, pUserApi_md, pUserSpi_md);//���캯����ʼ����������
	pUserApi_trade->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi_trade);// ע���¼���
	pUserApi_trade->SubscribePublicTopic(THOST_TERT_RESTART);// ע�ṫ����
	pUserApi_trade->SubscribePrivateTopic(THOST_TERT_QUICK);// ע��˽����THOST_TERT_QUICK
	pUserApi_trade->RegisterFront(readMessage.m_tradeFront);// ע�ύ��ǰ�õ�ַ
	pUserSpi_trade->setAccount(readMessage.m_appId, readMessage.m_userId, readMessage.m_passwd, readMessage.m_userProductInfo, readMessage.m_authCode);//���͹�˾��ţ��û���������	
	pUserSpi_trade->setInstId(readMessage.m_read_contract);//���Խ��׵ĺ�Լ

	g_pUserSpi_tradeAll = pUserSpi_trade;//ȫ�ֵ�TD�ص�����������˻������������õ�




	//--------------��������ʵ��--------------------------------------------------------
	g_strategy = new Strategy(pUserSpi_trade);
	g_strategy->Init(readMessage.m_strategy_strategytype, readMessage.m_read_contractdecoration, readMessage.m_read_contract, exePath, kdbPort, kdbScriptExePath, strategyVolumeTarget, strategykdbscript, par1, par2, par3, par4, par5, par6, strategyOrderType1, strategyOrderType2, strategyOrderType3, strategyOrderType4, strategyOrderType5, strategyOrderType6, strategyPairLeg1Symbol, strategyPairLeg2Symbol, strategyPairLeg2Symbol);
	



	//--------------TD�߳�������--------------------------------------------------------	
	pUserApi_trade->Init();//TD��ʼ����ɺ��ٽ���MD�ĳ�ʼ��
	



	//--------------�˻�����ģ��--------------------------------------------------------
	HANDLE hThread1=CreateThread(NULL,0,ThreadProc,NULL,0,NULL);
	CloseHandle(hThread1);
	WaitForSingleObject(hThread1, INFINITE);
	

	if (readMessage.m_strategy_strategytype == "Quote")
	{
		timer_start(kdbSetData, 60000);
		pUserApi_md->Join();//�ȴ��ӿ��߳��˳�
		pUserApi_trade->Join();
		while (true);
	}
	else
	{
		timer_start(kdbSetShortLong, 60000);
		pUserApi_md->Join();//�ȴ��ӿ��߳��˳�
		pUserApi_trade->Join();
		while (true);
	}
	
}

//�˻���������
DWORD WINAPI ThreadProc(
	LPVOID lpParameter
	)
{
	string str;

	int a;
	cerr<<"--------------------------------------------------------�˻���������������"<<endl;
	cerr<<endl<<"������ָ��(�鿴�ֲ�:show,ǿƽ�ֲ�:close,������:yxkc, ��ֹ����:jzkc)��";

	while(1)
	{
		cin>>str;
		if(str == "show")
			a = 0;	
		else if(str == "close")
			a = 1;
		else if(str == "yxkc")
			a = 2;
		else if(str == "jzkc")
			a = 3;
		else
			a = 4;

		switch(a){
		case 0:
			{		
				cerr<<"�鿴�˻��ֲ�:"<<endl;
				g_pUserSpi_tradeAll->printTrade_message_map();
				cerr<<"������ָ��(�鿴�ֲ�:show,ǿƽ�ֲ�:close,������:yxkc, ��ֹ����:jzkc)��"<<endl;
				break;
			}
		case 1:
			{
				cerr<<"ǿƽ�˻��ֲ�:"<<endl;
				g_pUserSpi_tradeAll->ForceClose();
				cerr<<"������ָ��(�鿴�ֲ�:show,ǿƽ�ֲ�:close,������:yxkc, ��ֹ����:jzkc)��"<<endl;
				break;
			}
		case 2:
			{
				cerr<<"������:"<<endl;
				g_strategy->set_allow_open(true);
				cerr<<"������ָ��(�鿴�ֲ�:show,ǿƽ�ֲ�:close,������:yxkc, ��ֹ����:jzkc)��"<<endl;
				break;

			}
		case 3:
			{
				cerr<<"��ֹ����:"<<endl;
				g_strategy->set_allow_open(false);
				cerr<<"������ָ��(�鿴�ֲ�:show,ǿƽ�ֲ�:close,������:yxkc, ��ֹ����:jzkc)��"<<endl;
				break;

			}
		case 4:
			{
				cerr<<endl<<"�����������������ָ��(�鿴�ֲ�:show,ǿƽ�ֲ�:close,������:yxkc, ��ֹ����:jzkc)��"<<endl;
				break;
			}


		}

	}

	return 0;

}