// main.cpp : Defines the entry point for the console application.
/*
By:
██████  ▄▄▄      ▄▄▄      ▓█████▄    ▒███████▒ ▄▄▄       ███▄ ▄███▓ ▄▄▄       ███▄    █
▒██    ▒ ▒████▄   ▒████▄    ▒██▀ ██▌   ▒ ▒ ▒ ▄▀░▒████▄    ▓██▒▀█▀ ██▒▒████▄     ██ ▀█   █
░ ▓██▄   ▒██  ▀█▄ ▒██  ▀█▄  ░██   █▌   ░ ▒ ▄▀▒░ ▒██  ▀█▄  ▓██    ▓██░▒██  ▀█▄  ▓██  ▀█ ██▒
▒   ██▒░██▄▄▄▄██░██▄▄▄▄██ ░▓█▄   ▌     ▄▀▒   ░░██▄▄▄▄██ ▒██    ▒██ ░██▄▄▄▄██ ▓██▒  ▐▌██▒
▒██████▒▒ ▓█   ▓██▒▓█   ▓██▒░▒████▓    ▒███████▒ ▓█   ▓██▒▒██▒   ░██▒ ▓█   ▓██▒▒██░   ▓██░
▒ ▒▓▒ ▒ ░ ▒▒   ▓▒█░▒▒   ▓▒█░ ▒▒▓  ▒    ░▒▒ ▓░▒░▒ ▒▒   ▓▒█░░ ▒░   ░  ░ ▒▒   ▓▒█░░ ▒░   ▒ ▒
░ ░▒  ░ ░  ▒   ▒▒ ░ ▒   ▒▒ ░ ░ ▒  ▒    ░░▒ ▒ ░ ▒  ▒   ▒▒ ░░  ░      ░  ▒   ▒▒ ░░ ░░   ░ ▒░
░  ░  ░    ░   ▒    ░   ▒    ░ ░  ░    ░ ░ ░ ░ ░  ░   ▒   ░      ░     ░   ▒      ░   ░ ░
░        ░  ░     ░  ░   ░         ░ ░          ░  ░       ░         ░  ░         ░
░         ░
*/
#include "GBN.h"
#include "ABP.h"
#include "EventScheduler.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
	string type(argv[1]);
	string NAK(argv[2]);
	string param;
	bool NAK_param;
	string fileName;

	if (argc != 3){
		std::cout << "Must specify simulator type (ABP/GBN) in the first parameter and must specify if NAK is enabled with (true/false) in the second parameter." << endl;
	}
	else{
		if (type == "GBN" || type == "gbn"){
			param = "GBN";
			fileName = "GBN.csv";
		}
		else if (type == "ABP" || type == "abp"){
			param = "ABP";
			
			if (NAK == "True" || NAK == "TRUE" || NAK == "true"){
				NAK_param = true;
				fileName = "ABP_NAK.csv";
			}
			else {
				NAK_param = false;
				fileName = "ABP.csv";
			}
		}	
	}
	
	double throughput;
	double del_tau = 2.5;
	double BER;
	int propDelay;

	vector<double> BERVals;
	BERVals.push_back(0);
	BERVals.push_back(0.00001);
	BERVals.push_back(0.0001);

	vector<int> propDelayVals;
	propDelayVals.push_back(5);
	propDelayVals.push_back(250);

	ofstream outfile(fileName.c_str(), ios_base::out);
	//outfile << "," << "10ms" << "," << "," <<"," << "100ms" << endl;
	//outfile << "Delta/Tau" << "," << "0" << "," << "0.00001" << "," << "0.0001" << "," << "0" << "," << "0.00001" << "," << "0.0001" << endl;

	while (del_tau < 13){
		for (int j = 0; j < 2; j++){
			propDelay = propDelayVals[j];
			for (int i = 0; i < 3; i++){
				BER = BERVals[i];
				if (param == "ABP" && NAK_param == true){
					ABP simulator(del_tau, BER, propDelay, true);
					throughput = simulator.simulate();
				}
				else if (param == "ABP" && NAK_param == false){
					ABP simulator(del_tau, BER, propDelay, false);
					throughput = simulator.simulate();
				}
				else if (param == "GBN"){
					GBN simulator(del_tau, BER, propDelay);
					throughput = (int)simulator.simulate();
				}
				else {
					cout << "Please give appropriate parameters" << endl;
				}
				outfile << throughput << ",";
			}
		}
		del_tau += 2.5;
		outfile << endl;	
	}

	return 0;
}

