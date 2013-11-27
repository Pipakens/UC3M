#define SAMPLE_TIME 250 
int soundPin = 11;
unsigned long timeOrig;


void play_bit() 
{
  static int bitwise = 1;
  static unsigned char data = 0;
    
    bitwise = (bitwise * 2);
    if (bitwise > 128) {
        bitwise = 1;
        if (Serial.available()>1) {
           data = Serial.read();
        }
    }
    digitalWrite(soundPin, (data & bitwise) );
}

void setup ()
{
    pinMode(soundPin, OUTPUT);
    Serial.begin(115200);
    timeOrig = micros();    
}

void loop ()
{
    unsigned long timeDiff;

    play_bit();
    timeDiff = SAMPLE_TIME - (micros() - timeOrig);
    timeOrig = timeOrig + SAMPLE_TIME;
    delayMicroseconds(timeDiff);
}
