/**********************************************************
 *  INCLUDES
 *********************************************************/
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ioLib.h>
#include "displayD.h"
#include "serialcallLib.h"

/**********************************************************
 *  Constants
 **********************************************************/
//CICLOS MODO NORMAL
#define NUM_SECONDARY_CYCLES_NM 2
//CICLOS MODO DE FRENADO
#define NUM_SECONDARY_CYCLES_BM 2
//CICLOS MODO DE PARADA
#define NUM_SECONDARY_CYCLES_SM 1
//CICLOS MODO DE EMERGENCIA
#define NUM_SECONDARY_CYCLES_EM
//TIEMPO CICLOS MODO NORMAL, FRENADO Y PARADA
#define SECONDARY_CYCLE_TIME_S 5
#define SECONDARY_CYCLE_TIME_N 5000000000
//TIEMPO CICLOS MODO EMERGENCIA
#define SECONDARY_CYCLE_TIME_S_EM 10
#define SECONDARY_CYCLE_TIME_N_EM 10000000000

#define NS_PER_S  1000000000

/**********************************************************
 *  Global Variables 
 *********************************************************/
float speed = 0.0;	//Variable para almacenar la velocidad que nos proporciona la carretilla
int mixerState = 0;	//Variable para el estado del mezclador, 0 o 1
struct timespec timeMixer;	//Tiempo de mezclado
int sensor = 0;	//Cantidad de luz leida del sensor
int distance = 40000; //Se inicializa la aventura a 40000 metros de distancia del punto de descarga
int mode = 0; //modoNormal = 0, modoFrenado = 1, modoParada = 2
char actualMovementStatus[5];
float meanSpeed = 55.0;

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
    
	if ( mode == 3){
		// request GAS CLR
		strcpy(request,"GAS: CLR\n");
		
		//uncomment to use the simulator
		//simulator(request, answer);
		
		// uncoment to access serial module
		writeSerialMod_9(request);
		readSerialMod_9(answer);
	
		// display GAS
		if (0 == strcmp(answer,"GAS:  OK\n")) displayGas(0);
		
	} else if (speed > meanSpeed) { //Si velocidad mayor de 55 (modo normal) o 2.5 (modo frenado)
		// request GAS CLR
		strcpy(request,"GAS: CLR\n");
		
		//uncomment to use the simulator
		//simulator(request, answer);
		
		// uncoment to access serial module
		writeSerialMod_9(request);
		readSerialMod_9(answer);
	
		// display GAS
		if (0 == strcmp(answer,"GAS:  OK\n")) displayGas(0);
	
	} else if (speed < meanSpeed) { //Si velocidad menor de 55 (modo normal) o 2.5 (modo frenado)
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
    
	if ( mode == 3){
		// request BRK SET
		strcpy(request,"BRK: SET\n");
		
		//uncomment to use the simulator
		//simulator(request, answer);
		
		// uncoment to access serial module
		writeSerialMod_9(request);
		readSerialMod_9(answer);
	
		// display BRK
		if (0 == strcmp(answer,"BRK:  OK\n")) displayBrake(1);
	
	} else if (speed > meanSpeed) { //Si velocidad mayor de 55 (modo normal) o 2.5 (modo frenado)
		// request BRK CLR
		strcpy(request,"BRK: SET\n");
		
		//uncomment to use the simulator
		//simulator(request, answer);
		
		// uncoment to access serial module
		writeSerialMod_9(request);
		readSerialMod_9(answer);
	
		// display BRK
		if (0 == strcmp(answer,"BRK:  OK\n")) displayBrake(1);
	
	} else if (speed < meanSpeed) { //Si velocidad menor de 55 (modo normal) o 2.5 (modo frenado)
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
		
	if (timeDiffMixer.tv_sec > 30) { //Si dicha diferencia mayor de 30s
		
		if (mixerState == 0) { //Si estado actual desactivado
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
		} else if (mixerState == 1) { //Si estado actual activado
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
		
		//clock_gettime(CLOCK_REALTIME, &timeMixer);
	}
	
	return 0;
}

/**********************************************************
 *  Function: readSensor
 *********************************************************/
int readSensorFunction()
{
	
   	//--------------------------------
    //  request light sensor and display it 
    //--------------------------------
    	
    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);
    
    // request light sensor
	strcpy(request,"LIT: REQ\n");
		
	//uncomment to use the simulator
	//simulator(request, answer);
		
	// uncoment to access serial module
	writeSerialMod_9(request);
	readSerialMod_9(answer);
	
	// display sensor
	if (1 == sscanf (answer,"LIT: %d%%\n",&sensor)) {
		if (sensor >= 50) {	//Si porcentaje de luz mayor igual de 50
			displayLightSensor(0);	//isDark: No
		} else if (sensor < 50) {	//Si porcentaje menor de 50
			displayLightSensor(1);	//isDark: Si
		}
		  
	}
	return 0; 
}

/**********************************************************
 *  Function: focusControl
 *********************************************************/
int focusControlFunction()
{
	
   	//--------------------------------
    //  request LAM and display it 
    //--------------------------------
	
		//clear request and answer
		memset(request,'\0',10);
		memset(answer,'\0',10);
		
	 if(mode == 0){ //Si estamos en el modo normal
		if (sensor > 50) { //Si porcentaje de luz mayor igual de la mitad
			// request LAM CLR
			strcpy(request,"LAM: CLR\n");
			
			//uncomment to use the simulator
			//simulator(request, answer);
			
			// uncoment to access serial module
			writeSerialMod_9(request);
			readSerialMod_9(answer);
		
			// display LAM
			if (0 == strcmp(answer,"LAM:  OK\n")) displayLamps(0);
		
		} else if (sensor < 50) { //Si porcentaje de luz menor de la mitad
			// request LAM SET
			strcpy(request,"LAM: SET\n");
			
			//uncomment to use the simulator
			//simulator(request, answer);
			
			// uncoment to access serial module
			writeSerialMod_9(request);
			readSerialMod_9(answer);
		
			// display LAM
			if (0 == strcmp(answer,"LAM:  OK\n")) displayLamps(1);
		
		}
	}else{ //Si estamos en cualquier otro modo que no sea el normal
		// request LAM SET
		strcpy(request,"LAM: SET\n");
		
		//uncomment to use the simulator
		//simulator(request, answer);
		
		// uncoment to access serial module
		writeSerialMod_9(request);
		readSerialMod_9(answer);

		// display LAM
		if (0 == strcmp(answer,"LAM:  OK\n")) displayLamps(1);
	}
	return 0;
}

/**********************************************************
 *  Function: readDistance
 *********************************************************/
int readDistanceFunction()
{
	
   	//--------------------------------
    //  request distance and display it 
    //--------------------------------
    	
    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);
    
    // request distance
	strcpy(request,"DS:  REQ\n");
		
	//uncomment to use the simulator
	//simulator(request, answer);
		
	// uncoment to access serial module
	writeSerialMod_9(request);
	readSerialMod_9(answer);
	
	// display distance
	if (1 == sscanf (answer,"DS:%lu\n",&distance)) {
		displayDistance(distance);  
	}
	
	if (distance < 0 || distance > 11000){ // Si la distancia es negativa o mayor que 11000
		mode = 0; // Modo normal
		displayStop(0); // loading: 0
	}else if (distance == 0){ // Si la distancia es igual a 0
		mode = 2; // Modo parada
		displayStop(1); //loading: 1
	}else if (distance <= 11000 && distance > 0){ // Si la distancia es menor o igual que 11000 y mayor que 0
		mode = 1; // Modo frenado
		displayStop(0); //loading: 0
	}	
		
	return 0;
}

/**********************************************************
 *  Function: readDischargeEnd
 *********************************************************/
int readDischargeEndFunction()
{
	
   	//--------------------------------
    //  request movement status and display it 
    //--------------------------------
    	
    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);
    
    // request movement status end
	strcpy(request,"STP: REQ\n");
		
	//uncomment to use the simulator
	//simulator(request, answer);
		
	// uncoment to access serial module
	writeSerialMod_9(request);
	readSerialMod_9(answer);
	
	// display movement status
	if (1 == sscanf (answer,"STP:%s\n",&actualMovementStatus)) {
		if (strcmp(actualMovementStatus, "STOP") == 0) {
			if (mode != 2) {
				mode = 2;
				displayStop(1);
			}
		}else if ( mode != 0){
			mode = 0;
			displayStop(0);
			distance = 40000;	//Reiniciamos la distancia a 40000 para forzar modo normal
		}	
	}
	return 0;
}

/**********************************************************
 *  Function: emergencyModeActivation
 *********************************************************/
void emergencyModeActivationFunction()
{
	
   	//--------------------------------
    //  request movement status and display it 
    //--------------------------------
    	
    //clear request and answer
    memset(request,'\0',10);
    memset(answer,'\0',10);
    
    // request error to activate emergency mode
	strcpy(request,"ERR: SET\n");
		
	//uncomment to use the simulator
	//simulator(request, answer);
		
	// uncoment to access serial module
	writeSerialMod_9(request);
	readSerialMod_9(answer);
	
	if (0 == strcmp(answer,"LAM:  OK\n")) displayLamps(1);
	
}


/**********************************************************
 *  Function: normalMode
 *********************************************************/
void normalMode(){
	printf("\nENTRO\n");
	int secCycle = 0;
	meanSpeed = 55.0;
	struct timespec timeInit, timeEnd, timeDiff, timePeriod;
	timePeriod.tv_sec = SECONDARY_CYCLE_TIME_S;
	timePeriod.tv_nsec = (long)SECONDARY_CYCLE_TIME_N;
	clock_gettime (CLOCK_REALTIME, &timeInit);
	clock_gettime(CLOCK_REALTIME, &timeMixer);
	
    // Endless loop
    while(mode == 0) {
		
		switch(secCycle) {
			case 0:
				readSlopeFunction();
				readSpeedFunction();
				mixerControlFunction();
				readSensorFunction();
				focusControlFunction();
				break;
				
			case 1:
				acceleratorControlFunction();
				brakeControlFunction();
				readSensorFunction();
				focusControlFunction();
				readDistanceFunction();
				break;
		}
		
		secCycle = (secCycle + 1) % NUM_SECONDARY_CYCLES_NM;
		clock_gettime (CLOCK_REALTIME, &timeEnd);
		diffTime(timeEnd, timeInit, &timeDiff);
		diffTime(timePeriod, timeDiff, &timeDiff);
		
		if (compTime(timePeriod, timeDiff) > 0) {  //Si nos hemos pasado no realizamos sleep
			nanosleep (&timeDiff, NULL);
		}else mode = 3;
		
		addTime(timeInit, timePeriod, &timeInit);
		
    }

} 

/**********************************************************
 *  Function: brakeMode
 *********************************************************/
void brakeMode(){
	int secCycle = 0;
	meanSpeed = 2.5;
	struct timespec timeInit, timeEnd, timeDiff, timePeriod;
	timePeriod.tv_sec = SECONDARY_CYCLE_TIME_S;
	timePeriod.tv_nsec = (long)SECONDARY_CYCLE_TIME_N;
	clock_gettime (CLOCK_REALTIME, &timeInit);
	clock_gettime(CLOCK_REALTIME, &timeMixer);
	
    // Endless loop
    while(mode == 1) {
		
		switch(secCycle) {
			case 0: 
				readSlopeFunction();
				readSpeedFunction();
				mixerControlFunction();
				acceleratorControlFunction();
				brakeControlFunction();
				break;
				
			case 1:
				readSpeedFunction();
				acceleratorControlFunction();
				brakeControlFunction();
				focusControlFunction();
				readDistanceFunction();
				break;
		}
		
		secCycle = (secCycle + 1) % NUM_SECONDARY_CYCLES_BM;
		clock_gettime (CLOCK_REALTIME, &timeEnd);
		diffTime(timeEnd, timeInit, &timeDiff);
		diffTime(timePeriod, timeDiff, &timeDiff);
		
		if (compTime(timePeriod, timeDiff) > 0) { //Si nos hemos pasado no realizamos sleep
			nanosleep (&timeDiff, NULL);
		}else mode = 3;
		
		addTime(timeInit, timePeriod, &timeInit);
		
    }

} 

/**********************************************************
 *  Function: stopMode
 *********************************************************/
void stopMode(){
	int secCycle = 0;
	struct timespec timeInit, timeEnd, timeDiff, timePeriod;
	timePeriod.tv_sec = SECONDARY_CYCLE_TIME_S;
	timePeriod.tv_nsec = (long)SECONDARY_CYCLE_TIME_N;
	clock_gettime (CLOCK_REALTIME, &timeInit);
	clock_gettime(CLOCK_REALTIME, &timeMixer);
	
    // Endless loop
    while(mode == 2) {
		
		switch(secCycle) {
			case 0: 
				readDischargeEndFunction();
				mixerControlFunction();
				focusControlFunction();
				break;
		}
		secCycle = (secCycle + 1) % NUM_SECONDARY_CYCLES_SM;
		clock_gettime (CLOCK_REALTIME, &timeEnd);
		diffTime(timeEnd, timeInit, &timeDiff);
		diffTime(timePeriod, timeDiff, &timeDiff);
		
		if (compTime(timePeriod, timeDiff) > 0) { //Si nos hemos pasado no realizamos sleep
			nanosleep (&timeDiff, NULL);
		}else mode = 3;
		
		addTime(timeInit, timePeriod, &timeInit);
		
    }
    //distance = 40000;
	mode = 0;

} 
 
 /**********************************************************
 *  Function: emergencyMode
 *********************************************************/
void emergencyMode(){
	int secCycle = 0;
	struct timespec timeInit, timeEnd, timeDiff, timePeriod;
	timePeriod.tv_sec = SECONDARY_CYCLE_TIME_S;
	timePeriod.tv_nsec = (long)SECONDARY_CYCLE_TIME_N;
	clock_gettime (CLOCK_REALTIME, &timeInit);
	clock_gettime(CLOCK_REALTIME, &timeMixer);
	
    // Endless loop
    while(mode == 3) /*&& !stopEnd)*/ {
		
		switch(secCycle) {
			case 0: 
				readSlopeFunction();
				readSpeedFunction();
				acceleratorControlFunction();
				brakeControlFunction();
				mixerControlFunction();
				focusControlFunction();
				emergencyModeActivationFunction();
				break;
		}
		secCycle = (secCycle + 1) % NUM_SECONDARY_CYCLES_SM;
		clock_gettime (CLOCK_REALTIME, &timeEnd);
		diffTime(timeEnd, timeInit, &timeDiff);
		diffTime(timePeriod, timeDiff, &timeDiff);
		
		if (compTime(timePeriod, timeDiff) > 0) {
			nanosleep (&timeDiff, NULL);
		}
		
		addTime(timeInit, timePeriod, &timeInit);
		
    }

} 
 
/**********************************************************
 *  Function: controller
 *********************************************************/
void *controller(void *arg)
{    
	while(1){
		
		if (mode != 3){
		//printf("\n~~~~~~~~~~~~~~~~~~~~~ MODO NORMAL ~~~~~~~~~~~~~~~~~~~~~\n");
		mode = 0;
		normalMode();
		
		//printf("\n~~~~~~~~~~~~~~~~~~~~~ MODO FRENADO ~~~~~~~~~~~~~~~~~~~~~\n");
		mode = 1;
		brakeMode();
		
		//printf("\n~~~~~~~~~~~~~~~~~~~~~ MODO PARADA ~~~~~~~~~~~~~~~~~~~~~\n");
		mode = 2;
		stopMode();
		} else{
		//printf("\n~~~~~~~~~~~~~~~~~~~~~ MODO EMERGENCIA ~~~~~~~~~~~~~~~~~~~~~\n");
		mode = 3;
		emergencyMode();
		}
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
    
