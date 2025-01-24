#include <delays.h>

#define Delay10us(us)		Delay10TCYx(((GetInstructionClock()/1000000)*(us)))
#define DelayMs(ms)												\
	do																\
	{																\
		unsigned int _iTemp = (ms); 								\
		while(_iTemp--)												\
			Delay1KTCYx((GetInstructionClock()+999999)/1000000);	\
	} while(0)