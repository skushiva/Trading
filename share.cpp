
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <conio.h>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <iomanip>
#include <cassert>
#include <thread>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;
set<string> gShareName;
bool gNeedRefresh = false;
class performace {
	string start_date;
	string end_date;
	int investment;
	int active_investment;
	int total_profit;

};
class Shares {
public:
	string share_name;
	string txn_type;
	int unit;
	double price;
	double total_price;
	string date;
};
class Summary {
public:
	string share_name;
	int bCount{ 0 };
	int sCount{ 0 };
	double bPrice{ 0 };
	double sPrice{ 0 };
	double buyAvg{ 0 };
	double sellAvg{ 0 };
	int pending{ 0 };
	double profit{ 0 };
	double xirr;
	double min_buy{ 0 };
	double max_buy{ 0 };
	double min_sell{ 0 };
	double max_sell{ 0 };
	double recent_buy{ 0 };
	double recent_sell{ 0 };
	
	Summary() {

	}
	~Summary() {

	}
	bool operator<(const Summary& other) const {
		return share_name < other.share_name;
	}

};

unordered_map<string, Summary> gSummary;
unordered_multimap<string, Shares> gTxnDetails;
enum class Selection : int {  SUMMARY=1, LEDGER, EXIT };
vector<vector<string>> parseCSV(const string& filename) {
	ifstream file(filename);

	if (!file.is_open()) {
		cerr << "Error opening file: " << filename << endl;
		return {};
	}

	vector<vector<string>> data;
	string line;

	while (getline(file, line)) {
		vector<string> row;
		stringstream ss(line);
		string cell;

		while (getline(ss, cell, '\n')) {
			row.push_back(cell);
		}
		/*
		for (auto itr : row) {
			cout << "cell " << itr << endl;
		}
		_getch();*/
		data.push_back(row);
	}

	file.close();
	return data;
}

int generateSummary() {
	for (auto name : gShareName) {
		class Summary shareSummary;
		auto details = gTxnDetails.equal_range(name);
		auto fElement = details.first;
		int unitPrice = fElement->second.total_price / fElement->second.unit;
		shareSummary.min_buy = unitPrice;
		shareSummary.max_buy = unitPrice;
		shareSummary.recent_buy = unitPrice;
		for (auto it = details.first; it != details.second; it++) {
			if (it->second.txn_type == "buy") {
				shareSummary.bCount += it->second.unit;
				shareSummary.bPrice += it->second.total_price;
				if ((it->second.total_price / it->second.unit) > shareSummary.max_buy) {
					shareSummary.max_buy = it->second.total_price / it->second.unit;
				}
				if ((it->second.total_price / it->second.unit) < shareSummary.min_buy) {
					shareSummary.min_buy = it->second.total_price / it->second.unit;
				}
				shareSummary.recent_buy = it->second.total_price / it->second.unit;
			}
			else {
				shareSummary.sCount += it->second.unit;
				shareSummary.sPrice += it->second.total_price;
				if ((it->second.total_price / it->second.unit) > shareSummary.max_sell) {
					shareSummary.max_sell = it->second.total_price / it->second.unit;
				}
				if (shareSummary.min_sell == 0)
					shareSummary.min_sell = shareSummary.max_sell;
				if ((it->second.total_price / it->second.unit) < shareSummary.min_sell) {
					shareSummary.min_sell = it->second.total_price / it->second.unit;
				}
				shareSummary.recent_sell = it->second.total_price / it->second.unit;
			}

			//assert(shareSummary.sCount <= shareSummary.bCount);
		}
		shareSummary.buyAvg = shareSummary.bPrice / shareSummary.bCount;
		if (shareSummary.sCount > 0) {
			shareSummary.sellAvg = (shareSummary.sPrice / shareSummary.sCount);
			shareSummary.profit = (shareSummary.sellAvg - (shareSummary.bPrice / shareSummary.bCount)) * shareSummary.sCount;
		}
		shareSummary.pending = shareSummary.bCount - shareSummary.sCount;
		gSummary.insert({ name, shareSummary });
	}
	return 0;
}


void fileLoader() {

	string filename = "data.csv";
	vector<vector<string>> parsedData = parseCSV(filename);

	// Process the parsed data
	int skip = 1;
	for (const vector<string>& row : parsedData) {
		if (skip) {
			skip = 0;
			continue;
		}
		for (const string& cell : row) {
			stringstream ss(cell);
			string data;
			int idx = 0;
			Shares txn;
			int temp;
			while (getline(ss, data, ',')) {
				switch (idx) {
				case 0:
					txn.share_name = data;
					gShareName.insert(txn.share_name);
					//cout << "company share " << data;
					break;
				case 2:
					txn.date = data;
					//cout << "date " << data;
					break;
				case 6:

					txn.txn_type = data;
					//cout << "txn " << data;
					break;
				case 8:
					//temp = stoi(data);
					txn.unit = stoi(data);
					//cout << "unit " << data;
					break;
				case 9:
					//cout << "price " << data;
					txn.price = stod(data);
					txn.total_price = txn.unit * txn.price;
					break;
				}

				idx++;
			}
			gTxnDetails.insert(make_pair(txn.share_name, txn));
		}
	}
	generateSummary();
}

int summary() {
	cout << "\n\n \t\t\t Transcation summary \n\n";
	cout << "Share Name" << "|" << "bCount" << "|" << "sCount" << "|" << "buy Avg" << "|" << "sell Avg" << "|" << "pending" \
		<< "|" << "profit" << "|" << "Percent" << "|" << "min/max Buy" << "|" << "min/max Sell" << "|" << "rBuy/Sell" << endl;

	for (auto name : gShareName) {
#if 0 //Set lamba expression 
		auto info = find(gSummary.begin(), gSummary.end(), [&](const Summary& p) mutable {
			return p.share_name == name;
			});

#endif
		
		auto it = gSummary.equal_range(name);
		auto info = it.first;
		if (info->second.sCount > info->second.bCount)
			continue;
		cout << "--------------------------------------------------------------------------------------------------------------" << endl;
		string temp_name = name.substr(0, 10);
		double pPer = 0;
		if (info->second.sCount > 0)
			pPer = ((info->second.profit) / ((info->second.buyAvg) * info->second.sCount)) * 100;

		//stringstream min_max_buy;
		//min_max_buy << int(info->second.min_buy) + "/" + int(info->second.max_buy);
		string min_max_buy = to_string(int(info->second.min_buy)) + "/" + to_string(int(info->second.max_buy));
		string min_max_sell = to_string(int(info->second.min_sell)) + "/" + to_string(int(info->second.max_sell));
		string rBuySell = to_string(int(info->second.recent_buy)) + "/" + to_string(int(info->second.recent_sell));
		cout << left << setw(10) << temp_name << "|" << left << setw(6) << info->second.bCount << "|" << left << setw(6) << info->second.sCount << "|" \
			<< left << setw(7) << int(info->second.buyAvg) << "|" << left << setw(8) << int(info->second.sellAvg) << "|" << left << setw(7) << info->second.pending << \
			"|" << left << setw(6) << int(info->second.profit) << "|" << left << setw(7) << int(pPer) << "%" << "|" << left << setw(11) << min_max_buy \
			<< "|" << left << setw(11) << min_max_sell << "|" << left << setw(12) << rBuySell << endl;
	}
	return 0;
}

int Ledger() {
	cout << "\n\n \t\t\t Transcation summary \n\n";
	cout << "Share Name\t" << "|" << "txn_type\t" << "|" << "unit price\t" << "|" << "total price\t" << "|" << "txn date \t" << endl;
	for (auto i : gTxnDetails) {
		cout << "--------------------------------------------------------------------------------------------------------------" << endl;
		cout << left << setw(16) << i.second.share_name << "|      " << left << setw(8) << i.second.txn_type << "|    " << left << setw(12) << i.second.unit << "|    " << left << setw(10) << i.second.total_price \
			<< left << " | " << i.second.date << "     | " << endl;
	}

	return 0;
}


int main()
{

	int exit = true;
	int choice;
	thread myThread(fileLoader);

	cout << "\n\n\n\t\t\tWelcome share management \n" << endl;

	while (exit) {
		cout << "1 -> summary" << endl;
		cout << "2 -> Ledger" << endl;
		cout << "3 -> exit" << endl;
		cin >> choice;
		switch (static_cast<Selection>(choice)) {
		case Selection::SUMMARY:
			summary();
			break;
		case Selection::LEDGER:
			Ledger();
			break;
		case Selection::EXIT:
			exit = 0;
			break;
		}
	}
	myThread.join();

}
