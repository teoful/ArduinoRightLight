#define curtainsID 254
#define controlByte 192     //ensure that following value is directed to actuators (distinguish from zigbee realted communication)
#define innerSteps 32       //giri interni in full step (11.25°)
#define outerSteps 2048     //InnerSteps(32)*GearReduction(64)
#define rpm 400             //motor speed defined in outer loop per minute
#define totSteps 9200       //curtain range in motor steps

#include <Stepper.h>

Stepper myStepper(innerSteps, 8, 10, 9, 11);                      //initialize the stepper library on pins 8 through 11:
int currentState=0;                                               //current openess of curtain expressed in percentage (initialy totally closed)

void setup() {
  myStepper.setSpeed(rpm);
  Serial.begin(9600);
  Serial.println("Remember to totally close the curtain");
  delay(1000);
}

void loop() {
  if (Serial.available()>2){
    if (Serial.read()==controlByte && Serial.read()==curtainsID){     //254 Curtains ID
      int newState=Serial.read();
      if(newState<=100 && newState>=0){                               //prudential because there is not checksum
        int stepsToOpen=(newState-currentState)*totSteps/100;
        if (stepsToOpen!=0){
          myStepper.step(stepsToOpen);
          currentState=newState;
        }
        Serial.print("Opened: ");
        Serial.print(stepsToOpen);
        Serial.print("steps --> ");
        Serial.print("New state: ");
        Serial.println(currentState);
      }
    }
  }
}
