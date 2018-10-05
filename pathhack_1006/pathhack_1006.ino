#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h> 
#include <Wire.h>


// A4,5는 LCD
int toTxPin = 2;
int toRxPin = 3;

int HumanPin = 5;
int servoPin = 9;


// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F,16, 2);  // I2C LCD 객체 선언

Servo servo; 
int angle = 0; // servo position in degrees 
SoftwareSerial btSerial(toTxPin,toRxPin); 

void setup()
{
  lcd.begin();
  Serial.begin(9600);
  btSerial.begin(9600); 
  lcd.backlight(); // backlight를 On 시킵니다.

  servo.attach(servoPin); 
  lcd.print("Hello, world!");

}

bool first = true;
void loop()
{
    if(digitalRead(HumanPin)){
      Serial.println("human\n");
    }
    if(Serial.available())
    {
      delay(5);  
      while(Serial.available())
      {
        btSerial.write(Serial.read());
      }
    }
    if(btSerial.available())
    {
      delay(5);  
      while(btSerial.available())
      {
        byte data = btSerial.read();
        if(data == '$'){
          if(!first){
            const char* ch = "can't use\n";
            btSerial.write(ch);
            lcd.clear();
            lcd.print(ch);
          }
          else{
            btSerial.write("ads\n");
            delay(1000);
            digitalWrite(7, HIGH);
            for(angle = 10; angle < 170; angle++) 
            { 
              servo.write(angle); 
              delay(10); 
            } 
            btSerial.write("open\n");
            const char* ch = "open\n";
            lcd.clear();
            lcd.print(ch);
          }
        }
        if(data == '%'){
          
          for(angle = 170; angle > 10; angle--) 
          { 
            servo.write(angle); 
            delay(10); 
          } 
            const char* ch = "close\n";
            btSerial.write(ch);
            lcd.clear();
            lcd.print(ch);
        }
        Serial.write(data);
      }
    }
    if(digitalRead(5) && first){
            const char* ch = "using\n";
            btSerial.write(ch);
            lcd.clear();
            lcd.print(ch);
            first = false;
    }
    if(digitalRead(5) == false && first == false){
      const char* ch = "empty\n";
      btSerial.write(ch);
      lcd.clear();
      lcd.print(ch);
      first = true;
    }
}
