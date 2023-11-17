//#define _TEST_DEBUG_

//Константы портов
#define PHOTO_SENSOR 3  //фотодатчик на 3 ноге
#define MOV_SEC_1 4
#define MOV_SEC_2 5
#define MOV_SEC_3 6
#define PWM_LED_1 9
#define PWM_LED_2 10
#define PWM_LED_3 11

//настройки
#define TIME_DELAY          2000    //время розжига
#define D1                  0x10    //яркость по секциям
#define D2                  0x10    //яркость по секциям
#define D3                  0x10    //яркость по секциям
#define LIGHT_SENSOR_DELAY  1000     //задержка, защищающая от мерцаний на граничных значениях датчика света

#define D1_SEC_POST         10000
#define D2_SEC_POST         0   //задержка на затухание
#define D3_SEC_POST         0

#define DREAD_D(PIN)        ((PIND & (1 << PIN)) != 0)

bool bMovement1     = false;  //зарегистрировано движение в секторе 1
bool bMovement2     = false;  //зарегистрировано движение в секторе 2
bool bMovement3     = false;  //зарегистрировано движение в секторе 3
bool bPhotoOk       = false;  //обработанный сигнал с датчика света
bool bPhotoLastState= false;
unsigned long uWhenLight = 0;

bool  bLight[3]       = {false,false,false};
bool  bLightFlag[3]   = {false,false,false};
unsigned long  uLightDelay[3] = {0,0,0};

//время начала розжига
unsigned long uStartTime_1;
unsigned long uStartTime_2;
unsigned long uStartTime_3;

unsigned long lastTick;
unsigned long thisTick;

void setup() {
  pinMode(PHOTO_SENSOR, INPUT_PULLUP);
  pinMode(MOV_SEC_1, INPUT);
  pinMode(MOV_SEC_2, INPUT);
  pinMode(MOV_SEC_3, INPUT);
  lastTick = millis();
  delay(20);

  analogWrite(PWM_LED_1, 0);
  analogWrite(PWM_LED_2, 0);
  analogWrite(PWM_LED_3, 0);

  #ifdef _TEST_DEBUG_
  Serial.begin(9600);
  #endif
}

bool b1,b2,b3;

void loop() {
  #ifdef _TEST_DEBUG_
  Serial.print(digitalRead(PHOTO_SENSOR));
  Serial.print("\t");
  Serial.print(digitalRead(MOV_SEC_1));
  Serial.print("\t");
  Serial.print(digitalRead(MOV_SEC_2));
  Serial.print("\t");
  Serial.println(digitalRead(MOV_SEC_3));

  analogWrite(PWM_LED_1, D1);
  analogWrite(PWM_LED_2, D2);
  analogWrite(PWM_LED_3, D3);
  #else
    
  thisTick = millis();
  
  //обработка фотодатчика

  if (DREAD_D(PHOTO_SENSOR)){
    if (!bPhotoLastState){
      uWhenLight = thisTick;
      bPhotoLastState = true;
    }

    bPhotoOk = (thisTick - uWhenLight) >= LIGHT_SENSOR_DELAY;
  } else {
    bPhotoLastState = false;
    bPhotoOk = false;
  }

  if (bPhotoOk){ 
    unsigned long delta = thisTick - lastTick;

    /*отработка задержки*/
    if (DREAD_D(MOV_SEC_1)){
      bLight[0] = true;
      bLightFlag[0] = true;
    } else {
      if (bLightFlag[0]){
        bLightFlag[0] = false;
        uLightDelay[0] = millis();
      } else {
        bLight[0] = millis() - uLightDelay[0] <= D1_SEC_POST;
      }
    }
    
    if (bLight[0]){ //обнаружено движение сектор 1
      if (!bMovement1){
        uStartTime_1 = thisTick;
        bMovement1 = true;    
        b1 = true;
      }
      //через TIME_DELAY миллисекунд выдаём на порт значение D1. Иначе плавно поднимаем значение
      if (thisTick - uStartTime_1 >= TIME_DELAY){
        analogWrite(PWM_LED_1, D1);
      } else {
        analogWrite(PWM_LED_1, constrain(map(thisTick - uStartTime_1, 0, TIME_DELAY, 0, D1), 0, D1));
      }
    } else {
      if (b1){
        uStartTime_1 = TIME_DELAY;
        b1 = false;
      }
      if (uStartTime_1 >= delta){
        analogWrite(PWM_LED_1, constrain(map(uStartTime_1, 0, TIME_DELAY, 0, D1), 0, D1));
        uStartTime_1 -= delta;
      } else {
        analogWrite(PWM_LED_1, 0);
        uStartTime_1 = 0;
        bMovement1 = false;
      }
    }

    /*обработчик цифрового сигнала с датчика движения*/
    if (DREAD_D(MOV_SEC_2)){
      bLight[1] = true;
      bLightFlag[1] = true;
    } else {
      if (bLightFlag[1]){
        bLightFlag[1] = false;
        uLightDelay[1] = millis();
      } else {
        bLight[1] = millis() - uLightDelay[1] <= D2_SEC_POST;
      }
    }
  

    if (bLight[1]){ //обнаружено движение сектор 1
      if (!bMovement2){
        uStartTime_2 = thisTick;
        bMovement2 = true;    
        b2 = true;
      }
      //через TIME_DELAY миллисекунд выдаём на порт значение D2. Иначе плавно поднимаем значение
      if (thisTick - uStartTime_2 >= TIME_DELAY){
        analogWrite(PWM_LED_2, D2);
      } else {
        analogWrite(PWM_LED_2, constrain(map(thisTick - uStartTime_2, 0, TIME_DELAY, 0, D2), 0, D2));
      }
    } else {
      if (b2){
        uStartTime_2 = TIME_DELAY;
        b2 = false;
      }
      if (uStartTime_2 >= delta){
        analogWrite(PWM_LED_2, constrain(map(uStartTime_2, 0, TIME_DELAY, 0, D2), 0, D2));
        uStartTime_2 -= delta;
      } else {
        uStartTime_2 = 0;
        analogWrite(PWM_LED_2, 0);
        bMovement2 = false;
      }
    }

    /*отработка задержки*/
    if (DREAD_D(MOV_SEC_3)){
      bLight[2] = true;
      bLightFlag[2] = true;
    } else {
      if (bLightFlag[2]){
        bLightFlag[2] = false;
        uLightDelay[2] = millis();
      } else {
        bLight[2] = millis() - uLightDelay[2] <= D3_SEC_POST;
      }
    }
      
    if (bLight[2]){ //обнаружено движение сектор 1
      if (!bMovement3){
        uStartTime_3 = thisTick;
        bMovement3 = true;    
        b3 = true;
      }
      //через TIME_DELAY миллисекунд выдаём на порт значение D3. Иначе плавно поднимаем значение
      if (thisTick - uStartTime_3 >= TIME_DELAY){
        analogWrite(PWM_LED_3, D3);
      } else {
        analogWrite(PWM_LED_3, constrain(map(thisTick - uStartTime_3, 0, TIME_DELAY, 0, D3), 0, D3));
      }
    } else {
      if (b3){
        uStartTime_3 = TIME_DELAY;
        b3 = false;
      }
      if (uStartTime_3 >= delta){
        analogWrite(PWM_LED_3, constrain(map(uStartTime_3, 0, TIME_DELAY, 0, D3), 0, D3));
        uStartTime_3 -= delta;
      } else {
        uStartTime_3 = 0;
        analogWrite(PWM_LED_3, 0);
        bMovement3 = false;
      }
    }

    lastTick = thisTick;
  } else {//свет включен, движение регистрировать не надо
    bMovement1 = false;
    bMovement2 = false;
    bMovement3 = false;
    uStartTime_1 = 0;
    uStartTime_2 = 0;
    uStartTime_3 = 0;
    digitalWrite(PWM_LED_1, 0);
    digitalWrite(PWM_LED_2, 0);
    digitalWrite(PWM_LED_3, 0);
  }
  #endif
}
