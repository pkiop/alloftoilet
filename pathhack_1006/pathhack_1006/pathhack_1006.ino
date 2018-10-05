#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rc522(SS_PIN, RST_PIN);                  // Create MFRC522 instance

int buttonPin = 2;
int toTxPin = 3;
int toRxPin = 4;
int HumanPin = 6;
int servoPin = 7;

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // I2C LCD 객체 선언

Servo servo;
int angle = 0; // servo position in degrees
SoftwareSerial btSerial(toTxPin, toRxPin);

void setup()
{
//  pinMode(HumanPin, INPUT);
  pinMode(buttonPin, INPUT);
  Serial.begin(9600);
  btSerial.begin(9600);
  servo.attach(servoPin);
  SPI.begin();
  rc522.PCD_Init();
  servo.write(180);
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

void doorOpen(){
  for (angle = 180; angle > 10; angle--) {
      servo.write(angle);
      delay(10);
    }
}

void doorClose(){
  for (angle = 10; angle < 180; angle++) {
      servo.write(angle);
      delay(10);
      Serial.println(angle);
    }
}

void Allprint(const char* ch) { //주의 btwrite 지우면 AT 못쓴다.
  btSerial.write(ch);
  Serial.write(ch);
}

int counter = 0;
bool NoHumanTimeCounter() {//false 가 사람 있다이번에 사용 x
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
      for (angle = 180; angle > 10; angle--)
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

void Apprequest(char data){
  if (data == '$') { // 사용 요청
    //if (!NOTusing) {  // 사용중이면 사용할 수 없다고 출력
    if(0){
      const char* ch = "can't use\n";
      Allprint(ch);
    }
    else {
      const char* ch = "ask\n";
      Allprint(ch);
      delay(1000);
      btSerial.write("okuse\n");
      Appright = true;
      paid = true; // 이번만
    }
  }
}

void paycheck(char data){
  if (data == '^' && Appright == true) { // 돈 내면 확인
    paid = true;
    Allprint("ok paid\n");
  }
}

void OpenByApp(){
  if (Appright && paid == true && NOTusing) { // 앱으로 문열기
    doorOpen();
    openstate = true;
    NOTusing = false;
    Allprint("open\n");
    delay(openANDclosetime); // 7초뒤 자동 문닫기
    doorClose();
    openstate = false;
    Allprint("autoclose\n");
  }
}

void OpenByRFID(){
  if (state && NOTusing && RFIDnotopened) { // RFID로 문열기
    doorOpen();
    RFIDnotopened = false;
    openstate = true;
    NOTusing = false;
    Serial.println("RFID OPEN");
    //delay 쓰레드로 해야 하는중에 앱 연결 방지 잘 안된다.
    delay(openANDclosetime); // 7초뒤 자동 문닫기
    doorClose();
    openstate = false;
    Allprint("autoclose\n");
    //RFID 접근중 앱으로 열기가능한 버그. 시연중엔 안나옴
    Serial.println("aa");
    Appright = false;
    Serial.println("bb");
    paid = false;
    Serial.println("cc");
  }
}

void OpenByButton(){
  if (digitalRead(buttonPin) == true) {
    if(openstate == false){
      doorOpen();
    }
    openstate = true;
    NOTusing = false;
    Allprint("open\n");
    delay(openANDclosetime); // 7초뒤 자동 문닫기
    doorClose();
    openstate = false;
    Allprint("autoclose\n");
    Appright = false;
    paid = false;
    NOTusing = true;
  }
}

void RFIDdelayComp(){
  if(RFIDnotopened == false){ //RFID 반응속도 차이 보정
    RFIDnotcnt++;
    if(RFIDnotcnt >= 20){
      RFIDnotcnt = 0;
      RFIDnotopened = true;
    }
  }
}

void BTprocess() {
  if (btSerial.available())
  {
    delay(5);
    while (btSerial.available()) // 앱에서 온 데이터 다 받아 들일때까지 (보통 char 1개)
    {
      byte data = btSerial.read(); // 앱에서 보낸 char (string 될수도)
      Apprequest(data);
      paycheck(data);
      OpenByApp();
    }
  }
}

void GetRFIDdata(){
  state = rc522.uid.uidByte[0] == 0x8D;
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

void loop()
{
  NOTusing = true;
  BTtoSerialmoniter();
  BTprocess();
  OpenByRFID();
  OpenByButton();
  RFIDdelayComp();
  GetRFIDdata();
}
