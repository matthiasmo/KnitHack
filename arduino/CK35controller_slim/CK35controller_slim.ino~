/*
Brother KH970 Controller
 2014 January
 So Kanno

May 2015:
Changes for stitching worlds(stitchingworlds.net) by Matthias Mold:
	+specific pinconfig for our board
	+added checksum
	+no need to go to the end switches
	+slim version (remove everything not neccesary)
	+various bug fixes
	+beeping feedback
 */

char receivedBin[203];
int pixelBin[256] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

byte dataSize = 202;
boolean dataReplace = false;
boolean data_valid=false;
volatile boolean request_newline=false;

volatile int header = 0;
byte footer = 126;
int columnNum = 0;
byte carriageMode = 124;
byte carriageK = 124;
byte carriageL = 125;

boolean running_lock=1;

volatile int newline_lock=0;   //locks the request for a newline, counts stitches since newline event
#define NEWLINE_LOCK_LIMIT 20  //dont accept new line until NEWLINE_LOCK_LIMIT stitches made;this means minimal width is 10 stitches
#define val_beep 150            //loaudness of beep

volatile int phase = 0;

//INPUT SYSTEM
const int enc1 = 2;  //encoder 1
const int enc2 = 3;  //encoder 2
const int enc3 = 4;  //phase encoder
const int LEnd = 1;   //endLineLeft for analog in
const int REnd = 0;   //endLineRight for analog in

//OUTPUT SYSTEM
const int beep= 9;  //Beep

//this is for kh930
//int solenoidsTemp[16] = 
//{
 // 22,24,26,28,30,32,34,36,37,35,33,31,29,27,25,23};

int solenoidsTemp[16] = {36,34,32,30,28,26,24,22,37,35,33,31,29,27,25,23};  //this finally works!

volatile int pos = 0;  //position of carriage

volatile boolean encState2 = 0;  //encoder 2 state

int zero = 0;       //left end switch value
int lastZero = 0;   
int right = 0;      //right end switch value
int lastRight = 0;  

volatile int carDirection = 0;  //direction of carriage　0:unknown　1:right　2:left

long timelastreceivedline=0;


void setup(){
  delay(1000);
  
  pinMode(beep, OUTPUT);
  pinMode(enc1, INPUT);
  pinMode(enc2, INPUT);
  pinMode(enc3, INPUT);

  digitalWrite(beep, LOW);
  
  delayMicroseconds(50);
  
  for(int i=0; i<16; i++){
    pinMode(solenoidsTemp[i], OUTPUT);
    digitalWrite(solenoidsTemp[i], LOW);
  }
    
     attachInterrupt(enc1, rotaryEncoder, RISING);
     
   
   delayMicroseconds(5000); 
   
  
     
  Serial.begin(57600);
  Serial.setTimeout(100) ;
  
  beeping();
  
  if(digitalRead(enc3)==false){  //phase ditection
  phase = true;
  }  
  
  
}


void loop(){

   //Serial.println(pos);
  if(((millis()-timelastreceivedline)>1000*60*14)&&((millis()-timelastreceivedline)<((1000*60*14)+5000))) {beeping();} //Warning beep 5secs
  if((millis()-timelastreceivedline)>1000*60*15) {for(int i=0; i<16; i++){digitalWrite(solenoidsTemp[i], LOW);}  }  //15min without reset or line -> turn coils off

  running_lock=0; //enable interrupt function

  if(request_newline) {Serial.write(header); request_newline=false;}
  
  
  if((Serial.available() > 62)){
    if(Serial.readBytesUntil(footer, receivedBin, dataSize)){     //read into received bin till it finds footer or reaches datasize+1
      int checksum_calculated=0;
      for (int i=0; i<200;i++)
      {
          checksum_calculated=checksum_calculated+receivedBin[i];
        
      }
      if (checksum_calculated%256==receivedBin[201])  //Everthing OK
       {     
          dataReplace = true; 
          timelastreceivedline=millis();    
          beeping();
       }
      else if(checksum_calculated==0 && receivedBin[200]==199) //Reset condition
      {
         header=0; 
         dataReplace = false;
         char i;
         while (Serial.readBytes(&i,1) != 0) {} //flush pipe
         Serial.write(header);
         beeping();beeping();beeping();
      }
      
      else  //checksum is wrong
        {
          dataReplace = false;
          beeping();beeping();
          
          char i;
          while (Serial.readBytes(&i,1) != 0) {} //flush pipe
          
          if (checksum_calculated==126) //dirty fix for weird checksum bug
          { dataReplace=true; }
          else Serial.write(header);  //ask again
        
      }
       /*for(int i=0; i<202; i++){
         Serial.write(receivedBin[i]);
       } */
      
    }
    
  }

  if(dataReplace){
    for(int i=24; i<224; i++){pixelBin[i] = receivedBin[i-24];}
    header++;
    dataReplace = false;

  }

  analogWrite(10,10*digitalRead(enc3));

  zero = (analogRead(LEnd) > 500) ? 1 : 0;
  right = (analogRead(REnd) > 500) ? 1 : 0;

  //rotation data correction
   // if left end switch pushed

    if(zero != lastZero){
      if(zero == true){      
        if(carDirection == 2){
//          pos = 27;
            pos = 30;
            if (phase==transform_pinstate(digitalRead(enc3))) {beeping();beeping();phase=transform_pinstate_inv(digitalRead(enc3));} //inverse than in service manual?!
        }
      } 
    }

    // if right end switch pushed
    if(right != lastRight){
      if(right == true){    
        if(carDirection == 1){
//          pos = 228;// lower than 225 doesnt works.
            pos = 225;
            if (phase==transform_pinstate_inv(digitalRead(enc3))) {beeping();beeping();phase=transform_pinstate(digitalRead(enc3));}//inverse than in service manual?!
        }
      } 
    }


  lastZero = zero;
  lastRight = right;
  
}


int transform_pinstate(boolean b) //this is so stupid, otherwise program crashes
{
  if (b == HIGH) return 1;
  else return 0;
}
int transform_pinstate_inv(boolean b)
{
  if (b == HIGH) return 0;
  else return 1;
}



void beeping()
{

  analogWrite(beep, val_beep);
  delay(50);
  analogWrite(beep, 0);
  delay(50);
  
}

void rotaryEncoder(){
  if (!running_lock){
  
  newline_lock++;
  
  encState2 = digitalRead(enc2);
  if(!encState2){
    if ((carDirection==2)&&(newline_lock>NEWLINE_LOCK_LIMIT)) { request_newline=true;  newline_lock=0;} //direction changed and lock limit has been reached
    carDirection = 1;
    pos++;
    
    out1();
      


  } 
  else if(encState2){
    if ((carDirection==1)&&(newline_lock>NEWLINE_LOCK_LIMIT)){ request_newline=true; newline_lock=0;}
    carDirection = 2;
    pos--;
    
      out2();
   } 
  if(pos <= 0) pos = 1;
  if (pos >254) pos=254;
  }
}


//solenoid output when carriage going to right
void out1(){


if(carriageMode == carriageK){
    if(pos > 15){
      if(pos<39){ //why does this make a difference?
        digitalWrite(solenoidsTemp[abs((pos+(8*phase))-8)%16], pixelBin[pos-16]); //-16
      }
      else if(pos>38){
        digitalWrite(solenoidsTemp[abs((pos-(8*phase))-8)%16], pixelBin[pos-16]); //-16
      }
   
    }
  }
}

//solenoid output when carriage going to left
void out2(){

  if(carriageMode == carriageK){
    if(pos < 256-8){
      if(pos<39){
        digitalWrite(solenoidsTemp[(pos+(8*phase))%16], pixelBin[pos+8]);
      }
      else if(pos>38){
        digitalWrite(solenoidsTemp[(pos-(8*phase))%16], pixelBin[pos+8]);
      }
 
    }
  }
}


