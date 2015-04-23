#ifndef ES_H 
#define ES_H


#include <queue>
#include <vector>

#define TIMEOUT 1
#define ACK 2
#define NIL 3
#define NO_ERROR 4
#define LOST 5
#define ERROR 6

struct Event{
	int type;
	double time;
	int seqNum;
	int flag;
};

struct CompareTime {
	bool operator() (const Event &e1, Event &e2) {
		return e1.time > e2.time;
	}
};

class EventScheduler{
public:
	EventScheduler();
	void pushEvent(Event e);
	void popEvent();
	void sendTimeOut(double time, int SN);
	void purgeTimeOut();
	bool isQueueEmpty();
	bool containsTimeOut();
	Event getEvent();
	
private:
	std::priority_queue<Event, std::vector<Event>, CompareTime> queue;
};

#endif

