/* External Light
 * Gets the brightness value from a fotoresistor and sends it to the Central System every 15 seconds
 */

#define extSensID 253
#define controlByte 192                           //ensure that following value is directed to actuators (distinguish from zigbee realted communication)
#define sleepTime 30000                           //sleeping time beetween two sampling.

#define LDRdark 530                               //530 is empirical darkness. From LDR specs dark resistance is 500kohm then it should be around 495(1023/(500k+470k)*470k) but it is a pretty ideal condition.

const int VTOT=5;                                 //NOT USED: 5v, initial voltage provided to the voltage divider circuit (currently commented in the code)
const double RNOTA=470000;                        //NOT USED: 470kohm, value of the known resistance in the voltage divider circuit (currently commented in the code)

int vNotaDig, luce, luce100;
double vNota, rTot, rX;                           //NOT USED: analogical voltage readed from v. divider circuit (vNota), total resistance of v. divider circuit (rTot), resistance of LDR (rX).(currently commented in the code)

void setup() {
  Serial.begin(9600);
}

void loop() {
  /***CALCULATING LDR RESISTANCE***
  vNotaDig=analogRead(A0);        //readed voltage is the one crossing the known resistance
  vNota=vNotaDig*VTOT/1023;       //getting voltage from 0 to 5v
  rTot=VTOT/(vNota/RNOTA);        //current in circuit is constant and can be obtained from vnota/RNOTA (I=V/R); then total voltage divided constant current give us total resistance (R=V/I)
  rX=rTot-RNOTA;
  */
  //READ
  luce=analogRead(A0);                                //LDR resistance is inversely related to light and readed voltage (a0) is inversely related to LDR res then light is directly related to a0
  luce=constrain(luce,LDRdark,1023);                  //For our purposes each value lower than the LDRdark can be considered darkness and will be 0.
  luce100=map(luce,LDRdark,1023,0,99);                //light value on a 100base. From 0 to 99.
  //SEND
  Serial.write(controlByte);
  Serial.write(extSensID);
  Serial.write(luce100);
  Serial.flush();
  delay(sleepTime);
}
  
