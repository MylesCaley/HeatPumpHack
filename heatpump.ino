
#include "Wire.h"
#include <SharpIR.h>

#define I2C_ADDR  0x20  // 0x20 is the address with all jumpers removed
SharpIR sharpA0(A0, 1000, 93, 1080);
unsigned long starttime;

void p(char *fmt, ... ){
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        Serial.print(buf);
}

void setup()
{
  starttime = millis();
  Serial.begin( 9600 );
  Serial.println("HVAC HEATPUMP CONTROL");
  Serial.println(starttime);
  pinMode(8, INPUT);
  pinMode (A0, INPUT);

  Wire.begin(); // Wake up I2C bus

  // Set I/O bank A to outputs
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(0x00); // IODIRA register
  Wire.write(0x00); // Set all of bank A to outputs
  Wire.endTransmission();
}

void loop()
{
  // let the heatpump have a chance to defrost when first turning on
  sendValueToLatch(1);  
  
  // allow 40 minutes of defrost
  allowDefrost(40);
  
  // lockout the defrost 
  sendValueToLatch(0);
  
  // hold for 5 hours unless the sharp sensor detects a frozen heat pump
  if (disableDefrostWithIRExit(300)==true)
  {
    Serial.println("we gots a freezing heat pump, opening the gates for 40 minutes");
  }
}

void allowDefrost(int minutes)
{
  unsigned long sometime;
  for (int i=1; i<minutes; i++)
  {
    sometime = millis();
    int disA0=sharpA0.distance(); 
    p("allowed, %lu, %d\n", sometime, disA0);
    delay(60000);
  }
}  

bool disableDefrostWithIRExit(int minutes)
{
  unsigned long sometime;
  for (int i=1; i<minutes; i++)
  {
    sometime = millis();
    int disA0=sharpA0.distance(); 
    p("disabled, %lu, %d\n", sometime, disA0);
    if (disA0 <= 13)
    {
      return true;
    }
    delay(60000);
  }
  return false;
}

void sendValueToLatch(int latchValue)
{
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(0x12);        // Select GPIOA
  Wire.write(latchValue);  // Send value to bank A
  Wire.endTransmission();
}
