// --------------------------------------
// Global Variables
// --------------------------------------
#include <string.h>
#include <stdio.h>

// --------------------------------------
// Pin Configuration
// --------------------------------------
#define GAS_LED  13		//Led verde
#define BRK_LED  12		//Led rojo
#define MIX_LED  11		//Led amarillo
#define SLP_UP_SWITCH  9	//Switch
#define SLP_DOWN_SWITCH  8	//Switch
#define SPD_LED  10		//Led azul
#define SENSOR_LDR  0           //Resistencia fotosensible LDR
#define FOCUS_LED  7            //Led blanco
#define DIST_POT  1             //Potenciometro
#define LED_DISPLAY_A  2        //Parte A display
#define LED_DISPLAY_B  3        //Parte B display
#define LED_DISPLAY_C  4        //Parte C display
#define LED_DISPLAY_D  5        //Parte D display
#define VAL_BTN  6              //Boton

// --------------------------------------
// Global Variables
// --------------------------------------
float speed = 55.0;
boolean accelerator = false;    //true==activado false==desactivado
boolean brake = false;          //true==activado false==desactivado
boolean mixer = false;          //true==activado false==desactivado
int slope = 0;                  //-1==subida 0==llano 1==bajada
boolean focus = false;          //true==activado false==desactivado
int sensor = 0;                 //valores de 0 a 1023

int currentTime;    //Variable que guarda el tiempo actual para calcular velocidad
int previousTime;   //Variable que guarda el tiempo previo para calcular velocidad
int diffTime;       //Variable que guarda diferencia de tiempo
int brightness;     //Varaible que guarda el nivel de brillo del led blanco

int mode = 0;  //Seleccion distancia = 0, acercamiento = 1, parada = 2
float distance;  //Variable que guarda la distancia al deposito a seleccionar
int distanceDisplay;  //Variable que guarda la distancia a seleccionar mostrada por el display
float validateDistance;  //Variable que guarda la distancia validada
int actualDistanceDisplay;  //Distancia al deposito mostrada en display
long pressTime;  //Variable que guarda el tiempo en el que se pulsa el boton

int numSecCycles = 2;      //numero de ciclos secundarios, igual para todos los planificadores
int timeSecCycle = 100;    //duracion ciclo secundario en milisegundos, igual para todos los planificadores

// --------------------------------------
// Function: comm_server
// --------------------------------------
int comm_server()
{
    int i;
    char request[10];
    char answer[10];
    int speed_int;
    int speed_dec;
    int timeIni = micros();
    
    // while there is enough data for a request
    if (Serial.available() >= 9) {
        // read the request
        i=0; 
        while ( (i<9) && (Serial.available() >= (9-i)) ) {
            // read one character
            request[i]=Serial.read();
           
            // if the new line is positioned wrong 
            if ( (i!=8) && (request[i]=='\n') ) {
                // Send error and start all over again
                sprintf(answer,"MSG: ERR\n");
                Serial.write(answer);
                memset(request,'\0', 9);
                i=0;
            } else {
              // read the next character
              i++;
            }
        }
        request[9]='\0';
        
        // cast the request
        if (0 == strcmp("SPD: REQ\n",request)) {
            // send the answer for speed request
            speed_int = (int)speed;
            speed_dec = ((int)(speed*10)) % 10;
            sprintf(answer,"SPD:%02d.%d\n",speed_int,speed_dec);
            Serial.write(answer);
            
        } else if (0 == strcmp("SLP: REQ\n",request)) {
             // send the answer for slope request
            switch (slope) {
               case -1: sprintf(answer,"SLP:  UP\n"); break;
               case 0: sprintf(answer,"SLP:FLAT\n"); break;
               case 1: sprintf(answer,"SLP:DOWN\n"); break;
            }
            Serial.write(answer);
            
        } else if (0 == strcmp("LIT: REQ\n",request)) {
             // send the answer for sensor request
            sprintf(answer,"LIT: %02d%%\n",sensor);
            Serial.write(answer);
               
        } else if ((0 == strcmp("DS:  REQ\n",request)) && mode == 1) {
             // send the answer for distance request
            sprintf(answer,"DS:%05ld\n",(long)validateDistance);
            Serial.write(answer);
               
        } else if (0 == strcmp("STP: REQ\n",request)) {
             // send the answer for movement request
            if (mode == 0 || mode == 1) {
              sprintf(answer,"STP:  GO\n");
            } else if (mode == 2) {
              sprintf(answer,"STP:STOP\n");
            }
            Serial.write(answer);
               
        } else if (0 == strcmp("GAS: SET\n",request)) {
             // send the answer for activate/enable gas request
            accelerator = true;    //activamos acelerador
            sprintf(answer,"GAS:  OK\n");
            Serial.write(answer);
               
        } else if (0 == strcmp("GAS: CLR\n",request)) {
             // send the answer for deactivate/disable gas request
            accelerator = false;  //desactivamos acelerador
            sprintf(answer,"GAS:  OK\n");
            Serial.write(answer);
            
        } else if (0 == strcmp("BRK: SET\n",request)) {
             // send the answer for activate/enable brake request
            brake = true;  //activamos freno
            sprintf(answer,"BRK:  OK\n");
            Serial.write(answer);
            
        } else if (0 == strcmp("BRK: CLR\n",request)) {
             // send the answer for deactivate/disable brake request
            brake = false;  //desactivamos freno
            sprintf(answer,"BRK:  OK\n");
            Serial.write(answer);
            
        } else if (0 == strcmp("MIX: SET\n",request)) {
             // send the answer for activate/enable mixer request
            mixer = true;  //activamos mezclador
            sprintf(answer,"MIX:  OK\n");
            Serial.write(answer);
            
        } else if (0 == strcmp("MIX: CLR\n",request)) {
             // send the answer for deactivate/disable mixer request
            mixer = false;  //desactivamos mezclador
            sprintf(answer,"MIX:  OK\n");
            Serial.write(answer);
            
        } else if (0 == strcmp("LAM: SET\n",request)) {
             // send the answer for activate/enable focus request
            focus = true;  //activamos focos
            sprintf(answer,"LAM:  OK\n");
            Serial.write(answer);
            
        } else if (0 == strcmp("LAM: CLR\n",request)) {
             // send the answer for deactivate/disable focus request
            focus = false;  //desactivamos focos
            sprintf(answer,"LAM:  OK\n");
            Serial.write(answer);
            
        } else {
            // error, send error message
            sprintf(answer,"MSG: ERR\n");
            Serial.write(answer);
        }
        
        int timeFin = micros();
        int time = timeFin - timeIni;
        sprintf(answer, "%d\n", time);
        //Serial.print(answer);
    }
    return 0;
}    

// --------------------------------------
// Function: setup
// --------------------------------------
void setup() {  
    Serial.begin(9600);
    pinMode(GAS_LED, OUTPUT);
    pinMode(BRK_LED, OUTPUT);
    pinMode(MIX_LED, OUTPUT);
    pinMode(SLP_UP_SWITCH, INPUT);
    pinMode(SLP_DOWN_SWITCH, INPUT);
    pinMode(SPD_LED, OUTPUT);
    //pinMode(SENSOR_LDR, INPUT);
    pinMode(FOCUS_LED, OUTPUT);
    //pinMode(DIST_POT, INPUT);
    pinMode(LED_DISPLAY_A, OUTPUT);
    pinMode(LED_DISPLAY_B, OUTPUT);
    pinMode(LED_DISPLAY_C, OUTPUT);
    pinMode(LED_DISPLAY_D, OUTPUT);
    pinMode(VAL_BTN, INPUT);
}

// --------------------------------------
// Function: loop
// --------------------------------------
void loop() {
    //comm_server();
    if (mode == 0) {
     distanceModeScheduler(); 
    } else if (mode == 1) {
     approachModeScheduler(); 
    } else if (mode == 2) {
     stopModeScheduler(); 
    }
}

// --------------------------------------
// Function: accelerate
// --------------------------------------
void accelerateFunction() {
    char answer[10];
    int timeIni = micros();
    
    if(!accelerator) {  //Si el acelerador esta desactivado
      digitalWrite(GAS_LED, LOW); //Led apagado
    } else if (accelerator) {  //Si el acelerador esta activado
       digitalWrite(GAS_LED, HIGH); //Led encendido
    }
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: brake
// --------------------------------------
void brakeFunction() {
    char answer[10];
    int timeIni = micros();
  
    if(!brake) {  //Si el freno esta desactivado
      digitalWrite(BRK_LED, LOW); //Led apagado
    } else if (brake) { //Si el freno esta activado
       digitalWrite(BRK_LED, HIGH); //Led encendido
    }
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: mixer
// --------------------------------------
void mixFunction() {
    char answer[10];
    int timeIni = micros();
  
    if(!mixer) {  //Si el mezclador esta desactivado
      digitalWrite(MIX_LED, LOW); //Led apagado
    } else if (mixer) { //Si el mezclador esta activado
       digitalWrite(MIX_LED, HIGH); //Led encendido
    }
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: readSlope
// --------------------------------------
void readSlopeFunction() {
    char answer[10];
    int timeIni = micros();
  
    int readSlope = digitalRead(SLP_UP_SWITCH);   //Vemos si el switch esta en la posicion de subida
                                                  //la variable readSlope tendra 1 si lo esta y 0 en caso contrario
    if (readSlope == 1) {                         //Si esta en dicha posicion
      slope = -1;                                 //Pendiente de subida
    } else {                                      //Si no esta en posicion de subida vemos si esta en posicion de bajada
        readSlope = digitalRead(SLP_DOWN_SWITCH);
        if (readSlope == 1) {                     //Si esta en posicion de bajada
          slope = 1;                              //Pendiente de bajada
        } else {                                  //Si no esta ni subida ni bajada
           slope = 0;                             //Esta llano
        } 
    }
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);  
}

// --------------------------------------
// Function: actualSpeed
// --------------------------------------
void actualSpeedFunction() {
    char answer[10];
    int timeIni = micros();
  
    if (mode == 0 || mode == 1) {  //Si no estamos en modo parada, velocidad normal
        currentTime = millis();                  //Obtenemos tiempo actual
        diffTime = currentTime - previousTime;   //Calculamos el tiempo que ha pasado desde la ultima accion
        previousTime = currentTime;              //Hacemos que el previo sea el actual para la siguiente pasada
  
        if (accelerator == true) {
            speed += ((0.5 / 1000) * diffTime);  //Calculamos la nueva velocidad V = V0 + a * t
        }
    
        if (brake == true) {
            speed += ((-0.5 / 1000) * diffTime);  //Calculamos la nueva velocidad
        }
    
        speed += (((0.25 * slope) / 1000) * diffTime);  //Calculamos la nueva velocidad
    
        if (speed < 0) {
            speed = 0;      //Velocidad no negativa
            brake = false;  //Dejamos de frenar 
        } 
    } else if (mode == 2) {  //Si modo parada, velocidad 0
         speed = 0; 
    }
    
    //Calculamos el brillo del led blanco
    if (speed <= 40) {
        brightness = 0;  //Si la velocidad es menor igual que 40 brilo minimo
    } else if (speed >= 70) {
         brightness = 255;  //Si la velocidad es mayor igual que 70 brilo maximo
    } else {
         brightness = map((int)speed, 40, 70, 0, 255);  //Mapeo para valores de velocidad intermedios
    }
    analogWrite(SPD_LED, brightness);
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: readSensor
// --------------------------------------
void readSensorFunction() {
    char answer[10];
    int timeIni = micros();
    
    int readValue = analogRead(SENSOR_LDR);  //Guardamos la intensidad leida por el LDR
    
    sensor = map(readValue, 0, 1023, 0, 99);  //Mapeo para calcular el porcentaje de 0 a 99 segun valor leido anteriormente
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: activateFocus
// --------------------------------------
void activateFocusFunction() {
    char answer[10];
    int timeIni = micros();
  
    if(!focus) {  //Si los focos estan desactivados
      digitalWrite(FOCUS_LED, LOW); //Led apagado
    } else if (focus) { //Si los focos estan activados
       digitalWrite(FOCUS_LED, HIGH); //Led encendido
    }
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: distanceSelector
// --------------------------------------
void distanceSelectorFunction() {
    char answer[10];
    int timeIni = micros();
    
    //Con estas líneas comentadas vemos que valores, mínimo y máximo, lee el potenciometro para el map
    /*char answer[10];
    sprintf(answer, "%d\n", analogRead(DIST_POT));
    Serial.print(answer);*/
    distance = map(analogRead(DIST_POT), 531, 1023, 10000, 90000); //Mapeo para valores de la distancia en el led, de 10000 a 90000
    distanceDisplay = distance / 10000;
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: numberDisplay
// --------------------------------------
void numberDisplayFunction(int value) {
     switch (value) {
         case 0:  //0000
             digitalWrite(LED_DISPLAY_A, LOW);
             digitalWrite(LED_DISPLAY_B, LOW);
             digitalWrite(LED_DISPLAY_C, LOW);
             digitalWrite(LED_DISPLAY_D, LOW);
             break;
         case 1:  //0001
             digitalWrite(LED_DISPLAY_A, HIGH);
             digitalWrite(LED_DISPLAY_B, LOW);
             digitalWrite(LED_DISPLAY_C, LOW);
             digitalWrite(LED_DISPLAY_D, LOW);
             break;
         case 2:  //0010
             digitalWrite(LED_DISPLAY_A, LOW);
             digitalWrite(LED_DISPLAY_B, HIGH);
             digitalWrite(LED_DISPLAY_C, LOW);
             digitalWrite(LED_DISPLAY_D, LOW);
             break;
         case 3:  //0011
             digitalWrite(LED_DISPLAY_A, HIGH);
             digitalWrite(LED_DISPLAY_B, HIGH);
             digitalWrite(LED_DISPLAY_C, LOW);
             digitalWrite(LED_DISPLAY_D, LOW);
             break;
         case 4:  //0100
             digitalWrite(LED_DISPLAY_A, LOW);
             digitalWrite(LED_DISPLAY_B, LOW);
             digitalWrite(LED_DISPLAY_C, HIGH);
             digitalWrite(LED_DISPLAY_D, LOW);
             break;
         case 5:  //0101
             digitalWrite(LED_DISPLAY_A, HIGH);
             digitalWrite(LED_DISPLAY_B, LOW);
             digitalWrite(LED_DISPLAY_C, HIGH);
             digitalWrite(LED_DISPLAY_D, LOW);
             break;
         case 6:  //0110
             digitalWrite(LED_DISPLAY_A, LOW);
             digitalWrite(LED_DISPLAY_B, HIGH);
             digitalWrite(LED_DISPLAY_C, HIGH);
             digitalWrite(LED_DISPLAY_D, LOW);
             break;
         case 7:  //0111
             digitalWrite(LED_DISPLAY_A, HIGH);
             digitalWrite(LED_DISPLAY_B, HIGH);
             digitalWrite(LED_DISPLAY_C, HIGH);
             digitalWrite(LED_DISPLAY_D, LOW);
             break;
         case 8:  //1000
             digitalWrite(LED_DISPLAY_A, LOW);
             digitalWrite(LED_DISPLAY_B, LOW);
             digitalWrite(LED_DISPLAY_C, LOW);
             digitalWrite(LED_DISPLAY_D, HIGH);
             break;
         case 9:  //1001
             digitalWrite(LED_DISPLAY_A, HIGH);
             digitalWrite(LED_DISPLAY_B, LOW);
             digitalWrite(LED_DISPLAY_C, LOW);
             digitalWrite(LED_DISPLAY_D, HIGH);
             break;    
     }
}

// --------------------------------------
// Function: distanceDisplay
// --------------------------------------
void distanceDisplayFunction() {
    char answer[10];
    int timeIni = micros();  
  
    numberDisplayFunction(distanceDisplay);  //Metodo que muestra la distancia por el led
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: distanceValidator
// --------------------------------------
void distanceValidatorFunction() {
    char answer[10];
    int timeIni = micros();
  
    int pulsation = digitalRead(VAL_BTN);  //Estado boton
    
    if (pulsation == HIGH) {  //Si se pulsa el boton
      //Serial.print(distance);
      pressTime = millis();  //Guardamos el tiempo en el que se pulsa el boton para calcular despues la distancia
      validateDistance = distance;
      mode = 1;  //Cambiamos al modo de acercamiento  
    }
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: actualDistanceDisplay
// --------------------------------------
void actualDistanceDisplayFunction() {
    char answer[10];
    int timeIni = micros();
  
    long timeDist = millis();  //Guardamos tiempo actual
    timeDist -= pressTime;  //Tiempo desde ultima distancia medida
    pressTime = millis();  //Nuevo tiempo
    
    //Serial.print(timeDist);
    validateDistance = validateDistance - ((speed / 1000.0) * (float)(timeDist));  //Tiempo en s para distancia en m
    actualDistanceDisplay = validateDistance / 10000.0;  //Distancia actual en formato display
    
    numberDisplayFunction(actualDistanceDisplay);  //Metodo que muestra la distancia por el led
    
    if (validateDistance <= 0.0) {  //Fin modo acercamiento
       if (speed < 10.0) {
         mode = 2;  //Si la velocidad es menor de 10m/s entramos en modo parada
       } else if (speed >= 10.0) {
         mode = 0;  //Si la velocidad es mayor igual a 10m/s modo seleccion distancia
       } 
    }
    
    int timeFin = micros();
    int time = timeFin - timeIni;
    sprintf(answer, "%d\n", time);
    //Serial.print(answer);
}

// --------------------------------------
// Function: readEndStop
// --------------------------------------
void readEndStopFunction() {
   char answer[10];
   int timeIni = micros();
  
   int pulsation = digitalRead(VAL_BTN);  //Estado boton
   
   if (pulsation == HIGH) {  //Si se pulsa el boton
      mode = 0;  //Volvemos al modo seleccion de distancia
   }
   
   int timeFin = micros();
   int time = timeFin - timeIni;
   sprintf(answer, "%d\n", time);
   //Serial.print(answer);
}

// --------------------------------------
// Function: distanceModeScheduler
// --------------------------------------
void distanceModeScheduler() {
    int timeStart, timeEnd, time;  //Variables para controlar el tiempo
    int wait;  //Variable para saber cuanto esperar
    int secCycle = 0;  //Ciclo secundario en el que se está, 0 o 1
    
    timeStart = millis();  //Obtenemos el tiempo en milisegundos
    
    while(mode == 0) {  //Mientras modo seleccion distancia
       
       switch (secCycle) {
        
        case 0:
          accelerateFunction();
          brakeFunction();
          mixFunction();
          readSlopeFunction();
          actualSpeedFunction();
          readSensorFunction();
          activateFocusFunction();
          distanceSelectorFunction();
          distanceDisplayFunction();
          distanceValidatorFunction();
          break;
         
        case 1:
          comm_server();
          accelerateFunction();
          brakeFunction();
          mixFunction();
          readSlopeFunction();
          actualSpeedFunction();
          readSensorFunction();
          activateFocusFunction();
          distanceSelectorFunction();
          distanceDisplayFunction();
          distanceValidatorFunction();
          break;
      }
       
       secCycle = (secCycle + 1) % numSecCycles; //Pasamos al siguiente ciclo secundario
       timeEnd = millis();  //Obtenemos el tiempo en milisegundos
       time = timeEnd - timeStart;  //Diferencia entre inicio y fin
       wait = timeSecCycle - time;  //Tiempo de espera sin hacer nada
       delay(wait);  //Espera
       timeStart += timeSecCycle;  //Tiempo de inicio mas duracion ciclo secundario
    }
}

// --------------------------------------
// Function: approachModeScheduler
// --------------------------------------
void approachModeScheduler() {
    int timeStart, timeEnd, time;  //Variables para controlar el tiempo
    int wait;  //Variable para saber cuanto esperar
    int secCycle = 0;  //Ciclo secundario en el que se está, 0 o 1
    
    timeStart = millis();  //Obtenemos el tiempo en milisegundos
    
    while(mode == 1) {  //Mientras modo acercamiento
       
       switch (secCycle) {
        
        case 0:
          accelerateFunction();
          brakeFunction();
          mixFunction();
          readSlopeFunction();
          actualSpeedFunction();
          readSensorFunction();
          activateFocusFunction();
          actualDistanceDisplayFunction();
          break;
         
        case 1:
          comm_server();
          accelerateFunction();
          brakeFunction();
          mixFunction();
          readSlopeFunction();
          actualSpeedFunction();
          readSensorFunction();
          activateFocusFunction();
          actualDistanceDisplayFunction();
          break;
      }
       
       secCycle = (secCycle + 1) % numSecCycles; //Pasamos al siguiente ciclo secundario
       timeEnd = millis();  //Obtenemos el tiempo en milisegundos
       time = timeEnd - timeStart;  //Diferencia entre inicio y fin
       wait = timeSecCycle - time;  //Tiempo de espera sin hacer nada
       delay(wait);  //Espera
       timeStart += timeSecCycle;  //Tiempo de inicio mas duracion ciclo secundario
    }
}

// --------------------------------------
// Function: stopModeScheduler
// --------------------------------------
void stopModeScheduler() {
    int timeStart, timeEnd, time;  //Variables para controlar el tiempo
    int wait;  //Variable para saber cuanto esperar
    int secCycle = 0;  //Ciclo secundario en el que se está, 0 o 1
    
    timeStart = millis();  //Obtenemos el tiempo en milisegundos
    
    while(mode == 2) {  //Mientras modo parada
       
       switch (secCycle) {
        
        case 0:
          accelerateFunction();
          brakeFunction();
          mixFunction();
          readSlopeFunction();
          actualSpeedFunction();
          readSensorFunction();
          activateFocusFunction();
          readEndStopFunction();
          break;
         
        case 1:
          comm_server();
          accelerateFunction();
          brakeFunction();
          mixFunction();
          readSlopeFunction();
          actualSpeedFunction();
          readSensorFunction();
          activateFocusFunction();
          readEndStopFunction();
          break;
      }
       
       secCycle = (secCycle + 1) % numSecCycles; //Pasamos al siguiente ciclo secundario
       timeEnd = millis();  //Obtenemos el tiempo en milisegundos
       time = timeEnd - timeStart;  //Diferencia entre inicio y fin
       wait = timeSecCycle - time;  //Tiempo de espera sin hacer nada
       delay(wait);  //Espera
       timeStart += timeSecCycle;  //Tiempo de inicio mas duracion ciclo secundario
    }
}
