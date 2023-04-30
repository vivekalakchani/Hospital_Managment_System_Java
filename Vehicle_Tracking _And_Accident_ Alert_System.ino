//included all the required libraries

#include<SoftwareSerial.h> // using GSM module
#include<LiquidCrystal.h> // Using LCD display

//------------------------------------------------------------
//GSM Module RX pin to Arduino 9
//GSM Module TX pin to Arduino 8

SoftwareSerial Serial1(8,9); 

//-------------------------------------------------------------
//GPS Module RX pin to Arduino 11
//GPS Module TX pin to Arduino 10

SoftwareSerial gps(10,11);

//--------------------------------------------------------------
//LCD Display RS pin to Arduino 2
//LCD Display E  pin to Arduino 3
//LCD Display D4 pin to Arduino 4
//LCD Display D5 pin to Arduino 5
//LCD Display D6 pin to Arduino 6
//LCD Display D7 pin to Arduino 7

LiquidCrystal lcd(2,3,4,5,6,7); 

//--------------------------------------------------------------
//Accelerometer Module

#define x A0  /* connect x_out of module to A0 of UNO board */
#define y A1  /* connect Y_out of module to A1 of UNO board */
#define z A2  /* connect Z_out of module to A2 of UNO board */

int xsample=0;
int ysample=0;
int zsample=0;

//----------------------------------------------------------------
//set the sensitivity of Accelerometer by putting min and max value

#define samples 10
#define minVal -50
#define MaxVal 50

//------------------------------------------------------------------
//declared various variables for calculations and storing data temporary

int i=0,k=0;
int  gps_status=0;
float latitude=0; 
float logitude=0;
String Speed="";                       
String gpsString="";
char *test="$GPRMC";

/*****************************************************************************************
 * initModule() function
 *****************************************************************************************/

// created a function void initModule(String cmd, char *res, int t) to initialize the GSM module and checking its response using AT commands.

void initModule(String cmd, char *res, int t)
{
  while(1)
  {
    Serial.println(cmd);
    Serial1.println(cmd);
    delay(100);
    while(Serial1.available()>0)
    {
       if(Serial1.find(res))
       {
        Serial.println(res);
        delay(t);
        return;
       }
       else
       {
        Serial.println("Error");
       }
    }
    delay(t);
  }
}

/*****************************************************************************************
 * setup() function
 *****************************************************************************************/

void setup() 
{
  Serial1.begin(9600); //Serial.println("SIM900A serial initialize");
  Serial.begin(9600);//Serial moniter initialize at 9600 bps
  lcd.begin(16,2);  //initialize lcd screen

//--------------------------------------------------------------

  lcd.print("Vehicle Accident"); //"Vehicle Accident" Print in LCD Display 
  lcd.setCursor(0,1); //Declare the lcd display position (0 is index number and 1 is the row number)
  lcd.print("Alert System");
  delay(2000);  //2 seconds delay
  lcd.clear();  //  Clear LCD disply
  lcd.print("Getting Ready");
  lcd.setCursor(0,1);
  lcd.print("...");
  delay(1000);
  Serial.println("Getting Ready");

//----------------------------------------------------------------
  //Check GSM Module
  
  initModule("AT","OK",1000); //Check GSM Module
  initModule("ATE1","OK",1000); //Echo ON
  initModule("AT+CPIN?","READY",1000); //Check SIM ready 
  initModule("AT+CMGF=1","OK",1000);   //SMS text mode  
  initModule("AT+CNMI=2,2,0,0,0","OK",1000);  // Decides how newly arrived SMS should be handled

  //--------------------------------------------------------------

  Serial.println("System Ready");
  lcd.clear();
  lcd.print("System");
  lcd.setCursor(0,1);
  lcd.print("Ready");
  delay(2000);
  lcd.clear();

  //----------------------------------------------------------------
  // Accelerometer calibration process
  // find the average values for the x-axis, y-axis, and z-axis
  // used these sample values to read changes in accelerometer axis when vehicle gets accident.  

  lcd.print("Sensing");
  lcd.setCursor(0,1);
  lcd.print("Accelerometer");

  for(int i=0;i<samples;i++)
  {
    xsample+=analogRead(x); 
    ysample+=analogRead(y);
    zsample+=analogRead(z);
  }
  xsample/=samples;
  ysample/=samples;
  zsample/=samples;
  Serial.println(xsample);
  Serial.println(ysample);
  Serial.println(zsample);
  delay(1000);
  
  lcd.clear();
  lcd.print("Getting GPS");
  lcd.setCursor(0,1);
  lcd.print("Ready");
  delay(2000);
  gps.begin(9600);
  get_gps(); //call gps function
  show_coordinate(); //call coordinate function
  delay(2000);
  lcd.clear();
  lcd.print("GPS is Ready");
  delay(1000);
  lcd.clear();
  lcd.print("System Ready");
  Serial.println("System Ready");
}

/*****************************************************************************************
 * loop() function
 *****************************************************************************************/
 /* read accelerometer axis values and done a calculation to extract changes with the help 
 of samples that are taken in Calibration. Now if any changes are more or less then defined 
 level then Arduino sends a message to the predefined number.*/

void loop() 
{
    int value1=analogRead(x); //analog read from X pin
    int value2=analogRead(y); //analog read from Y pin
    int value3=analogRead(z); //analog read from Z pin
    int xValue=xsample-value1;
    int yValue=ysample-value2;
    int zValue=zsample-value3;
    
    Serial.print("x=");
    Serial.println(xValue);
    Serial.print("y=");
    Serial.println(yValue);
    Serial.print("z=");
    Serial.println(zValue);
    if(xValue < minVal || xValue > MaxVal  || yValue < minVal || yValue > MaxVal  || zValue < minVal || zValue > MaxVal)
    {
      get_gps();
      show_coordinate();
      lcd.clear();
      lcd.print("Sending SMS ");
      Serial.println("Sending SMS");
      Send();
      Serial.println("SMS Sent");
      delay(2000);
      lcd.clear();
      lcd.print("System Ready");
    }       
}

/*****************************************************************************************
 * gpsEvent() function
 *****************************************************************************************/
//Function for getting GPS data with coordinates

void gpsEvent()
{
  gpsString="";
  while(1)
  {
   while (gps.available()>0)     //Serial incoming data from GPS  
   {
    char inChar = (char)gps.read();
     gpsString+= inChar;        //store incoming data from GPS to temparary string str[]     
     i++;
    // Serial.print(inChar);
     if (i < 7)                      
     {
      if(gpsString[i-1] != test[i-1])     //check for right string
      {
        i=0;
        gpsString="";
      }
     }
    if(inChar=='\r')
    {
     if(i>60)
     {
       gps_status=1;
       break;
     }
     else
     {
       i=0;
     }
    }
  }
   if(gps_status)
    break;
  }
}

/*****************************************************************************************
 * get_gps()function
 *****************************************************************************************/

void get_gps()
{
  lcd.clear();
  lcd.print("Loading GPS");
  lcd.setCursor(0,1);
  lcd.print("...");
   gps_status=0;
   int x=0;
   while(gps_status==0)
   {
    gpsEvent();
    int str_lenth=i;
    coordinate2dec();
    i=0;x=0;
    str_lenth=0;
   }
}

/*****************************************************************************************
 * show_coordinate()function
 *****************************************************************************************/
//showing coordinate on the LCD
 
void show_coordinate()
{
    lcd.clear();
    lcd.print("Lat:");
    lcd.print(latitude);
    lcd.setCursor(0,1);
    lcd.print("Log:");
    lcd.print(logitude);
    Serial.print("Latitude:");
    Serial.println(latitude);
    Serial.print("Longitude:");
    Serial.println(logitude);
     Serial.print("Speed(in knots)=");
    Serial.println(Speed);
    delay(2000);
    lcd.clear();
    lcd.print("Speed(Knots):");
    lcd.setCursor(0,1);
    lcd.print(Speed);
    
}

/*****************************************************************************************
 * coordinate2dec() function
 *****************************************************************************************/
/*Function for extracting data from GPS string and convert that data to
 decimal degree format from the decimal minutes format.*/

void coordinate2dec()
{
  String lat_degree="";
    for(i=20;i<=21;i++)       //extract latitude from string  
      lat_degree+=gpsString[i];
  String lat_minut="";
     for(i=22;i<=28;i++)         
      lat_minut+=gpsString[i];

  String log_degree="";
    for(i=32;i<=34;i++)       //extract longitude from string
      log_degree+=gpsString[i];
  String log_minut="";
    for(i=35;i<=41;i++)
      log_minut+=gpsString[i];
    
    Speed="";
    for(i=45;i<48;i++)           
      Speed+=gpsString[i];
      
     float minut= lat_minut.toFloat();
     minut=minut/60;
     float degree=lat_degree.toFloat();
     latitude=degree+minut;
     
     minut= log_minut.toFloat();
     minut=minut/60;
     degree=log_degree.toFloat();
     logitude=degree+minut;
}

/*****************************************************************************************
 * Send() function
 *****************************************************************************************/
 //sent message to phone
void Send()
{ 
   Serial1.println("AT");
   delay(500);
   serialPrint();
   Serial1.println("AT+CMGF=1");
   delay(500);
   serialPrint();
   Serial1.print("AT+CMGS=");
   Serial1.print('"');
   Serial1.print("+94713215698");   //emergency phone number with country code
   Serial1.println('"');
   delay(500);
   serialPrint();
   Serial1.println("Vehicle Accident");
   delay(500);
   serialPrint();
   Serial1.print("Latitude:");
   Serial1.println(latitude);
   delay(500);
   serialPrint();
   Serial1.print("longitude:");
   Serial1.println(logitude);
   delay(500);
   serialPrint();
   Serial1.print(" Speed:");
   Serial1.print(Speed);
   Serial1.println("Knots");
   delay(500);
   serialPrint();
   Serial1.print(" Google map:");
   serialPrint();
   Serial1.print("http://maps.google.com/maps?&z=15&mrt=yp&t=k&q="); //google map link
   Serial1.print(latitude,6);
   Serial1.print("+");              
   Serial1.print(logitude,6);
   Serial1.write(26);
   delay(2000);
   serialPrint();
}

/*****************************************************************************************
 * serialPrint() function
 *****************************************************************************************/
 // Read and print the message given by the gsm module
void serialPrint()
{
  while(Serial1.available()>0)  
  {
    Serial.print(Serial1.read()); 
  }
}