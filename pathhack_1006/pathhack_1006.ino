#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rc522(SS_PIN, RST_PIN);                  // Create MFRC522 instance

                         // A4,5는 LCD
int buttonPin = 2;
int toTxPin = 3;
int toRxPin = 4;
int HumanPin = 5;
int servoPin = 7;
//int ButtonPin = 8; // 나중에 RFID


// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // I2C LCD 객체 선언

Servo servo;
int angle = 0; // servo position in degrees
SoftwareSerial btSerial(toTxPin, toRxPin);

void setup()
{
  pinMode(buttonPin, INPUT);
  lcd.begin();
  Serial.begin(9600);
  btSerial.begin(9600);
  lcd.backlight(); // backlight를 On 시킵니다.
  pinMode(8, INPUT);

  servo.attach(servoPin);
  lcd.print("Hello, world!");

  SPI.begin();
  rc522.PCD_Init();

}

bool NOTusing = true;
bool Appright = false;
bool paid = false;
bool state = false;
bool openstate = false;
bool RFIDnotopened = true;
int openANDclosetime = 3000;
int RFIDinitcnt = 0;
int RFIDnotcnt = 0;


void Allprint(const char* ch) { //주의 btwrite 지우면 AT 못쓴다.
  btSerial.write(ch);
  Serial.write(ch);
  lcd.clear();
  lcd.print(ch);
}


int counter = 0;
bool NoHumanTimeCounter() {//false 가 사람 있다 
  if (!Appright || !paid) {
    return true;
  }
  //Serial.println(counter);
  if (digitalRead(HumanPin) == false) {
    counter++;
  }
  else {
    counter = 0;
  }

  if (counter >= 100) {
    if (openstate == true) {
      Serial.println("counter close\n");
      counter = 0;
      for (angle = 170; angle > 10; angle--)
      {
        servo.write(angle);
        delay(10);
      }
      openstate = false;
      Allprint("close\n");
    }
    Appright = false;
    paid = false;
    return true;
  }
  else {
    return false;
  }
}

void BTtoSerialmoniter() {
  if (Serial.available()) // 시리얼 모니터에 BT내용 띄우는 코드
  {
    delay(5);
    while (Serial.available())
    {
      btSerial.write(Serial.read());
    }
  }
}

void process() {
  //  Serial.print("Appright : ");
  //  Serial.println(Appright);
  //  Serial.print("paid : ");
  //  Serial.println(paid);
  //  Serial.print("NOTusing : ");
  //  Serial.println(NOTusing);
  //  Serial.println("ButtonPin : ");
  //  Serial.println(digitalRead(ButtonPin));
  if (btSerial.available())
  {
    delay(5);
    while (btSerial.available()) // 앱에서 온 데이터 다 받아 들일때까지 (보통 char 1개)
    {
      byte data = btSerial.read();
      if (data == '$') { // 사용 요청
        if (!NOTusing) {  // 사용중이면 사용할 수 없다고 출력
          const char* ch = "can't use\n";
          Allprint(ch);
        }
        else {
          const char* ch = "ads\n";
          Allprint(ch);
          delay(1000);
          btSerial.write("okuse\n");
          Appright = true;
        }
      }
      if (data == '^' && Appright == true) { // 돈 내면 확인
        paid = true;
        Allprint("ok paid\n");
      }
      if (Appright && paid == true && NOTusing) { // 앱으로 문열기
        for (angle = 10; angle < 170; angle++) {
          servo.write(angle);
          delay(10);
        }
        openstate = true;
        NOTusing = false;
        Allprint("open\n");
        delay(openANDclosetime); // 7초뒤 자동 문닫기
        for (angle = 170; angle > 10; angle--)
        {
          servo.write(angle);
          delay(10);
        }
        openstate = false;
        Allprint("autoclose\n");
      }


    }
  }

//  Serial.print("Appright : ");
//  Serial.println(Appright);
//  Serial.print("paid : ");
//  Serial.println(paid);
//  Serial.print("NOTusing : ");
//  Serial.println(NOTusing);
//  Serial.println("ButtonPin : ");
//  Serial.print("state : ");
//  Serial.println(state);
  if (state && !Appright && NOTusing && RFIDnotopened) { // RFID로 문열기
    for (angle = 10; angle < 170; angle++) {
      servo.write(angle);
      delay(10);
    }
    RFIDnotopened = false;
    openstate = true;
    NOTusing = false;
    Serial.println("RFID OPEN");
    //delay 쓰레드로 해야 하는중에 앱 연결 방지 잘 안된다. 
    delay(openANDclosetime); // 7초뒤 자동 문닫기
    for (angle = 170; angle > 10; angle--)
    {
      servo.write(angle);
      delay(10);
    }
    openstate = false;
    Allprint("autoclose\n");
    //RFID 접근중 앱으로 열기가능한 버그. 시연중엔 안나옴
    Appright = true;
    paid = true;
  }
}

void loop()
{

  NOTusing = NoHumanTimeCounter();
  BTtoSerialmoniter();
  process();
  if (digitalRead(buttonPin) == true) {
    for (angle = 10; angle < 170; angle++) {
      servo.write(angle);
      delay(10);
    }
    openstate = true;
    NOTusing = false;
    Allprint("open\n");
    delay(openANDclosetime); // 7초뒤 자동 문닫기
    for (angle = 170; angle > 10; angle--)
    {
      servo.write(angle);
      delay(10);
    }
    openstate = false;
    Allprint("autoclose\n");
    Appright = false;
    paid = false;
    NOTusing = true;
  }

  if(RFIDnotopened == false){ //RFID 반응속도 차이 보정 
    RFIDnotcnt++;
    if(RFIDnotcnt >= 20){
      RFIDnotcnt = 0;
      RFIDnotopened = true;
    }
  }




  state = rc522.uid.uidByte[0] == 0x8D;

  Serial.println(rc522.uid.uidByte[0]);
  if (state == true) {
    RFIDinitcnt++;
    if (RFIDinitcnt >= 10) {
      RFIDinitcnt = 0;
      rc522.uid.uidByte[0] = 0x00;
      state = false;
      delay(100);
    }
  }
  if (!rc522.PICC_IsNewCardPresent()) return;     // Look for new cards
  if (!rc522.PICC_ReadCardSerial()) return;       // Select one of the cards
}
// 보류
// if(digitalRead(HumanPin) && NOTusing){
//   const char* ch = "using\n";
//   btSerial.write(ch);
//   lcd.clear();
//   lcd.print(ch);
// }
