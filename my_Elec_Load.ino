

//Library for ADS1115 ADC
#include <Adafruit_ADS1015.h>       
Adafruit_ADS1115 ads(0x48);         
#define ADS1115_CONVERSIONDELAY  (1)
#define ADS1015_CONVERSIONDELAY  (1)


//Library for i2c LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>   
LiquidCrystal_I2C lcd(0x27,16,2); 
uint8_t arrow[8] = {0x0, 0x4 ,0x6, 0x3f, 0x6, 0x4, 0x0};
uint8_t ohm[8] = {0xE ,0x11, 0x11, 0x11, 0xA, 0xA, 0x1B};
uint8_t up[8] = {0x0 ,0x0, 0x4, 0xE , 0x1F, 0x4, 0x1C, 0x0};
unsigned long Pre_millis = 0;   //Variables used for LCD refresh loop
unsigned long I_millis = 0;    //Variables used for LCD refresh loop

//Library for MCP4725 DAC
#include <Adafruit_MCP4725.h>            
Adafruit_MCP4725 dac;
#define DAC_RESOLUTION    (8) 

int SW = 8;         //encoder pushbutton
int ST = 11;    //Pause pushbutton
int SM = 12;   //Menu pushbutton
int Rotary_counter = 0;      //Current RC position
int Rotary_counter_prev = 0;   //Previous RC
int Menu_level = 1;   //Menu levels
int Menu_row = 1;  //Each rows
int push_ON = 0;    //detect the change of the pushbutton when pushed
int push_OFF = 0;             //detect the change of the pushbuton when released 

bool clk_State;    //State of the CLK
bool Last_State;   //Last state of CLK
bool dt_State;   //State of the DT
//strings for pointer and names
String StringSpace = "      "; 
String StringSpace_mA = "    ";   
String StringPause = "";  
bool SW_STATUS = false;   //status of the rc pushbutton
bool ST_status = false;  //status of the pausebutton
bool pause = false;  //status of pasue
int Delay = 300;  
//initial Values before enter the values
//For cnt Current
byte mA_0 = 0;
byte mA_1 = 0;
byte mA_2 = 0;
byte mA_3 = 0;
//For cnt power
byte mW_0 = 0;
byte mW_1 = 0;
byte mW_2 = 0;
byte mW_3 = 0;
byte mW_4 = 0;
//For cnt Load 
byte Ohms_0 = 0;
byte Ohms_1 = 0;
byte Ohms_2 = 0;
byte Ohms_3 = 0;
byte Ohms_4 = 0;
byte Ohms_5 = 0;
byte Ohms_6 = 0;


//Variables for ADC readings
float ohm_setpoint = 0;
float mA_setpoint = 0;
float mW_setpoint = 0;
int dac_value = 0;

const float mul_1 = 0.0001827;     //Multiplier used for "current" read between ADC0 and ADC1 of the ADS1115    
const float mul_2 = 0.0020645;   //Multiplier for voltage read from the 10K/100K divider


void setup() {
  lcd.init();  //lcd i2c communication 
  lcd.backlight();    //No the backlight
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" ECL2 s***** "); 
  lcd.setCursor(0,1);
  lcd.print("ELECTRONIC  LOAD");  
  delay(2000);
  lcd.createChar(0, arrow);   //arrow 
  lcd.createChar(1, ohm);  //ohm Symbol
  lcd.createChar(2, up);    //up arrow 
  /*attachInterrupt(digitalPinToInterruppt(9,ISR,CHANGE);
  attachInterrupt(digitalPinToInterruppt(10,ISR,CHANGE);
  attachInterrupt(digitalPinToInterruppt(8,ISR,CHANGE);
  pinMode(SW,RISING);
  pinMode(SM,RISING);
  pinMode(SP,RISING);*/
  
  PCICR |= (1 << PCIE0);      //enable PCMSK0 scan                                                 
  PCMSK0 |= (1 << PCINT1);    // interrupt DT
  PCMSK0 |= (1 << PCINT2);    //interrupt clk
  DDRB &= B11111001;          //set D8 to D13 as inputs
  pinMode(SW,INPUT_PULLUP);       //Encoder button set as input with pullup
  pinMode(SM,INPUT_PULLUP);  //Menu button set as input with pullup
  pinMode(SP,INPUT_PULLUP);   //Pause button set as input with pullup
  delay(10);
//i2c comm with adc
  ads.begin();  
  ads.startComparator_SingleEnded(2, ADS1015_REG_CONFIG_MODE_CONTIN); //as in ADS1115 Lib
  ads.startComparator_SingleEnded(3, ADS1015_REG_CONFIG_MODE_CONTIN); //as in ADS1115 Lib
  delay(10);

  dac.begin(0x62);  //i2c comm with the DAC
  delay(10);
  dac.setVoltage(0, false); //Disable the current input of the mosfet V dac=0V
  delay(10);
   
  Pre_millis = millis();

}

void loop() {
  if(!digitalRead(ST) && !ST_status){
    push_OFF+=1;
    if(push_OFF > 10){            
      pause = !pause;
      ST_status = true;
      push_OFF=0;
    }   
  }
  if(digitalRead(ST) && ST_status){
    ST_status = false;
  }

  

  
  if(Menu_level == 1)
  {
    if(!digitalRead(SW) && !SW_STATUS)    {
      
      Rotary_counter = 0;
      if(Menu_row == 1){
        Menu_level = 2;
        Menu_row = 1;
      }
      else if(Menu_row == 2){
        Menu_level = 3;
        Menu_row = 1;
      }
      else if(Menu_row == 3){
        Menu_level = 4;
        Menu_row = 1;
      }
      
      SW_STATUS = true;
    }

    if(digitalRead(SW) && SW_STATUS)
    {      
      SW_STATUS = false;
    }


    
    
    if (Rotary_counter <= 4)
    {
      Menu_row = 1;
    }
    else if (Rotary_counter > 4 && Rotary_counter <= 8)
    {
      Menu_row = 2;
    }
    else if (Rotary_counter > 8 && Rotary_counter <= 12)
    {
      Menu_row = 3;
    }

    if(Rotary_counter < 0)
    {
      Rotary_counter = 0;
    }
    if(Rotary_counter > 12)
    {
      Rotary_counter = 12;
    }
    
    I_millis = millis();
    if(I_millis - Pre_millis >= Delay){
      Pre_millis += Delay;
      if(Menu_row == 1)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.write(0); 
        lcd.print(" >Cnt Current");
        lcd.setCursor(0,1);
        
        lcd.print("  >Cnt Load"); 
      }
    
      else if(Menu_row == 2)
      {
        lcd.clear();
        lcd.setCursor(0,0);     
        lcd.print("  >Cnt Current");
        lcd.setCursor(0,1);
        lcd.write(0);
        lcd.print(" >Cnt Load"); 
      }
    
      else if(Menu_row == 3)
      {
        lcd.clear();
        lcd.setCursor(0,0);  
        lcd.write(0);   
        lcd.print(" >Cnt Power");    
      }
    }
  }
  
//CNT CURRENT
   if(Menu_level == 2)
  {
    if(Rotary_counter < 0)
    {
      Rotary_counter = 0;
    }
    if(Rotary_counter > 9)
    {
      Rotary_counter = 9;
    }
    
    if(!digitalRead(SW) && !SW_STATUS)
    {
      
      push_ON = push_ON + 1;
      push_OFF = 0;
      if(push_ON > 20)
      {
        Menu_row = Menu_row + 1;
        if(Menu_row > 4)
        {
          Menu_level = 5;
          pause = false;
          mA_setpoint = mA_0*1000 + mA_1*100 + mA_2*10 + mA_3; 
          
        }
        Rotary_counter = 0;
        SW_STATUS = true;
        StringSpace_mA = StringSpace_mA + "_";
        push_ON = 0;
      }      
    }

    if(digitalRead(SW) && SW_STATUS)
    {      
      push_ON = 0; 
      push_OFF = push_OFF + 1; 
      if(push_OFF > 20){
        SW_STATUS = false;
        push_OFF = 0;
      }
        
    }
    

    if(Menu_row == 1)
    {
      mA_0 = Rotary_counter;      
    }
    if(Menu_row == 2)
    {
      mA_1 = Rotary_counter;      
    }
    if(Menu_row == 3)
    {
      mA_2 = Rotary_counter;
    }
    if(Menu_row == 4)
    {
      mA_3 = Rotary_counter;     
    }
    
    
    I_millis = millis();
    if(I_millis - Pre_millis >= Delay){
      Pre_millis += Delay;
      lcd.clear();
      lcd.setCursor(0,0);     
      lcd.print("mA: ");
      lcd.print(mA_0);
      lcd.print(mA_1);
      lcd.print(mA_2);
      lcd.print(mA_3);     
      lcd.setCursor(0,1);    
      lcd.print(StringSpace_mA);
      lcd.write(2);
    }
    if(!digitalRead(SM)){
      Menu_level = 1;
      Menu_row = 1;
      Rotary_counter = 0;
      Rotary_counter_prev = 0;
      dac.setVoltage(0, false);
      Pre_millis = millis();
      SW_STATUS = true;
      StringSpace_mA = "    ";  
      mA_setpoint = 0;   
      mA_0 = 0;
      mA_1 = 0;
      mA_2 = 0;      
    }     
  }
  //CNT LOAD
  if(Menu_level == 3)
  {
    if(Rotary_counter < 0)
    {
      Rotary_counter = 0;
    }
    if(Rotary_counter > 9)
    {
      Rotary_counter = 9;
    }
    if(!digitalRead(SW) && !SW_STATUS)
    {
     
      push_ON = push_ON + 1;
      push_OFF = 0;
      if(push_ON > 20)
      {
        Menu_row = Menu_row + 1;
        if(Menu_row > 7)
        {
          Menu_level = 6;
          pause = false;
          ohm_setpoint = Ohms_0*1000000 + Ohms_1*100000 + Ohms_2*10000 + Ohms_3*1000 + Ohms_4*100 + Ohms_5*10 + Ohms_6; 
          
        }
        Rotary_counter = 0;
        SW_STATUS = true;
        StringSpace = StringSpace + "_";
        push_ON = 0;
      }      
    }

    if(digitalRead(SW) && SW_STATUS)
    {      
      push_ON = 0; 
      push_OFF = push_OFF + 1; 
      if(push_OFF > 20){
        SW_STATUS = false;
        push_OFF = 0;
      }
        
    }
    

    if(Menu_row == 1)
    {
      Ohms_0 = Rotary_counter;      
    }
    if(Menu_row == 2)
    {
      Ohms_1 = Rotary_counter;      
    }
    if(Menu_row == 3)
    {
      Ohms_2 = Rotary_counter;
    }
    if(Menu_row == 4)
    {
      Ohms_3 = Rotary_counter;     
    }
    if(Menu_row == 5)
    {
      Ohms_4 = Rotary_counter;      
    }
    if(Menu_row == 6)
    {
      Ohms_5 = Rotary_counter;      
    }
    if(Menu_row == 7)
    {
      Ohms_6 = Rotary_counter;      
    }
    
    I_millis = millis();
    if(I_millis - Pre_millis >= Delay){
      Pre_millis += Delay;
      lcd.clear();
      lcd.setCursor(0,0);     
      lcd.print("Ohms: ");
      lcd.print(Ohms_0);
      lcd.print(Ohms_1);
      lcd.print(Ohms_2);
      lcd.print(Ohms_3);
      lcd.print(Ohms_4);
      lcd.print(Ohms_5);
      lcd.print(Ohms_6);
      lcd.setCursor(0,1);    
      lcd.print(StringSpace);
      lcd.write(2);
    }

    if(!digitalRead(SM)){
      Menu_level = 1;
      Menu_row = 1;
      Rotary_counter = 0;
      Rotary_counter_prev = 0;
      dac.setVoltage(0, false);
      Pre_millis = millis();
      SW_STATUS = true;
      StringSpace = "      ";    
      ohm_setpoint = 0;  
      Ohms_1 = 0;
      Ohms_2 = 0;
      Ohms_3 = 0;
      Ohms_4 = 0;
      Ohms_5 = 0;
      Ohms_6 = 0;
    }
    if(!digitalRead(SM)){
      Menu_level = 1;
      Menu_row = 1;
      Rotary_counter = 0;
      Rotary_counter_prev = 0;
      dac.setVoltage(0, false);
      Pre_millis = millis();
      SW_STATUS = true;
      StringSpace = "      ";    
      ohm_setpoint = 0;  
      Ohms_1 = 0;
      Ohms_2 = 0;
      Ohms_3 = 0;
      Ohms_4 = 0;
      Ohms_5 = 0;
      Ohms_6 = 0;
    }
  }

  //CNT POWER
  if(Menu_level == 4)
  {
    if(Rotary_counter < 0)
    {
      Rotary_counter = 0;
    }
    if(Rotary_counter > 9)
    {
      Rotary_counter = 9;
    }
    
    if(!digitalRead(SW) && !SW_STATUS)
    {
      
      push_ON = push_ON + 1;
      push_OFF = 0;
      if(push_ON > 20)
      {
        Menu_row = Menu_row + 1;
        if(Menu_row > 5)
        {
          Menu_level = 7;
          pause = false;
          mW_setpoint = mW_0*10000 + mW_1*1000 + mW_2*100 + mW_3*10 + mW_4; 
          
        }
        Rotary_counter = 0;
        SW_STATUS = true;
        StringSpace_mA = StringSpace_mA + "_";
        push_ON = 0;
      }      
    }

    if(digitalRead(SW) && SW_STATUS)
    {      
      push_ON = 0; 
      push_OFF = push_OFF + 1; 
      if(push_OFF > 20){
        SW_STATUS = false;
        push_OFF = 0;
      }
        
    }
    

    if(Menu_row == 1)
    {
      mW_0 = Rotary_counter;      
    }
    if(Menu_row == 2)
    {
      mW_1 = Rotary_counter;      
    }
    if(Menu_row == 3)
    {
      mW_2 = Rotary_counter;
    }
    if(Menu_row == 4)
    {
      mW_3 = Rotary_counter;
    }
    if(Menu_row == 5)
    {
      mW_4 = Rotary_counter;
    }
    
    
    
    I_millis = millis();
    if(I_millis - Pre_millis >= Delay){
      Pre_millis += Delay;
      lcd.clear();
      lcd.setCursor(0,0);     
      lcd.print("mW: ");
      lcd.print(mW_0);
      lcd.print(mW_1);
      lcd.print(mW_2);
      lcd.print(mW_3); 
      lcd.print(mW_4);            
      lcd.setCursor(0,1);    
      lcd.print(StringSpace_mA);
      lcd.write(2);
    }
    if(!digitalRead(SM)){
      Menu_level = 1;
      Menu_row = 1;
      Rotary_counter = 0;
      Rotary_counter_prev = 0;
      dac.setVoltage(0, false);
      Pre_millis = millis();
      SW_STATUS = true;
      StringSpace_mA = "    ";
      mW_setpoint = 0;
      mW_0 = 0;
      mW_1 = 0;
      mW_2 = 0;  
      mW_3 = 0; 
      mW_4 = 0;     
    }
  }





  //Constant Load Mode
  if(Menu_level == 6)
  {
    if(Rotary_counter > Rotary_counter_prev)
    {
      ohm_setpoint = ohm_setpoint + 1;
      Rotary_counter_prev = Rotary_counter;
      
    }

    if(Rotary_counter < Rotary_counter_prev)
    {
      ohm_setpoint = ohm_setpoint - 1;
      Rotary_counter_prev = Rotary_counter;
      
    }
    
    float voltage_on_load, sensosed_voltage, voltage_read, power_read;  
    voltage_on_load = ads.readADC_Differential_0_1();      //Read DIFFERENTIAL voltage between ADC0 and ADC1. (the load is 1ohm, so this is equal to the current)
    voltage_on_load = (voltage_on_load * mul_1)*-1000;

    voltage_read = ads.readADC_SingleEnded(2);
    voltage_read = (voltage_read * mul_2);
    
    //sensosed_voltage = ads.readADC_SingleEnded(3);
    //sensosed_voltage = (sensosed_voltage * mul_1);
    
    power_read = voltage_on_load * voltage_read;

    float setpoint_I = (voltage_read / ohm_setpoint) * 1000;

    float error = abs(setpoint_I - voltage_on_load);
    
    if (error > (setpoint_I * 0.8))
    {
      if(setpoint_I > voltage_on_load){
        dac_value = dac_value + 300;
      }

      if(setpoint_I < voltage_on_load){
        dac_value = dac_value - 300;
      }
    }

    else if (error > (setpoint_I*0.6))
    {
      if(setpoint_I > voltage_on_load){
        dac_value = dac_value + 170;
      }

      if(setpoint_I < voltage_on_load){
        dac_value = dac_value - 170;
      }
    }

    else if (error > (setpoint_I*0.4))
    {
      if(setpoint_I > voltage_on_load){
        dac_value = dac_value + 120;
      }

      if(setpoint_I < voltage_on_load){
        dac_value = dac_value - 120;
      }
    }
    else if (error > (setpoint_I*0.3))
    {
      if(setpoint_I > voltage_on_load){
        dac_value = dac_value + 60;
      }

      if(setpoint_I < voltage_on_load){
        dac_value = dac_value - 60;
      }
    }
    else if (error > (setpoint_I*0.2))
    {
      if(setpoint_I > voltage_on_load){
        dac_value = dac_value + 40;
      }

      if(setpoint_I < voltage_on_load){
        dac_value = dac_value - 40;
      }
    }
    else if (error > (setpoint_I*0.1))
    {
      if(setpoint_I > voltage_on_load){
        dac_value = dac_value + 30;
      }

      if(setpoint_I < voltage_on_load){
        dac_value = dac_value - 30;
      }
    }
    else
    {
      if(setpoint_I > voltage_on_load){
        dac_value = dac_value + 1;
      }

      if(setpoint_I < voltage_on_load){
        dac_value = dac_value - 1;
      }
    }
    
    
    
    if(dac_value > 4095)
    {
      dac_value = 4095;
    }
    
  
    
    if(!pause){
      dac.setVoltage(dac_value, false);
      StringPause = "";
    }
    else{
      dac.setVoltage(0, false);
      StringPause = " PAUSE";
    }
    
    I_millis = millis();
    if(I_millis - Pre_millis >= Delay){
      Pre_millis += Delay;
      lcd.clear();
      lcd.setCursor(0,0);     
      lcd.print(ohm_setpoint,0); lcd.write(1); lcd.print(" "); lcd.print(voltage_read,3); lcd.print("V");
      lcd.setCursor(0,1);    
      lcd.print(voltage_on_load,0);  lcd.print("mA"); lcd.print(" "); lcd.print(power_read,0);  lcd.print("mW"); 
      lcd.print(StringPause);
    }
    if(!digitalRead(SM)){
      Menu_level = 1;
      Menu_row = 1;
      Rotary_counter = 0;
      Rotary_counter_prev = 0;
      dac.setVoltage(0, false);
      Pre_millis = millis();
      SW_STATUS = true;
      StringSpace = "      ";    
      ohm_setpoint = 0;  
      Ohms_1 = 0;
      Ohms_2 = 0;
      Ohms_3 = 0;
      Ohms_4 = 0;
      Ohms_5 = 0;
      Ohms_6 = 0;
    }
  }








  //Constant Current Mode
  if(Menu_level == 5)
  {
    if(Rotary_counter > Rotary_counter_prev)
    {
      mA_setpoint = mA_setpoint + 1;
      Rotary_counter_prev = Rotary_counter;
    }

    if(Rotary_counter < Rotary_counter_prev)
    {
      mA_setpoint = mA_setpoint - 1;
      Rotary_counter_prev = Rotary_counter;
    }
    
    float voltage_on_load, sensosed_voltage, voltage_read, power_read;
      
    voltage_on_load = ads.readADC_Differential_0_1();      //Read DIFFERENTIAL voltage between ADC0 and ADC1
    voltage_on_load = (voltage_on_load * mul_1)*-1000;

    voltage_read = ads.readADC_SingleEnded(2);
    voltage_read = (voltage_read * mul_2);
    
    //sensosed_voltage = ads.readADC_SingleEnded(3);
    //sensosed_voltage = (sensosed_voltage * mul_1);
    
    power_read = voltage_on_load * voltage_read;

    float error = abs(mA_setpoint - voltage_on_load);
    
    if (error > (mA_setpoint*0.8))
    {
      if(mA_setpoint > voltage_on_load){
        dac_value = dac_value + 300;
      }

      if(mA_setpoint < voltage_on_load){
        dac_value = dac_value - 300;
      }
    }

    else if (error > (mA_setpoint*0.6))
    {
      if(mA_setpoint > voltage_on_load){
        dac_value = dac_value + 170;
      }

      if(mA_setpoint < voltage_on_load){
        dac_value = dac_value - 170;
      }
    }

    else if (error > (mA_setpoint*0.4))
    {
      if(mA_setpoint > voltage_on_load){
        dac_value = dac_value + 120;
      }

      if(mA_setpoint < voltage_on_load){
        dac_value = dac_value - 120;
      }
    }
    else if (error > (mA_setpoint*0.3))
    {
      if(mA_setpoint > voltage_on_load){
        dac_value = dac_value + 60;
      }

      if(mA_setpoint < voltage_on_load){
        dac_value = dac_value - 60;
      }
    }

    else if (error > (mA_setpoint*0.2))
    {
      if(mA_setpoint > voltage_on_load){
        dac_value = dac_value + 40;
      }

      if(mA_setpoint < voltage_on_load){
        dac_value = dac_value - 40;
      }
    }
    
    else if (error > (mA_setpoint*0.1))
    {
      if(mA_setpoint > voltage_on_load){
        dac_value = dac_value + 30;
      }

      if(mA_setpoint < voltage_on_load){
        dac_value = dac_value - 30;
      }
    }
    else
    {
      if(mA_setpoint > voltage_on_load){
        dac_value = dac_value + 1;
      }

      if(mA_setpoint < voltage_on_load){
        dac_value = dac_value - 1;
      }
    }
    
    
    
    if(dac_value > 4095)
    {
      dac_value = 4095;
    }
    
  
    if(!pause){
      dac.setVoltage(dac_value, false);
      StringPause = "";
    }
    else{
      dac.setVoltage(0, false);
      StringPause = " PAUSE";
    }
   
    

    I_millis = millis();
    if(I_millis - Pre_millis >= Delay){
      Pre_millis += Delay;
      lcd.clear();
      lcd.setCursor(0,0);     
      lcd.print(mA_setpoint,0); lcd.print("mA "); lcd.print(voltage_read); lcd.print("V");
      lcd.setCursor(0,1);    
      lcd.print(voltage_on_load,0);  lcd.print("mA"); lcd.print(" "); lcd.print(power_read,0);  lcd.print("mW"); 
      lcd.print(StringPause);
    }
    if(!digitalRead(SM)){
      Menu_level = 1;
      Menu_row = 1;
      Rotary_counter = 0;
      Rotary_counter_prev = 0;
      dac.setVoltage(0, false);
      Pre_millis = millis();
      SW_STATUS = true;
      StringSpace_mA = "____";  
      mA_setpoint = 0;   
      mA_0 = 0;
      mA_1 = 0;
      mA_2 = 0;      
    }      
  }










  //Constant Power Mode
  if(Menu_level == 7)
  {
    if(Rotary_counter > Rotary_counter_prev)
    {
      mW_setpoint = mW_setpoint + 1;
      Rotary_counter_prev = Rotary_counter;
    }

    if(Rotary_counter < Rotary_counter_prev)
    {
      mW_setpoint = mW_setpoint - 1;
      Rotary_counter_prev = Rotary_counter;
    }
    
    float voltage_on_load, sensosed_voltage, voltage_read, power_read;
      
    voltage_on_load = ads.readADC_Differential_0_1();      //Read DIFFERENTIAL voltage between ADC0 and ADC1
    voltage_on_load = (voltage_on_load * mul_1)*-1000;

    voltage_read = ads.readADC_SingleEnded(2);
    voltage_read = (voltage_read * mul_2);
    
    //sensosed_voltage = ads.readADC_SingleEnded(3);
    //sensosed_voltage = (sensosed_voltage * mul_1);
    
    power_read = voltage_on_load * voltage_read;




    float error = abs(mW_setpoint - power_read);    
    if (error > (mW_setpoint*0.8))
    {
      if(mW_setpoint > power_read){
        dac_value = dac_value + 300;
      }

      if(mW_setpoint < power_read){
        dac_value = dac_value - 300;
      }
    }

    else if (error > (mW_setpoint*0.6))
    {
      if(mW_setpoint > power_read){
        dac_value = dac_value + 170;
      }

      if(mW_setpoint < power_read){
        dac_value = dac_value - 170;
      }
    }

    else if (error > (mW_setpoint*0.4))
    {
      if(mW_setpoint > power_read){
        dac_value = dac_value + 120;
      }

      if(mW_setpoint < power_read){
        dac_value = dac_value - 120;
      }
    }
    else if (error > (mW_setpoint*0.3))
    {
      if(mW_setpoint > power_read){
        dac_value = dac_value + 60;
      }

      if(mW_setpoint < power_read){
        dac_value = dac_value - 60;
      }
    }
    else if (error > (mW_setpoint*0.2))
    {
      if(mW_setpoint > power_read){
        dac_value = dac_value + 40;
      }

      if(mW_setpoint < power_read){
        dac_value = dac_value - 40;
      }
    }
    else if (error > (mW_setpoint*0.1))
    {
      if(mW_setpoint > power_read){
        dac_value = dac_value + 30;
      }

      if(mW_setpoint < power_read){
        dac_value = dac_value - 30;
      }
    }
    else
    {
      if(mW_setpoint > power_read){
        dac_value = dac_value + 1;
      }

      if(mW_setpoint < power_read){
        dac_value = dac_value - 1;
      }
    }
    
    
    
    if(dac_value > 4095)
    {
      dac_value = 4095;
    }
    
  
    
    
    if(!pause){
      dac.setVoltage(dac_value, false);
      StringPause = "";
    }
    else{
      dac.setVoltage(0, false);
      StringPause = " PAUSE";
    }
    

    I_millis = millis();
    if(I_millis - Pre_millis >= Delay){
      Pre_millis += Delay;
      lcd.clear();
      lcd.setCursor(0,0);     
      lcd.print(mW_setpoint,0); lcd.print("mW "); lcd.print(voltage_read); lcd.print("V");
      lcd.setCursor(0,1);    
      lcd.print(power_read,0);  lcd.print("mW"); lcd.print(" "); lcd.print(voltage_on_load,0);  lcd.print("mA"); 
      lcd.print(StringPause);
    }
    if(!digitalRead(SM)){
      Menu_level = 1;
      Menu_row = 1;
      Rotary_counter = 0;
      Rotary_counter_prev = 0;
      dac.setVoltage(0, false);
      Pre_millis = millis();
      SW_STATUS = true;
      StringSpace_mA = "____";
      mW_setpoint = 0;
      mW_0 = 0;
      mW_1 = 0;
      mW_2 = 0;  
      mW_3 = 0; 
      mW_4 = 0;     
    }      
  }














}//end void loop





ISR(PCINT0_vect){  
cli(); //stop interrupts happening before we read pin values
clk_State =   (PINB & B00000100); //pin 10 state? 
dt_State  =   (PINB & B00000010); 
if (clk_State != Last_State){
  // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
  if (dt_State != clk_State){ 
    Rotary_counter ++;    
    
    Last_State = clk_State; // Updates the previous state of the outputA with the current state
    sei(); //restart interrupts
  }
  else {
    Rotary_counter --;  
     
    Last_State = clk_State; // Updates the previous state of the outputA with the current state    
    sei(); //restart interrupts
  } 
 }  
}
