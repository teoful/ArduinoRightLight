/* Central System
 * Receives the value of brightness from external and internal fotoresistors;
 * Elaborates the data received and, if needed, sends data to actuators
 */

#include <SoftwareSerial.h>

#define RANGEMIN 70
#define RANGEMAX 80

#define extSensID 253
#define curtainsID 254
#define artificialLightID 255
#define controlByte 192               //ensure that following value is directed to actuators (distinguish from zigbee realted communication)

#define IRnodes 3
#define MaxExternalWait 10800000      //3hours
#define MaxInternalWait 600000        //10 minutes

SoftwareSerial SwSerial =  SoftwareSerial(2, 3);  //create a new serial port via software
int curtains=0;
int lamps=0;
int externalLight=-1;
int internalLight=-1;
int lastExternal=0;
int lastInternal=0;
int sumValues, nValues;


void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT);                              //RX of SwSerial
  pinMode(3, OUTPUT);                             //TX of SwSerial
  SwSerial.begin(9600);
  delay(500);
  uint8_t start_IR[3];
  String("IR").toCharArray(start_IR,3);
  SwSerial.write(start_IR,3);
}

void loop() {
  Serial.flush();
  int t=millis();
  if((t-lastExternal)>=MaxExternalWait){                                        //External sensor is probably down ---> WARN USER IN SOMEWAY
    Serial.println();
    Serial.println("External sensor is probably down");
  }                                       
  if((t-lastInternal)>=MaxInternalWait){                                        //IR network is probably down ---> WARN USER IN SOMEWAY
    Serial.println();
    Serial.println("IR network is probably down");
  }                                       
  if(Serial.available()>2){
    if(Serial.read()==controlByte && Serial.read()==extSensID){
      externalLight=Serial.read();
      lastExternal=millis();
      Serial.println();
      Serial.print("External light ");
      Serial.println(externalLight);
    }
  }
  if(SwSerial.available()>2){
    sumValues=0;
    nValues=0;
    for(int i=0; i<IRnodes; i++){
      int value=SwSerial.parseInt();
      if(value==-1){                                                            //A sensor is in completely darkness ---> WARN USER IN SOMEWAY (?)
        value++;
      }
      if(value>0){
        sumValues+=value;
        nValues++;
      }
    }
    if(nValues>0){
      internalLight=sumValues/nValues;
      lastInternal=millis();
      Serial.println();
      Serial.print("n ");
      Serial.println(nValues);
      Serial.print("sum ");
      Serial.println(sumValues);
      Serial.print("Internal light ");
      Serial.println(internalLight);
      //
      if (internalLight>=RANGEMIN && internalLight<=RANGEMAX && lamps>0){       //RANGE (action needed only if lamps are on in order to reduce them)
        lamps--;
        Serial.println("lamps --");
        Serial.write(controlByte);
        Serial.write(artificialLightID);
        Serial.write(lamps);
      }
      //
      if (internalLight<RANGEMIN){                                              //UNDER RANGE 
        if((externalLight<=RANGEMIN && externalLight!=-1) || curtains==100){      //useless to open curtains: increase artificial light NB! if no external value available open curtains (ECO)
          if(lamps<95){
            lamps+=5;
            Serial.println("lamps +5");
            Serial.write(controlByte);
            Serial.write(artificialLightID);
            Serial.write(lamps);
          }else if (lamps!=100){
            lamps=100;
            Serial.println("lamps max");
            Serial.write(controlByte);
            Serial.write(artificialLightID);
            Serial.write(lamps);
          }
        }else{                                                                    //useful to open curtains: open the curtains
          if(curtains<95){
            curtains+=5;
            Serial.println("curtains +5");
            Serial.write(controlByte);
            Serial.write(curtainsID);
            Serial.write(curtains);
          }else if (curtains!=100){
            curtains=100;
            Serial.println("curtains max");
            Serial.write(controlByte);
            Serial.write(curtainsID);
            Serial.write(curtains);
          }
        }
      }
      //
      if (internalLight>RANGEMAX){                                              //OVER RANGE
        if (lamps==0){                                                            //only if lamps already off close curtains
            if (curtains>5){
              curtains-=5;
              Serial.println("curtains -5");
              Serial.write(controlByte);
              Serial.write(curtainsID);
              Serial.write(curtains);
            }else if (curtains!=0){
              curtains=0;
              Serial.println("curtains off");
              Serial.write(controlByte);
              Serial.write(curtainsID);
              Serial.write(curtains);
            }
        }else{                                                                    //otherwise reduce lamps
          if (lamps>5){
            lamps-=5;
            Serial.println("lamps -5");
          }else{
            lamps=0;
            Serial.println("lamps off");
          }
          Serial.write(controlByte);
          Serial.write(artificialLightID);
          Serial.write(lamps);
        }
      }
    }
  }
}
