#ifndef RIPPLEDEFINES_H
#define RIPPLEDEFINES_H

enum RIPPLE_TASK_STATE {
	RTS_OPEN,
	RTS_ACCEPTED,
	RTS_STARTED,
	RTS_COMPLETED,
	RTS_CLOSED, //positive confirmation
	RTS_CANCELED
};

enum RIPPLE_LOG_FLAVOR {
	RLF_CREATED, 	//0
	RLF_NOTE,		//1
	RLF_FORWARDED,	//2
	RLF_FEEDBACK,	//3
	RLF_DECLINED,	//4
	RLF_ACCEPTED,	//5
	RLF_STARTED,	//6
	RLF_COMPLETED,	//7
	RLF_REOPENED,	//8
	RLF_CLOSED,		//9
	RLF_CANCELED	//10
};
#endif //RIPPLEDEFINES_H
