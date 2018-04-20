// From https://web.archive.org/web/20161223060411/http://www.engscope.com/pic24-tutorial/10-2-i2c-basic-functions/

#include "i2c_reg.h"

void delay_by_nop(uint32_t num_nops){    // 1 nop= 375ns
    uint32_t count = 0;
    while (count < num_nops){
        __asm__("nop");
        count +=1;
    }
}

// initiates I2C3 module to baud rate BRG
void i2c_init(int BRG){      // I2CBRG = 157 for 16Mhz OSC with 100kHz I2C clock
   int temp;
   I2C3BRG = BRG;           // Sets freq to 100kHz
   I2C3CONbits.I2CEN = 0;	// Disable I2C Mode
   I2C3CONbits.DISSLW = 1;	// Disable slew rate control
   IFS5bits.MI2C3IF = 0;	 // Clear Interrupt
   I2C3CONbits.I2CEN = 1;	// Enable I2C Mode
   temp = I2C3RCV;	 // read buffer to clear buffer full
   reset_i2c_bus();	 // set bus to idle
}

//function iniates a start condition on bus
void i2c_start(void){
   int x = 0;
   I2C3CONbits.ACKDT = 0;	//Reset any previous Ack
   delay_by_nop(10);
   I2C3CONbits.SEN = 1;	//Initiate Start condition
   __asm__("nop");

   while (I2C3CONbits.SEN){    //the hardware will automatically clear Start Bit
      delay_by_nop(1);      //wait for automatic clear before proceding
      x++;
      if (x > 20)
      break;
   }
   delay_by_nop(2);
}

void i2c_restart(void){
   int x = 0;
   I2C3CONbits.RSEN = 1;	//Initiate restart condition
   __asm__("nop");
   //the hardware will automatically clear restart bit
   //wait for automatic clear before proceding
   while (I2C3CONbits.RSEN){
      delay_by_nop(1);
      x++;
      if (x > 20)	break;
   }
   delay_by_nop(2);
}


//Resets the I2C bus to Idle
void reset_i2c_bus(void){
   int x = 0;
   I2C3CONbits.PEN = 1;     //initiate stop bit
   while (I2C3CONbits.PEN) {     //wait for hardware clear of stop bit
      // delay_by_nop(1);
      delay_by_nop(1);
      x ++;
      if (x > 20) break;
   }

   I2C3CONbits.RCEN = 0;
   IFS5bits.MI2C3IF = 0; // Clear Interrupt
   I2C3STATbits.IWCOL = 0;
   I2C3STATbits.BCL = 0;
   // delay_by_nop(10);
   delay_by_nop(20);
}


//basic I2C byte send
char send_i2c_byte(int data){
   int i;
   while (I2C3STATbits.TBF) { }
   IFS5bits.MI2C3IF = 0; // Clear Interrupt
   I2C3TRN = data; // load the outgoing data byte

   for (i=0; i<500; i++){           // wait for transmission
      if (!I2C3STATbits.TRSTAT) break; // if master transmit not in progress break
      delay_by_nop(1); }

    if (i == 500) {     // if i got to 500, then there was an issue`
        return(1); }    // return 1 to indicate failure

   if (I2C3STATbits.ACKSTAT == 1){   // Check for NO_ACK from slave,
      reset_i2c_bus();
      return(1); }            // abort if not found

   delay_by_nop(2);
   return(0);
}


//function reads data, returns the read data, no ack
// helper function
char i2c_read(void){
   int i = 0;
   char data = 0;
   I2C3CONbits.RCEN = 1;            //set I2C module to receive

   while (!I2C3STATbits.RBF) {           //if no response, break
      i ++;
      if (i > 2000) break;  }

   data = I2C3RCV;       //get data from I2C3RCV register
   return data;
}


//function reads data, returns the read data, with ack
char i2c_read_ack(void){	//does not reset bus!!!
   int i = 0;
   char data = 0;
   I2C3CONbits.RCEN = 1;            //set I2C module to receive

   while (!I2C3STATbits.RBF) {       //if no response, break
      i++;
      if (i > 2000) break;}

   data = I2C3RCV;               //get data from I2C3RCV register
   I2C3CONbits.ACKEN = 1;        //start ACK generation
   delay_by_nop(10);             //wait before exiting
   return data;
}

char i2c_read_nack(void){	//does not reset bus!!!
   int i = 0;
   char data = 0;
   I2C3CONbits.RCEN = 1;            //set I2C module to receive

   while (!I2C3STATbits.RBF) {       //if no response, break
      i++;
      if (i > 2000) break;}

   data = I2C3RCV;               //get data from I2C3RCV register
   I2C3CONbits.ACKDT = 1;        // set mastrer to nack instead of ack
   I2C3CONbits.ACKEN = 1;        //start ack generation
   delay_by_nop(10);             //wait before exiting
   return data;
}

// function puts together I2C protocol for random write
void I2Cwrite(char addr, char subaddr, char value){
   i2c_start();
   send_i2c_byte(addr & 0xfffe); // set /W bit, turns least sig, bit to 0
   send_i2c_byte(subaddr);
   send_i2c_byte(value);
   reset_i2c_bus();
}

// function puts together I2C protocol for random read
char I2Cread(char addr, char subaddr){
   char temp;
   i2c_start();
   send_i2c_byte(addr);
   send_i2c_byte(subaddr);
   delay_by_nop(10);
   i2c_restart();

   send_i2c_byte(addr | 0x01); // set R bit, turn least sig. bit to 1
   temp = i2c_read();

   reset_i2c_bus();
   return temp;
}

// function checks if device at addr is on bus
// 1 means it is not there, 0 means it is there
unsigned char I2Cpoll(char addr){
   unsigned char temp = 0;
   i2c_start();
   temp = send_i2c_byte((addr) & (0xfffe)); // set /W bit
   reset_i2c_bus();
   return temp;
}

// function writes a byte array over i2c
unsigned char I2Cwritearray(char addr, char dat[], uint8_t len) {
  unsigned char err = 0;
  i2c_start();
  err = send_i2c_byte(addr & (0xfffe)); // takes 7 bit address and adds /W bit
  uint8_t j;
  for (j = 0; j < len; j++) { // keep sending data bytes til we're done unless it misses an ack
    err |= send_i2c_byte(dat[j]);
  }
  reset_i2c_bus();
  return err;
}

////////////////////////////////////////////////////////////////////////////////
// I2C2 ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// initiates I2C3 module to baud rate BRG
void i2c2_init(int BRG){      // I2CBRG = 157 for 16Mhz OSC with 100kHz I2C clock
   int temp;
   I2C2BRG = BRG;           // Sets freq to 100kHz
   I2C2CONbits.I2CEN = 0;	// Disable I2C Mode
   I2C2CONbits.DISSLW = 1;	// Disable slew rate control
   IFS3bits.MI2C2IF = 0;	 // Clear Interrupt
   I2C2CONbits.I2CEN = 1;	// Enable I2C Mode
   temp = I2C2RCV;	 // read buffer to clear buffer full
   reset_i2c_bus();	 // set bus to idle
}

//function iniates a start condition on bus
void i2c2_start(void){
   int x = 0;
   I2C2CONbits.ACKDT = 0;	//Reset any previous Ack
   delay_by_nop(10);
   I2C2CONbits.SEN = 1;	//Initiate Start condition
   __asm__("nop");

   while (I2C2CONbits.SEN){    //the hardware will automatically clear Start Bit
      delay_by_nop(1);      //wait for automatic clear before proceding
      x++;
      if (x > 20)
      break;
   }
   delay_by_nop(2);
}

void i2c2_restart(void){
   int x = 0;
   I2C2CONbits.RSEN = 1;	//Initiate restart condition
   __asm__("nop");
   //the hardware will automatically clear restart bit
   //wait for automatic clear before proceding
   while (I2C2CONbits.RSEN){
      delay_by_nop(1);
      x++;
      if (x > 20)	break;
   }
   delay_by_nop(2);
}


//Resets the I2C bus to Idle
void reset_i2c2_bus(void){
   int x = 0;
   I2C2CONbits.PEN = 1;     //initiate stop bit
   while (I2C2CONbits.PEN) {     //wait for hardware clear of stop bit
      // delay_by_nop(1);
      delay_by_nop(1);
      x ++;
      if (x > 20) break;
   }

   I2C2CONbits.RCEN = 0;
   IFS3bits.MI2C2IF = 0; // Clear Interrupt
   I2C2STATbits.IWCOL = 0;
   I2C2STATbits.BCL = 0;
   // delay_by_nop(10);
   delay_by_nop(20);
}


//basic I2C byte send
char send_i2c2_byte(int data){
   int i;
   while (I2C2STATbits.TBF) { }
   IFS3bits.MI2C2IF = 0; // Clear Interrupt
   I2C2TRN = data; // load the outgoing data byte

   for (i=0; i<500; i++){           // wait for transmission
      if (!I2C2STATbits.TRSTAT) break; // if master transmit not in progress break
      delay_by_nop(1); }

    if (i == 500) {     // if i got to 500, then there was an issue`
        return(1); }    // return 1 to indicate failure

   if (I2C2STATbits.ACKSTAT == 1){   // Check for NO_ACK from slave,
      reset_i2c2_bus();
      return(1); }            // abort if not found

   delay_by_nop(2);
   return(0);
}


//function reads data, returns the read data, no ack
// helper function
char i2c2_read(void){
   int i = 0;
   char data = 0;
   I2C2CONbits.RCEN = 1;            //set I2C module to receive

   while (!I2C2STATbits.RBF) {           //if no response, break
      i ++;
      if (i > 2000) break;  }

   data = I2C2RCV;       //get data from I2C2RCV register
   return data;
}


//function reads data, returns the read data, with ack
char i2c2_read_ack(void){	//does not reset bus!!!
   int i = 0;
   char data = 0;
   I2C2CONbits.RCEN = 1;            //set I2C module to receive

   while (!I2C2STATbits.RBF) {       //if no response, break
      i++;
      if (i > 2000) break;}

   data = I2C2RCV;               //get data from I2C2RCV register
   I2C2CONbits.ACKEN = 1;        //start ACK generation
   delay_by_nop(10);             //wait before exiting
   return data;
}

char i2c2_read_nack(void){	//does not reset bus!!!
   int i = 0;
   char data = 0;
   I2C2CONbits.RCEN = 1;            //set I2C module to receive

   while (!I2C2STATbits.RBF) {       //if no response, break
      i++;
      if (i > 2000) break;}

   data = I2C2RCV;               //get data from I2C2RCV register
   I2C2CONbits.ACKDT = 1;        // set mastrer to nack instead of ack
   I2C2CONbits.ACKEN = 1;        //start ack generation
   delay_by_nop(10);             //wait before exiting
   return data;
}

// function checks if device at addr is on bus
// 1 means it is not there, 0 means it is there
unsigned char I2C2poll(char addr){
   unsigned char temp = 0;
   i2c2_start();
   temp = send_i2c2_byte((addr) & (0xfffe)); // set /W bit
   reset_i2c2_bus();
   return temp;
}
