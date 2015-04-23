#ifndef ABP_H 
#define ABP_H


#include "EventScheduler.h"
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <random>

#define FWD 1
#define REV 2

using namespace std;

class ABP{
public:
	ABP(double del_tau, double BER, int propDelay, bool NAK);
	Event send(double time, int SN);
	Event channel(Event e, int type);
	Event receiver(Event e);
	int calcBitError(int length);
	double getRanNum();
	double simulate();

private:
	bool NAK_enabled;
	int pckLength;
	int headerLength;
	int seqNum;
	int nextACK;
	int nextFrame;
	int transRate;
	int pckSent;
	int pckReceived;
	int pckNum;
	int propDelay;
	double BER;
	double currTime;
	double tempTime;
	double timeOut;
	EventScheduler scheduler;
	mt19937 generator;
	uniform_real_distribution<> distribution;
};
#endif
