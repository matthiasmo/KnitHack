public void change_mode(){
  if(carriageMode == carriageK){
    carriageMode = carriageL;
    ControlFont cfont = new ControlFont(pfont, 16);   
    // cp5.addButton("Mesh_rev")
    //   .setPosition(850, 541)
    //     .setSize(120, 30);
    // cp5.getController("Mesh_rev")
    //   .getCaptionLabel()
    //     .setFont(cfont)
    //       .setSize(16);
    // cp5.addButton("Mesh_Phase")
    //   .setPosition(990, 541)
    //     .setSize(120, 30);
    // cp5.getController("Mesh_Phase")
    //   .getCaptionLabel()
    //     .setFont(cfont)
    //       .setSize(16);
  }
  else if(carriageMode == carriageL){
    carriageMode = carriageK;
    cp5.remove("Mesh_rev");
  }
}

public void Mesh_rev(){
  meshSwitch = !meshSwitch;
}

public void Mesh_Phase(){
  meshPhase = !meshPhase;
}

public void Reset(int theValue) {
  header = 0;
    
    
    for (int i=0;i<200;i++) {port.write(0);} //200 zeros
   // port.write(carriageMode);
    port.write(199); //fake magic checksum
    port.write(footer);
    //Reset condition

  print("Reset sent");
  for (int i=0; i<row; i++) {
    sendStatus[i][0] = false;
  }
  reset.trigger();
}

public void Connect() {
  String portName = Serial.list()[0];  //WTF SELECT PORT HERE!!!!
  println(Serial.list());
  print("chosen: "); println(portName);
  port = new Serial(this, portName, 57600);
  port.clear();
  done.trigger();
  cp5.remove("Connect");
  ControlFont cfont = new ControlFont(pfont, 16); 

  cp5.addButton("Send_to_KnittingMachine")
    .setPosition(850, 641)
      .setSize(260, 30);
  cp5.getController("Send_to_KnittingMachine")
    .getCaptionLabel()
      .setFont(cfont)
        .setSize(16);
}

// void serialEvent(Serial p) {
//   int a = p.read();
//   println(a);
// }

public void Send_to_KnittingMachine(int theValue) {
  //sending pixelBin[][] to knitting Machine! 
  
  send_data();
  
  sendStatus[header][0] = true;
  header++;
  ready.trigger();
}

void serialEvent(Serial p) {

  
  header = p.read();
  //draw();
  print(header);
  println(" requested");
  header = int(header);
  
  send_data();
  
    sendStatus[header][0] = true;
    completeFlag = false;
    sent.trigger();
 
  }

 void send_data()
 {
    int checksum=0;
   
  // print("next is ");
  //print(header);
  //print(": ");
  if (header < row) {
    for (int i=0; i<maxColumn; i++) {
      port.write(displayBin[header][i]);
   //   print(displayBin[header][i]);
      checksum=checksum+displayBin[header][i];
    }
   // println();//checksum%265);
    port.write(carriageMode);
    port.write(char(checksum%256));
    port.write(footer);
    print(header);
    println(" sent");
 }
 }
 
 





