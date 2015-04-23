#include "GBN.h"

GBN::GBN(double delta_tau, double i_BER, int i_propDelay){
	srand((int)time(NULL));
	headerLength = 54 * 8;
	transRate = 5000;
	BER = i_BER;
	propDelay = i_propDelay;
	timeOut = delta_tau*propDelay;
	pckNum = 5000;
	pckLength = (1500 * 8) + headerLength;
	buffSize = 4;
    packetsAvailable = 4;

    P = 0;
    lastAcked = -1;
	nextSendFrame = 0;
    nextRecFrame = 1;
    
	pckSent = 0;
	pckReceived = 0;
	currTime = 0.0;
}

// Function to calculate the number of zero error using the bit error rate
int GBN::calcBitError(int length){
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
double GBN::getRanNum(){
	//return (((double)rand()) / ((double)RAND_MAX + 1));
	return distribution(generator);
}

bool GBN::isAcceptableFrame(int SN){
	bool result = SN != P;
	return result;
}

Event GBN::send(double time, int SN){
	Event e;
	e.time = time;
	e.seqNum = SN;
	e = channel(e, FWD);
	e = receiver(e);
	e = channel(e, REV);

	// Set the event time to current time plus the total round trip time of the packet
	e.time = time + propDelay + double(pckLength) / (double)transRate + propDelay + double(headerLength) / (double)transRate;
	return e;

}

// Channel method takes in an Event and type (FWD/REV) and set the flag of the event after calculating the number of errors
Event GBN::channel(Event e, int type){
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
Event GBN::receiver(Event e){
	// If there is no error from the forward channel and the sequence number matches the next frame
	// increment the next frame counter and set the type and sequence number of the ACK message
	if (e.flag == NO_ERROR && e.seqNum == nextRecFrame){
        nextRecFrame = (nextRecFrame + 1) % (buffSize + 1);
        e.seqNum = nextRecFrame;
		e.type = ACK;
	}
	// If the frame was lost then set the type for the event and do nothing else
	else if (e.flag == LOST){
		e.type = NIL;
	}
	// If there is an error or if the sequence number doesnt match send an ACK without incrementing the next frame counter
	else{
		e.type = ACK;
        e.seqNum = nextRecFrame;
	}

	return e;
}

// Function which returns true if available packets are sent
bool GBN::sendPacketIfAvailable()
{
    if (packetsAvailable == 0) {
        return false;
    }
        
    double packetTimeout = currTime + (double)pckLength / (double)transRate + timeOut;

    if (scheduler.containsTimeOut()) {
        timeoutTimes.push(packetTimeout);
    }
    else {
        scheduler.sendTimeOut(packetTimeout, nextSendFrame);
    }

    Event e = send(currTime, nextSendFrame);
    if (e.type != NIL) {
        scheduler.pushEvent(e);
    }

    nextSendFrame = (nextSendFrame + 1) % (buffSize + 1);
    packetsAvailable--;

    return true;
}

// Main simulator method which returns the throughput value
double GBN::simulate(){
    while (pckReceived < pckNum) {
        bool sent = sendPacketIfAvailable();

        Event e = scheduler.getEvent();
        if (sent) {
            double sendPropDelay = (double)pckLength / (double)transRate;
            currTime += sendPropDelay;


            if (e.time > currTime + sendPropDelay) {
                // Nothing happened so we can send another packet if available
                continue;
            }
            else {
                // Readjust event time to after the packet finished sending
                e.time = currTime;
            }
        }

        scheduler.popEvent();
        currTime = e.time;

        if (TIMEOUT == e.type) {
            // retransmit all packets
            packetsAvailable = buffSize;

            // compensate for shifts
            nextSendFrame = P;

            while (!timeoutTimes.empty()) {
                timeoutTimes.pop();
            }

            while (!scheduler.isQueueEmpty()) {
                scheduler.popEvent();
            }

            scheduler.purgeTimeOut();

            continue;
        }
        else if (ACK == e.type) {
            if (e.flag != ERROR && isAcceptableFrame(e.seqNum)) {
                int amountToShift = e.seqNum - P;
                if (amountToShift < 0) {
                    amountToShift += buffSize + 1;
                }

                packetsAvailable += amountToShift;
                P = e.seqNum;
                pckReceived += amountToShift;

                scheduler.purgeTimeOut();

                for (int i = 1; i < amountToShift; i++) {
                    timeoutTimes.pop();
                }

                if (!timeoutTimes.empty()) {
                    // Schedule a timeout with an arbitrary sequence number
                    scheduler.sendTimeOut(timeoutTimes.front(), 0);
                    timeoutTimes.pop();
                }
            }
        }
	}

	double throughput = (double)(pckReceived * 1500 * 8) / (double)(currTime / 1000);
	return throughput;
}

