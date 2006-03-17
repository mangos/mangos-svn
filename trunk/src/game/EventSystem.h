#ifndef WIN32  
#include <sys/timeb.h>  
#endif  

typedef void (*EventHandler) (void *arg);
	

#define ES_RESOLUTION 33

typedef struct{

	EventHandler handler;
	void *param;
	uint32 time;
	bool st;

	void * pNext;
	

}Event;


typedef struct{

	EventHandler handler;
	void *param;
	uint32 time;
	bool st;
	uint32 id;
	uint32 period;
	void * pNext;


}PeriodicEvent;




uint32 AddEvent(EventHandler  func,void* param,uint32 timer,bool separate_thread=false,bool bPeriodic = false);
void RemovePeriodicEvent(uint32 eventid);
void StartEventSystem();
