#include "ABP.h"

ABP::ABP(double delta_tau, double i_BER, int i_propDelay, bool NAK){
	srand((int)time(NULL));
	NAK_enabled = NAK;

	pckSent = 0;
	pckReceived = 0;
	headerLength = 54*8;
	transRate = 5000;
	BER = i_BER;
	propDelay = i_propDelay;
	timeOut = delta_tau*propDelay;
	pckNum = 5000;
	pckLength = (1500 * 8) + headerLength;

	seqNum = 0;
	nextACK = 1;
	nextFrame = 0;
	pckSent = 0;
	pckReceived = 0;
	currTime = 0.0;
	tempTime = 0.0;
}

// Send function takes in a time and sequence number and returns and event
Event ABP::send(double time, int SN){
	Event e;
	e.time = time;
	e.seqNum = SN;
	e = channel(e, FWD);
	e = receiver(e);
	e = channel(e, REV);

	// Set the event time to current time plus the total round trip time of the packet
	e.time = currTime + propDelay + double(pckLength) / (double)transRate + propDelay + double(headerLength) / (double)transRate;
	return e;
}

// Channel method takes in an Event and type (FWD/REV) and set the flag of the event after calculating the number of errors
Event ABP::channel(Event e, int type){
	int length;
	if (type == FWD){
		length = pckLength;
	}
	else if (type == REV){
		length = headerLength;
	}
	int numErrors = calcBitError(length);

	if (numErrors >= 5){
		e.flag = LOST;
	}
	else if (numErrors >= 1 && numErrors <= 4){
		e.flag = ERROR;
	}
	else
		e.flag = NO_ERROR;

	return e;
}

// Receiver method which takes in and returns an event
Event ABP::receiver(Event e){
	// If there is no error from the forward channel and the sequence number matches the next frame
	// increment the next frame counter and set the type and sequence number of the ACK message
	if (e.flag == NO_ERROR && e.seqNum == nextFrame){
		nextFrame = (nextFrame + 1) % 2;
		e.seqNum = nextFrame;
		e.type = ACK;
	}
	// If the frame was lost then set the type for the event and do nothing else
	else if (e.flag == LOST){
		e.type = NIL;
	}
	// If there is an error or if the sequence number doesnt match send an ACK without incrementing the next frame counter
	else{
		e.type = ACK;
		e.seqNum = nextFrame;
	}

	return e;
}

// Function to calculate the number of zero error using the bit error rate
int ABP::calcBitError(int length){
	int bitErrorCounter = 0;
	for (int i = 0; i < length; i++){
		double ranNum = getRanNum();
		if (ranNum < BER){
			bitErrorCounter++;
		}
	}
	return bitErrorCounter;
}

// Function to retrieve a random variable 
double ABP::getRanNum(){
	//return (((double)rand()) / ((double)RAND_MAX + 1));
	return distribution(generator);
}

double ABP::simulate(){
	// Seed Random Variable
	srand((int)time(NULL));

	while (pckReceived < pckNum){
		// If there are no remaining timeouts in the queue then the next packet can be send.
		// This also applies to the initial packet
		if (!scheduler.containsTimeOut()){
			// Purge old timeout
			scheduler.purgeTimeOut();

			// Register new timeout with the timeout delay and transmission delay
			tempTime = currTime + timeOut + (double)pckLength / (double)transRate;
			scheduler.sendTimeOut(tempTime, seqNum);

			// Send packet with current time and sequence number
			Event e = send(currTime, seqNum);
			pckSent++;

			if (e.type != NIL)
				scheduler.pushEvent(e);
		}
		// Read the next event from the queue and set the current time to event time and dequeue the event
		Event e = scheduler.getEvent();
		currTime = e.time;
		scheduler.popEvent();

		// Case for a successfull ACK without error and with a sequence number matching nextACK
		// We will purge the existing timeouts so the next packet can be sent 
		if (e.type == ACK && e.flag == NO_ERROR && e.seqNum == nextACK){
			pckReceived++;
			scheduler.purgeTimeOut();
			seqNum = (seqNum + 1) % 2;
			nextACK = (seqNum + 1) % 2;
		}
		// Case for ACK with error or ACK without a matching sequence number
		// Purge the timeouts and do not increment the counters so the same packet gets sent
		else if ((e.type == ACK && e.flag == ERROR) || (e.type == ACK && e.seqNum != nextACK)){
			if (NAK_enabled){
				scheduler.purgeTimeOut();
			}
			continue;
		}
		// If the event is a timout, do nothing, the timeout will be dequeued and the packet will be retransmitted
		else if (e.type == TIMEOUT){
		}

	}
	double throughput = (double)(pckReceived * 1500 * 8) / (double)(currTime / 1000);
	return throughput;
}
