/**********************************************************
 *  INCLUDES
 *********************************************************/
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ioLib.h>
#include "displayA.h"
#include "serialcallLib.h"

/**********************************************************
 *  Constants
 **********************************************************/
#define NUM_SECONDARY_CYCLES 1
#define SECONDARY_CYCLE_TIME_S 10
#define SECONDARY_CYCLE_TIME_N 10000000000
#define NS_PER_S  1000000000
 
/**********************************************************
 *  Global Variables 
 *********************************************************/
float speed = 0.0;	//Variable para almacenar la velocidad que nos proporciona la carretilla
int mixerState = 0;	//Variable para el estado del mezclador, 0 o 1
struct timespec timeMixer;	//Tiempo de mezclado

char request[10];	//Array para el mensaje de petición
char answer[10];	//Array para el mensaje de respuesta

/**********************************************************
 *  Function: diffTime 
 *********************************************************/
void diffTime(struct timespec end, 
			  struct timespec start, 
			  struct timespec *diff) 
{
	if (end.tv_nsec < start.tv_nsec) {
		diff->tv_nsec = NS_PER_S - start.tv_nsec + end.tv_nsec;
		diff->tv_sec = end.tv_sec - (start.tv_sec+1);
	} else {
		diff->tv_nsec = end.tv_nsec - start.tv_nsec;
		diff->tv_sec = end.tv_sec - start.tv_sec;
	}
}

/**********************************************************
 *  Function: addTime 
 *********************************************************/
void addTime(struct timespec end, 
			  struct timespec start, 
			  struct timespec *add) 
{
	unsigned long aux;
	aux = start.tv_nsec + end.tv_nsec;
	add->tv_sec = start.tv_sec + end.tv_sec + 
			      (aux / NS_PER_S);
	add->tv_nsec = aux % NS_PER_S;
}

/**********************************************************
 *  Function: compTime 
 *********************************************************/
int compTime(struct timespec t1, 
			  struct timespec t2)
{
	if (t1.tv_sec == t2.tv_sec) {
		if (t1.tv_nsec == t2.tv_nsec) {
			return (0);
		} else if (t1.tv_nsec > t2.tv_nsec) {
			return (1);
		} else if (t1.tv_nsec < t2.tv_nsec) {
			return (-1);
		}
	} else if (t1.tv_sec > t2.tv_sec) {
		return (1);
	} else if (t1.tv_sec < t2.tv_sec) {
		return (-1);
	} 
	return (0);
}

/**********************************************************
 *  Function: readSpeed
 *********************************************************/
int readSpeedFunction()
{
	
   	//--------------------------------
    //  request speed and display it 
    //--------------------------------
    	
    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);
    
    // request speed
	strcpy(request,"SPD: REQ\n");
		
	//uncomment to use the simulator
	//simulator(request, answer);
		
	// uncoment to access serial module
	writeSerialMod_9(request);
	readSerialMod_9(answer);
	
	// display speed
	if (1 == sscanf (answer,"SPD:%f\n",&speed)) {
		displaySpeed(speed);  
	}
	return 0;
}

/**********************************************************
 *  Function: readSlope
 *********************************************************/
int readSlopeFunction()
{
	
	//--------------------------------
	//  request slope and display it 
	//--------------------------------

	//clear request and answer
	memset(request,'\0',10);
	memset(answer,'\0',10);

	// request slope
	strcpy(request,"SLP: REQ\n");

	//uncomment to use the simulator
	//simulator(request, answer);

	// uncoment to access serial module
	writeSerialMod_9(request);
	readSerialMod_9(answer);  

	// display slope
	if (0 == strcmp(answer,"SLP:DOWN\n")) displaySlope(-1);	
	if (0 == strcmp(answer,"SLP:FLAT\n")) displaySlope(0);	
	if (0 == strcmp(answer,"SLP:  UP\n")) displaySlope(1);

	return 0;
}

/**********************************************************
 *  Function: acceleratorControl
 *********************************************************/
int acceleratorControlFunction()
{
	
   	//--------------------------------
    //  request GAS and display it 
    //--------------------------------
    	
    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);
    
	if (speed > 55.0) {	//Si velocidad mayor de 55
		// request GAS CLR
		strcpy(request,"GAS: CLR\n");
		
		//uncomment to use the simulator
		//simulator(request, answer);
		
		// uncoment to access serial module
		writeSerialMod_9(request);
		readSerialMod_9(answer);
	
		// display GAS
		if (0 == strcmp(answer,"GAS:  OK\n")) displayGas(0);
	
	} else if (speed < 55.0) {	//Si velocidad menor de 55
		// request GAS SET
		strcpy(request,"GAS: SET\n");
		
		//uncomment to use the simulator
		//simulator(request, answer);
		
		// uncoment to access serial module
		writeSerialMod_9(request);
		readSerialMod_9(answer);
	
		// display GAS
		if (0 == strcmp(answer,"GAS:  OK\n")) displayGas(1);
	}
	
	return 0;
}

/**********************************************************
 *  Function: brakeControl
 *********************************************************/
int brakeControlFunction()
{
	
   	//--------------------------------
    //  request BRK and display it 
    //--------------------------------
    	
    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);
    
	if (speed > 55.0) {	//Si velocidad mayor de 55
		// request BRK CLR
		strcpy(request,"BRK: SET\n");
		
		//uncomment to use the simulator
		//simulator(request, answer);
		
		// uncoment to access serial module
		writeSerialMod_9(request);
		readSerialMod_9(answer);
	
		// display BRK
		if (0 == strcmp(answer,"BRK:  OK\n")) displayBrake(1);
	
	} else if (speed < 55.0) {	//Si velocidad menor de 55
		// request BRK CLR
		strcpy(request,"BRK: CLR\n");
		
		//uncomment to use the simulator
		//simulator(request, answer);
		
		// uncoment to access serial module
		writeSerialMod_9(request);
		readSerialMod_9(answer);
	
		// display BRK
		if (0 == strcmp(answer,"BRK:  OK\n")) displayBrake(0);
	}
	
	return 0;
}

/**********************************************************
 *  Function: mixerControl
 *********************************************************/
int mixerControlFunction()
{
	
   	//--------------------------------
    //  request MIX and display it 
    //--------------------------------
    	
    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);
    
	struct timespec timeFinMixer, timeDiffMixer;	//Tiempos para saber cuando activar y desactivar mixer
	clock_gettime(CLOCK_REALTIME, &timeFinMixer);	//Obtenemos el tiempo real
	diffTime(timeFinMixer, timeMixer, &timeDiffMixer);	//Calculamos el tiempo que se lleva en un mismo estado
		
	if (timeDiffMixer.tv_sec > 30) {	//Si dicha diferencia mayor de 30s
		
		if (mixerState == 0) {	//Si estado actual desactivado
			clock_gettime(CLOCK_REALTIME, &timeMixer);
			
			// request MIX SET
			strcpy(request,"MIX: SET\n");
			
			//uncomment to use the simulator
			//simulator(request, answer);
					
			// uncoment to access serial module
			writeSerialMod_9(request);
			readSerialMod_9(answer);
				
			// display MIX
			if (0 == strcmp(answer,"MIX:  OK\n")) {
				mixerState = 1;
				displayMix(1);
			}
		} else if (mixerState == 1) {	//Si estado actual activado
			clock_gettime(CLOCK_REALTIME, &timeMixer);
			
			// request MIX CLR
			strcpy(request,"MIX: CLR\n");
						
			//uncomment to use the simulator
			//simulator(request, answer);
								
			// uncoment to access serial module
			writeSerialMod_9(request);
			readSerialMod_9(answer);
							
			// display MIX
			if (0 == strcmp(answer,"MIX:  OK\n")) {
				mixerState = 0;
				displayMix(0);
			}
		}
	}
	
	return 0;
}

/**********************************************************
 *  Function: controller
 *********************************************************/
void *controller(void *arg)
{    
	
	int secCycle = 0;
	struct timespec timeInit, timeEnd, timeDiff, timePeriod;
	timePeriod.tv_sec = SECONDARY_CYCLE_TIME_S;
	timePeriod.tv_nsec = (long)SECONDARY_CYCLE_TIME_N;
	clock_gettime (CLOCK_REALTIME, &timeInit);
	clock_gettime(CLOCK_REALTIME, &timeMixer);
	
    // Endless loop
    while(1) {
		
		switch(secCycle) {	//Solo un ciclo secundario, switch no sería necesario pero valdrá para futura implementacion
			case 0: 
				readSlopeFunction();
				readSpeedFunction();
				acceleratorControlFunction();
				brakeControlFunction();
				mixerControlFunction();
				break;
		}
		
		secCycle = (secCycle + 1) % NUM_SECONDARY_CYCLES;
		clock_gettime (CLOCK_REALTIME, &timeEnd);
		diffTime(timeEnd, timeInit, &timeDiff);
		diffTime(timePeriod, timeDiff, &timeDiff);
		
		if (compTime(timePeriod, timeDiff) > 0) {	//Si nos hemos pasado no realizamos sleep
			nanosleep (&timeDiff, NULL);
		}
		
		addTime(timeInit, timePeriod, &timeInit);
		
    }
}

/**********************************************************
 *  Function: main
 *********************************************************/
int main ()
{
    pthread_t thread_ctrl;
	sigset_t alarm_sig;
	int i;

	/* Block all real time signals so they can be used for the timers.
	   Note: this has to be done in main() before any threads are created
	   so they all inherit the same mask. Doing it later is subject to
	   race conditions */
	sigemptyset (&alarm_sig);
	for (i = SIGRTMIN; i <= SIGRTMAX; i++) {
		sigaddset (&alarm_sig, i);
	}
	sigprocmask (SIG_BLOCK, &alarm_sig, NULL);

    // init display
	displayInit(SIGRTMAX);
	
	// initSerialMod_9600 uncomment to work with serial module
	initSerialMod_WIN_9600 ();

    /* Create first thread */
    pthread_create (&thread_ctrl, NULL, controller, NULL);
    pthread_join (thread_ctrl, NULL);
    return (0);
}
