/**
 * @file iv6_clock.ino
 * @author Dizer31 (dizerdef@gmail.com)
 * @date 2022-07-09
 *
 * @copyright Copyright (c) 2022
*/

//-----setting-----//
#define debugMode 1

#define invertSeg 1   
#define invertAnod 1   
//anod 1   sed 1

#define settingPin A1
#define plusPin A2
#define minusPin A3

#define buzzPin 13  

#define enableTimeSetting 1 //включить/отключить настройку времени
#define enableAlarmSetting 1    //включить/отключить настройку будильника
//-----setting-----//


//-----lib & define & init-----//
#if debugMode == 1
#define debug(x) Serial.println(x)
#else 
#define debug(x) 
#endif

#if invertSeg == 1
#define setSegPin(pin, x) digitalWrite(pin, !x)
#else
#define setSegPin(pin, x) digitalWrite(pin, x)
#endif

#if invertAnod == 1
#define setAnodPin(pin, x) digitalWrite(pin, !x)
#else
#define setAnodPin(pin, x) digitalWrite(pin, x)
#endif

//сегменты
#define __A 0
#define __B 1
#define __C 2
#define __D 3
#define __E 4
#define __F 5
#define __G 6

#include "DS3231.h"
DS3231 rtc;
Time t;

#include "buttonLib.h"
Button setBtn(settingPin);
Button plusBtn(plusPin);
Button minusBtn(minusPin);
//-----lib & define & init-----//


//volatile uint8_t segments[] = { 6,7,8,9,10,11,12 }; //пины сегментов (A B C D E F G)
//volatile uint8_t anod[] = { 2,3,4,5 };  //пины разрядов
//-----special variables-----//
volatile uint8_t segments[] = { 2,3,4,5,6,7,8 }; //пины сегментов (A B C D E F G)
volatile uint8_t anod[] = { 9,10,11,12 };  //пины разрядов
volatile uint8_t buf[4];    //буфер дисплея
uint8_t mode = 0;

int16_t alarmTime = -1;
bool alarmFlag = false;
uint32_t modeTmr = 0;
//-----special variables-----//


//-----func-----//
#pragma region
#if enableTimeSetting == 1
void setTime() {
    static int8_t sH = 0, sM = 0;
    static bool sMode = false;

    static uint32_t tmr = 0;
    static bool indFlag = true;
    if (millis() - tmr >= 500) {   //мигать часами/минутами раз в секунду
        tmr = millis();
        indFlag = !indFlag;
    }

    if (plusBtn.isPress() || plusBtn.isHolded()) {
        modeTmr = millis();
        tmr = millis();
        debug("setTime: plusBtn isPress/isHolded");
        if (!sMode) {
            if (++sH >= 24)sH = 0;
        } else {
            if (++sM >= 60)sM = 0;
        }

        rtc.setTime(sH, sM, t.sec);
    }

    if (minusBtn.isPress() || minusBtn.isHolded()) {
        modeTmr = millis();
        tmr = millis();
        debug("setTime: minusBtn isPress/isHolded");
        if (!sMode) {
            if (--sH < 0)sH = 23;
        } else {
            if (--sM < 0)sM = 59;
        }

        rtc.setTime(sH, sM, t.sec);
    }

    if (setBtn.isSingle()) {
        modeTmr = millis();
        tmr = millis();
        sMode = !sMode;   //смена режима часы/минуты
        debug("setTime: change hour/min, setBtn.isSingle");
    }

    if (!sMode) {
        buf[0] = (indFlag ? sH / 10 : 10);
        buf[1] = (indFlag ? sH % 10 : 10);
        buf[2] = sM / 10;
        buf[3] = sM % 10;
    } else {
        buf[0] = sH / 10;
        buf[1] = sH % 10;
        buf[2] = (indFlag ? sM / 10 : 10);
        buf[3] = (indFlag ? sM % 10 : 10);
    }
}
#endif

#if enableAlarmSetting == 1
void alarmFunc() {
    static bool aMode = false;  //режим часы/минуты
    static int8_t ah = 0, am = 0;  //*alarm time

    if (plusBtn.isPress() || plusBtn.isHolded()) {
        if (!aMode) {
            if (++ah >= 24)ah = 0;
        } else {
            if (++am >= 60)am = 0;
        }
        modeTmr = millis();
        debug("alarmFunc: plusBtn isPress/isHolded");
    }

    if (minusBtn.isPress() || minusBtn.isHolded()) {
        if (!aMode) {
            if (--ah < 0)ah = 23;
        } else {
            if (--am < 0)am = 59;
        }
        modeTmr = millis();
        debug("alarmFunc: minusBtn isPress/isHolded");
    }

    if (setBtn.isSingle()) {
        modeTmr = millis();
        aMode = !aMode;   //смена режима часы/минуты
        debug("alarmFunc: change hour/min, setBtn.isSingle");
    }

    static uint32_t tmr = 0;
    static bool indFlag = true;
    if (millis() - tmr >= 500) {   //мигать часами/минутами раз в секунду
        tmr = millis();
        indFlag = !indFlag;
    }

    if (!aMode) {
        buf[0] = (indFlag ? ah / 10 : 10);
        buf[1] = (indFlag ? ah % 10 : 10);
        buf[2] = am / 10;
        buf[3] = am % 10;
    } else {
        buf[0] = ah / 10;
        buf[1] = ah % 10;
        buf[2] = (indFlag ? am / 10 : 10);
        buf[3] = (indFlag ? am % 10 : 10);
    }
    //мигать часами/минутами в зависимости от режима(aMode) и миганием(indFlag)

    alarmTime = ah * 100 + am;
}
#endif

void setAnod(uint8_t num) {
    num = 0x01 << num;
    setAnodPin(anod[0], bitRead(num, 0));
    setAnodPin(anod[1], bitRead(num, 1));
    setAnodPin(anod[2], bitRead(num, 2));
    setAnodPin(anod[3], bitRead(num, 3));
}

void setDigit(uint8_t num) {
    uint8_t arr[] = { 0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111 };
    for (uint8_t i = 0;i < 7;i++)setSegPin(segments[i], bitRead(num != 10 ? arr[num] : 0x00, i));
}

void setSegment(uint8_t x) {//тест сегментов
    x = 0x01 << x;
    for (uint8_t i = 0;i < 7;i++)setSegPin(segments[i], bitRead(x, i));
}

void sptlit(uint16_t num) {
    for (int8_t i = 3;i >= 0;i--) {
        buf[i] = num % 10;
        num /= 10;
    }
}

ISR(TIMER2_COMPA_vect) {
    static uint8_t counter = 0;
    static uint8_t i = 0;
    if (++counter == 20)setAnodPin(anod[i], false);

    if (counter > 21) {
        counter = 0;
        if (++i >= 4)i = 0;

        setDigit(buf[i]);
        setAnodPin(anod[i], true);
    }
}
#pragma endregion
//-----func-----//

void setup() {
#if debugMode == 1
    Serial.begin(9600);
#endif

    TCCR2B = (TCCR2B & B11111000) | 1;  //timer2 (~7kHz)
    bitSet(TCCR2A, WGM21);
    bitSet(TIMSK2, OCIE2A);     //прерывание по таймеру

    rtc.begin();
    pinMode(buzzPin, OUTPUT);
    for (uint8_t i = 0;i < 11;i++)pinMode(i < 7 ? segments[i] : anod[i - 7], OUTPUT);
}

void loop() {
    setBtn.tick();
    plusBtn.tick();
    minusBtn.tick();

    if (millis() - modeTmr >= 10000) {   //возврат к главному экрану
        modeTmr = millis();
        mode = 0;
    }

    static bool alarmFlagLast = true;
    alarmFlag = (t.hour * 100 + t.min == alarmTime && alarmFlagLast);

    static uint8_t lastMin;
    if (lastMin != t.min) {
        lastMin = t.min;
        alarmFlagLast = true;
    }

    static uint32_t alarmTmr = 0;
    if (millis() - alarmTmr >= 500) {
        alarmTmr = millis();
        debug("loop: alarmClock, " + (String)(alarmTime / 100) + ":" + (String)(alarmTime % 100));
        if (alarmFlag)digitalWrite(buzzPin, !digitalRead(buzzPin));
        else digitalWrite(buzzPin, false);
    }

    static uint32_t tmr = 0;
    if (millis() - tmr >= 100) {
        tmr = millis();
        if (mode == 0) {
            debug("loop: time update ==>" + (String)t.hour + ":" + (String)t.min + ":" + (String)t.sec);
            t = rtc.getTime();
            buf[0] = t.hour / 10;
            buf[1] = t.hour % 10;
            buf[2] = t.min / 10;
            buf[3] = t.min % 10;
        }
    }

    switch (mode) {
    case 1:
    #if enableTimeSetting == 1
        setTime();
    #endif
        break;
    case 2:
   #if enableAlarmSetting == 1
        alarmFunc();
    #endif
        break;
    }

    if (setBtn.isHold()) { debug("loop: change mode, setBtn.isHold");if (++mode >= 3)mode = 0; }
    if (setBtn.isSingle())alarmFlagLast = false;
}
