#include "EventScheduler.h"
#include <iterator>

EventScheduler::EventScheduler(){

}

bool EventScheduler::isQueueEmpty(){
	return queue.empty();
}

void EventScheduler::pushEvent(Event e){
	queue.push(e);
}

void EventScheduler::popEvent(){
	queue.pop();
}

void EventScheduler::sendTimeOut(double time, int SN){
	Event e;
	e.type = TIMEOUT;
	e.flag = NO_ERROR;
	e.time = time;
	e.seqNum = SN;
	pushEvent(e);
	
}

void EventScheduler::purgeTimeOut(){
	std::vector<Event> eventQueue;

	while (!isQueueEmpty()){
		Event e = getEvent();
		if (e.type != TIMEOUT){
			eventQueue.push_back(e);
		}
		popEvent();
	}
	
	for (std::vector<Event>::iterator it = eventQueue.begin(); it != eventQueue.end(); it++){
		pushEvent(*it);
	}
}

bool EventScheduler::containsTimeOut(){
	std::vector<Event> eventQueue;
	bool result = false; 

	while (!isQueueEmpty()){
		Event e = getEvent();
		if (e.type == TIMEOUT){
			result = true;
		}
        eventQueue.push_back(e);
		popEvent();
	}

	for (std::vector<Event>::iterator it = eventQueue.begin(); it != eventQueue.end(); it++){
		pushEvent(*it);
	}
	return result;
}
Event EventScheduler::getEvent(){
	return queue.top();
}
