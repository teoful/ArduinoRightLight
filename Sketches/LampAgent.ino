#include <RBDdimmer.h>//

#define artificialLightID 255
#define controlByte 192             //ensure that following value is directed to actuators (distinguish from zigbee realted communication)
#define outputPin  12

dimmerLamp dimmer(outputPin);

int lamps,value=0;

void setup() {
  Serial.begin(9600); 
  dimmer.begin(NORMAL_MODE, ON);                              //dimmer initialisation: name.begin(MODE, STATE)
  dimmer.setPower(value);                                     //set first power to 0
}

void loop() {
  if (Serial.available()>2) {
    if (Serial.read()==controlByte && Serial.read()==artificialLightID){
      int oldValue=value;
      lamps = Serial.read();
      if(lamps<=100 && lamps>=0){                             //prudential because there is not checksum
        if(lamps==0){
          value=0;
        }else{
          value = map(lamps,1,100,10,85); 
        }
        delay(200);
        if (value!=oldValue){
          dimmer.setPower(value);                             //setPower(0-100%);
        }
        Serial.print("lampValue -> ");
        Serial.print(dimmer.getPower());
        Serial.print(" -> ");
        Serial.print(lamps);
        Serial.println("%");
        delay(50);
      }
    }
  }
}
