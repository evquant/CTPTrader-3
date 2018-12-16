#include "strategy.h"
#include <fstream>
#include "kdb_function.h"
#include "StructFunction.h"


using namespace std;
using namespace kdb;
 
kdb::Connector kdbConnector;
int m_kdbQuotePort = 5005;
string m_kdbScript;
int m_ShortLongInitNum = 0;
int m_ReadShortLongInitNum = 0;
int m_ReadShortLongSignal = 0;
TThostFtdcVolumeType m_VolumeTarget = 1;
int m_OrderVolumeMultiple = 2;
int m_IndexFuturePositionLimit = 20;
string m_StrategyType = "";
string m_exePath = "";
int m_LastedVolume = 0;
double m_ReadLegOneAskPrice1 = 0.0;
double m_ReadLegOneBidPrice1 = 0.0;
double m_CheckLegOneAskPrice1 = 0.0;
double m_CheckLegOneBidPrice1 = 0.0;
int m_DAQuoteCount = 0;
int m_MarketDataCount = 0;
int m_OnTickCount = 0;
vector<int> orderType = { 0, 0, 0, 0, 0, 0 };
vector<int> orderType_erase = { 1, 1, 10, 10, 1, 0 };
int n = 0;
int n_quote = 0;
bool m_IsMarketOpen = false;
int m_currentIntUpdateTime = 0;
string m_str_UpdateTime;

//���׳̶�:   buy  1,-1,0 �ĺ�����:		��AskPrice1�µ�һ������
//           sell 1,-1,0 �ĺ�����:  ��BidPrice1�ϵ�һ������
//           sell -1,1,0 �ĺ�����:  ��AskPrice1�µ�һ������
//           buy  -1,1,0 �ĺ�����:  ��BidPrice1�ϵ�һ������
//           ���һ�����ֱ������������¹ң�0����ÿ��Tick����飬����������־��ǹ����������¹ң�С��0Ӧ���Ǵ���

void Strategy::OnTickData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	if (IsMarketOpen(pDepthMarketData))
	{

		if (m_StrategyType == "Quote")
		{
			TDSpi_stgy->Set_CThostFtdcDepthMarketDataField(pDepthMarketData);
			Set_CThostFtdcDepthMarketDataField(pDepthMarketData);
			//Insert Data To KDB+ DataBase
			DataInsertToKDB(pDepthMarketData);
			string currentInstrumentId = pDepthMarketData->InstrumentID;
			if (TDSpi_stgy->CheckSameProduct_CThostFtdcDepthMarketDataField(currentInstrumentId) == 3)
			{
				vector<string> v_product = TDSpi_stgy->Data3MContracts(currentInstrumentId);
				Data3MInsertToKDB(v_product, pDepthMarketData, 123456789101112);
			}	
			if (TDSpi_stgy->CheckSameProduct_CThostFtdcDepthMarketDataField(currentInstrumentId) == 2)
			{
				vector<string> v_product = TDSpi_stgy->Data3MContracts(currentInstrumentId);
				Data3MInsertToKDB(v_product, pDepthMarketData, 159);
			}
			//Reboost DA Data Source
			if (n_quote == 0)
			{
				DataRebootDADataSource();
				n_quote++;
			}
			else if (n_quote < 1000)
			{
				n_quote++;
			}
			else if (n_quote >= 1000)
			{
				n_quote = 0;
			}
		}
		else
		{
			//�����˻���ӯ����Ϣ
			CalculateEarningsInfo(pDepthMarketData);

			if (strcmp(pDepthMarketData->InstrumentID, m_instId) == 0)
			{
				TDSpi_stgy->Set_CThostFtdcDepthMarketDataField(pDepthMarketData);

				cerr << "�����յ�����:" << pDepthMarketData->InstrumentID << "," << pDepthMarketData->TradingDay << "," << pDepthMarketData->UpdateTime << ",���¼�:" << pDepthMarketData->LastPrice << ",��ͣ��:" << pDepthMarketData->UpperLimitPrice << ",��ͣ��:" << pDepthMarketData->LowerLimitPrice << endl;

				//�������ݵ�vector
				SaveDataVec(pDepthMarketData);

				//�������ݵ�txt��csv
				//SaveDataTxtCsv(pDepthMarketData);

				//����׷��,�����ɹ�����׷��
				m_LastedVolume = 0;
				TDSpi_stgy->MaintainOrder(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, "CheckOnTick", &m_LastedVolume, pDepthMarketData->InstrumentID);

				//����ƽ�֣��������߼��ļ��㣩
				StrategyCalculate(pDepthMarketData);
				//����λ
				CheckingPosition(pDepthMarketData);
			}
			else
			{
				MaintainContract(pDepthMarketData, m_instId);
				TDSpi_stgy->MaintainOrder(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, "CheckOnTick", &m_LastedVolume, pDepthMarketData->InstrumentID);
			}
		}
	}
}

//���ý��׵ĺ�Լ����
void Strategy::Init(string strategyType, string instIddecoration, string instId, string exePath, int kdbPort, string kdbScrpit, int strategyVolumeTarget, string strategykdbscript, double par1, double par2, double par3, double par4, double par5, double par6, int strategyOrderType1, int strategyOrderType2, int strategyOrderType3, int strategyOrderType4, int strategyOrderType5, int strategyOrderType6, string strategyPairLeg1Symbol, string strategyPairLeg2Symbol, string strategyPairLeg3Symbol)
{
	vector<string> instIdDecorationList;
	vector<string> instIdList;
	SplitString(instIddecoration, instIdDecorationList, ",");
	SplitString(instId, instIdList, ",");
	m_exePath = exePath.append("\\Bats\\DAReboot.bat");
	for (vector<string>::size_type i = 0; i != instIdDecorationList.size(); ++i)
	{
		m_instIddecoration_instId.insert(pair<string, string>(instIdDecorationList[i], instIdList[i]));
	}
	m_StrategyType = strategyType;
	orderType[0] = strategyOrderType1;
	orderType[1] = strategyOrderType2;
	orderType[2] = strategyOrderType3;
	orderType[3] = strategyOrderType4;
	orderType[4] = strategyOrderType5;
	orderType[5] = strategyOrderType6;
	strcpy_s(m_instId, instId.c_str());
	strcpy_s(m_instIddecoration, instIddecoration.c_str());
	kdbConnector.connect("localhost", kdbPort);
	m_kdbScript = strategykdbscript;
	m_VolumeTarget = strategyVolumeTarget;
	string kdb_par_string = "KindleLengthOnSecond:" + to_string((int)par1) + ";" + "WindowFrameLength:" + to_string((int)par2) + ";" + "StandardDeviationTimes:" + to_string(par3) + ";" + "PairLeg1Symbol:" + strategyPairLeg1Symbol + ";" + "PairLeg2Symbol:" + strategyPairLeg2Symbol + ";" + "PairLeg3Symbol:" + strategyPairLeg3Symbol + ";";
	string kdb_init_string = "h:hopen `::";
	kdb_init_string.append(to_string(m_kdbQuotePort));
	kdb_init_string.append(";PairFormula:{(x%y)};f:{x%y};bollingerBands: {[k;n;data]      movingAvg: mavg[n;data];    md: sqrt mavg[n;data*data]-movingAvg*movingAvg;      movingAvg+/:(k*-1 0 1)*\\:md};ReceiveTimeToDate:{(\"\"z\"\" $ 1970.01.01+ floor x %86400000000 )+ 08:00:00.000 +\"\"j\"\"$ 0.001* x mod  86400000000};isTable:{if[98h=type x;:1b];if[99h=type x;:98h=type key x];0b};isTable2: {@[{isTable value x}; x; 0b]};");
	//kdbConnector.sync(kdb_init_string.c_str());
	kdbConnector.sync(kdb_par_string.c_str());
	m_kdbScript = kdbScrpit + m_kdbScript;
	////////////////////////////////////////////////////
	/////�����񲻴������ó�ʼ�ź�Ϊ0             //////
	///////////////////////////////////////////////////
	kdb::Result res = kdbConnector.sync("isTable2 `ShortLong");
	if (res.res_->g)
	{
		res = kdbConnector.sync("exec count Signal from ShortLong");
		m_ShortLongInitNum = res.res_->j;

	}
	else
	{
		kdbConnector.sync("FinalSignal:([] Date:(); ReceiveDate:(); Symbol:(); LegOneBidPrice1:(); LegOneBidVol1:(); LegOneAskPrice1:(); LegOneAskVol1:(); LegTwoBidPrice1:(); LegTwoBidVol1:(); LegTwoAskPrice1:(); LegTwoAskVol1:(); LegThreeBidPrice1:(); LegThreeBidVol1:(); LegThreeAskPrice1:(); LegThreeAskVol1:(); BidPrice1:(); BidVol1:(); AskPrice1:(); AskVol:(); LowerBand:(); HigherBand:(); Signal:());ShortLong:FinalSignal;");
		kdbConnector.sync(m_kdbScript.c_str());
		m_ShortLongInitNum = 0;

	}

}

void Strategy::StrategyCalculate(CThostFtdcDepthMarketDataField *pDepthMarketData)
{

	TThostFtdcInstrumentIDType    instId;//��Լ,��Լ�����ڽṹ�����Ѿ�����
	strcpy_s(instId, m_instId);
	string instIDString = instId;
	string order_type = TDSpi_stgy->Send_TThostFtdcCombOffsetFlagType(instIDString.substr(0,2));
	

	TThostFtdcDirectionType       dir;//����,'0'��'1'��
	TThostFtdcCombOffsetFlagType  kpp = "8";//��ƽ��"0"����"1"ƽ,"3"ƽ��
	TThostFtdcPriceType           price = 0.0;//�۸�0���м�,��������֧��
	TThostFtdcVolumeType          vol = 0;//����
	m_LastedVolume = 0;

	kdbConnector.sync(m_kdbScript.c_str());
	kdb::Result res = kdbConnector.sync("exec count Signal from ShortLong");
	m_ReadShortLongInitNum = res.res_->j;
	if (m_ReadShortLongInitNum > 0)
	{
		res = kdbConnector.sync("exec last LegOneAskPrice1 from ShortLong");
		m_ReadLegOneAskPrice1 = res.res_->f;
		res = kdbConnector.sync("exec last LegOneBidPrice1 from ShortLong");
		m_ReadLegOneBidPrice1 = res.res_->f;
	}

	if (m_ReadShortLongInitNum > m_ShortLongInitNum)
	{
		m_ShortLongInitNum = m_ReadShortLongInitNum;
		kdb::Result res = kdbConnector.sync("exec last Signal from ShortLong");
		m_ReadShortLongSignal = res.res_->j;


		if (m_ReadShortLongInitNum == 1)
		{
			m_OrderVolumeMultiple = 1;
		}
		else
		{
			m_OrderVolumeMultiple = 2;
		}

		if (m_ReadShortLongSignal > 0)
		{
			dir = '0';
			price = pDepthMarketData->AskPrice1;
			vol = m_VolumeTarget * m_OrderVolumeMultiple;
			//����������ַ�ת�ź���ȡ���������Ʒ�ֵĶ���
			TDSpi_stgy->MaintainOrder(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, "CancelInstrumentIDOrder", &m_LastedVolume, instId);
			if (vol - m_LastedVolume > 0)
			{
				//TDSpi_stgy->Twap_Prep(instId, order_type, dir, &kpp, m_ReadLegOneAskPrice1, m_ReadLegOneBidPrice1, price, vol - m_LastedVolume, orderType, pDepthMarketData);

			}

		}
		else if (m_ReadShortLongSignal < 0)
		{
			dir = '1';
			price = pDepthMarketData->BidPrice1;
			vol = m_VolumeTarget * m_OrderVolumeMultiple;
			TDSpi_stgy->MaintainOrder(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, "CancelInstrumentIDOrder", &m_LastedVolume, instId);
			if (vol - m_LastedVolume > 0)
			{
				//TDSpi_stgy->Twap_Prep(instId, order_type, dir, &kpp, m_ReadLegOneAskPrice1, m_ReadLegOneBidPrice1, price, vol - m_LastedVolume, orderType, pDepthMarketData);
			}
			
		}
		m_LastedVolume = 0;
	}

}

//�����Ƿ�������
void Strategy::set_allow_open(bool x)
{
	m_allow_open = x;

	if(m_allow_open == true)
	{
		cerr<<"��ǰ���ý����������"<<endl;


	}
	else if(m_allow_open == false)
	{
		cerr<<"��ǰ���ý������ֹ����"<<endl;

	}

}

//�������ݵ�txt��csv
void Strategy::SaveDataTxtCsv(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//�������鵽txt
	string date = pDepthMarketData->TradingDay;
	string time = pDepthMarketData->UpdateTime;
	double buy1price = pDepthMarketData->BidPrice1;
	int buy1vol = pDepthMarketData->BidVolume1;
	double new1 = pDepthMarketData->LastPrice;
	double sell1price = pDepthMarketData->AskPrice1;
	int sell1vol = pDepthMarketData->AskVolume1;
	double vol = pDepthMarketData->Volume;
	double openinterest = pDepthMarketData->OpenInterest;//�ֲ���



	string instId = pDepthMarketData->InstrumentID;

	//����date�ĸ�ʽ
	string a = date.substr(0,4);
	string b = date.substr(4,2);
	string c = date.substr(6,2);

	string date_new = a + "-" + b + "-" + c;


	ofstream fout_data("output/" + instId + "_" + date + ".txt",ios::app);

	fout_data<<date_new<<","<<time<<","<<buy1price<<","<<buy1vol<<","<<new1<<","<<sell1price<<","<<sell1vol<<","<<vol<<","<<openinterest<<endl;

	fout_data.close();




	ofstream fout_data_csv("output/" + instId + "_" + date + ".csv",ios::app);

	fout_data_csv<<date_new<<","<<time<<","<<buy1price<<","<<buy1vol<<","<<new1<<","<<sell1price<<","<<sell1vol<<","<<vol<<","<<openinterest<<endl;

	fout_data_csv.close();

}

//�������ݵ�vector
void Strategy::SaveDataVec(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	m_OnTickCount++;
	if (m_OnTickCount > 120)
	{
		m_OnTickCount = 0;
		string date = pDepthMarketData->TradingDay;
		string time = pDepthMarketData->UpdateTime;
		double buy1price = pDepthMarketData->BidPrice1;
		int buy1vol = pDepthMarketData->BidVolume1;
		double new1 = pDepthMarketData->LastPrice;
		double sell1price = pDepthMarketData->AskPrice1;
		int sell1vol = pDepthMarketData->AskVolume1;
		double vol = pDepthMarketData->Volume;
		double openinterest = pDepthMarketData->OpenInterest;//�ֲ���


		futureData.date = date;
		futureData.time = time;
		futureData.buy1price = buy1price;
		futureData.buy1vol = buy1vol;
		futureData.new1 = new1;
		futureData.sell1price = sell1price;
		futureData.sell1vol = sell1vol;
		futureData.vol = vol;
		futureData.openinterest = openinterest;
		m_futureData_vec.push_back(futureData);
	}
	

}

void Strategy::set_instMessage_map_stgy(map<string, CThostFtdcInstrumentField*>& instMessage_map_stgy)
{
	m_instMessage_map_stgy = instMessage_map_stgy;
	cerr<<"�յ���Լ����:"<<m_instMessage_map_stgy.size()<<endl;

}

//�����˻���ӯ����Ϣ
void Strategy::CalculateEarningsInfo(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//���º�Լ�����¼ۣ�û�гֲ־Ͳ���Ҫ���£��гֲֵģ����ǽ��׵ĺ�ԼҲҪ���¡�Ҫ�ȼ���ӯ����Ϣ�ټ�������߼�

	//�жϸú�Լ�Ƿ��гֲ�
	if(TDSpi_stgy->send_trade_message_map_KeyNum(pDepthMarketData->InstrumentID) > 0)
		TDSpi_stgy->setLastPrice(pDepthMarketData->InstrumentID, pDepthMarketData->LastPrice);


	//�����˻��ĸ���ӯ��,�����ּ���
	m_openProfit_account = TDSpi_stgy->sendOpenProfit_account(pDepthMarketData->InstrumentID, pDepthMarketData->LastPrice); 

	//�����˻���ƽ��ӯ��
	m_closeProfit_account = TDSpi_stgy->sendCloseProfit();

	cerr<<" ƽ��ӯ��:"<<m_closeProfit_account<<",����ӯ��:"<<m_openProfit_account<<"��ǰ��Լ:"<<pDepthMarketData->InstrumentID<<" ���¼�:"<<pDepthMarketData->LastPrice<<" ʱ��:"<<pDepthMarketData->UpdateTime<<endl;//double����Ϊ0��ʱ�����-1.63709e-010����С��0�ģ���+1���ֵ��1

	//TDSpi_stgy->printTrade_message_map();



}

void Strategy::GetHistoryData()
{
	vector<string> data_fileName_vec;
	Store_fileName("input/��ʷK������", data_fileName_vec);

	cout<<"��ʷK���ı���:"<<data_fileName_vec.size()<<endl;

	for(vector<string>::iterator iter = data_fileName_vec.begin(); iter != data_fileName_vec.end(); iter++)
	{
		cout<<*iter<<endl;
		ReadDatas(*iter, history_data_vec);
				
	}

	cout<<"K������:"<<history_data_vec.size()<<endl;

}

void Strategy::MaintainContract(CThostFtdcDepthMarketDataField *pDepthMarketData, TThostFtdcInstrumentIDType m_instId)
{

	if (strcmp(pDepthMarketData->InstrumentID, m_instId) != 0 && String_StripNum(pDepthMarketData->InstrumentID) == String_StripNum(m_instId))
	{		
		if (TDSpi_stgy->SendHolding_long(pDepthMarketData->InstrumentID) > 0 && !TDSpi_stgy->Check_OrderList_TwapMessage(pDepthMarketData->InstrumentID))
		{
			string instIDString = pDepthMarketData->InstrumentID;
			TThostFtdcCombOffsetFlagType  kpp = "8";
			vector<int> orderType_erase = {1, 1, 0, 0, 1, 0 };

 			TDSpi_stgy->Twap_Prep(pDepthMarketData->InstrumentID, TDSpi_stgy->Send_TThostFtdcCombOffsetFlagType(instIDString.substr(0, 2)), '1', &kpp, pDepthMarketData->AskPrice1, pDepthMarketData->BidPrice1, pDepthMarketData->AskPrice1, TDSpi_stgy->SendHolding_long(pDepthMarketData->InstrumentID), orderType_erase, pDepthMarketData);

			//TDSpi_stgy->Twap_Prep(m_instId, TDSpi_stgy->Send_TThostFtdcCombOffsetFlagType(instIDString.substr(0, 2)), '0', &kpp, pDepthMarketData->AskPrice1, pDepthMarketData->BidPrice1, pDepthMarketData->AskPrice1, TDSpi_stgy->SendHolding_long(pDepthMarketData->InstrumentID), orderType_erase, pDepthMarketData);
		}
		else if (TDSpi_stgy->SendHolding_long(pDepthMarketData->InstrumentID) == 0 && TDSpi_stgy->SendHolding_short(pDepthMarketData->InstrumentID) > 0 && !TDSpi_stgy->Check_OrderList_TwapMessage(pDepthMarketData->InstrumentID))
		{
			string instIDString = pDepthMarketData->InstrumentID;
			TThostFtdcCombOffsetFlagType  kpp = "8";
			vector<int> orderType_erase = {1, 1, 0, 0, 1, 0 };

			TDSpi_stgy->Twap_Prep(pDepthMarketData->InstrumentID, TDSpi_stgy->Send_TThostFtdcCombOffsetFlagType(instIDString.substr(0, 2)), '0', &kpp, pDepthMarketData->AskPrice1, pDepthMarketData->BidPrice1, pDepthMarketData->BidPrice1, TDSpi_stgy->SendHolding_short(pDepthMarketData->InstrumentID), orderType_erase, pDepthMarketData);

			//TDSpi_stgy->Twap_Prep(m_instId, TDSpi_stgy->Send_TThostFtdcCombOffsetFlagType(instIDString.substr(0, 2)), '1', &kpp, pDepthMarketData->AskPrice1, pDepthMarketData->BidPrice1, pDepthMarketData->BidPrice1, TDSpi_stgy->SendHolding_short(pDepthMarketData->InstrumentID), orderType_erase, pDepthMarketData);
		}
		
	}
}

void Strategy::CheckingPosition(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	
	if (strcmp(pDepthMarketData->InstrumentID, m_instId) == 0 && !TDSpi_stgy->Check_OrderList_TwapMessage(pDepthMarketData->InstrumentID))
	{
		TThostFtdcInstrumentIDType    instId;//��Լ�����ڽṹ�����Ѿ�����
		strcpy_s(instId, m_instId);
		string instIDString = instId;
		string order_type = TDSpi_stgy->Send_TThostFtdcCombOffsetFlagType(instIDString.substr(0, 2));


		TThostFtdcDirectionType       dir;//����,'0'��'1'��
		TThostFtdcCombOffsetFlagType  kpp = "8";//��ƽ��"0"����"1"ƽ,"3"ƽ��
		TThostFtdcPriceType           price = 0.0;//�۸�0���м�,��������֧��
		TThostFtdcVolumeType          vol = 0;//����

		kdbConnector.sync(m_kdbScript.c_str());
		kdb::Result res = kdbConnector.sync("exec count Signal from ShortLong");
		m_ReadShortLongInitNum = res.res_->j;
		res = kdbConnector.sync("exec last LegOneAskPrice1 from ShortLong");
		m_ReadLegOneAskPrice1 = res.res_->f;
		res = kdbConnector.sync("exec last LegOneBidPrice1 from ShortLong");
		m_ReadLegOneBidPrice1 = res.res_->f;

		if (m_ReadLegOneAskPrice1 >= pDepthMarketData->UpperLimitPrice || m_ReadLegOneAskPrice1 <= pDepthMarketData->LowerLimitPrice || m_ReadLegOneBidPrice1 >= pDepthMarketData->UpperLimitPrice || m_ReadLegOneBidPrice1 <= pDepthMarketData->LowerLimitPrice)
		{
			m_ReadLegOneAskPrice1 = pDepthMarketData->AskPrice1;
			m_ReadLegOneBidPrice1 = pDepthMarketData->BidPrice1;
		}

		m_ShortLongInitNum = m_ReadShortLongInitNum;
		m_CheckLegOneAskPrice1 = pDepthMarketData->AskPrice1;
		m_CheckLegOneBidPrice1 = pDepthMarketData->BidPrice1;
		if (m_ReadShortLongInitNum == m_ShortLongInitNum)
		{			
			kdb::Result res = kdbConnector.sync("exec last Signal from ShortLong");
			m_ReadShortLongSignal = res.res_->j;


			if (m_ReadShortLongInitNum == 1)
			{
				m_OrderVolumeMultiple = 1;
			}
			else
			{
				m_OrderVolumeMultiple = 1;
			}

			int currentPosition = 0;//TDSpi_stgy->SendHolding_long(instId) - TDSpi_stgy->SendHolding_short(instId);
			vol = m_VolumeTarget * m_OrderVolumeMultiple;
			if (m_ReadShortLongSignal > 0)
			{
				currentPosition = TDSpi_stgy->SendHolding_long(instId) - TDSpi_stgy->SendHolding_short(instId);
				if (currentPosition - vol > 0)
				{
					dir = '1';
					price = pDepthMarketData->BidPrice1;					
					TDSpi_stgy->Twap_Prep(instId, order_type, dir, &kpp, m_CheckLegOneAskPrice1, m_CheckLegOneBidPrice1, price, currentPosition - vol, orderType_erase, pDepthMarketData);
				}
				else if (vol - currentPosition > 0)
				{
					if (currentPosition + vol < 0)
					{
						dir = '0';
						price = pDepthMarketData->AskPrice1;
						TDSpi_stgy->Twap_Prep(instId, order_type, dir, &kpp, m_CheckLegOneAskPrice1, m_ReadLegOneBidPrice1, price, -(vol + currentPosition), orderType_erase, pDepthMarketData);
					}
					else
					{
						dir = '0';
						price = pDepthMarketData->AskPrice1;
						TDSpi_stgy->Twap_Prep(instId, order_type, dir, &kpp, m_ReadLegOneAskPrice1, m_ReadLegOneBidPrice1, price, vol - currentPosition, orderType, pDepthMarketData);
					}
					
				}
				

			}
			else if (m_ReadShortLongSignal < 0)
			{
				currentPosition = -(TDSpi_stgy->SendHolding_long(instId) - TDSpi_stgy->SendHolding_short(instId));
				if (currentPosition - vol > 0)
				{
					dir = '0';
					price = pDepthMarketData->AskPrice1;
					TDSpi_stgy->Twap_Prep(instId, order_type, dir, &kpp, m_CheckLegOneAskPrice1, m_ReadLegOneBidPrice1, price, currentPosition - vol, orderType_erase, pDepthMarketData);
				}
				else if (vol - currentPosition > 0)
				{
					if (currentPosition + vol < 0)
					{
						dir = '1';
						price = pDepthMarketData->BidPrice1;
						TDSpi_stgy->Twap_Prep(instId, order_type, dir, &kpp, m_CheckLegOneAskPrice1, m_CheckLegOneBidPrice1, price, -(currentPosition + vol), orderType_erase, pDepthMarketData);
					}
					else
					{
						dir = '1';
						price = pDepthMarketData->BidPrice1;
						TDSpi_stgy->Twap_Prep(instId, order_type, dir, &kpp, m_ReadLegOneAskPrice1, m_ReadLegOneBidPrice1, price, vol - currentPosition, orderType, pDepthMarketData);
					}
					
				}
			}
			m_LastedVolume = 0;
		}
	}
}
//�����ݵ�kdb
void Strategy::DataInsertToKDB(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	string insertstring;
	string date = pDepthMarketData->TradingDay;
	string datesplit = "D";
	string time = pDepthMarketData->UpdateTime;
	string receivedate = return_current_time_and_date();
	date = receivedate.substr(0, 10);
	string symbol = pDepthMarketData->InstrumentID;
	double bidPrice1 = pDepthMarketData->BidPrice1;
	int    bidvol1 = 1;
	double askPrice1 = pDepthMarketData->AskPrice1;
	int    askvol1 = 1;

	insertstring.append("`Quote insert (");
	insertstring.append(date);
	insertstring.append(datesplit);
	insertstring.append(time);
	insertstring.append(";");
	insertstring.append(receivedate);
	insertstring.append(";");
	insertstring.append("`");
	insertstring.append(symbol);
	insertstring.append(";");
	insertstring.append(to_string(bidPrice1));
	insertstring.append(";");
	insertstring.append(to_string(bidvol1));
	insertstring.append(";");
	insertstring.append(to_string(askPrice1));
	insertstring.append(";");
	insertstring.append(to_string(askvol1));
	insertstring.append(")");
	
	kdbConnector.sync(insertstring.c_str());
}

void Strategy::Data3MInsertToKDB(vector<string> v_product, CThostFtdcDepthMarketDataField *pDepthMarketData, int mode)
{
	if (mode == 123456789101112)
	{
		string insertstring;
		string date = pDepthMarketData->TradingDay;
		string datesplit = "D";
		string time = pDepthMarketData->UpdateTime;
		string receivedate = return_current_time_and_date();
		date = receivedate.substr(0, 10);
		string symbol = String_StripNum(pDepthMarketData->InstrumentID);
		string product = String_StripNum(pDepthMarketData->InstrumentID);
		symbol.append("3M");
		string symbolCON1 = product + "CON1";
		string symbolCON2 = product + "CON2";
		string symbolCON3 = product + "CON3";
		double bidPrice1 = 1.0;
		int    bidvol1 = 1;
		double askPrice1 = 1.0;
		int    askvol1 = 1;
		int monthdate = stoi(date.substr(8, 2));
		double bidpricecon1 = m_pMarketData_map[m_instIddecoration_instId[symbolCON1]]->bidprice1;
		double bidpricecon2 = m_pMarketData_map[m_instIddecoration_instId[symbolCON2]]->bidprice1;
		double bidpricecon3 = m_pMarketData_map[m_instIddecoration_instId[symbolCON3]]->bidprice1;
		double askpricecon1 = m_pMarketData_map[m_instIddecoration_instId[symbolCON1]]->askprice1;
		double askpricecon2 = m_pMarketData_map[m_instIddecoration_instId[symbolCON2]]->askprice1;
		double askpricecon3 = m_pMarketData_map[m_instIddecoration_instId[symbolCON3]]->askprice1;
		if (monthdate < 8)
		{
			bidPrice1 = bidpricecon2 + ((15 + monthdate) / 30.0)*(bidpricecon3 - bidpricecon2);
			askPrice1 = askpricecon2 + ((15 + monthdate) / 30.0)*(askpricecon3 - askpricecon2);
		}
		else if (monthdate >= 8 && monthdate <= 15)
		{
			bidPrice1 = bidpricecon1 + ((15 + monthdate) / 30.0)*(bidpricecon2 - bidpricecon1);
			askPrice1 = askpricecon1 + ((15 + monthdate) / 30.0)*(askpricecon2 - askpricecon1);
		}
		else if (monthdate > 15)
		{
			bidPrice1 = bidpricecon2 + ((monthdate - 15) / 30.0)*(bidpricecon3 - bidpricecon2);
			askPrice1 = askpricecon2 + ((monthdate - 15) / 30.0)*(askpricecon3 - askpricecon2);
		}



		insertstring.append("`Quote insert (");
		insertstring.append(date);
		insertstring.append(datesplit);
		insertstring.append(time);
		insertstring.append(";");
		insertstring.append(receivedate);
		insertstring.append(";");
		insertstring.append("`");
		insertstring.append(symbol);
		insertstring.append(";");
		insertstring.append(to_string(bidPrice1));
		insertstring.append(";");
		insertstring.append(to_string(bidvol1));
		insertstring.append(";");
		insertstring.append(to_string(askPrice1));
		insertstring.append(";");
		insertstring.append(to_string(askvol1));
		insertstring.append(")");

		kdbConnector.sync(insertstring.c_str());
	}
	
	if (mode == 159)
	{

	}
}

void Strategy::Set_CThostFtdcDepthMarketDataField(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	if (m_pMarketData_map.find(pDepthMarketData->InstrumentID) == m_pMarketData_map.end())
	{
		pMarketData_message* m_pMarketData_map_p = new pMarketData_message();
		m_pMarketData_map_p->instId = pDepthMarketData->InstrumentID;
		m_pMarketData_map_p->askprice1 = pDepthMarketData->AskPrice1;
		m_pMarketData_map_p->bidprice1 = pDepthMarketData->BidPrice1;
		m_pMarketData_map_p->vol = pDepthMarketData->Volume;

		m_pMarketData_map.insert(pair<string, pMarketData_message*>(pDepthMarketData->InstrumentID, m_pMarketData_map_p));
	}
	else
	{
		m_pMarketData_map[pDepthMarketData->InstrumentID]->instId = pDepthMarketData->InstrumentID;
		m_pMarketData_map[pDepthMarketData->InstrumentID]->askprice1 = pDepthMarketData->AskPrice1;
		m_pMarketData_map[pDepthMarketData->InstrumentID]->bidprice1 = pDepthMarketData->BidPrice1;
		m_pMarketData_map[pDepthMarketData->InstrumentID]->vol = pDepthMarketData->Volume;
	}
}

void Strategy::DataRebootDADataSource()
{
	kdb::Result res = kdbConnector.sync("temp:-500#select from Quote;exec count Date from temp where Symbol = `CA3M");
	m_DAQuoteCount = res.res_->j;
	if (m_DAQuoteCount == 0)
	{
		system(m_exePath.c_str());
	}
}

bool Strategy::IsMarketOpen(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	
	m_str_UpdateTime = pDepthMarketData->UpdateTime;// * 100000 + pDepthMarketData->UpdateTime[2] * 100000;
	m_currentIntUpdateTime = stoi(m_str_UpdateTime.substr(0, 2)) * 10000 + stoi(m_str_UpdateTime.substr(3, 2)) * 100 + stoi(m_str_UpdateTime.substr(6, 2));
	//+pDepthMarketData->UpdateTime[3] * 10000 + pDepthMarketData->UpdateTime[4] * 100 + pDepthMarketData->UpdateTime[6] * 10 + pDepthMarketData->UpdateTime[7];

	if ((m_currentIntUpdateTime < 150000 && m_currentIntUpdateTime > 90000) || m_currentIntUpdateTime > 210000 || (m_currentIntUpdateTime >= 0 && m_currentIntUpdateTime < 23000))
	{
		return true;
	}
	else
	{
		return false;
	}
}

string  Strategy::return_current_time_and_date()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y.%m.%dD%X");
	auto duration = now.time_since_epoch();

	typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<8>
	>::type> Days; /* UTC: +8:00 */
	Days days = std::chrono::duration_cast<Days>(duration);
	duration -= days;
	auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
	duration -= hours;
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
	duration -= minutes;
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
	duration -= seconds;
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
	duration -= milliseconds;
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
	duration -= microseconds;
	auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
	cout << (ss.str()).append(".").append(to_string(milliseconds.count())) << endl;
	return (ss.str()).append(".").append(to_string(milliseconds.count()));
}
