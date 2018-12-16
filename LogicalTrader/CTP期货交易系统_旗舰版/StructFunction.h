#ifndef _STRUCTFUNCTION_H
#define _STRUCTFUNCTION_H

#include "ThostFtdcUserApiDataType.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

struct twap_message
{
	twap_message()
	{
	}
	string instId;
	string order_type;
	char dir;
	double askprice; 
	double bidprice;
	double price;
	int vol;
	int executedvol;
	vector<int> orderType;	
	//vector<int> splitvolumelist;
	//vector<string> instIdList;
};

struct pMarketData_message
{
	pMarketData_message()
	{
	}
	string instId;
	double askprice1;
	double bidprice1;
	double price;
	int vol;
};

//������Ϣ�Ľṹ��
struct trade_message
{
	trade_message()
	{
		instId = "";
		lastPrice = 0.0;
		PreSettlementPrice = 0.0;
		holding_long = 0;
		holding_short = 0;

		TodayPosition_long = 0;
		YdPosition_long = 0;
		TodayPosition_short = 0;
		YdPosition_short = 0;

		TodayPosition_longlocked = 0;
		YdPosition_longlocked = 0;
		TodayPosition_shortlocked = 0;
		YdPosition_shortlocked = 0;

		closeProfit_long = 0.0;
		OpenProfit_long = 0.0;
		closeProfit_short = 0.0;
		OpenProfit_short = 0.0;
	}


	string instId;//��Լ����
	double lastPrice;//���¼ۣ�ʱ�̱����Լ�����¼ۣ�ƽ����
	double PreSettlementPrice;//�ϴν���ۣ��Ը�ҹ����ʱ��Ҫ�ã���������
	int holding_long;//�൥�ֲ���
	int holding_short;//�յ��ֲ���

	int TodayPosition_long;//�൥���ճֲ�
	int YdPosition_long;//�൥���ճֲ�

	int TodayPosition_short;//�յ����ճֲ�
	int YdPosition_short;//�յ����ճֲ�

	int TodayPosition_longlocked;
	int YdPosition_longlocked;//�൥���ճֲ�

	int TodayPosition_shortlocked;//�յ����ճֲ�
	int YdPosition_shortlocked;//�յ����ճֲ�
	double closeProfit_long;//�൥ƽ��ӯ��
	double OpenProfit_long;//�൥����ӯ��

	double closeProfit_short;//�յ�ƽ��ӯ��
	double OpenProfit_short;//�յ�����ӯ��

};




struct FutureData//����ṹ�嶨��
{
	string date;
	string time;
	double buy1price;
	int buy1vol;
	double new1;
	double sell1price;
	int sell1vol;
	double vol;
	double openinterest;//�ֲ���

};



//��ʷK��
struct History_data
{
	string date;
	string time;
	double buy1price;//��һ
	double sell1price;//��һ
	double open;
	double high;
	double low;
	double close;

};



//��ȡ��ʷK��
void ReadDatas(string fileName, vector<History_data> &history_data_vec);

int Store_fileName(string path, vector<string> &FileName);

string String_StripNum(string s);

string String_StripChar(string s);

int UpdateTime_Int(const string time);

string replace(std::string& str, const std::string& from, const std::string& to);

string replaceAll(std::string& str, const std::string& from, const std::string& to);

vector<char*> StringSplit(char* splittingstring);

void SplitString(const string& s, vector<string>& v, const string& c);

#endif