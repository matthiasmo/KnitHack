/*
Brother KH970 Controller
 2014 January
 So Kanno

Changes for stitching worlds by Matthias Mold:
+specific pinconfig for our board
+added checksum
+no need to go to the end switches
 */

char receivedBin[201];
int pixelBin[256] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

int dataSize = 202;
boolean dataReplace = false;
boolean data_valid=false;
volatile boolean request_newline=true;
int header = 0;
byte footer = 126;
int columnNum = 0;
byte carriageMode = 124;
byte carriageK = 124;
byte carriageL = 125;

byte newline_lock=0;   //locks the request for a newline, counts stitches since newline event
#define NEWLINE_LOCK_LIMIT 10  //dont accept new line until NEWLINE_LOCK_LIMIT stitches made;this means minimal width is 10 stitches

int phase = 0;

//INPUT SYSTEM
const int enc1 = 2;  //encoder 1
const int enc2 = 3;  //encoder 2
const int enc3 = 4;  //phase encoder
const int LEnd = 1;   //endLineLeft for analog in
const int REnd = 0;   //endLineRight for analog in

//OUTPUT SYSTEM
const int LED = 13;

//this is for kh930
//int solenoidsTemp[16] = 
//{
 // 22,24,26,28,30,32,34,36,37,35,33,31,29,27,25,23};
//for my kh930
//int solenoidsTemp[16] = 
//{22,24,26,28,30,32,34,36,23,25,27,29,31,33,35,37};
//test error fix
//int solenoidsTemp[16] = {36,34,32,30,28,26,24,22,37,35,33,31,29,27,25,23};
//int solenoidsTemp[16] = {23,25,27,29,31,33,35,37,22,24,26,28,30,32,34,36};
//int solenoidsTemp[16] = {22,24,26,28,30,32,34,36,23,25,27,29,31,33,35,37}; //solenoids look in order
int solenoidsTemp[16] = {36,34,32,30,28,26,24,22,37,35,33,31,29,27,25,23};  //this finally works!
//int solenoidsTemp[16] = {37,35,33,31,29,27,25,23,36,34,32,30,28,26,24,22}; //from shematic and service manual
//int solenoidsTemp[16] = {37,35,33,31,29,27,25,23,36,34,32,30,28,26,24,22};
//for CK35
//int solenoidsTemp[16] = 
//{
//  22,24,26,28,30,32,34,36,33,31,29,27,25,23,35,37};

int pos = 0;  //position of carriage
int lastPos = 0;
int encState2 = 0;  //encoder 2 state

int zero = 0;       //left end switch value
int lastZero = 0;   
int right = 0;      //right end switch value
int lastRight = 0;  
int barSwitch = 0;  //row counter value
int lastBarSwitch = 0;  
int barCounter = 0;    //current row count
int carDirection = 0;  //direction of carriage　0:unknown　1:right　2:left

int pos_last=0;
boolean sendFlag;

void setup(){
  pinMode(LED, OUTPUT);
  pinMode(enc1, INPUT);
  pinMode(enc2, INPUT);
  pinMode(enc3, INPUT);

  for(int i=22; i<38; i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  attachInterrupt(enc1, rotaryEncoder, RISING);
  Serial.begin(57600);
  
  if(digitalRead(enc3)==false){  //phase ditection
    phase = 1;
  }  
}


void loop(){

  //  Serial.println(pos);

if (!(pos_last==pos)) {pos_last=pos;Serial.print(pos); Serial.print(":");Serial.print(digitalRead(enc1));Serial.print(",");Serial.print(digitalRead(enc2));Serial.print(",");Serial.print(digitalRead(enc3));Serial.print(" Left Sens:");Serial.print(analogRead(LEnd));Serial.print(" Right Sens:");Serial.print(analogRead(REnd));Serial.println();}



if(request_newline) {Serial.write(header); request_newline=false;}

  if(Serial.available() > 62){
    if(Serial.readBytesUntil(footer, receivedBin, dataSize+1)){
      Serial.write(receivedBin);
      int checksum_calculated=0;
      for (int i=0; i<200;i++)
      {
          checksum_calculated=checksum_calculated+receivedBin[i];
        
      }
      if (checksum_calculated%256==receivedBin[201])
       {     
          dataReplace = true;     
          
       }
        else
        {
          dataReplace = false;
          
          Serial.write(header);
        }
      // for(int i=0; i<200; i++){
      //   Serial.write(receivedBin[i]);
      // }
      
    }
  }

  if(dataReplace){
    digitalWrite(13, HIGH);  //beep
    for(int i=24; i<225; i++){
      if(i < 224){
        pixelBin[i] = receivedBin[i-24];
      }
      else if(i == 224){
        carriageMode = receivedBin[i-24];
      }
    }
    header++;
    dataReplace = false;
    columnNum++;
    digitalWrite(13, LOW);
  }

  zero = (analogRead(LEnd) > 460) ? 1 : 0;
  right = (analogRead(REnd) > 460) ? 1 : 0;

  //rotation data correction
   // if left end switch pushed
//  if(carriageMode == carriageK){
    if(zero != lastZero){
      if(zero == true){      
        if(carDirection == 2){
//          pos = 27;
            pos = 30;
        }
      } 
    }

    // if right end switch pushed
    if(right != lastRight){
      if(right == true){    
        if(carDirection == 1){
//          pos = 228;// lower than 225 doesnt works.
            pos = 225;
        }
      } 
    }
//  }

  lastZero = zero;
  lastRight = right;
  
}

void rotaryEncoder(){
  newline_lock++;
  
  encState2 = digitalRead(enc2);
  if(!encState2){
    if ((carDirection==2)&&(newline_lock>NEWLINE_LOCK_LIMIT)) { request_newline=true;  newline_lock=0;} //direction changed and lock limit has been reached
    carDirection = 1;
    pos++;
    if(pos != 255){
      sendFlag = true;
      out1();
      
    }
    /*else if(pos == 255 && sendFlag){
      Serial.write(header);
      sendFlag = false; 
    }*/
  } 
  else if(encState2){
    if ((carDirection==1)&&(newline_lock>NEWLINE_LOCK_LIMIT)){ request_newline=true; newline_lock=0;}
    carDirection = 2;
    pos--;
    if(pos != 1){
      sendFlag = true;
      out2();
      
    }
    /*else if(pos == 1 && sendFlag){
      Serial.write(header);
      sendFlag = false;
    }*/
  } 
  if(pos < 0) pos = 0;
  
  
}


//solenoid output when carriage going to right
void out1(){
 // digitalWrite(LED, pixelBin[pos]);

  if(carriageMode == carriageL){
    if(pos > 15){
      if(pos<39){
        digitalWrite(solenoidsTemp[abs((pos+(8*phase))-8)%16], pixelBin[pos+1]);
      }
      else if(pos>38){
        digitalWrite(solenoidsTemp[abs((pos-(8*phase))-8)%16], pixelBin[pos+1]);
      }
      // digitalWrite(abs(pos-8)%16+31,pixelBin[pos+1]);    
    }
  }
  else if(carriageMode == carriageK){
    if(pos > 15){
      if(pos<100){
        for(int i=0;i<16;i++) {digitalWrite(solenoidsTemp[i], HIGH);}
      }
      else if(pos>100){
        for(int i=0;i<16;i++) {digitalWrite(solenoidsTemp[i], LOW);}
      }
      
      // digitalWrite(abs(pos-8)%16+31,pixelBin[pos-16]);    
    }
  }
}

//solenoid output when carriage going to left
void out2(){
  //digitalWrite(LED, pixelBin[pos]);
  if(carriageMode == carriageL){
    if(pos < 256-8){
      if(pos<39){
        digitalWrite(solenoidsTemp[(pos+(8*phase))%16], pixelBin[pos+1]);
      }
      else if(pos>38){
        digitalWrite(solenoidsTemp[(pos-(8*phase))%16], pixelBin[pos+1]);
      }
      // digitalWrite((pos)%16+31,pixelBin[pos+1]);    
    }
  }
  else if(carriageMode == carriageK){
    if(pos < 256-8){
      if(pos<100){
        for(int i=0;i<16;i++) {digitalWrite(solenoidsTemp[i], LOW);}
      }
      else if(pos>100){
        for(int i=0;i<16;i++) {digitalWrite(solenoidsTemp[i], HIGH);}
      }
      // digitalWrite((pos)%16+31,pixelBin[pos+8]);    
    }
  }
}

