/*************************************************************
Title         :   Home automation using blynk
Description   :   To control light's brigntness with brightness,monitor temperature , monitor water level in the tank through blynk app
Pheripherals  :   Arduino UNO , Temperature system, LED, LDR module, Serial Tank, Blynk cloud, Blynk App.
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "*************"
#define BLYNK_DEVICE_NAME "***************"
#define BLYNK_AUTH_TOKEN "*****************"



// Comment this out to disable prints 

// #define BLYNK_PRINT Serial

#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "main.h"
#include "temperature_system.h"
#include "ldr.h"
#include "serial_tank.h"

char auth[] = BLYNK_AUTH_TOKEN; 
bool heater_sw;
bool inlet_sw,outlet_sw;
unsigned int tank_volume;

BlynkTimer timer;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// This function is called every time the Virtual Pin 0 state changes
/*To turn ON and OFF cooler based virtual PIN value*/
BLYNK_WRITE(COOLER_V_PIN)
{
  int value = param.asInt(); 
// if cooler button is on blynk mobile application, then turn on thr cooler
  if (value) 
  { 
    cooler_control(ON);
    lcd.setCursor(7,0);
    lcd.print("CO_LR ON ");
  }
  else
  { 
    cooler_control(OFF);
    lcd.setCursor(7,0);
    lcd.print("CO_LR OFF");
  } 
  
}
/*To turn ON and OFF heater based virtual PIN value*/
BLYNK_WRITE(HEATER_V_PIN)
{
  heater_sw = param.asInt(); 
// if cooler button is on blynk mobile application, then turn on thr cooler
  if (heater_sw) 
  { 
    heater_control(ON);
    lcd.setCursor(7,0);
    lcd.print("HT_R ON ");   
  }
  else 
  { 
    heater_control(OFF);
    lcd.setCursor(7,0);
    lcd.print("HT_R OFF");
  } 
}
/*To turn ON and OFF inlet vale based virtual PIN value*/
BLYNK_WRITE(INLET_V_PIN)
{
  inlet_sw = param.asInt();
  // if inlet valve button is at logic high turn ON the inlet valve else off
  
  if(inlet_sw)
  {
    enable_inlet();
    lcd.setCursor(7, 1);
    lcd.print("IN_FL_ON ")  ;  
  }   
  else
  {
    disable_inlet(); 
    lcd.setCursor(7,1);
    lcd.print("IN_FL_OFF");
   } 
}
/*To turn ON and OFF outlet value based virtual switch value*/
BLYNK_WRITE(OUTLET_V_PIN)
{
  outlet_sw = param.asInt();
  // if inlet valve button is at logic high turn ON the inlet valve else off
  
  if(outlet_sw)
  {
    enable_outlet();
    lcd.setCursor(7, 1);
    lcd.print("OT_FL_ON ")  ;  
  }   
  else
  {
    disable_outlet(); 
    lcd.setCursor(7,1);
    lcd.print("OT_FL_OFF");
     } 
}
/* To display temperature and water volume as gauge on the Blynk App*/  
void update_temperature_reading()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(TEMPERATURE_GAUGE,read_temperature());
  Blynk.virtualWrite(WATER_VOL_GAUGE, volume()) ;
}

/*To turn off the heater if the temperature raises above 35 deg C*/
void handle_temp(void)
{
  // compare temperature with 35 and check if heater is on
    
if((read_temperature() > float(35)) && heater_sw)
{
  heater_sw = 0;
  heater_control(OFF);

  lcd.setCursor(7,0);
  lcd.print("HT_R OFF");
  Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Temperature is above 35 degrees celsius \n")  ;
  Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Turning OFF the heater \n") ; 
//   to reflect off in the heater button
  Blynk.virtualWrite(HEATER_V_PIN, OFF);
 
}

}

/*To control water volume above 2000ltrs*/
void handle_tank(void)
{
// compare the volume of water with 2000ltr and check the status of the inlet valve 
  if( (tank_volume < 2000 ) && (inlet_sw == OFF) )
  {
    enable_inlet();
    lcd.setCursor(7,1);
    lcd.print("IN_FL_ON ");
    inlet_sw = ON;
    Blynk.virtualWrite(INLET_V_PIN, ON);
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Water volume is less than 2000\n") ;
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Turning ON the inlet valve\n");          
  }

  if((tank_volume == 3000) && (inlet_sw == ON))
  {
    disable_inlet();
    lcd.setCursor(7,1);
    lcd.print("IN_FL_ON ");
    inlet_sw = OFF;
    Blynk.virtualWrite(INLET_V_PIN, OFF);
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Water level is full \n") ;
    Blynk.virtualWrite(BLYNK_TERMINAL_V_PIN, "Turning oFF the inlet valve\n");              
  }
  

}


void setup(void)
{
      
    Blynk.begin(auth);
    lcd.init();                     
    lcd.backlight();
    lcd.clear();
    lcd.home();
    init_ldr();
    /*To display string*/
    init_temperature_system();
    lcd.setCursor(0,0);
    lcd.print("T=");
    lcd.setCursor(0, 1);
    lcd.print("V=");
    init_serial_tank();
//  update temperature on the blynk app for every 0.5 seconds   
    timer.setInterval(500L,update_temperature_reading);
    
    // delay(1000);    
        
}

void loop(void) 
{
// control the brightness of garden light
      brightness_control();
      String temperature;
      temperature= String(read_temperature(), 2);
      lcd.setCursor(2,0);
      lcd.print(temperature); 

      tank_volume = volume();
      lcd.setCursor(2,1);
      lcd.print(tank_volume); 
      handle_temp();
      handle_tank();
      Blynk.run();
      timer.run();
// to monitor the volume of the water and if less 2000 lts turn on the inlet valve
      

 // to run blynk app
           
           
}