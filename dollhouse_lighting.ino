#include <FastLED.h>

/**
   (C) jarduino.ru

   Эксперимент № 14. ИК пульт дистанционного управления

   Эксперименты с ардуино.
   Управление arduino с помощью ИК пульта ДУ
*/
#include <IRremote.h> // подключаем библиотеку для ИК приемника
//#include <LiquidCrystal.h>

// Указываем, какое количество пикселей у нашей ленты.
#define LED_COUNT 44
#define ROOM_COUNT 9

#define BTN_UP    16736925
#define BTN_DOWN  16754775
#define BTN_LEFT  16720605
#define BTN_RIGHT 16761405
#define BTN_STAR  16728765
#define BTN_GRID  16732845

#define BTN_1     16738455
#define BTN_2     16750695
#define BTN_3     16756815
#define BTN_4     16724175
#define BTN_5     16718055
#define BTN_6     16743045
#define BTN_7     16716015
#define BTN_8     16726215
#define BTN_9     16734885
#define BTN_0     16730805
#define BTN_OK    16712445

#define BTN_1b     67404413
#define BTN_2b     67388093
#define BTN_3b     67420733
#define BTN_4b     67396253
#define BTN_5b     67412573
#define BTN_6b     67379933
#define BTN_7b     67401863
#define BTN_8b     67410023
#define BTN_9b     67377383
#define BTN_0b     67400333

#define BTN_UPb    67405943
#define BTN_DOWNb  67373303
#define BTN_LEFTb  67414103
#define BTN_RIGHTb 67381463



#define BTN_VPLUSb  67424303
#define BTN_VMINUSb 67416143
#define BTN_PPLUSb  67432463
#define BTN_PMINUSb 67383503
#define BTN_ROTATEb 67389623

#define BTN_REDb    67403903
#define BTN_GREENb  67387583
#define BTN_YELLOWb 67420223
#define BTN_BLUEb   67379423

#define BTN_POWERb  67393703

#define BTN_OKb    67422263

#define BTN_MENUb  67412063
#define BTN_EPGb   67395743
#define BTN_INFOb  67428383

#define BTN_RADb   67385543
#define BTN_STBb   67418183
#define BTN_FUNKb   67408493
#define BTN_MUTEb   67399823


#define BTN_REPEAT 4294967295

#define OFF_PAUSE 1000
#define BLINK_PAUSE 50
#define MOVECOLOR_PAUSE 10

// Указываем, к какому порту подключен вход ленты DIN.
#define LED_PIN 3


// Константы программы
const int irPin = A0; // аналоговый порт для ИК приемника
//const int ledPin = 3; // порт для светодиода

// Глобальные переменные
bool ledState = false;
bool flForAll = false;

IRrecv irRec(irPin); // Инициализируем объект ИК приемника
decode_results irData; // переменная для получения данных от ИК приемника

// Создаем переменную strip для управления нашей лентой.
CRGB strip[LED_COUNT];

byte  Hue = 0;
byte Saturation = 0;
byte Value = 255;
long lastButton = 0;
byte curInd = 0;

unsigned long interruptTime = 0;
byte QIntrval = 0;
unsigned long SumCurInterval = 0;
unsigned int QHold = 0;
unsigned long lastButtonTime = 0;      // время последнего события от пульта
unsigned long newBattonTime = 0;      // время начала нажатия кнопки
unsigned long pressNewButtonTerm = 0; // интервал нажатия (удержания) кнопки
byte buttonOn = 0;
byte numBlink = 0;
unsigned long blinkTime = 0;
unsigned long MoveColorTime = 0;

class LedCond /*имя класса*/  // имя класса принято писать с Большой Буквы
{
  public:
    byte LedQuant = 0;
    byte Hue = 0;
    byte Saturation = 0;
    byte Value = 255;
    byte LastValue = 255;
    byte On = 255;
    byte AutoColorMove = 0;

    //static unsigned long UpdateTime;
    //    void LedCond(Quant)
    //    {
    //      LedQuant = Quant;
    //    }

    void Switch()
    {
      if (On == 0)
      {
        On = 255;
        
        if (LastValue == 0)
        {
          LastValue = 255;
        }
        Value = LastValue;
      }
      else
      {
        On = 0;
        LastValue = Value;
        Value = 0;
      }
    }

    void ValueUp()
    {
      if (On == 0)
      {
        On = 255;
        Value = LastValue;
      }

      if (Value < 246)
      {
        Value += 10;
        LastValue = Value;
      }
      else
      {
        Value = 255;
        LastValue = Value;
      }

    }

    void ValueDown()
    {
      if (On == 0) return;

      if (Value > 9)
      {
        Value -= 10;
        LastValue = Value;
      }
      else
      {
        Value = 0;
        On = 0;
        LastValue = Value;
      }
    }

    void MoveColor(byte ColorStep)
    {
      Hue = Hue + ColorStep;
      //Serial.println("Hue = " + String(Hue));
    }


};



LedCond ledConds[ROOM_COUNT];


void SetForAll(int ind)
{
  for (int i = 0; i < ROOM_COUNT; i++)
  {
    if (i == ind)
    {
      continue;
    }

    ledConds[i].Hue = ledConds[ind].Hue;
    ledConds[i].Saturation = ledConds[ind].Saturation;
    ledConds[i].Value = ledConds[ind].Value;
    ledConds[i].LastValue = ledConds[ind].LastValue;
    ledConds[i].On = ledConds[ind].On;
    ledConds[i].AutoColorMove = ledConds[ind].AutoColorMove;
  }
}


void setup()
{
  Serial.begin(9600); // Иницилизация последовательного порта, для вывода отладочной информации

  irRec.enableIRIn(); // запускаем прием инфракрасного сигнала

  //pinMode(ledPin, OUTPUT);

  //FastLED.addLeds(strip, LED_COUNT);
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(strip, LED_COUNT);


  ledConds[0].LedQuant = 6;
  ledConds[1].LedQuant = 6;
  ledConds[2].LedQuant = 6;
  ledConds[3].LedQuant = 6;
  ledConds[4].LedQuant = 3;
  ledConds[5].LedQuant = 3;
  ledConds[6].LedQuant = 6;
  ledConds[7].LedQuant = 6;
  ledConds[8].LedQuant = 2;

  //byte ledInd = 0;
  for (int i = 0; i < ROOM_COUNT; i++)
  {

    ledConds[i].Hue = 100;
    ledConds[i].Saturation = 0;
    ledConds[i].Value = 0;
    ledConds[i].LastValue = 250;
    ledConds[i].On = 0;
  }
  LedToStrip();


  // Передаем цвета ленте.
  FastLED.show();
  //  byte  Hue = 0;
  //  byte Saturation = 0;
  //  byte Value = 255;
  //  unsigned long UpdateTime = 0;


  //LiquidCrystal lcd(7,6,5,4,3,2);
  //lcd.begin(16,2);
  //lcd.print("t");
}


void LedToStrip()
{
  byte ledInd = 0;

  for (int i = 0; i < ROOM_COUNT; i++)
  {
    for (int j = 0; j < ledConds[i].LedQuant ; j++)
    {
      strip[ledInd] = CHSV(ledConds[i].Hue, ledConds[i].Saturation, ledConds[i].Value); // CRGB::Green; // Черный цвет, т.е. выключено.
      ledInd++;
    }
  }
}

void blink(byte curInd)
{
  if ( ledConds[curInd].On != 0)
  {
    ledConds[curInd].Switch();

    numBlink = curInd + 1;

    LedToStrip();
    FastLED.show();

    blinkTime = millis();

  }
}

// Обработка цифровых клавиш
void HendleNumBution(byte curInd, unsigned long lastButton,  unsigned long irDataValue, unsigned long pressTerm)
{
  if (irDataValue == lastButton)  // нажатие кнопки (не повтор при удерживании)
  {
    if ( ledConds[curInd].On == 0)// диод  был выключен
    {
      ledConds[curInd].Switch(); // переключеаем
    }
    else
    {
      blink(curInd);                   // был включен. нужно мигнуть
    }
  }
  else
  {
    if (irDataValue == BTN_REPEAT) // повтор при удержании
    {
      if ((pressTerm > OFF_PAUSE)  && (ledConds[curInd].On != 0)) // Если время удержания больше 500 чего-то там и диод горел, выключаем его
      {
        ledConds[curInd].Switch();
      }
    }
  }
}


void HendleButton(unsigned long lastButton,  unsigned long irDataValue, unsigned long pressTerm)
{

  //if(irDataValue != 0)   // не отжатие
  if (lastButton == BTN_OK || lastButton == BTN_POWERb)
  {
    flForAll = false;
    for (int i = 0; i < ROOM_COUNT; i++)
    {
      //if (ledConds[i].On != 0)
      //{
        ledConds[i].LastValue = 0;//ledConds[i].Value;
        ledConds[i].Value = 0;
        ledConds[i].LastValue = 255;
        ledConds[i].Saturation = 0;
        ledConds[i].On = 0;
        ledConds[i].AutoColorMove = 0;
      //}
    }

    LedToStrip();
    FastLED.show();
  }

  //Hue, Saturation, Value
  if (lastButton == BTN_UP || lastButton == BTN_UPb) //UP  BTN_UP 16736925
  {
    ledConds[curInd].ValueUp();

    if (flForAll==true)
    {
      //SetForAll(curInd);
    }
  }
  if (lastButton == BTN_DOWN || lastButton == BTN_DOWNb)  //down BTN_DOWN 16754775
  {
    ledConds[curInd].ValueDown();
    if (flForAll==true)
    {
      //SetForAll(curInd);
    }
  }

  if (lastButton == BTN_LEFT || lastButton == BTN_LEFTb) // Left BTN_LEFT 16720605
  {
    ledConds[curInd].MoveColor(-1);
    if (flForAll==true)
    {
      //SetForAll(curInd);
    }
  }

  if (lastButton ==  BTN_RIGHT || lastButton ==  BTN_RIGHTb) // right BTN_RIGHT 16761405
  {
    ledConds[curInd].MoveColor(1);
    if (flForAll==true)
    {
      //SetForAll(curInd);
    }
  }

  if (lastButton == BTN_STAR || lastButton == BTN_PPLUSb) // Left  BTN_STAR 16728765
  {
    if (ledConds[curInd].Saturation > 9 ) // Цветность --
    {
      ledConds[curInd].Saturation -= 10;
      if (flForAll==true)
      {
        //SetForAll(curInd);
      }
    }
  }

  if (lastButton == BTN_PMINUSb) // // Цветность ++
  {
    if (ledConds[curInd].Saturation < 246)
    {
      ledConds[curInd].Saturation += 10;
      if (flForAll==true)
      {
        //SetForAll(curInd);
      }
    }
  }

  if (lastButton ==  BTN_GRID || lastButton ==  BTN_VMINUSb) // Цветность максимум
  {
    ledConds[curInd].AutoColorMove = 0;
    ledConds[curInd].Saturation = 255;
    if (flForAll==true)
    {
      //SetForAll(curInd);
    }
  }

  if (lastButton ==  BTN_VPLUSb) // Цветность минимум
  {
    ledConds[curInd].AutoColorMove = 0;
    ledConds[curInd].Saturation = 0;
    if (flForAll==true)
    {
      //SetForAll(curInd);
    }
  }


  if ((lastButton == BTN_REDb) && (irDataValue != BTN_REPEAT))
  {

    if (ledConds[curInd].On != 0)
    {
      ledConds[curInd].AutoColorMove = 0;
      ledConds[curInd].Saturation = 255;
      ledConds[curInd].Hue = HUE_GREEN;//HUE_RED; //CRGB::Red;
      //      Serial.println("Hue = " + String(ledConds[curInd].Hue));
      //      Serial.println("HUE_GREEN = " + String(HUE_GREEN));
      //      Serial.println("HUE_RED = " + String(HUE_RED));
      if (flForAll==true)
      {
        //SetForAll(curInd);
      }
    }
  }

  if ((lastButton == BTN_GREENb) && (irDataValue != BTN_REPEAT))  // Left
  {

    if (ledConds[curInd].On != 0)
    {
      ledConds[curInd].AutoColorMove = 0;
      ledConds[curInd].Saturation = 255;
      ledConds[curInd].Hue = 0;//95; //CRGB::Green;
      //      Serial.println("Hue = " + String(ledConds[curInd].Hue));
      //      Serial.println("HUE_GREEN = " + String(HUE_GREEN));
      //      Serial.println("HUE_RED = " + String(HUE_RED));
      if (flForAll==true)
      {
        //SetForAll(curInd);
      }

    }
  }

  if ((lastButton == BTN_YELLOWb) && (irDataValue != BTN_REPEAT))  // Left
  {

    if (ledConds[curInd].On != 0)
    {
      ledConds[curInd].AutoColorMove = 0;
      ledConds[curInd].Saturation = 255;
      ledConds[curInd].Hue = 74; //HUE_YELLOW; //45; //CRGB::Yellow;
      //Serial.println("HUE_YELLOW = " + String(HUE_YELLOW));
      if (flForAll==true)
      {
        //SetForAll(curInd);
      }
    }
  }

  if ((lastButton == BTN_BLUEb) && (irDataValue != BTN_REPEAT))  // Left
  {

    if (ledConds[curInd].On != 0)
    {
      ledConds[curInd].AutoColorMove = 0;
      ledConds[curInd].Saturation = 255;
      ledConds[curInd].Hue = HUE_BLUE ;//120; //CRGB::Blue;
      if (flForAll==true)
      {
        //SetForAll(curInd);
      }
    }
  }


  //if ((lastButton == BTN_0 || lastButton == BTN_EPGb) && (irDataValue != BTN_REPEAT))  // Left
  if (irDataValue == BTN_0 || irDataValue == BTN_ROTATEb)

  {
    if (ledConds[curInd].AutoColorMove == 0 )
    {
      ledConds[curInd].AutoColorMove = 255;
      if (ledConds[curInd].Saturation <= 5)
      {
        ledConds[curInd].Saturation = 255;
      }
           // Serial.println("curInd = " + String(curInd) + "   ledConds[curInd].AutoColorMove = " + String(ledConds[curInd].AutoColorMove));
    }
    else
    {
      ledConds[curInd].AutoColorMove = 0;
      //Serial.println("curInd = " + String(curInd) + "   ledConds[curInd].AutoColorMove = " + String(ledConds[curInd].AutoColorMove));
    }
    
    if (flForAll==true)
    {
      //SetForAll(curInd);
    }
  }

  if (lastButton == BTN_1 || lastButton == BTN_1b) // соответветствует цифре 1 на пульте
  {
    curInd = 0;
    HendleNumBution(curInd, lastButton, irDataValue, pressTerm);
    //Serial.println("Odin");
  }

  if (lastButton == BTN_2 || lastButton == BTN_2b) // соответветствует цифре 2 на пульте
  {
    curInd = 1;
    HendleNumBution(curInd, lastButton, irDataValue, pressTerm);
    //Serial.println("Dva");
  }

  if (lastButton == BTN_3 || lastButton == BTN_3b) // соответветствует цифре 3 на пульте
  {
    curInd = 2;
    HendleNumBution(curInd, lastButton, irDataValue, pressTerm);
    //Serial.println("Tri");
  }

  if (lastButton == BTN_4 || lastButton == BTN_4b) // соответветствует цифре 4 на пульте
  {
    curInd = 3;
    HendleNumBution(curInd, lastButton, irDataValue, pressTerm);
    //Serial.println("Chetyre");
  }

  if (lastButton == BTN_5 || lastButton == BTN_5b) // соответветствует цифре 5 на пульте
  {
    curInd = 4;
    HendleNumBution(curInd, lastButton, irDataValue, pressTerm);
    //Serial.println("Pjat");
  }

  if (lastButton == BTN_6 || lastButton == BTN_6b) // соответветствует цифре 6 на пульте
  {
    curInd = 5;
    HendleNumBution(curInd, lastButton, irDataValue, pressTerm);
    //Serial.println("Shest");
  }

  if (lastButton == BTN_7 || lastButton == BTN_7b) // соответветствует цифре 6 на пульте
  {
    curInd = 6;
    HendleNumBution(curInd, lastButton, irDataValue, pressTerm);
    //Serial.println("Shest");
  }

  if (lastButton == BTN_8 || lastButton == BTN_8b) // соответветствует цифре 6 на пульте
  {
    curInd = 7;
    HendleNumBution(curInd, lastButton, irDataValue, pressTerm);
    //Serial.println("Shest");
  }

  if (lastButton == BTN_9 || lastButton == BTN_9b) // соответветствует цифре 6 на пульте
  {
    curInd = 8;
    HendleNumBution(curInd, lastButton, irDataValue, pressTerm);
    //Serial.println("Shest");
  }

  if (lastButton == BTN_FUNKb)
  {
    flForAll = true;
    Serial.println("flForAll = true");
    //SetForAll(curInd);
    //    LedToStrip();
    //    FastLED.show();
  }

  if (lastButton == BTN_MUTEb)
  {
    Serial.println("flForAll = false");
    flForAll = false;
    blink(curInd);
  }
}


void loop()
{
  int i = 0;
  byte flShow = 0;

  if (irRec.decode(&irData)) // Получение данных с ИК приемник
  {

    //    unsigned long CurInterval = millis() - interruptTime;
    //    interruptTime = millis();
    //    Serial.println("CurInterval = " + String(CurInterval));

    // Данные пришли - анализируем, выполняем команды с пульта:
    Serial.println(irData.value); // выводим полученные данные на порт

    buttonOn = 1;               // кнопка нажата
    lastButtonTime = millis();  // время нажатие любой кнопки


    if (irData.value != BTN_REPEAT)
    {
      lastButton = irData.value;
      newBattonTime = millis();  // время нажатия новой кнопки
      //QHold = 0;
    }
    else
    {
      //QHold ++;
    }

    pressNewButtonTerm = millis() - newBattonTime; // интервал от последнего нажатияновой кнопки (начала нажатия и удержания кнопки)
    HendleButton(lastButton, irData.value, pressNewButtonTerm);

    if (flForAll==true)
    {
      Serial.println("flForAll = " + String(flForAll) + " curInd = " + String(curInd));
      SetForAll(curInd);
    }
    
    LedToStrip();
    FastLED.show();

    irRec.resume(); // принимаем следующий сигнал на ИК приемнике
  }

  else

  {
    //    if (buttonOn != 0)
    //    {
    //      unsigned long pressTerm = millis() - lastButtonTime;  // интервал от последнего события от пульта
    //      if (pressTerm > 150)                                  // если от пульта нет сигнала более 150 чего-то там, то кнопку отжали
    //      {
    //
    //        //        pressNewButtonTerm = millis() - newBattonTime; // интервал от последнего нажатияновой кнопки (начала нажатия и удержания кнопки)
    //        //        HendleButton(lastButton, 0, pressNewButtonTerm);
    //
    //        lastButtonTime = 0;
    //        buttonOn = 0;
    //      }
    //    }
  }

  if (millis() - MoveColorTime > MOVECOLOR_PAUSE)
  {
    MoveColorTime = millis();
    for (int i = 0; i < ROOM_COUNT; i++)
    {
      if (ledConds[i].AutoColorMove != 0)
      {
        ledConds[i].MoveColor(2);
        flShow = 255;
      }
    }
  }

  if (numBlink != 0)
  {
    if (millis() - blinkTime > BLINK_PAUSE)
    {
      if (ledConds[numBlink - 1].On == 0)
      {
        ledConds[numBlink - 1].Switch();
        numBlink = 0;

        flShow = 255;
      }
    }
  }

  if (flShow != 0)
  {
    flShow = 0;
    if (flForAll==true)
    {
      SetForAll(curInd);
    }
    LedToStrip();
    FastLED.show();
  }

  delay(100); // небольшая пауза, 0,1 сек.
}
