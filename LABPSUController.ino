//LABPSUController.ino

#include <Arduino.h>
#include <Wire.h>
#include <MicroLCD.h>
#include <SPI.h>
#include <Encoder.h>
#include <Bounce2.h>
#include <SimpleTimer.h>
#include <DallasTemperature.h>
#include <OneWire.h>


#define NDEBUG                        // enable local debugging information


#define PSUFAN_PIN 3 
#define CONVFAN_PIN 5 //6,9 
#define HALL_PIN 2 //6,9 
#define VOLT_PIN A3 //6,9
#define AMPERMETER_PIN A2 //6,9

#define TEMPERATURE_PIN                   A1

LCD_SSD1306 lcd; /* for SSD1306 OLED module */

#define HMC5883L  0xB8

///////////////

volatile unsigned int  NbTopsFan; 


int Calc;

                                  //The pin location of the sensor
int hallsensor = 2;


float vout = 0.0;
float vin = 0.0;
float R1 = 100900.0; // resistance of R1 (100K) -see text!
float R2 = 9970.0; // resistance of R2 (10K) - see text!
int voltageraw = 0;
                        
//typedef struct{                  //Defines the structure for multiple fans and their dividers
//  char fantype;
//  unsigned int fandiv;
//}fanspec;

//Definitions of the fans
//fanspec fanspace[3]={{0,1},{1,2},{2,8}};

//char fan = 1;   //This is the varible used to select the fan and it's divider, set 1 for unipole hall effect sensor 
               //and 2 for bipole hall effect sensor 

//////////////

double Amps = 0;
int VQ=512;


byte bLastPSURPM = 0;

SimpleTimer PSUTimer;
SimpleTimer BoardFanTimer;
SimpleTimer TempTimer;
//SimpleTimer VoltAmperTimer;


OneWire oneWire(TEMPERATURE_PIN); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 

void rpm ()      //This is the function that the interupt calls 
{ 
 NbTopsFan++; 
} 


void checkTemp()
{

	    sensors.requestTemperatures();

        float temp1 =0, temp2 = 0;
        float maxtemp = 0;
        byte rpm=0;
        byte psuTemp=0;
        byte psuRPM=0;

        temp1 = static_cast<float>(static_cast<int> (sensors.getTempCByIndex(0) * 10.)) / 10.; // Temp sensor relay

        if ( temp1 == -127 || temp1 == 85 )
        {

        	checkTemp();	
        }

#ifdef NDEBUG 
  		Serial.print("Board temp1: : "); // print em out
  		Serial.println(temp1, DEC); // print em out
#endif

        temp2 = static_cast<float>(static_cast<int> (sensors.getTempCByIndex(1) * 10.)) / 10.; // Temp sensor relay

        if ( temp2 == -127 || temp2 == 85 )
        {

        	checkTemp();	
        }


#ifdef NDEBUG 
  		Serial.print("Board temp2: : "); // print em out
  		Serial.println(temp2, DEC); // print em out
#endif

if (temp1 > temp2)
{
	maxtemp = temp1;
}
else
{
maxtemp=temp2;
}


        lcd.setFont(FONT_SIZE_SMALL);
        lcd.setCursor(62, 6);   
        lcd.print( "   " );
        lcd.setCursor(62, 6);         
        lcd.print(maxtemp, 0 ); // 


if (maxtemp > 40 )
{

rpm = map(maxtemp, 40, 90, 35, 254);

if (rpm < 50)
{

	analogWrite(CONVFAN_PIN, 100);
	delay(2000);
}

analogWrite(CONVFAN_PIN, rpm);

}
else
{

analogWrite(CONVFAN_PIN, 0);

}


		byte brpmPercentCNV = map(rpm, 35, 254, 0, 100);
		byte bpercentPosCNV=0;

 	       lcd.setCursor(104, 6);  
			lcd.print( "    " );
 	       lcd.setCursor(104, 6);  

		if (rpm > 35)
		{ 
 
  	      lcd.print( brpmPercentCNV ); 

			if (brpmPercentCNV <10)
			{
				bpercentPosCNV = 111;
			}
			else if (brpmPercentCNV >=10 && brpmPercentCNV <100)
			{
				bpercentPosCNV = 116;
			}
			else
			{
						bpercentPosCNV = 120;
			}

     		   lcd.setCursor(bpercentPosCNV, 6);   
    		    lcd.print( "%" ); // 

  	  	}
  	  	else
  	  	{
  	      lcd.print( "OFF" ); 
  	  	}





//check PSU temp

Wire.beginTransmission(HMC5883L); // start transmission
Wire.write(0x09); // Temperature
Wire.endTransmission(false); // don't send a stop condition
int l=Wire.requestFrom((uint8_t)HMC5883L, (uint8_t)1,true); // request expected number of bytes to be returned
while(l>0 && Wire.available()) // loop all bytes if any are available
{

psuTemp = Wire.read();

#ifdef NDEBUG 
  Serial.print("PSU Temp: "); // print em out
  Serial.println(psuTemp, DEC); // print em out
#endif

}

        lcd.setFont(FONT_SIZE_SMALL);
        lcd.setCursor(62, 0);
        lcd.print( "  " );   
        lcd.setCursor(62, 0);                
        lcd.print(psuTemp, DEC ); // 

if ( psuTemp < 0x28 ) // <40
{

  analogWrite(PSUFAN_PIN, 180);
}
else
{

//psuRPM = map(psuTemp, 0x28, 0x41, 180, 254);
psuRPM = map(psuTemp, 40, 80, 180, 254);

if (bLastPSURPM != psuRPM)
{
  analogWrite(PSUFAN_PIN, psuRPM);
  bLastPSURPM = psuRPM;
}


}
    byte brpmPercent = map(psuRPM, 180, 254, 5, 100);
		//byte brpmPercent = map(psuRPM, 130, 254, 5, 100);
		byte bpercentPos=0;

        lcd.setCursor(104, 0);  
        lcd.print( "   " ); 
        lcd.setCursor(104, 0);                   
        lcd.print( brpmPercent ); // 




#ifdef NDEBUG 
  Serial.print("New PSU RPM: "); // print em out
  Serial.println(psuRPM, DEC); // print em out
#endif

	if (brpmPercent <10)
	{
		bpercentPos = 111;
	}
	else if (brpmPercent >=10 && brpmPercent <100)
	{
		bpercentPos = 116;
	}
	else
	{
				bpercentPos = 120;
	}

        lcd.setCursor(bpercentPos, 0);   
        lcd.print( "%" ); // 


checkVolt();

}

void checkVolt()
{

   voltageraw = analogRead(VOLT_PIN);
   vout = (voltageraw * 5.024) / 1024.0; // see text
   vin = vout / (R2/(R1+R2)); 
   if (vin<0.09) {
   vin=0.0;//statement to quash undesired reading !
				}

        lcd.setFont(FONT_SIZE_SMALL);
        lcd.setCursor(62, 2);   
        lcd.print( "     " );
        lcd.setCursor(62, 2); 
        lcd.print(vin, 2 ); 


#ifdef NDEBUG   
  Serial.print("Voltage: "); // print em out
  Serial.println(vin, DEC); // print em out
#endif

float amps=abs(readCurrent(AMPERMETER_PIN));

        lcd.setFont(FONT_SIZE_SMALL);
        lcd.setCursor(62, 4);   
        lcd.print( "     " );
        lcd.setCursor(62, 4); 
        lcd.print(amps, 2 ); 


#ifdef NDEBUG   
  Serial.print("Current: "); // print em out
  Serial.println(amps, DEC); // print em out
#endif

}

void checkPSU ()
{

unsigned int psuFanRPM=0;
byte speed=0;


Wire.beginTransmission(HMC5883L); // start transmission
Wire.write(0x0B); // Temperature
Wire.endTransmission(false); // don't send a stop condition
int l=Wire.requestFrom((uint8_t)HMC5883L, (uint8_t)1,true); // request expected number of bytes to be returned
while(l>0 && Wire.available()) // loop all bytes if any are available
{

#ifdef NDEBUG 
  Serial.print("DC Good: "); // print em out
  Serial.println(Wire.read(), DEC); // print em out
#endif

}

 
//RPM = (1/0.262) *(Fan Pulse Count * 60 /2)

Wire.beginTransmission(HMC5883L); // start transmission
Wire.write(0x0A); // Fan speed
Wire.endTransmission(false); // don't send a stop condition
int m=Wire.requestFrom((uint8_t)HMC5883L, (uint8_t)1,true); // request expected number of bytes to be returned
while(m>0 && Wire.available()) // loop all bytes if any are available
{

speed = Wire.read();
//psuFanRPM = (1/0.262) * (speed * 60  /2 ); //
psuFanRPM = (speed  *  60 ) ; //
  //Serial.println(Wire.read(), HEX); // print em out

#ifdef NDEBUG 
  Serial.print("PSU Fan RPM: "); // print em out  
  Serial.print(psuFanRPM, DEC); // print em out
  Serial.print("     val: : "); // print em out  
  Serial.println(speed, DEC); // print em out
#endif

}




} // checkPSU



void checkBoardFan ()
{

showRPM();

} //checkBoardFan


void setup() {

  		lcd.begin(); 
  		lcd.setFont(FONT_SIZE_MEDIUM);
        lcd.setCursor(0, 3);   
        lcd.print("Checking unit." ); // 



 // put your setup code here, to run once:
  pinMode(PSUFAN_PIN, OUTPUT);  
  analogWrite(PSUFAN_PIN, 254);
  delay(2000);      
  analogWrite(PSUFAN_PIN, 130);

    pinMode(CONVFAN_PIN, OUTPUT);
    analogWrite(CONVFAN_PIN, 254);
    delay(2000);      
    analogWrite(CONVFAN_PIN, 0);

  pinMode(AMPERMETER_PIN, INPUT); 


    lcd.clear();

        lcd.setFont(FONT_SIZE_MEDIUM);
        lcd.setCursor(0, 0);   
//        lcd.backlight(FALSE);   
        lcd.print("PSU" ); // 
        lcd.setFont(FONT_SIZE_SMALL);
        lcd.setCursor(32, 0);   
        lcd.print("TEMP:" ); // 
        lcd.setCursor(80, 0);   
        lcd.print("FAN:" ); // 
 
       lcd.setCursor(32, 2);   
        lcd.print("VOUT:" ); // 

       lcd.setCursor(32, 4);   
        lcd.print("CURR:" ); // 

        lcd.setFont(FONT_SIZE_MEDIUM);
        lcd.setCursor(0, 6);   
        lcd.print("CNV" ); // 
        lcd.setFont(FONT_SIZE_SMALL);
        lcd.setCursor(32, 6);   
       lcd.print("TEMP:" ); // 
       lcd.setCursor(80, 6);   
        lcd.print("FAN:" ); // 

 // Wire.begin();

 pinMode(VOLT_PIN, INPUT); 
 pinMode(HALL_PIN, INPUT); 
 digitalWrite(HALL_PIN, HIGH); 
 


 attachInterrupt(0, rpm, CHANGE); 

        Serial.begin(115200);


          //delay(500);      
    //analogWrite(CONVFAN_PIN, 35);

   
//TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00); //disable FastPWM on Timer0. mills lasts twice longer


PSUTimer.setInterval(10000, checkPSU);
BoardFanTimer.setInterval(10000, checkBoardFan);
//VoltAmperTimer.setInterval(15000, checkVolt);
TempTimer.setInterval(5000, checkTemp);

}



void loop() {


  PSUTimer.run();
  BoardFanTimer.run();
  //VoltAmperTimer.run();
  TempTimer.run();


        
}


void showRPM()
{

   NbTopsFan = 0;	//Set NbTops to 0 ready for calculations
   


   //sei();		//Enables interrupts
   delay (500);	//Wait 1 second
   //cli();		//Disable interrupts
   //Calc = ((NbTopsFan * 60)/fanspace[fan].fandiv); //Times NbTopsFan (which is apprioxiamately the fequency the fan is spinning at) by 60 seconds before dividing by the fan's divider
  

  Calc = (NbTopsFan*60)/2;
  Serial.print (Calc, DEC); //Prints the number calculated above
  Serial.print (" rpm"); //Prints " rpm" and a new line
  Serial.print ("       val: "); //Prints " rpm" and a new line   
  Serial.println (NbTopsFan, DEC); //Prints the number calculated above



}


float readCurrent(int PIN) {
int current = 0;
int sensitivity = 66.0;//change this to 100 for ACS712-20A or to 66 for ACS712-30A
//read 5 samples to stabilise value
for (int i=0; i<20; i++) {

	//I=0.0148*Vcc*(count-512)
//(.044 * analogRead(A0) -3.78) for 30A mode
//average = average + (.0264 * analogRead(A6) -13.51) / 100;

    current += analogRead(PIN) - VQ;
    delay(1);
}
current = map(current/20, 0, 1023, 0, 5000); //5000
return float(current)/sensitivity;
}



