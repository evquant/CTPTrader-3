#ifndef _STRATEGY_H
#define _STRATEGY_H

#include "ThostFtdcMdApi.h"
#include <vector>
#include "StructFunction.h"
#include <string>
#include <iostream>
#include <map>
#include "traderspi.h"
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time




class Strategy
{
public:
	Strategy(CtpTraderSpi* TDSpi):TDSpi_stgy(TDSpi)
	{		
		m_allow_open = false;

		//GetHistoryData();

	}

	//����ص�������ÿ�յ�һ������ʹ���һ��
	void OnTickData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	//���ý��׵ĺ�Լ����
	void Init(string strategyType, string instIddecoration, string instId, string exePath, int kdbPort, string kdbScript, int strategyVolumeTarget, string strategykdbscript, double par1, double par2, double par3, double par4, double par5, double par6, int strategyOrderType1, int strategyOrderTyep2, int strategyOrderType3, int strategyOrderType4, int strategyOrderType5, int strategyOrderType6, string strategyPairLeg1Symbol, string strategyPairLeg2Symbol, string strategyPairLeg3Symbol);
	
	//�������߼��ļ��㣬70������������0.6Ԫ�������࣬�µ������գ�ϵͳĬ�Ͻ�ֹ���֣����ڽ�������"yxkc"�������֣�
	void StrategyCalculate(CThostFtdcDepthMarketDataField *pDepthMarketData);

	//�����Ƿ�������
	void set_allow_open(bool x);
	
	//�����������ݵ�vector
	void SaveDataVec(CThostFtdcDepthMarketDataField *pDepthMarketData);
	
	//�����������ݵ�txt��csv
	void SaveDataTxtCsv(CThostFtdcDepthMarketDataField *pDepthMarketData);

	//���ú�Լ-��Լ��Ϣ�ṹ���map
	void set_instMessage_map_stgy(map<string, CThostFtdcInstrumentField*>& instMessage_map_stgy);

	//�����˻���ӯ����Ϣ
	void CalculateEarningsInfo(CThostFtdcDepthMarketDataField *pDepthMarketData);
	
	//��ȡ��ʷ����
	void GetHistoryData();

	//�����ݵ�kdb
	void DataInsertToKDB(CThostFtdcDepthMarketDataField *pDepthMarketData);

	//3M���ݵ�kdb
	void Data3MInsertToKDB(vector<string> v_product, CThostFtdcDepthMarketDataField *pDepthMarketData, int mode);
	
	//�����Ҫ���µĲ�λ����
	void MaintainContract(CThostFtdcDepthMarketDataField *pDepthMarketData, TThostFtdcInstrumentIDType m_instId);

	//����λ�Ƿ�ƥ��
	void CheckingPosition(CThostFtdcDepthMarketDataField *pDepthMarketData);

	//��õ�ǰʱ��Ϊkdb+���ݲ���ʹ��
	std::string return_current_time_and_date();

	//����DA�������ĳ���
	void Strategy::DataRebootDADataSource();

	//���ݵ�ǰKDB���ݿ��������ݼ���Ƿ���
	bool Strategy::IsMarketOpen(CThostFtdcDepthMarketDataField *pDepthMarketData);

	void Strategy::Set_CThostFtdcDepthMarketDataField(CThostFtdcDepthMarketDataField *pDepthMarketData);

private:

	CtpTraderSpi* TDSpi_stgy;//TDָ��

	TThostFtdcInstrumentIDSubscribeType m_instIddecoration;//��Լ����

	TThostFtdcInstrumentIDSubscribeType m_instId;//��Լ����

	FutureData futureData;//�Զ�����������ݽṹ��

	vector<FutureData> m_futureData_vec;//�����������ݵ�vector,�����ָ����Ҫnew���ٱ���

	bool m_allow_open;//TRUE�����֣�FALSE��ֹ����
		
	map<string, CThostFtdcInstrumentField*> m_instMessage_map_stgy;//�����Լ��Ϣ��map,ͨ��set_instMessage_map_stgy()������TD����
	
	double m_openProfit_account;//�����˻��ĸ���ӯ��,�����ּ���
	
	double m_closeProfit_account;//�����˻���ƽ��ӯ��

	vector<History_data> history_data_vec;//������ʷ���ݵ�vector

	map<string, string> m_instIddecoration_instId;//CThostFtdcInputOrderField

	map<string, pMarketData_message*> m_pMarketData_map;//�����Զ���ļ۸���Ϣ��map������׷��
	
};

#endif