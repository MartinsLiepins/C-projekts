// Stends D3 
// 1 displejs ar 3 enkoderiem un "skrienoðâm" diodçm

/*Veicot kalibrâciju enkoderiem (grozâmiem kloíiem)  jâbût nulles punktâ (vidus
stâvoklî).*/

//******************************************************************************
//                        Signal connection
//                        ____________  
//                       |TM4C129NCPDT|
//                       |            | 
//                       |         PB1|-> UART1 baud rate 2500000kbps 
//                       |         PK1|-> UART4 baud rate 2500000kbps   
//                       |            | 
//   ENC 1 16bit data  ->|PA5         |
//   ENC 2 16bit data  ->|PA2         |
//   ENC 3 16bit data  ->|PF1      PK0|-> ENC clk 100kHz
//    Calibrate button ->|PA0      PK3|-> ENC clk Enable (Chip select) 10Hz   
//                       |         PD1|-> LED_R calibrate indicator
//                       |            |
//                       |____________|
//
//  ENC - Encoder Bourns EMS22A
//******************************************************************************
//                      Connectors connection on PCB    
//
//                                      __________ 
//                                     |PCB RGB   |
//                                     |digital   |
//       ___________________________   |x6 v1.1.  |          
//      |        Bank_CPU v1        |  |    |-->X4|=>Kreditçðanas resursi displ.
//      |        ____________       |  |    |-->X5|=>Kreditçðanas resursi displ.
//      |       |TM4C129NCPDT|      |  |    |-->X6|=>Kreditçðanas resursi displ.
//      |       |            |      |  |    |-->X7|=>Kreditçðanas resursi displ.
//      |       |         PB1|->DMX1|=>|X3--|-->X8|=>Kreditçðanas resursi displ.  
//      |       |            |      |  |    |-->X9|=> LED strip A          
//      |       |            |      |  |__________|
//ENC1=>|X11  ->|PA5         |      |   __________
//ENC2=>|X12  ->|PA2         |      |  |PCB RGB   | 
//ENC3=>|X13  ->|PF1         |      |  |digital   |
//      |       |            |      |  |x6 v1.1.  |
//      |       |            |      |  |    |-->X4|=> LED strip C 
//      |       |            |      |  |    |-->X5|=> LED strip D        
//      |       |         PK1|->DMX2|=>|X3--|-->X6|=> LED strip E
//      |       |            |      |  |    |-->X7|=> LED strip B
//      |       |            |      |  |    |-->X8|=> LED strip F
//      |       |            |      |  |    |-->X9|=> LED strip G     
//      |       |            |      |  |__________|
//      |       |            |      |____
//      |       |            |           |
//      |       |         PK0|->X11/12/13|=> Encoder Bourns EMS22A   
//      |       |         PK3|->X11/12/13|=> Encoder Bourns EMS22A   
//      |       |            |           |
//      |SW1-1->|PA0         |           |
//      |       |         PD1|->    LED_R|-> Red LED on PoE-Node TM4C v1.0  
//      |       |____________|           |  
//      |________________________________|  
//
//  *
//Encoders:
// ENC 1 => Obligâtâs rezerves
// ENC 2 => Aizdevumu % likmes
// ENC 3 => Vçrtspapîtu pirkðana
//
//DMX 1:
// ENC1+ENC2+ENC3 => kreditçðans resursi => X4 (ch1) uz PCB RGB digital x6 v1.1. 
// ENC1+ENC2+ENC3 => kreditçðans resursi => X5 (ch2) uz PCB RGB digital x6 v1.1.
// ENC1+ENC2+ENC3 => kreditçðans resursi => X6 (ch3) uz PCB RGB digital x6 v1.1.
// ENC1+ENC2+ENC3 => kreditçðans resursi => X7 (ch4) uz PCB RGB digital x6 v1.1.
// ENC1+ENC2+ENC3 => kreditçðans resursi => X8 (ch5) uz PCB RGB digital x6 v1.1.
// => X9(ch6) LED strip A uz PCB RGB digital x6 v1.1.
//
//DMX 2:
// => X4(ch1) LED strip C uz PCB RGB digital x6 v1.1.
// => X5(ch2) LED strip D uz PCB RGB digital x6 v1.1.
// => X6(ch3) LED strip E uz PCB RGB digital x6 v1.1.
// => X7(ch4) LED strip B uz PCB RGB digital x6 v1.1.
// => X8(ch5) LED strip F uz PCB RGB digital x6 v1.1.
// => X9(ch6) LED strip B uz PCB RGB digital x6 v1.1.
              
//******************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/ssi.h"
#include "D3_main.h"
#include "string.h"
#include "driverlib/systick.h"
#include "driverlib/eeprom.h"
#include "driverlib/pwm.h"

//****************************************************************************


//*****************************************************************************
// ***************************   VARIABLES   **********************************
//*****************************************************************************

int32_t  sysClock, 
         PulseCounter, 
         mask, 
         SizeOfArray,
         EncoderIndex, 
         EncoderDataIndex,
         ENCPosition[ENC_COUNT], 
         EncoderRAWData[ENC_COUNT][ENC_DATA_LENGHT],
         ENCPositionNormalized[ENC_COUNT],
         encoderPozitionNormalizedTau[ENC_COUNT],
         calibratePinState;

int8_t bit,
       i, 
       idx, 
       ButtonPressCount, 
       UARTInterfaceNumber;

//Mainîgie enkodera aizturei
int32_t encoderArrayShiftRegisterSum[ENC_COUNT],
        encoderArrayShiftRegister[ENC_COUNT][INTEGRATION_DELAY],
        ENCPositionDelayNormalized[ENC_COUNT],
        index,
        tau;

int32_t  *ptrToDisplayValues;

uint8_t *ptrToColorMap;
        

//enkodera kalibrçðanas mainîgie
struct calEncoderData{
  int32_t EncCalMinState[ENC_COUNT];
  int32_t EncCalMaxState[ENC_COUNT];
  int32_t EncCalZeroStateNorm[ENC_COUNT];
  int32_t EncCalMaxStateNorm[ENC_COUNT];
  int8_t calibrationDone;  
};

//*****************************************************************************
// The error routine that is called if the driver library encounters an error.
//*****************************************************************************

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
// *********************   INTERNAL FUNCTION DECLARATION   ********************
//*****************************************************************************

//uint32_t Bin2IntConvert(uint32_t g_ui32PinRead[15]);
void initEncoder();
void initTimer();
void ConfigureEPROM();
unsigned int power(unsigned int x, unsigned int y);
void CalibrateEncoderState();

int32_t calcIntegrationDelay(int32_t position, int32_t *idx, 
                             int32_t *ptrToENCPosNorm, 
                             int32_t *ptrToENCPosDelayNorm, 
                             int32_t *ptrToEncArrShiftRegSum, 
                             int32_t (*ptrToEncArrShiftReg)[INTEGRATION_DELAY]);

void InitEncoderCalibration();
int32_t EncoderSampleDelay(int32_t encoderArray[], int32_t count);
void InitUART(uint32_t sysClock);
struct calEncoderData reaedDataFromEEPROM();

//*****************************************************************************
// *********************  EXTERNAL FUNCTION DECLARATION  **********************
//*****************************************************************************

int32_t * CalculateEconomicValue(int32_t PositionDelayNormalized[], 
                                 int32_t *pointerToEncoderCalibrateZero, 
                                 int32_t *pointerToEncoderCalibrateMaxNorm);

uint8_t * LEDColorsColormap(int32_t *pointerToValues);
void SendDataToLED(uint32_t sysClock, uint8_t *pointerToColorMap);

//bootloader
void bootload_init (uint32_t sysClk);
void bootload_task (void);

//*****************************************************************************
// ***********************   INTERRUPTS   ************************************
//*****************************************************************************

/*Pârtraukums clk ìenerçðanai Enkoderiem. Ieejot pâtraukumâ, tiek raustîts 
("toggle") PK0 pins (enkoderu takts frekvence).*/
void Timer1IntHandler(void){
  ROM_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
  toggle_PIN_ENC_CLOCK ^= 0x01;
  GPIOPinWrite(GPIO_PORTK_BASE, ENC_CLOCK, toggle_PIN_ENC_CLOCK);//clk pin to Encoder
  PulseCounter++;
  if (PulseCounter == ENC_DATA_LENGHT * 2 + 1){ 
    ROM_TimerDisable(TIMER1_BASE, TIMER_A);
    ReadDataFlag = 1;      
  }
  else if(toggle_PIN_ENC_CLOCK == 0x01){
    SysCtlDelay(50);  //375ns read data delay Encoder clk validation 
    EncoderRAWData[0][EncoderDataIndex] = GPIOPinRead(GPIO_PORTA_BASE, ENC_DATA_IN_1);     
    EncoderRAWData[1][EncoderDataIndex] = GPIOPinRead(GPIO_PORTA_BASE, ENC_DATA_IN_2);      
    EncoderRAWData[2][EncoderDataIndex] = GPIOPinRead(GPIO_PORTF_BASE, ENC_DATA_IN_3);
    EncoderDataIndex = EncoderDataIndex + 1;      
    ROM_IntMasterEnable();
  }
  else if(toggle_PIN_ENC_CLOCK == 0){
    ROM_IntMasterEnable();
  }    
}

//*****************************************************************************

//Pârtraukums CS ìenerçðanai Enkoderiem
void Timer0IntHandler(void){
  ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  GPIOPinWrite(GPIO_PORTK_BASE, ENC_DATA_ENABLE, 0);//clk enable pin to Encoder    
  EncoderDataIndex = 0;
  PulseCounter = 0;
  SysCtlDelay(1000);
  memset(EncoderRAWData, 0, ENC_COUNT*sizeof(uint32_t)*16);
  ROM_TimerEnable(TIMER1_BASE, TIMER_A);
  ROM_IntMasterEnable();
    
  //nelietojam timer 0 ja esam kalibracija
  if(calibratePinState == 0){
    ROM_TimerDisable(TIMER0_BASE, TIMER_A);
  }
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

int main(void)
{
  IntPrioritySet(INT_TIMER1A, 0);
  IntPrioritySet(INT_TIMER0A, 0);
  
  ReadDataFlag = 0;
  ButtonPressCount = 0;
  index = 0;
  tau = INTEGRATION_DELAY;
  
  sysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 120000000);
  InitUART(sysClock);  
  InitEncoderCalibration();
  ConfigureEPROM();
  
  SizeOfArray = ((sizeof(EncoderRAWData) / sizeof(uint32_t))/ENC_COUNT)-1;
  
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   
//Read EEPROM  calibration data   
  
  struct calEncoderData calData;
  calData = reaedDataFromEEPROM();
  
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
  
  initEncoder();
  initTimer();
  bootload_init(sysClock);
  
  while(1){
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//Calibrate encoders end states     
 
/*kalibrâcija tiek veikta enkoderi kloíiem esot vidus (neitrâlâ) stâvoklî. Ar 
 slçdzi SW1(atrodas uz PCB "PoE-Node TM4C")ieslçdz kalibrâcijas reþîmu un veic 
 stenda D3 kloíu grozîðanu (2-3 reizes) pa labi un pa kreizi lîdz gala stâvoklim. 
 Esot kalibrâcijas reþîmâ, uz PCB "PoE-Node TM4C" tiek ieslçgta sarkana diode 
 LD1. Izejot no kalibrâcjias reþîma, slçdzi SW1 pârbîda ON stâvoklî (diode LD1 
 tiek izslçta). Kalibrçtie dati tiek saglabâti TM4C EEPROM atmiòâ*/ 
    
/*Pârbauda kalibrâcijas reþîma ieslçgðas slçdþa stâvokli */    
    calibratePinState = GPIOPinRead(CALIBRATE_BUTTON_PORT, 
                                    CALIBRATE_BUTTON_PIN);
/*Ja kalibrâcijas reþîma slçdþa stâvoklis ir 0, veic enkoderu kalibrâciju. */    
    if (calibratePinState == 0){    
      ROM_TimerDisable(TIMER1_BASE, TIMER_A);
      ROM_TimerDisable(TIMER0_BASE, TIMER_A);
      //ieslçdz LED_R ieejot kalibrçðanas reþîmâ
      ROM_GPIOPinWrite(GPIO_PORTD_BASE, LED_1, 0x00);
      SysCtlDelay(100000000);      
      CalibrateEncoderState();      
      calData = reaedDataFromEEPROM();
      //iezslçdz LED_R izejot no kalibrçðanas reþîmâ
      ROM_GPIOPinWrite(GPIO_PORTD_BASE, LED_1, 0x02);
      ROM_TimerEnable(TIMER0_BASE, TIMER_A);
      ROM_IntMasterEnable();     
    }
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//Data from encoders data processing
    
    if(ReadDataFlag == 1){
      SysCtlDelay(1000);
      ROM_GPIOPinWrite(GPIO_PORTK_BASE, ENC_DATA_ENABLE, 0x08);
      ROM_GPIOPinWrite(GPIO_PORTK_BASE, ENC_CLOCK, 0x01);
      toggle_PIN_ENC_CLOCK = 0x01;
      ReadDataFlag = 0;
      for(i=0; i<=ENC_COUNT-1; i++){
        for(bit=SizeOfArray-6; bit>=0; bit--){ //-6, jo ENC pozîcijas dati ir 10 biti no 16 
          ENCPosition[i] = ENCPosition[i] +
              (EncoderRAWData[i][bit]!= 0) * power(2, SizeOfArray - bit - 6);
        }  
          
/*Nodroðinâs lai gala stâvokïi netiek pârstûrçti, mainoties enkodera gala 
pozîcijai.Ja nomçrîtais gala stâvoklis ir lielâks vai mazâks par kalibrçto
vçrtîbu, gala stâvoklis tiek pieòemts kalibrçtâ vçrtîba */ 
        if(ENCPosition[i] > calData.EncCalMaxState[i]){
           ENCPosition[i] = calData.EncCalMaxState[i]; 
        }
        if(ENCPosition[i] < calData.EncCalMinState[i]){
           ENCPosition[i] = calData.EncCalMinState[i];
        }
	//255 - vçrtîba pret kuru normalizç enkoderu vçrtîbas (0 - 255)
        ENCPositionNormalized[i] = (uint32_t)
                  (((ENCPosition[i] - calData.EncCalMinState[i]) * 255)/
                    (calData.EncCalMaxState[i] - calData.EncCalMinState[i]));
      }   
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *
/*Pievieno integrçðanas aizturi un saliek datus colormap masîvâ. ja enkoderu
 kalibrâcija nav veikta (EEPROM adresç x400 neglabâjas 0xFFFFFFFF vçrtîba),
 tâlâkâs darbîbas netiek veiktas un displeji neko nerâda*/ 
      
      if(calData.calibrationDone == 1){   
          
/*Pievieno mçrîjumam aizturi tau (videja vertiba laikâ). 
aizture aktarîga no enkoderu datu nolasîðanas âtruma (Timer0).
ja lasa ar 100Hz, un aizture ir uzstâdîta 0,5s, tau=50 (skat. main.h failâ
INTEGRATION_DELAY definçto vçrtîbu)*/             
        tau = calcIntegrationDelay(tau, &index, ENCPositionNormalized,
                                   ENCPositionDelayNormalized, 
                                   encoderArrayShiftRegisterSum,
                                   encoderArrayShiftRegister);
            
        if(tau == 0){
          ptrToDisplayValues = 
            CalculateEconomicValue(ENCPositionDelayNormalized, 
                                   calData.EncCalZeroStateNorm, 
                                   calData.EncCalMaxStateNorm);  
            
          ptrToColorMap = LEDColorsColormap(ptrToDisplayValues);
          SendDataToLED(sysClock, ptrToColorMap);
        }
      }else{
          //kods ja nav veikta kalibrâcija. 
      }
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *        
       memset(ENCPosition, 0, ENC_COUNT*sizeof(uint32_t));
    }
//run ethernet and bootloader tasks
    bootload_task();

  }
} 
//*****************************************************************************
// ***********************   INTERNAL FUNCTION DEFINITION   *******************
//*****************************************************************************

void InitEncoderCalibration(){
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinTypeGPIOInput(CALIBRATE_BUTTON_PORT, CALIBRATE_BUTTON_PIN);
  GPIOPadConfigSet(CALIBRATE_BUTTON_PORT, CALIBRATE_BUTTON_PIN, 
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  
  //LED_R indikâcijas ieejot kalibrçðanas reþîmâ (sarkana LED deg)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); 
  GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, LED_1);
  GPIOPadConfigSet(GPIO_PORTD_BASE, LED_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

//*****************************************************************************
//*****************************************************************************

void CalibrateEncoderState(){ 
    
  uint32_t putData[ENC_COUNT*3+1],
           minStateCicleCount = 0,
           initState = 0,
           i;
  
  int32_t ENCPositionOld[ENC_COUNT],
          ENC_MaxState[ENC_COUNT],
          ENC_MinState[ENC_COUNT],
          ENCZeroState[ENC_COUNT];

  initState = 0;
  
  for(i = 0; i < ENC_COUNT; i++){
    ENC_MaxState[i] = 0;
    ENC_MinState[i] = 1023;
    ENCZeroState[i] = 0;
  }
  ROM_TimerEnable(TIMER0_BASE, TIMER_A);
  ROM_IntMasterEnable();
  
  while(calibratePinState == 0){
   calibratePinState = GPIOPinRead(CALIBRATE_BUTTON_PORT, CALIBRATE_BUTTON_PIN);
    if(ReadDataFlag == 1){
      SysCtlDelay(1000);
      ROM_GPIOPinWrite(GPIO_PORTK_BASE, ENC_DATA_ENABLE, 0x08);
      ROM_GPIOPinWrite(GPIO_PORTK_BASE, ENC_CLOCK, 0x01);
      toggle_PIN_ENC_CLOCK = 0x01;
      ReadDataFlag = 0;
      for(i=0; i<=ENC_COUNT-1; i++){  
        for(bit=SizeOfArray-6; bit>=0; bit--){ //-6, jo ENC pozîcijas dati ir 10 biti no 16 
          ENCPosition[i] = ENCPosition[i] + (EncoderRAWData[i][bit]!= 0) * power(2, SizeOfArray - bit - 6);
        }

        if(initState == 1){
        
          if(minStateCicleCount == ENC_COUNT * 20){
            if(ENCPosition[i] > ENC_MaxState[i])
            { 
              ENC_MaxState[i] = ENCPosition[i];
            }
            else if(ENCPosition[i] < ENC_MinState[i])
            {   
              ENC_MinState[i] = ENCPosition[i];
            }
          }
          if((ENCPosition[i] == 0)&&(i==2)){
           ENCPosition[2] = 1;
        }
         
          else if(minStateCicleCount < ENC_COUNT * 20){
            if((ENCPosition[i] >=  ENCPositionOld[i] - 2) && (ENCPosition[i] <=  (ENCPositionOld[i] + 2))){
              ENCZeroState[i] = ENCPosition[i];
              minStateCicleCount += 1;
            }            
          }         
        }
        else{
          ENCPositionOld[i] = ENCPosition[i];           
        }
      } 
      memset(ENCPosition, 0, ENC_COUNT*sizeof(int32_t));
      initState = 1;      
      
      //palaizam nakamo lasisanas ciklu
      ROM_TimerEnable(TIMER0_BASE, TIMER_A);
    }    
  }
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *         
    ROM_TimerDisable(TIMER1_BASE, TIMER_A);
    ROM_TimerDisable(TIMER0_BASE, TIMER_A);
    EEPROMMassErase(); 
    putData[0] = 0xFFFFFFFF;//EEPROM data Control value
    for(i=1; i<=ENC_COUNT*3; i+=3){
//ENC_MaxState un ENC_MinState pârveido uz two's compliant formu, jo putData
//funkcija darbojas ar uint mainîgajaiem.      
      putData[i] = ~(ENC_MaxState[(i-1)/3]) + 1;
      putData[i+1] = ~(ENC_MinState[(i-1)/3]) + 1;
      putData[i+2] = ~(ENCZeroState[(i-1)/3]) + 1;
    }
    EEPROMProgram(putData, 0x400, sizeof(putData));    
 
}

//*****************************************************************************
//*****************************************************************************

int32_t calcIntegrationDelay(int32_t position, int32_t *idx, 
                             int32_t *ptrToENCPosNorm, 
                             int32_t *ptrToENCPosDelayNorm, 
                             int32_t *ptrToEncArrShiftRegSum, 
                             int32_t (*ptrToEncArrShiftReg)[INTEGRATION_DELAY]){
 
  int32_t i;  
  int32_t a=*idx;
  
  if(position==0){
    for(i = 0; i< ENC_COUNT; i++){
      *(ptrToEncArrShiftRegSum + i) = *(ptrToEncArrShiftRegSum + i) + 
                                        *(ptrToENCPosNorm + i) - 
                                          *(*(ptrToEncArrShiftReg + i) + a);
      
      *(*(ptrToEncArrShiftReg + i) + a) = *(ptrToENCPosNorm + i);
      
      *(ptrToENCPosDelayNorm + i) =  *(ptrToEncArrShiftRegSum + i) / INTEGRATION_DELAY;
    }
  }
//Uzkrâj aizturi LED izmaiòas straujumam. Aizture 50 (INTEGRATION_DELAY) 
//  lasîðanas cikli t.i. 0,5 sekundes 
  else{
    position--;
    for(i = 0; i< ENC_COUNT; i++){
      *(ptrToEncArrShiftRegSum + i) = *(ptrToEncArrShiftRegSum + i) + 
                                      *(ptrToENCPosNorm + i);
      *(*(ptrToEncArrShiftReg + i) + a)  = *(ptrToENCPosNorm + i);
    } 
  }
  *idx = *(idx)+1; 
  if(a == INTEGRATION_DELAY-1){
    *(idx) = 0;
  }                               
  return position;
}

//*****************************************************************************
//*****************************************************************************

void initEncoder(){
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
  GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, ENC_CLOCK | ENC_DATA_ENABLE);
  GPIOPadConfigSet(GPIO_PORTK_BASE, ENC_CLOCK, GPIO_STRENGTH_10MA, GPIO_PIN_TYPE_STD);
  GPIOPadConfigSet(GPIO_PORTK_BASE, ENC_DATA_ENABLE, GPIO_STRENGTH_10MA, GPIO_PIN_TYPE_STD_WPU);
  
  ROM_GPIOPinWrite(GPIO_PORTK_BASE, ENC_DATA_ENABLE, 0x08);//set Encoder data Enable pin to 1
  ROM_GPIOPinWrite(GPIO_PORTK_BASE, ENC_CLOCK, 0x01); //set Encoder clk pin to "1"
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
                         
  GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, ENC_DATA_IN_1 | ENC_DATA_IN_2);
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, ENC_DATA_IN_3);

}

void initTimer(){
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);// Timer for ENC xx clk pulse generation
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);// Timer for ENC clk timeout 
  
  IntMasterEnable();

  TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
  TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
  
    
  TimerLoadSet(TIMER1_BASE, TIMER_A, sysClock / 200000); //1MHz => 2000000; 100kHz => 200000; 10kHz=>20000 
  TimerLoadSet(TIMER0_BASE, TIMER_A, sysClock / 100);//10 Hz Encoder sampling rate 100
   
  IntEnable(INT_TIMER1A);
  IntEnable(INT_TIMER0A);
    
  TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    
  TimerEnable(TIMER0_BASE, TIMER_A);
}

//*****************************************************************************
//*****************************************************************************

void ConfigureEPROM(){
  
  uint32_t EEPROMInitFlag;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0)){}
  EEPROMInitFlag = EEPROMInit();
  if(EEPROMInitFlag != EEPROM_INIT_OK){
    while(1){
    }
  }
}

//*****************************************************************************
//*****************************************************************************

unsigned int power(unsigned int x, unsigned int y){
  
  unsigned int result = 1;    
    while (y > 0) {
      result *= x;
      y--;
    } 
    return result;
}

//*****************************************************************************
//*****************************************************************************

void InitUART(uint32_t sysClock){

//UART4 uz PK1  portu
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART4);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
  GPIOPinConfigure(GPIO_PK1_U4TX);
  GPIOPinTypeUART(GPIO_PORTK_BASE, GPIO_PIN_1);
  GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);  
  UARTConfigSetExpClk(UART4_BASE, sysClock, UART_BAUD_RATE,
                        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE);
  UARTEnable(UART4_BASE);
  
  
//UART1 uz PB1  portu
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  GPIOPinConfigure(GPIO_PB1_U1TX);
  GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_1);
  GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);  
  UARTConfigSetExpClk(UART1_BASE, sysClock, UART_BAUD_RATE,
                        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE);
  UARTEnable(UART1_BASE); 
}

//*****************************************************************************
//*****************************************************************************

struct calEncoderData reaedDataFromEEPROM(){
  
  static struct calEncoderData calData;
  
  uint32_t EEPROMReadData[ENC_COUNT *3 + 1];
  
  const uint32_t valueToNormalize = 254;
  
  EEPROMRead(EEPROMReadData, 0x400, sizeof(EEPROMReadData));
  if((EEPROMReadData[0]) == 0xFFFFFFFF){
    calData.calibrationDone = 1;
    for(i=1; i<ENC_COUNT*3; i+=3){
      
      calData.EncCalMaxState[(i-1)/3] = ~EEPROMReadData[i] + 1;
      calData.EncCalMinState[(i-1)/3] = ~EEPROMReadData[i+1] + 1;
      calData.EncCalZeroStateNorm[(i-1)/3] = (((~EEPROMReadData[i+2] + 1) -
                                           calData.EncCalMinState[(i-1)/3]) * 
                                              valueToNormalize) / 
                                           (calData.EncCalMaxState[(i-1)/3] - 
                                            calData.EncCalMinState[(i-1)/3]);
      
      calData.EncCalMaxStateNorm[(i-1)/3] = (((~EEPROMReadData[i] + 1) -
                                           calData.EncCalMinState[(i-1)/3]) * 
                                             valueToNormalize) / 
                                           (calData.EncCalMaxState[(i-1)/3] - 
                                            calData.EncCalMinState[(i-1)/3]);
     }
  } 
  else{
    calData.calibrationDone = 0;
  }   
  return calData;
}
//*****************************************************************************
//*****************************************************************************