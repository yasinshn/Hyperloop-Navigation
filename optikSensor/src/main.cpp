#include <Arduino.h>


struct optikSensor{
  uint16_t totalBantCounter=-1;
  uint16_t bantCounter;
  uint64_t speedTimer;

  uint16_t speed;
 
};

uint32_t m=0;

optikSensor *optik1 = new optikSensor();
optikSensor *optik2 = new optikSensor();

float secSpeed = 0;  // Reflektörler Arası HıZ

float totalDistance = 0;

float speed = 0;  // ÖLçülen Hız
volatile uint16_t speedKm = 0; 

bool readSpeed = false;

hw_timer_t *timer1 = NULL;
hw_timer_t *timer2 = NULL;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR beginCounter2(){
  portENTER_CRITICAL_ISR(&timerMux);
  optik2->speedTimer = timerRead(timer2);
  optik2->bantCounter++;
  
  timerWrite(timer2, 0);
  portEXIT_CRITICAL_ISR(&timerMux);

}

void IRAM_ATTR beginCounter1(){
  portENTER_CRITICAL_ISR(&timerMux);
  optik1->bantCounter++;
  timerWrite(timer1, 0);
  portEXIT_CRITICAL_ISR(&timerMux);
}



float reflectorDistance(uint16_t bantCnt){
  switch(bantCnt){
    // Ilk 6 metre 
    case 1:
    readSpeed = true;
    return 6.00;
    break;

    // 6 - 78 metre
    case 2 ... 19:
    readSpeed = true;
    return 4.00;
    break;

    // 78 - 79 metre
    case 20 ... 38:
    readSpeed = false;
    return 0.1;
    break;

    // 79 - 82 metre
    case 39:
    readSpeed = false;
    return 2.10;
    break;

    // 82 - 130 metre
    case 40 ... 51:
    readSpeed = true;
    return 4.00;
    break;

    // 130 - 130.95 metre
    case 52 ... 60:
    readSpeed = false;
    return 0.1;
    break;

   // 130.95 - 134 metre
   case 61:
   readSpeed = false;
   return 3.10;
   break;
   
   // 134 - 178 metre
   case 62 ... 71:
   readSpeed = true;
   return 4.00;
   break;

   default:
   readSpeed = false;
   return -4.00;
   break;
  }
}

void setup() {
setCpuFrequencyMhz(40);
Serial.begin(9600);

attachInterrupt(digitalPinToInterrupt(5), beginCounter2, HIGH);
attachInterrupt(digitalPinToInterrupt(5), beginCounter2, FALLING);

timer1 = timerBegin(0, 1000, true); // Prescale Oranı : 1000
timer2 = timerBegin(1, 1000, true); // Prescale : 1000

optik1->totalBantCounter = 0;
optik1->bantCounter = 0;
optik1->speed = 0;

optik2->bantCounter = 0;
optik2->totalBantCounter = 0;
optik2->speed = 0;
 
}

void loop() {


if(optik1->bantCounter > 0){
  
  portENTER_CRITICAL_ISR(&timerMux);
  optik1->bantCounter--;
  portEXIT_CRITICAL_ISR(&timerMux);
  
  optik1->totalBantCounter++;

  if(readSpeed){
    secSpeed = 1.00 / (float)((float)(optik1->speedTimer) / 40000);
  }

  printf("Timer 1: %d\n", optik1->speedTimer);
  printf("Timer 2: %d\n",optik2->speedTimer);
  printf("1.Sensör Bant No: %d\n",optik1->totalBantCounter);
  printf("Sensörler Arasi Hiz: %d\n",secSpeed);
}
/*
if(millis()>m+50){
  Serial.print("IO:");
  Serial.println(digitalRead(4));
  m=millis();
}
*/
if(optik2->bantCounter > 0){

portENTER_CRITICAL_ISR(&timerMux);
optik2->bantCounter--;
portEXIT_CRITICAL_ISR(&timerMux);  


optik2->totalBantCounter++; // sensör 2

totalDistance += reflectorDistance(optik2->totalBantCounter);

if(readSpeed){
  speed = reflectorDistance(optik2->totalBantCounter) / (float)((float)optik2->speedTimer / 40000);
  printf("Speed: %.2f\n",speed);
}
printf("2.Sensör Bant No: %d\n", optik2->totalBantCounter);

} 

}