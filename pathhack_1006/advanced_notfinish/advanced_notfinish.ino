#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>


// A4,5는 LCD
int toTxPin = 2;
int toRxPin = 3;

int HumanPin = 5;
int servoPin = 7;
int ButtonPin = 8; // 나중에 RFID


// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F,16, 2);  // I2C LCD 객체 선언

Servo servo;
int angle = 0; // servo position in degrees
SoftwareSerial btSerial(toTxPin,toRxPin);

void setup()
{
  pinMode(ButtonPin, INPUT);

  lcd.begin();
  Serial.begin(9600);
  btSerial.begin(9600);
  lcd.backlight(); // backlight를 On 시킵니다.

  servo.attach(servoPin);
  lcd.print("Hello, world!");
}

bool NOTusing = true;
bool Appright = false;
bool paid = false;

void Allprint(const char* ch){ //주의 btwrite 지우면 AT 못쓴다.
  btSerial.write(ch);
  Serial.write(ch);
  lcd.clear();
  lcd.print(ch);
}

int counter = 0;
bool NoHumanTimeCounter(){
  if(digitalRead(HumanPin) == false){
    counter++;
  }
  else{
    counter = 0;
  }

  if(counter >= 10000){
    return true;
  }
  else{
    return false;
  }
}

void BTtoSerialmoniter(){
  if(Serial.available()) // 시리얼 모니터에 BT내용 띄우는 코드
  {
    delay(5);
    while(Serial.available())
    {
      btSerial.write(Serial.read());
    }
  }
}

void process(){
  if(btSerial.available())
  {
    delay(5);
    while(btSerial.available()) // 앱에서 온 데이터 다 받아 들일때까지 (보통 char 1개)
    {
      byte data = btSerial.read();
      if(data == '$'){ // 사용 요청
        if(!NOTusing){  // 사용중이면 사용할 수 없다고 출력
          const char* ch = "can't use\n";
          Allprint(ch);
        }
        else{
          const char* ch = "ads\n";
          Allprint(ch);
          delay(1000);
          btSerial.write("okuse\n");
          Appright = true;
        }
      }

      if(data == '^'){ // 돈 내면 확인
        paid = true;
        Allprint("ok paid\n");
      }

      if(Appright && paid == true){ // 앱으로 문열기
        for(angle = 10; angle < 170; angle++){
          servo.write(angle);
          delay(10);
        }
      }
      Allprint("open\n");
       if(digitalRead(ButtonPin) == true && Appright == false && NOTusing){
      for(angle = 10; angle < 170; angle++){
        servo.write(angle);
        delay(10);
      }
      Serial.println("RFID OPEN");
    }

    if(data == '%'){ // 이용완료 조건문
      for(angle = 170; angle > 10; angle--)
      {
        servo.write(angle);
        delay(10);
      }
      Allprint("close\n");

    }
    }

   
  }
}

void loop()
{
  NOTusing = NoHumanTimeCounter();
  BTtoSerialmoniter();
  process();
}
// 보류
// if(digitalRead(HumanPin) && NOTusing){
//   const char* ch = "using\n";
//   btSerial.write(ch);
//   lcd.clear();
//   lcd.print(ch);
// }
