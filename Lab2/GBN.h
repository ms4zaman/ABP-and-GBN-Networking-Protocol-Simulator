#ifndef GBN_H 
#define GBN_H 


#include "EventScheduler.h"
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <random>

#define FWD 1
#define REV 2

using namespace std;

class GBN{
public:
	GBN(double del_tau, double BER, int propDelay);
	void sendBuffFrames();
	Event send(double time, int SN);
	Event channel(Event e, int type);
	Event receiver(Event e);
	int calcBitError(int length);
	double getRanNum();
	bool isAcceptableFrame(int SN);
    bool sendPacketIfAvailable();
	double simulate();

private:
	int pckLength;
	int headerLength;
	queue<double> timeoutTimes;

    // Sender
	int P;
    int nextSendFrame;
    int lastAcked;
    int packetsAvailable;

    int buffSize;

    // Receiver
	int nextRecFrame;


	int transRate;
	int pckSent;
	int pckReceived;
	int pckNum;
	int propDelay;
	double BER;
	double currTime;
	double timeOut;
	EventScheduler scheduler;
	mt19937 generator;
	uniform_real_distribution<> distribution;
};

#endif

