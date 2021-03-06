#include <stdio.h>
#include "wblib.h"
#include "demo.h"
#include "w55fa92_ts_adc.h"
static volatile BOOL TouchPanel_time = FALSE;
static volatile UINT16 u16X, u16Y;
//static volatile UINT32 u32TouchPressure = FALSE;
//static volatile BOOL bIsValidTouchPanel = FALSE;

static void TouchPanel_timer(void)
{
	TouchPanel_time = TRUE;
}

static void TouchPanel_callback(UINT32 u32code)
{

}
static void Pressure_callback(UINT32 u32code)
{
#if 1
	UINT16 u16Z1, u16Z2;
	float Z1, Z2; 
	float Rtouch; 
	UINT32 u32Dec;	
	UINT32 u32Fraction=3;
	
	u16Z1 =  (u32code>>16)&0x7FFF;
	u16Z2 =  u32code & 0x7FFF;
	Z1 = u16Z1;
	Z2 = u16Z2;
	if((u32code&(BIT31 | BIT15)) == (BIT31 | BIT15)){
		sysprintf("(Z1, Z2) = %dx%d\n", u16Z1, u16Z2);
		
		//Resistor between XP-XM for 4 wire
		Rtouch = 671*u16X/4096.0*(Z2/Z1-1); 					
		
		//Resistor between XP-XM for 5 wire
		//Rtouch = 173*u16X/4096.0*(Z2/Z1-1); 
		
		u32Dec=Rtouch;
		sysprintf("Rtouch = %d.", u32Dec);	
		Rtouch = Rtouch-u32Dec;
		while((Rtouch!=0.) && (u32Fraction!=0))
		{		
			Rtouch = Rtouch*10.;	
			u32Dec = Rtouch;	
			sysprintf("%d",u32Dec); 	
			Rtouch = Rtouch - u32Dec;
			u32Fraction = u32Fraction-1;
		}
		sysprintf("\n\n"); 			
	}else
		sysprintf("Pressure is invalid 0x%x\n", u32code);
#endif					
}	
static void Position_callback(UINT32 u32code)	
{	
	u16X =  (u32code>>16)&0x7FFF;
	u16Y =  u32code & 0x7FFF;
#if 1
	if((u32code&(BIT31 | BIT15)) == (BIT31 | BIT15))
		sysprintf("(X, Y) = %dx%d\n", u16X, u16Y);
	else
		sysprintf("Position is invalid 0x%x\n", u32code);	
#endif		
}
INT32 TouchPanel(void)
{
	UINT32 tmp, btime, etime;
	UINT32 u32ExtFreq, u32Item;
	BOOL bIs5Wire;
	PFN_ADC_CALLBACK pfnOldCallback;
	INT32 ret;
	DBG_PRINTF("ADC Touch Panel Demo...\n");	
	DBG_PRINTF("Please input 4 wire(0) or 5 wire (!0)\n");
	//u32Item = sysGetChar();
	u32Item = '0';
	if(u32Item=='0')
		bIs5Wire = FALSE;
	else
		bIs5Wire = TRUE;	
	
	u32ExtFreq = sysGetExternalClock();
	sysSetTimerReferenceClock(TIMER0, u32ExtFreq); 					//External Crystal
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);						/* 100 ticks/per sec ==> 1tick/10ms */
	tmp = sysSetTimerEvent(TIMER0, 2, (PVOID)TouchPanel_timer);		/* 2 ticks  call back */
	DBG_PRINTF("No. of Event [%d]\n", tmp);
	
	DrvADC_Open();
	DrvADC_InstallCallback(eADC_TOUCH,
						TouchPanel_callback,
						&pfnOldCallback);
	
	DrvADC_InstallCallback(eADC_POSITION,
						Position_callback,
						&pfnOldCallback);
						
	DrvADC_InstallCallback(eADC_PRESSURE,
						Pressure_callback,
						&pfnOldCallback);					
						
	btime = sysGetTicks(TIMER0);
	etime = btime;
	while ((etime - btime) <= 300)	
	{
		while(TouchPanel_time==TRUE){
			TouchPanel_time = FALSE;
			do{
				ret = DrvADC_PenDetection(bIs5Wire);
			}while(ret != Successful);
		}
		etime = sysGetTicks(TIMER0);
	}	
	sysClearTimerEvent(TIMER0, tmp);
	return Successful;
}
