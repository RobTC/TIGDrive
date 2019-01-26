#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);    //set LCD output pins

//define stepper driver pins
const int stp = 11;    //can't use pin 10 with the SS LCD as it's the backlight control. 
//if it goes low, backlight turns off!
const int dir = 12; //counter-clockwise if motor's facing you, clockwise if motor is pointing vertically up, for right-handers.

//set up speed and direction values
int dirVal = 0; //boolean value of set direction, seems to work better as type int
char* dirString[] = {"CCW", "CW"};

float dia = 1.5; //0.25 to 4.5" diameter- unlikely to be welding anything above 2", realistically

volatile byte isrFlag = 0;

//BUTTONS
//define button values
const int btnUp = 0;
const int btnDn = 1;
const int btnL = 2;
const int btnR = 3;
const int btnSel = 4;
const int btnNone = 5;

//define button-reading variables
int btnVal = 5;
int adcIn = 0;

//declare button poll function
int readLcdButtons() {
  delay(90); //debounce delay, tuned experimentally. delay is fine as program shouldn't be doing anything else at this point anyway- a proper debounce would be better though
  adcIn = analogRead(0); //read value from pin A0

  /*threshold values confirmed by experimentation with button calibration sketch returning the following ADC read values:
   right: 0
   up: 143
   down: 328
   left: 504
   select: 741
   */

  if (adcIn > 1000) return btnNone;
  if (adcIn < 50)   return btnR;
  if (adcIn < 250)  return btnUp;
  if (adcIn < 450)  return btnDn;
  if (adcIn < 650)  return btnL;
  if (adcIn < 850)  return btnSel;

  return btnNone; //if it can't detect anything, return no button pressed
}

//-----------------------------------------------------------------------------------------------------------------------------------

void setup() {

  lcd.begin(16, 2);               // initialise LCD lib full-screen
  lcd.setCursor(0,0);             // set cursor position

  pinMode(stp, OUTPUT);           //initialise stepper pins
  pinMode(dir, OUTPUT);

  pinMode(2, INPUT_PULLUP); //enable pullup on interrupt pin (more GND pins available than 5V pins!)
  attachInterrupt(digitalPinToInterrupt(2), pedal, CHANGE); //initialise trigger pin for foot pedal

  lcd.print("Welcome to");  //welcome screen
  lcd.setCursor(0,1);
  lcd.print("TIGDrive");
  delay(1000);
  lcd.clear();

   lcd.print("This program");  //welcome screen
  lcd.setCursor(0,1);
  lcd.print("assumes .125in/s");
  delay(1000);
  lcd.clear();

  //INITIALIZE SCREEN
  lcd.setCursor(9,0);
  lcd.print("DIR> ");
  lcd.setCursor(13,0); //direction
  lcd.print(dirString[dirVal]);
  lcd.setCursor(0,0);
  lcd.print("DIA: "); //diameter of part
  lcd.setCursor(0,1);
  lcd.print(dia);
  lcd.setCursor(4,1);
  lcd.print("\" (");
  lcd.setCursor(7,1);
  lcd.print(60/((dia*3.141)*8));
  lcd.setCursor(11,1);
  lcd.print("RPM)");

  Serial.begin(9600); //set up debugging serial channel

}

void loop() {
  do {
    btnVal = readLcdButtons();      //continually read the buttons...
    if (isrFlag == 1) actuation(); //...unless the pedal gets depressed...
  } 
  while (btnVal==5); //...until a key is pressed

  switch (btnVal){
  case  btnL:
    {
      if (dia <= 0.3) {
       dia=0.3;
       lcd.setCursor(0,1);
       lcd.print(dia);
       break;
      }
      else {
       dia-=0.1;
       lcd.setCursor(0,1);
       lcd.print(dia);
       break;
      }
    }
  case  btnR:
   {
      if (dia >= 4.5) {
       dia=4.5;
       lcd.setCursor(0,1);
       lcd.print(dia);
       break;
      }
      else {
       dia+=0.1;
       lcd.setCursor(0,1);
       lcd.print(dia);
        break;
      }
   }
  case  btnUp:
   {
      if (dia >= 4.1) {
       dia=4.5;
       lcd.setCursor(0,1);
       lcd.print(dia);
       break;
      }
      else {
       dia+=0.5;
       lcd.setCursor(0,1);
       lcd.print(dia);
        break;
      }
   }
  case  btnDn:
   {
      if (dia <= 0.7) {
       dia=0.3;
       lcd.setCursor(0,1);
       lcd.print(dia);
       break;
      }
      else {
       dia-=0.5;
       lcd.setCursor(0,1);
       lcd.print(dia);
        break;
      }
   }
  case  btnSel:
   {
      if (dirVal == 0) dirVal = 1;
      else dirVal = 0;
      lcd.setCursor(13,0);
      lcd.print("   ");
      lcd.setCursor(13,0);
      lcd.print(dirString[dirVal]);
      break;
   }

  }  //end Switch

  lcd.setCursor(7,1);
  lcd.print(60/((dia*3.141)*8)); //refresh the diameter conversion after button push
  lcd.setCursor(11,1);
  lcd.print("RPM)");

} //end main loop

void pedal() { //PEDAL ACTIVATION INTERRUPT SERVICE ROUTINE
isrFlag = digitalRead(2);

if (isrFlag == LOW) isrFlag = 0;
if (isrFlag == HIGH) isrFlag = 1;
}

void actuation() { //function to drive the motor per the settings selected on the display
  digitalWrite(dir, dirVal); //set the direction for the controller

  float rpm = 60/((dia*3.141)*8); //assign the rpm we've been displaying to an actual variable for calculating with
  int steps = 400; //number of steps per rev (200 = no microstepping, 400 = 2x, 800 = 4x, 1600 = 8x) to balance motion smoothing with available torque
  int pulseDelay = 1000/((steps*rpm*3)/60); //milliseconds divided by steps per second to get millseconds per step (multiplied by 3 for the 3:1 gear reduction)
  
  do {
    digitalWrite(stp, HIGH); //fire motor driver step
    delay(pulseDelay); //pause period defined by required RPM
    digitalWrite(stp, LOW); //reset driver
  }
  while (isrFlag == true);
} //end actuation function
