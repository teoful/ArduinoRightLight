#define myID 0
#define myStart "S0"
#define nodes 3

#define LDRdark 530         //530 is empirical darkness. From LDR specs dark resistance is 500kohm then it should be around 495(1023/(500k+470k)*470k) but it is a pretty ideal condition.

#define IrStartup 2800      //time needed from IR module in order to start working
#define sleepTime 10000     //sleeping time common to all nodes. NB! it's not time between two sampling that is given by sleepTime+longest Uplink time (ie Uplink[n])
#define sendTime 280        //estimated avg IR transmission time
#define firstSend 350       //estimated first IR transmission time (longer)
#define lazyTime 450        //in order to increment time distance between receiving and sending
#define timeout 400         //has to be less than lazyTime


const int VTOT=5;                                 //NOT USED: 5v, initial voltage provided to the voltage divider circuit (currently commented in the code)
const double RNOTA=470000;                        //NOT USED: 470kohm, value of the known resistance in the voltage divider circuit (currently commented in the code)
const uint8_t initial_bytes[2]={0xA1, 0xF1};      //hex values for setting IR module before each message: A1 is broadcast address and F1 set transmission status

uint8_t messages[myID+1][3];
uint8_t feedback_bytes[myID+1];                   //NOT USED: feedback bytes (currently commented in the code)
int vNotaDig, luce, luce100;
double vNota, rTot, rX;                           //NOT USED: analogical voltage readed from v. divider circuit (vNota), total resistance of v. divider circuit (rTot), resistance of LDR (rX).(currently commented in the code)
int failures=0;                                   //number of consective failures


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(timeout);
  pinMode(13,OUTPUT);                             //pin 13 used as power source of IR module
  digitalWrite(13,HIGH);
  while (strcmp(messages[0],myStart)!=0){         //using messages[0] just for saving memory
    if(Serial.available()>=3){
      Serial.readBytes(messages[0],3);
    }
  }
  messages[0][1]++;                               //getting myStart of next node
  Serial.write(initial_bytes,2);
  Serial.write(messages[0],3);
  delay(300);                                     //time needed before switching off IR module
  digitalWrite(13,LOW);
  int gapTime=0;
  for (int i=0; i<myID; i++){                     //in order to sync each node sleep more than previous one. Time added corresponds to recTime of prev. nodes plus lazyTime cause at each step we have: prevNodeStartupTime+prevNodeRecTime+prevNodeLazyTime-thisNodeStartupTime
    gapTime+=(sendTime*i)-firstSend+lazyTime;     //recTime(i)=sendTime*i NB! firstSend is subtracted at each step because due to above execution nodes have already this "distance"
  }
  delay(sleepTime+gapTime);                       //first "sleep" 
}

void loop() {
  if (failures<3){
    /***CALCULATING LDR RESISTANCE***
    vNotaDig=analogRead(A0);        //readed voltage is the one crossing the known resistance
    vNota=vNotaDig*VTOT/1023;       //getting voltage from 0 to 5v
    rTot=VTOT/(vNota/RNOTA);        //current in circuit is constant and can be obtained from vnota/RNOTA (I=V/R); then total voltage divided constant current give us total resistance (R=V/I)
    rX=rTot-RNOTA;
    */
    digitalWrite(13,HIGH);
    long t =millis();
    while((millis()-t)<(IrStartup)){
      if (Serial.available())Serial.read();
    }
    //
    //RECEIVE
    int totLen=0;
    for (int i=0; i<myID; i++){                         //reading one time less than array size because last value it's mine
      int len=Serial.readBytes(messages[i],3);          //waiting for a 3byte message otherwise go on after timeout (no testing)
      if (len<3){
        len=0;
        for (int j=0; j<3; j++){
          messages[i][j]=(char(0));
        }
      }
      totLen+=len;
    }
    if ((totLen==0)&&(myID!=0)){
      failures++;
    }else{
      failures=0;
    }
    //
    //READ
    luce=analogRead(A0);                                //LDR resistance is inversely related to light and readed voltage (a0) is inversely related to LDR res then light is directly related to a0
    luce=constrain(luce,LDRdark,1023);                  //avoiding negative values. For our purposes each value lower than the LDRdark can be considered darkness and will be 0.
    luce100=map(luce,LDRdark,1023,0,99);                //light value on a 100base. From 0 to 99.
    if(luce100==0)luce100--;                            //avoid 0 value cause will confuse parseInt output in centralsystem
    String(luce100).toCharArray(messages[myID],3);
    //
    //WAIT
    delay(lazyTime);
    //
    //SEND
    for (int i=0; i<=myID; i++){
      Serial.write(initial_bytes,2);
      Serial.write(messages[i],3);
      delay(sendTime);
    }
    /***READING FEEDBACK FROM IR MODULE***
    if(Serial.available()==myID+1){
      Serial.readBytes(feedback_bytes,myID+1);        //if needed I can read and store feedback of transmitted messages
    }
    */
    digitalWrite(13,LOW);
    //
    delay(sendTime*2*(nodes-1-myID));                   //Padding: add time to my sleep in order to keep constant gap between node's uplink (ie simulating to have same uplink of last node)
    delay(sleepTime);                                   //"sleep" (could be deepsleep with other boards)
  }
}
