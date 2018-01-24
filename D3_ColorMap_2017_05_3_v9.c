#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "D3_main.h"
#include "D3_colormap.h"

//*****************************************************************************
// *********************   EXTERNAL FUNCTION DECLARATION  *********************
//*****************************************************************************


//*****************************************************************************
// *********************   INTERNAL FUNCTION DECLARATION   ********************
//*****************************************************************************
void putDisplayDataInColorMap(uint8_t *map, int32_t adress, int32_t i,
                              int8_t r, int8_t b, int8_t g, 
                              int32_t *ptrToBubbleArr, 
                              int32_t ledOnCount, double varLedValue);

void putLedStripDataInColorMap(uint8_t *map, int32_t adress, 
                               uint32_t ledInStrip, int32_t *chanXLedPos, 
                               uint8_t r, uint8_t b, uint8_t g);

//******************************************************************************
//******************************************************************************
//******************************************************************************

uint8_t * LEDColorsColormap(int32_t *pointerToValues){
  
  static uint8_t colorMap[3 * (LED_COUNT * (LED_STRIP_COUNT + 
                                            DISPLAY_CHANNEL_COUNT))];
  
  int8_t red,
         green,
         blue;
 
  int32_t  i,
           chIdx,
           startAdress,
           fullLedOnCount;
  double   ledOnCount,
           variableLedColor; 
 
  const uint32_t  normalizeToScale = 127,
                  fullScale = normalizeToScale * ENC_COUNT * 2,//"2" tap�c, ka skala ir no 0-762, nevis -381 - +381
                               
/* interv�li kr�su p�rsl�g�anai: 
colormap displejam aizpilda, �emot v�r� diapazonus no economicValue
(lielums  0-762):
      0-15%  - blue (113)
      15-40% - blue+green (114 - 303)
      40-50% - green (304 - 380)
      50-60% - green (381 - 456)
      60-85% - green+red (456 - 646)
      85-100%- red        (647 - 762)
*/       
                 colorLimitOne = (uint32_t)(fullScale * 0.15),//0.1
                 colorLimitTwo = (uint32_t)(fullScale * 0.4),
                 colorLimitThree = (uint32_t)(fullScale * 0.5),
                 colorLimitFour = (uint32_t)(fullScale * 0.6),
                 colorLimitFive = (uint32_t)(fullScale * 0.85);//9

//V�r��an�s efekta main�gie

  static int32_t bubbleArr[LED_COUNT * 5];  
  static uint8_t rndNum[LED_COUNT * 5];
  static uint32_t deltaValue[LED_COUNT * 5];
  static uint8_t deltaFlag[LED_COUNT * 5];//up vai down skait�t�ja nor�de

  uint8_t deltaIncrement;
  
  const int32_t NormMin = 2,//skalas (ekonomikas v�rt�ba 0-762) minim�l� norm��anas v�rt�ba 0,2*10
                NormMax = 28;//skalas (ekonomikas v�rt�ba 0-762) maksim�l� norm��anas v�rt�ba 3*10

  int32_t bubbleStopValue,
          blueLedValue;   
  
 int32_t bubbleArrValue;
 //*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   

  ledOnCount = (((LED_COUNT * *pointerToValues) * convertToDbl) / fullScale);
  fullLedOnCount = (int32_t)(ledOnCount);
  variableLedColor = ledOnCount - fullLedOnCount;

//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
  
//Ekr�na "v�r��an�s" efekta izveide  
  bubbleStopValue = bubbleStopLimit*fullScale/LED_COUNT;//v�rt�ba 141
     
  if(*pointerToValues >= bubbleStopValue){
    for(i=0; i<LED_COUNT * ledChannelCount; i++ ){
      if(deltaValue[i] == 0){
        rndNum[i] = (rand()%DELTA_RAND_NUM) + DELTA_RAND_NUM-1;
        deltaFlag[i] = 1;//skaita LED spo�umu no 0->255       
      }
      
//skaita LED spo�umu no 255->0       
      if(deltaValue[i] == DELTA_CTR_MAX){
        rndNum[i] = (rand()%DELTA_RAND_NUM) + DELTA_RAND_NUM-1;
        deltaFlag[i] = 2;
      }
      
//skaita LED spo�umu no 0->DELTA_CTR_MAX      
      if(deltaFlag[i] == 1){
        deltaIncrement =(rndNum[i] * 
                         (NormMin +
                          (((*pointerToValues - bubbleStopValue) * NormMax) /
                           (fullScale - bubbleStopValue)))) / 10;
                
        if((DELTA_CTR_MAX - deltaValue[i]) < deltaIncrement){
          deltaValue[i] = DELTA_CTR_MAX;
        }else{
          deltaValue[i] += deltaIncrement;
        }
        if(*pointerToValues < bubbleDecreaseIntensityValue ){
         bubbleArrValue = COLOR_DEVIATION/2 - 
                         (deltaValue[i]*COLOR_DEVIATION / DELTA_CTR_MAX);
    
         bubbleArr[i] = (int32_t)
           (((int32_t)(*pointerToValues) * (bubbleArrValue)) / 
            bubbleDecreaseIntensityValue);
        }
        else{
          bubbleArr[i] = COLOR_DEVIATION/2 - 
                         (deltaValue[i]*COLOR_DEVIATION / DELTA_CTR_MAX);
        }
      }
      
      if(deltaFlag[i] == 2){//skaita LED spo�umu no DELTA_CTR_MAX->0 
        deltaIncrement =(rndNum[i] * 
                         (NormMin +
                          (((*pointerToValues - bubbleStopValue) * NormMax) /
                           (fullScale - bubbleStopValue)))) / 10;
   
        if (deltaValue[i] < deltaIncrement){
          deltaValue[i] = 0;
        }else{
          deltaValue[i] -= deltaIncrement;
        }
        if(*pointerToValues < bubbleDecreaseIntensityValue ){
          bubbleArrValue = COLOR_DEVIATION/2 - 
                           (deltaValue[i]*COLOR_DEVIATION / DELTA_CTR_MAX);
        
          
          bubbleArr[i] = (int32_t)
            (((int32_t)(*pointerToValues) * (bubbleArrValue) ) / 
             bubbleDecreaseIntensityValue);
        }
        else{
          bubbleArr[i] = COLOR_DEVIATION/2 - 
                         (deltaValue[i]*COLOR_DEVIATION / DELTA_CTR_MAX);
       }
      }     
    }    
  }
  
//*   *   *   *   
  else{
    for(i=0; i<LED_COUNT * ledChannelCount; i++ ){
      bubbleArr[i] =  0;
      
    }
  }
  
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   
  
//Ekr�na ColorMap izveide  
  for(chIdx=0; chIdx < ledChannelCount; chIdx++){
    startAdress = chIdx * LED_COUNT * ledPerPixel;
    for(i=0; i < LED_COUNT * ledPerPixel; i+=ledPerPixel){
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *     
//Interv�ls 0 - 15% no pilna displeja (zils tonis)
      if(*pointerToValues < colorLimitOne){ 
        blueLedValue = (int32_t)(blueLedStopLimit * fullScale / LED_COUNT);
        if(*pointerToValues < blueLedValue){//Palieko��s zil�s diodes
          if(i >= blueLedStopLimit * ledPerPixel){
            red = 0;
            blue = 0;
            green = 0;
          
            putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                   bubbleArr, fullLedOnCount, variableLedColor);
          } 
          else{
          red = 0;
          blue = (uint8_t)(colorBlue * 
                           ((fullScale - 
                             (*pointerToValues * convertToDouble)) / fullScale));
          green = 0;
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                   bubbleArr, fullLedOnCount, variableLedColor);
        }
        }
        
        else{
          if(i < (fullLedOnCount * ledPerPixel)){
            red = 0;
            blue = (uint8_t)(colorBlue * 
                              ((fullScale - 
                              (*pointerToValues * convertToDouble)) / fullScale));
          
            green = 0;
            putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                   bubbleArr, fullLedOnCount, variableLedColor);
         }
          else if(i == fullLedOnCount * 3){
            red = 0;
            blue = (uint8_t)(colorBlue * 
                            (variableLedColor * 
                             ((fullScale-(*pointerToValues * convertToDouble))/
                              fullScale)));
          
            green = 0;
          
            putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                   bubbleArr, fullLedOnCount, variableLedColor);
          } 
          else if(i > fullLedOnCount * ledPerPixel){
            red = 0;
            blue = 0;
            green = 0;
          
            putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                   bubbleArr, fullLedOnCount, variableLedColor);
          } 
        }
      }
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *    
//Interv�ls 15 - 40% no pilna displeja (za�s/zils tonis)
      else if((*pointerToValues >= colorLimitOne) && 
             (*pointerToValues < colorLimitTwo)){
               
        if(i < fullLedOnCount * ledPerPixel){
          red = 0;
          blue = (uint8_t)(colorBlue * 
                           ((fullScale - (*pointerToValues * convertToDouble))/
                            fullScale));
          green = (uint8_t)(colorGreen * 
                            (((*pointerToValues - colorLimitOne)  * 
                              convertToDouble) / (fullScale-colorLimitOne)));
         
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                  bubbleArr, fullLedOnCount, variableLedColor);
        }
        else if(i == fullLedOnCount * ledPerPixel){
          red = 0;
          blue = (uint8_t)(colorBlue * 
                           (variableLedColor * 
                            ((fullScale - (*pointerToValues * convertToDouble))/
                             fullScale)));
         
          green = (uint8_t)(colorGreen * 
                            (variableLedColor * 
                             (((*pointerToValues - colorLimitOne)* 
                               convertToDouble) / (fullScale-colorLimitOne)))); 
         
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                  bubbleArr, fullLedOnCount, variableLedColor);
        } 
        else if(i > fullLedOnCount * ledPerPixel){
          red = 0;
          blue = 0;
          green = 0;
          
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                  bubbleArr, fullLedOnCount, variableLedColor);
        }
      }
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *
//Interv�ls 40 - 50% no pilna displeja (za�� tonis)     
      else if((*pointerToValues >= colorLimitTwo) && 
              (*pointerToValues < colorLimitThree)){
               
        if(i <fullLedOnCount * ledPerPixel){   
          red = 0;
          blue = 0;
          green = (uint8_t)(colorGreen * 
                            (((*pointerToValues - colorLimitOne)* 
                              convertToDouble) / (fullScale-colorLimitOne)));
//          green = (uint8_t)(colorGreen * 
//                            ((fullScale/2 - 
//                              ((*pointerToValues - colorLimitOne)* 
//                               convertToDouble)) / (fullScale/2)));      
          
         
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                  bubbleArr, fullLedOnCount, variableLedColor);
        }
        else if(i == fullLedOnCount * ledPerPixel){
          red = 0;
          blue = 0;
          green = (uint8_t)(colorGreen * (variableLedColor) *
                           (((*pointerToValues - colorLimitOne)* 
                              convertToDouble) / fullScale));
         
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                  bubbleArr, fullLedOnCount, variableLedColor);
        } 
        else if(i > fullLedOnCount * ledPerPixel){
          red = 0;
          blue = 0;
          green = 0;
          
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                 bubbleArr, fullLedOnCount, variableLedColor);
        }
      }
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *    
//Interv�ls 50 - 60% no pilna displeja (za�� tonis)
      else if((*pointerToValues >= colorLimitThree) && 
              (*pointerToValues < colorLimitFour)){
               
        if(i < fullLedOnCount * ledPerPixel){  
          red = 0;
          blue = 0;
          green = (uint8_t)(colorGreen * 
                            ((fullScale - 
                              ((*pointerToValues - colorLimitThree)* 
                               convertToDouble)) / fullScale));
   
         
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                  bubbleArr, fullLedOnCount, variableLedColor);
        }
        else if(i == fullLedOnCount * ledPerPixel){
          red = 0;
          blue = 0;
          green = (uint8_t)(colorGreen * variableLedColor * 
                             ((fullScale-((*pointerToValues - colorLimitThree)* 
                                            convertToDouble)) / fullScale)); 
         
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                  bubbleArr, fullLedOnCount, variableLedColor);
        }
        else if(i > fullLedOnCount * ledPerPixel){
          red = 0;
          blue = 0;
          green = 0;
          
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                 bubbleArr, fullLedOnCount, variableLedColor);
        }
      }
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *    
//Interv�ls 60 - 85% no pilna displeja (sarkans/za�� tonis)
      else if((*pointerToValues >= colorLimitFour) && 
              (*pointerToValues < colorLimitFive)){
               
        if(i < fullLedOnCount * ledPerPixel){       
          red = (uint8_t)(colorRed * 
                          (((*pointerToValues - colorLimitFour) *
                            convertToDouble) / fullScale));
         
          blue = 0;
          green = (uint8_t)(colorGreen * 
                            ((fullScale - 
                              ((*pointerToValues - colorLimitThree) * 
                               convertToDouble)) / fullScale));
         
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                  bubbleArr, fullLedOnCount, variableLedColor);
        }
        else if(i == fullLedOnCount * ledPerPixel){
          red = (uint8_t)(colorRed * 
                          (variableLedColor * 
                           (((*pointerToValues - colorLimitFour) * 
                             convertToDouble) / fullScale)));
         
          blue = 0;
          green = (uint8_t)(colorGreen * 
                            (variableLedColor * 
                             ((fullScale-((*pointerToValues - colorLimitThree)* 
                                          convertToDouble)) / fullScale)));
         
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                  bubbleArr, fullLedOnCount, variableLedColor);
        } 
        else if(i > fullLedOnCount * ledPerPixel){
          red = 0;
          blue = 0;
          green = 0;
          
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                   bubbleArr, fullLedOnCount, variableLedColor);
        }
      }
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *    
//Interv�ls 85 - 100% no pilna displeja (sarkans tonis)
      else if((*pointerToValues >= colorLimitFive) && 
            (*pointerToValues <= fullScale)){
              
        if(i < fullLedOnCount * ledPerPixel){ 
          red = (uint8_t)(colorRed *
                          (((*pointerToValues - colorLimitFour) * 
                            convertToDouble) / fullScale));       
          blue = 0;
          green = 0;
        
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                 bubbleArr, fullLedOnCount, variableLedColor);
        }
        else if(i == fullLedOnCount * ledPerPixel){
          red = (uint8_t)(colorRed * variableLedColor * 
                          (((*pointerToValues - colorLimitFour) *
                            convertToDouble) / fullScale));       
          blue = 0;
          green = 0;    
        
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                 bubbleArr, fullLedOnCount, variableLedColor);
        }
        else if(i > fullLedOnCount * ledPerPixel){
          red = 0;
          blue = 0;
          green = 0;
          
          putDisplayDataInColorMap(colorMap, startAdress, i, red, blue, green, 
                                    bubbleArr, fullLedOnCount, variableLedColor);
        }  
      }
    }
  }   
 
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
 //Form� LED virk�u diodes (LED strips): 
//
//       RS485 channels location                LED count in strip
// 
//                     D     C     C                          35    2     2
//                     D     C     C                          35    2     2  
//     GGG             DCCCCCCCCCCCC       2-2|33             35-20-20-20-20
//       G             D                       33             35
//       G             D                       33             35
//     GGGBBBBB   BBBBBD                   2-2|33|7-7-   7-7-|35
//       G             D                       33             35
//       G             D                       33             35
//     GGG             D                   2-2|33             35
//       G             D                       33             35
//       G             D                       33             35
//       G             D                       33             35
//     BBBBBBBB   BBBBBB                   10-10-10-10   9-9-9-9
//       F             E                       33             35
//       F             E                       33             35
//       F             E                       33             35
//     FFF             E                   2-2|33             35
//       F             E                       33             35
//       F             E                       33             35
//     FFFBBBBB   BBBBBE                   2-2|33|7-7-   7-7-|35
//       F             E                       33             35
//       F             E                       33             35
//     FFF             EAAAAAAAAAAAA       2-2|33             35-20-20-20-20
//                     E     A     A                          35    2     2
//                     E     A     A                          35    2     2
// 
//  A - 24 LED X9 (ch6) on PCB RGB digital x6 v1.1. brd1 running Addr 
//  C - 24 LED X4 (ch1) on PCB RGB digital x6 v1.1. brd2 running Addr 
//  D - 35 LED X5 (ch2) on PCB RGB digital x6 v1.1. brd2 running Addr 
//  E - 35 LED X6 (ch3) on PCB RGB digital x6 v1.1. brd2 running Addr
//  B - 52 LED X7 (ch4) on PCB RGB digital x6 v1.1. brd2 running Addr 
//  F - 39 LED X8 (ch5) on PCB RGB digital x6 v1.1. brd2 running Addr
//  G - 39 LED X9 (ch6) on PCB RGB digital x6 v1.1. brd2 running Addr

  int32_t runLedPeriod, 
          runLedSpeed,
          channelX,
          chGState=0,
          chFState=0;
  
  static int32_t  chGStartPointFlag,
                  chFStartPointFlag,
                  moveLedDirection,
                  moveLedDirectionOld;  
                  
  static int32_t runLedPeriodCtr,
                 runLedStepCtr,
                 ledMoveFlag;
  
//==============================================================================  
       
  static int32_t chanALedPos[LED_IN_STRIP_A],
                 chanCLedPos[LED_IN_STRIP_C],
                 chanDLedPos[LED_IN_STRIP_D],
                 chanELedPos[LED_IN_STRIP_E],
                 chanBLedPos[LED_IN_STRIP_B],
                 chanFLedPos[LED_IN_STRIP_F],
                 chanGLedPos[LED_IN_STRIP_G];

  
  static int32_t chALedPosOld[LED_IN_STRIP_A],
                 chCLedPosOld[LED_IN_STRIP_C],
                 //chDLedPosOld[LED_IN_STRIP_D],
                 chELedPosOld[LED_IN_STRIP_E],
                 chBLedPosOld[LED_IN_STRIP_B],
                 chFLedPosOld[LED_IN_STRIP_F],
                 chGLedPosOld[LED_IN_STRIP_G];
//==============================================================================    
        
//Skrieno��s diodes �truma apr��ins
  
  if((*pointerToValues >= fullScale/2 - zeroPoint)&&(*pointerToValues <= fullScale/2 + zeroPoint)){
    runLedSpeed = runLedZeroSpeed;
  }
  else{
    runLedPeriod = (int32_t)((*pointerToValues * runLedFullPeriod)/ (fullScale));
    runLedSpeed = runLedFullPeriod/2 - abs(runLedFullPeriod/2-runLedPeriod) + 
                  runLedSpeedCorection;
  }  
  
  moveLedDirectionOld = moveLedDirection;
  
  if(runLedPeriodCtr >= runLedSpeed){
    runLedPeriodCtr = 0;
    runLedStepCtr++;
    ledMoveFlag = 1;
    
//********* 
    if(*pointerToValues >= fullScale/2 - zeroPoint){
      moveLedDirection = 2;//virziens uz priv�tbank�m (pa labi)
      }
    else if(*pointerToValues < fullScale/2 - zeroPoint){
      moveLedDirection = 1;//Uz valsts bank�m (pa kreisi)
    }      
//*********
    if(moveLedDirection != moveLedDirectionOld){
      runLedStepCtr = (runLedStep - runLedStepCtr);
    }
    
//Ieliek jaunu skrieno�o LED 
    if((runLedStepCtr >= runLedStep)){
        runLedStepCtr = 0; 
        
//virziens uz priv�t bank�m (pa labi)
      if(*pointerToValues > fullScale/2 + zeroPoint){
        
        chanBLedPos[chanBBreakPointAddrTwo] = 1;        
        moveLedDirection = 2;     
      }
      else if((*pointerToValues >= fullScale/2 - zeroPoint)&&(*pointerToValues <= fullScale/2 + zeroPoint)){
        chanBLedPos[chanBBreakPointAddrTwo] = 1;        
        moveLedDirection = 2; 
      } 
     
//virziens uz valsts bank�m (pa kreisi)
      else if(*pointerToValues < fullScale/2 - zeroPoint){
        
        chanALedPos[chanABreakPointAddrStart] = 1;
        chanCLedPos[chanABreakPointAddrStart] = 1;
        
        moveLedDirection = 1;//Uz valsts bank�m (pa kreisi)
      }
    }
  }
  
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  
//Nauda pl�st uz valsts bank�m pa kreisi  
  
  else if(moveLedDirection == 1){ 
    
  for(channelX = 0; channelX < LED_STRIP_COUNT; channelX++){
    startAdress += LED_COUNT * ledPerPixel;

    switch(channelX){
        
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      A - 24 LED X9 (ch6) brd1
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       
    case 0:{
//B�da visas LED uz priek�u pa 1 poz�ciju
        if (ledMoveFlag == 1){  
         for(i=(LED_IN_STRIP_A-1); i>=0; i--){ 
          if(i == (LED_IN_STRIP_A-1)){
            chanALedPos[i] = 0;
          }
//saglab� eso�o LED poz�ciju          
          chALedPosOld[i] = chanALedPos[i];
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�     
          if((chanALedPos[i] == 1)&&(i!=LED_IN_STRIP_A-1)){
            if((runLedStepCtr == 0)&&(i==0)){}
            else if(chanALedPos[i] == 1){
              
            chanALedPos[i+1] = chanALedPos[i];
            chanALedPos[i] = 0;
            }
          } 
        }
//iz��muma st�vok�i katrai dio�u rinai         
        for(i=(LED_IN_STRIP_A-1); i>=0; i--){     
          if((chanALedPos[chanABreakPointAddrOne+1] == 1)&&
             (i==chanABreakPointAddrOne)&&(chALedPosOld[i] == 1)){
               
            chanALedPos[chanABreakPointAddrOne+1] = 0;
            chanALedPos[chanABreakPointAddrFour] = 1;  
          }
                    
          if((chanALedPos[chanABreakPointAddrFive-2] == 1)&&
             (i==chanABreakPointAddrFive-2)){
               
            chanALedPos[chanABreakPointAddrTwo] = 1;
          }
          
          if((chanALedPos[chanABreakPointAddrThree + 1] == 1)&&
             (i==chanABreakPointAddrThree)&&(chALedPosOld[i] == 1)){
               
            chanALedPos[chanABreakPointAddrThree + 1] = 0;                   
          }          
        }     
      }

//dio�u poz�ocijas saliek kop�j� colorMap mas�v� 
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_A,
                                chanALedPos, colorRed, colorBlue, colorGreen);
      
     break;  
    }
      
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      C - 24 LED X4 (ch1) brd2
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *     
    case 1:{
//B�da visas LED uz priek�u pa 1 poz�ciju
        if (ledMoveFlag == 1){  
         for(i=(LED_IN_STRIP_C-1); i>=0; i--){ 
          if(i == (LED_IN_STRIP_C-1)){
            chanCLedPos[i] = 0;
          }
//saglab� eso�o LED poz�ciju          
          chCLedPosOld[i] = chanCLedPos[i];
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�     
          if((chanCLedPos[i] == 1)&&(i!=LED_IN_STRIP_C-1)){
            if((runLedStepCtr == 0)&&(i==0)){}
            else if(chanCLedPos[i] == 1){
              
            chanCLedPos[i+1] = chanCLedPos[i];
            chanCLedPos[i] = 0;
            }
          } 
        }
//iz��muma st�vok�i katrai dio�u rinai         
        for(i=(LED_IN_STRIP_C-1); i>=0; i--){     
          if((chanCLedPos[chanCBreakPointAddrOne+1] == 1)&&
             (i==chanCBreakPointAddrOne)&&(chCLedPosOld[i] == 1)){
               
            chanCLedPos[chanCBreakPointAddrOne+1] = 0;
            chanCLedPos[chanCBreakPointAddrFour] = 1;  
          }
                    
          if((chanCLedPos[chanCBreakPointAddrFive-2] == 1)&&
             (i==chanCBreakPointAddrFive-2)){
               
            chanCLedPos[chanCBreakPointAddrTwo] = 1;
          }
          
          if((chanCLedPos[chanCBreakPointAddrThree + 1] == 1)&&
             (i==chanCBreakPointAddrThree)&&(chCLedPosOld[i] == 1)){
               
            chanCLedPos[chanCBreakPointAddrThree + 1] = 0;                   
          }          
        }     
      }
      
//dio�u poz�ocijas saliek kop�j� colorMap mas�v�  
      
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_C, 
                                chanCLedPos, colorRed, colorBlue, colorGreen);

    break;
    }
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      D - 35 LED X5 (ch2) brd2
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *     
    case 2:{
      if (ledMoveFlag == 1){
        if(chanCLedPos[chanCBreakPointAddrEnd - 1] == 1){
          chanDLedPos[chanDBreakPointAddrOne-2] = 1;
        }
        
        for(i=(LED_IN_STRIP_D-1); i>=0; i--){ 
          
          if(i == (LED_IN_STRIP_D-1)){
            chanDLedPos[i] = 0;
          }
 //saglab� eso�o LED poz�ciju 
          //chDLedPosOld[i] = chanDLedPos[i];
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�     
          if((chanDLedPos[i] == 1)&&(i != LED_IN_STRIP_D-1)){
            if((chanDLedPos[chanDBreakPointAddrOne-2] == 1) && 
               (chanCLedPos[chanCBreakPointAddrEnd-1] == 1) && (i == 0)){}
            
            else if(chanDLedPos[i] == 1){
              
              chanDLedPos[i+1] = chanDLedPos[i];
              chanDLedPos[i] = 0;
            }
          } 
        } 
      }
      
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_D, 
                                chanDLedPos, colorRed, colorBlue, colorGreen);

    break; 
    }
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      E - 35 LED X6 (ch3) brd2
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *
    case 3:{
      if (ledMoveFlag == 1){
        if(chanALedPos[chanABreakPointAddrEnd - 1] == 1){
          chanELedPos[chanEBreakPointAddrOne-2] = 1;
        }
        
        for(i=(LED_IN_STRIP_E-1); i>=0; i--){ 
          chELedPosOld[i] = chanELedPos[i];
          if(i == (LED_IN_STRIP_E-1)){
            chanELedPos[i] = 0;
          }
//saglab� eso�o LED poz�ciju 
    
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�     
          if((chanELedPos[i] == 1)&&(i != LED_IN_STRIP_E-1)){
            if((chanELedPos[chanEBreakPointAddrOne-2] == 1) && 
               (chanALedPos[chanABreakPointAddrEnd-1] == 1) && (i == 0)){}
            
            else if(chanELedPos[i] == 1){
              
              chanELedPos[i+1] = chanELedPos[i];
              chanELedPos[i] = 0;
            }
          } 
        } 
      }      
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_E, 
                                chanELedPos, colorRed, colorBlue, colorGreen);
    break;
    }       
     
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      B - 52 LED X7 (ch4) brd1
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *    
    case 4:{
      if (ledMoveFlag == 1){
        for(i=(LED_IN_STRIP_B-1); i>=0; i--){ 
          chBLedPosOld[i] = chanBLedPos[i];
//Ja skrieno�� LED ir "kritiskaj� pukt�", kas virtu�l� celi�a beigu bunts, 
//ieliek 0 v�rt�bu, jo �aj� viet� skrieno�ai LED ir j�beidzas. ja �� pras�ba 
//nav izpild�ta, ja LED skrien t�l�k          
          if(i == (LED_IN_STRIP_B-1)){
            chanBLedPos[i] = 0;
          }
          if(i == chanBBreakPointAddrTwo){
            chanBLedPos[i] = 0;
          }
          if(i == chanBBreakPointAddrFour){
            chanBLedPos[i] = 0;
          }
          
 //saglab� eso�o LED poz�ciju 
          //chELedPosOld[i] = chanELedPos[i];
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�     
          if((chanBLedPos[i] == 1)&&(i != LED_IN_STRIP_B-1)){
            if((chanBLedPos[chanBBreakPointAddrThree] == 1) && 
               (chanDLedPos[chanDBreakPointAddrTwo+1] == 1) && (i == chanBBreakPointAddrThree)){
            }
            if((chanBLedPos[chanBBreakPointAddrFive] == 1) && 
               (chanELedPos[chanEBreakPointAddrTwo+1] == 1) && (i == chanBBreakPointAddrFive)){
            }            
            if((chanBLedPos[chanBBreakPointAddrStart] == 1) && 
               (chELedPosOld[chanEBreakPointAddrEnd] == 1) && (i == chanBBreakPointAddrStart)){
            } 
            else{// if(chanBLedPos[i] == 1){
              
              chanBLedPos[i+1] = chanBLedPos[i];
              chanBLedPos[i] = 0;
            }
          } 
        } 
        if(chanDLedPos[chanDBreakPointAddrTwo+1] == 1){
          chanBLedPos[chanBBreakPointAddrThree] = 1;
        }
        if(chanELedPos[chanEBreakPointAddrTwo+1] == 1){
          chanBLedPos[chanBBreakPointAddrFive] = 1;
        }
        if(chELedPosOld[chanEBreakPointAddrEnd] == 1){
          chanBLedPos[chanBBreakPointAddrStart] = 1;
        }  
      }
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_B, 
                                chanBLedPos, colorRed, colorBlue, colorGreen);                
    break;   
    }       
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  
//                      F - 39 LED X8 (ch5) brd2    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *      
    case 5:{
    if (ledMoveFlag == 1){ 
      
/* Cikls iet cauri visai dio�i celi�a rinai un b�da skrieno�o LED uz priek�u
pieskaitot adresei 1 un iev�rojot kritisko punktu noteiktos iz��muma gad�jumus*/     
      for(i=LED_IN_STRIP_F-1; i>=0; i--){
        chFLedPosOld[i] = chanFLedPos[i];
        if(i == (LED_IN_STRIP_F-1)){
          chanFLedPos[i] = 0;
        }
/*        
Ja skrieno�� LED ir "kritiskaj� pukt�", kas virtu�l� celi�a beigu punts, 
adres� ieliek 0 v�rt�bu, jo �aj� viet� skrieno�ai LED ir j�beidzas. ja �� pras�ba 
nav izpild�ta, LED skrien t�l�k uz n�kamo celi�u*/  
        
        if(i == chanFBreakPointAddrThree){
          chanFLedPos[i] = 0;
        }
/*        
iz��muma st�vok�i katrai dio�u rinai. Ja pirm� adres� ir "1", �o poz�ciju 
neb�da uz priek�u, jo ielikta ir jauna skrieno�� diode pirmaj� adres�. 
To b�da tikai n�kamaj� b�d��anas takt�*/
        
        if((chanFLedPos[chanFBreakPointAddrOne] == 1) && 
           (chBLedPosOld[chanBBreakPointAddrEnd] == 1) && 
           (i == chanFBreakPointAddrOne)){
           }
/*Ja ir skrieno�� LED ir 1, un izpikl�ds ieprieks�j� sol� noteiktais 
(skrienio��) LED nav iz��muma gad�jums, skrieno�o LEDto b�da uz priek�u par 
1 adresi */
        
        else if((chanFLedPos[i] == 1) && (i >= chanFBreakPointAddrOne)){
          
          chanFLedPos[i+1] = chanFLedPos[i];
          chanFLedPos[i] = 0;
        }
        
        else if((chanFLedPos[i] == 1) && (i < chanFBreakPointAddrOne) && 
                (chFState == 0)){
          chFState = 1;
          chanFLedPos[i-1] = chanFLedPos[i];
          chanFLedPos[i] = 0;
          chFLedPosOld[i] = 0;
        }
        
      } 
/* jaunas skrieno��s LED inicializ�cija un 0 uzst�d��ana uz virtu�l� ce�i�a. Inicializ�cija 
notiek, ja iepriek��j� celi�a adres� ir "1"*/  
      
      if(chBLedPosOld[chanBBreakPointAddrEnd] == 1){
        chanFLedPos[chanFBreakPointAddrOne] = 1;
      }
      if(chanFLedPos[chanFBreakPointAddrTwo+1] == 1){
        chanFLedPos[chanFBreakPointAddrFour] = 1;
      }      
      if(chFLedPosOld[chanFBreakPointAddrFour+1] == 1){
        chanFLedPos[chanFBreakPointAddrFour+2] = 0;
      } 
      if(chanFLedPos[chanFBreakPointAddrOne+1] == 1){
        chanFLedPos[chanFBreakPointAddrFive] = 1;
        chanFLedPos[chanFBreakPointAddrOne-1] = 1;
      }  
      if(chFLedPosOld[chanFBreakPointAddrFive+1] == 1){
        chanFLedPos[chanFBreakPointAddrFive+2] = 0;
      }
      if(chFStartPointFlag == 1){
        chanFLedPos[chanFBreakPointAddrStart] = 0;
        chFStartPointFlag = 0;
        chanFLedPos[chanFBreakPointAddrSix] = 1;
      }
      if(chanFLedPos[chanFBreakPointAddrStart] == 1){
        chFStartPointFlag = 1;
      }  
      chFState = 0;
    }
    putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_F, 
                                chanFLedPos, colorRed, colorBlue, colorGreen);
    break; 
    } 
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  
//                      G - 39 LED X9 (ch6) brd2    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *      
  case 6:{    
    if (ledMoveFlag == 1){
       
/* Cikls iet cauri visai dio�i celi�a rinai un b�da skrieno�o LED uz priek�u
pieskaitot adresei 1 un iev�rojot kritisko punktu noteiktos iz��muma gad�jumus*/     
      for(i=LED_IN_STRIP_G-1; i>=0; i--){
        chGLedPosOld[i] = chanGLedPos[i];
 
        if(i == (LED_IN_STRIP_G-1)){
          chanGLedPos[i] = 0;
        }
/*        
Ja skrieno�� LED ir "kritiskaj� pukt�", kas virtu�l� celi�a beigu punts, 
adres� ieliek 0 v�rt�bu, jo �aj� viet� skrieno�ai LED ir j�beidzas. ja �� pras�ba 
nav izpild�ta, LED skrien t�l�k uz n�kamo celi�u*/  
        
        if(i == chanGBreakPointAddrThree){
          chanGLedPos[i] = 0;
        }
/*        
iz��muma st�vok�i katrai dio�u rinai. Ja pirm� adres� ir "1", �o poz�ciju 
neb�da uz priek�u, jo ielikta ir jauna skrieno�� diode pirmaj� adres�. 
To b�da tikai n�kamaj� b�d��anas takt�*/
        
        if((chanGLedPos[chanGBreakPointAddrOne] == 1) && 
           (chBLedPosOld[chanBBreakPointAddrFour] == 1) && 
           (i == chanGBreakPointAddrOne)){
           }

/*Ja ir skrieno�� LED ir 1, un izpikl�ds ieprieks�j� sol� noteiktais 
(skrienio��) LED nav iz��muma gad�jums, skrieno�o LEDto b�da uz priek�u par 
1 adresi */
        
        else if((chanGLedPos[i] == 1) && (i >= chanGBreakPointAddrOne)){
          
          chanGLedPos[i+1] = chanGLedPos[i];
          chanGLedPos[i] = 0;
        }
        
        else if((chanGLedPos[i] == 1) && (i < chanGBreakPointAddrOne) && 
                (chGState == 0)){
          chGState = 1;
          chanGLedPos[i-1] = chanGLedPos[i];
          chanGLedPos[i] = 0;
          chGLedPosOld[i] = 0;
        }
        
      } 
/* jaunas skrieno��s LED inicializ�cija un 0 uzst�d��ana uz virtu�l� ce�i�a. Inicializ�cija 
notiek, ja iepriek��j� celi�a adres� ir "1"*/  
      
      if(chBLedPosOld[chanBBreakPointAddrFour] == 1){
        chanGLedPos[chanGBreakPointAddrOne] = 1;
      }
      if(chanGLedPos[chanGBreakPointAddrTwo+1] == 1){
        chanGLedPos[chanGBreakPointAddrFour] = 1;
      }      
      if(chGLedPosOld[chanGBreakPointAddrFour+1] == 1){
        chanGLedPos[chanGBreakPointAddrFour+2] = 0;
      } 
      if(chanGLedPos[chanGBreakPointAddrOne+1] == 1){
        chanGLedPos[chanGBreakPointAddrFive] = 1;
        chanGLedPos[chanGBreakPointAddrOne-1] = 1;
      }  
      if(chGLedPosOld[chanGBreakPointAddrFive+1] == 1){
        chanGLedPos[chanGBreakPointAddrFive+2] = 0;
      }
      if(chGStartPointFlag == 1){
        chanGLedPos[chanGBreakPointAddrStart] = 0;
        chGStartPointFlag = 0;
        chanGLedPos[chanGBreakPointAddrSix] = 1;
      }
      if(chanGLedPos[chanGBreakPointAddrStart] == 1){
        chGStartPointFlag = 1;
      }      
      chGState = 0;
    }
    putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_G, 
                                chanGLedPos, colorRed, colorBlue, colorGreen);
    break; 
    } 
    }
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  
      
  }
  
  ledMoveFlag = 0;
  }
  
//##############################################################################
//##############################################################################
//############################################################################## 
//############################################################################## 
//Nauda pl�st uz komercbank�m
  
  else if(moveLedDirection == 2){ 
    
  for(channelX = 0; channelX < LED_STRIP_COUNT; channelX++){
    startAdress += LED_COUNT * ledPerPixel;
  
    switch(channelX){
        
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      A - 24 LED X9 (ch6) brd1
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       
    case 0:{
   //A - 24 LED X9 (ch6) brd1

      if (ledMoveFlag == 1){  
        //LED neiet t�l�k par s�kuma adresi
        if(chanALedPos[chanABreakPointAddrTwo] == 1){
        chanALedPos[chanABreakPointAddrTwo] = 0;
        } 
        if(chanALedPos[chanABreakPointAddrStart] == 1){
        chanALedPos[chanABreakPointAddrStart] = 0;
        }          

        for(i=0; i<LED_IN_STRIP_A; i++){
          chALedPosOld[i] = chanALedPos[i];
         
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�  
          if((chanELedPos[chanEBreakPointAddrOne] == 1) && 
             (chanALedPos[chanABreakPointAddrEnd] == 1) && (i == chanABreakPointAddrEnd))
          {} 
          if((chanALedPos[chanABreakPointAddrFive-1] == 1) && 
             (chanALedPos[chanABreakPointAddrTwo+1] == 1) && (i == chanABreakPointAddrTwo+1))
          {}  
          
          else if (chanALedPos[i] == 1){ 
            
            chanALedPos[i-1] = chanALedPos[i];
            chanALedPos[i] = 0;
          }
        }
 //LED s�kuma v�rt�bas (celi�a inicializ�cija)       
        if(chanELedPos[chanEBreakPointAddrOne] == 1){
          chanALedPos[chanABreakPointAddrEnd] = 1;
        }
        if(chanALedPos[chanABreakPointAddrFive-1] == 1){
          chanALedPos[chanABreakPointAddrTwo+1] = 1;
        }
        if(chALedPosOld[chanABreakPointAddrFour] == 1){
           chanALedPos[chanABreakPointAddrOne] = 1;
        }
      }

//dio�u poz�ocijas saliek kop�j� colorMap mas�v� 
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_A,
                                chanALedPos, colorRed, colorBlue, colorGreen);
      
     break;  
    }
      
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      C - 24 LED X4 (ch1) brd2
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *     
    case 1:{

//B�da visas LED uz priek�u pa 1 poz�ciju
      if (ledMoveFlag == 1){  

        //LED neiet t�l�k par s�kuma adresi
      //  if(chanCLedPos[chanCBreakPointAddrFour] == 1){
      //  chanCLedPos[chanCBreakPointAddrFour] = 0;
      //  } 
        if(chanCLedPos[chanCBreakPointAddrTwo] == 1){
        chanCLedPos[chanCBreakPointAddrTwo] = 0;
        } 
        if(chanCLedPos[chanCBreakPointAddrStart] == 1){
        chanCLedPos[chanCBreakPointAddrStart] = 0;
        }          

        for(i=0; i<LED_IN_STRIP_C; i++){
          chCLedPosOld[i] = chanCLedPos[i];
         
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�  
          if((chanDLedPos[chanDBreakPointAddrOne] == 1) && 
             (chanCLedPos[chanCBreakPointAddrEnd] == 1) && (i == chanCBreakPointAddrEnd))
          {} 
          if((chanCLedPos[chanCBreakPointAddrFive-1] == 1) && 
             (chanCLedPos[chanCBreakPointAddrTwo+1] == 1) && (i == chanCBreakPointAddrTwo+1))
          {}  
          
          else if (chanCLedPos[i] == 1){ 
            
            chanCLedPos[i-1] = chanCLedPos[i];
            chanCLedPos[i] = 0;
          }
        }
 //LED s�kuma v�rt�bas (celi�a inicializ�cija)       
        if(chanDLedPos[chanDBreakPointAddrOne] == 1){
          chanCLedPos[chanCBreakPointAddrEnd] = 1;
        }
        if(chanCLedPos[chanCBreakPointAddrFive-1] == 1){
          chanCLedPos[chanCBreakPointAddrTwo+1] = 1;
        }
        if(chCLedPosOld[chanCBreakPointAddrFour] == 1){
           chanCLedPos[chanCBreakPointAddrOne] = 1;
        }        
      }        
      
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_C, 
                                chanCLedPos, colorRed, colorBlue, colorGreen);

    break;
    }
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      D - 35 LED X5 (ch2) brd2
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *     
    case 2:{

      if (ledMoveFlag == 1){
        //LED neiet t�l�k par s�kuma adresi
        if(chanDLedPos[chanDBreakPointAddrStart] == 1){
        chanDLedPos[chanDBreakPointAddrStart] = 0;
        } 
       
        
        for(i=0; i<LED_IN_STRIP_D; i++){
          //chDLedPosOld[i] = chanDLedPos[i];
         
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�  
          if((chanBLedPos[chanBBreakPointAddrStart] == 1) && 
             (chanDLedPos[chanDBreakPointAddrEnd] == 1) && (i == chanDBreakPointAddrEnd))
          {}          
          else if(chanDLedPos[i] == 1){          
          chanDLedPos[i-1] = chanDLedPos[i];
          chanDLedPos[i] = 0;
          }
        }
 //LED s�kuma v�rt�bas (celi�a inicializ�cija)       
        if(chanBLedPos[chanBBreakPointAddrStart] == 1){
          chanDLedPos[chanDBreakPointAddrEnd] = 1;
        }        
      }
      
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_D, 
                                chanDLedPos, colorRed, colorBlue, colorGreen);

    break; 
    }
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      E - 35 LED X6 (ch3) brd2
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *
    case 3:{
      
      if (ledMoveFlag == 1){
        //LED neiet t�l�k par s�kuma adresi
        if(chanELedPos[chanEBreakPointAddrStart] == 1){
        chanELedPos[chanEBreakPointAddrStart] = 0;
        } 
       
        
        for(i=0; i<LED_IN_STRIP_E; i++){
          chELedPosOld[i] = chanELedPos[i];
         
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�  
          if((chanBLedPos[chanBBreakPointAddrStart] == 1) && 
             (chanELedPos[chanEBreakPointAddrEnd] == 1) && (i == chanEBreakPointAddrEnd))
          {}          
          else if(chanELedPos[i] == 1){          
          chanELedPos[i-1] = chanELedPos[i];
          chanELedPos[i] = 0;
          }
        }
 //LED s�kuma v�rt�bas (celi�a inicializ�cija)       
        if(chanBLedPos[chanBBreakPointAddrStart] == 1){
          chanELedPos[chanEBreakPointAddrEnd] = 1;
        }
      }
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_E, 
                                chanELedPos, colorRed, colorBlue, colorGreen);
    break;
    }       
     
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * 
//                      B - 52 LED X7 (ch4) brd1
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *    
    case 4:{
      
      if (ledMoveFlag == 1){
        //LED neiet t�l�k par s�kuma adresi
        if(chanBLedPos[chanBBreakPointAddrStart] == 1){
        chanBLedPos[chanBBreakPointAddrStart] = 0;
        } 
        if(chanBLedPos[chanBBreakPointAddrThree] == 1){
        chanBLedPos[chanBBreakPointAddrThree] = 0;
        }
        if(chanBLedPos[chanBBreakPointAddrFive] == 1){
        chanBLedPos[chanBBreakPointAddrFive] = 0;
        }         
        
        for(i=0; i<LED_IN_STRIP_B; i++){
          chBLedPosOld[i] = chanBLedPos[i];
         
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� adres�. To b�da tikai n�kamaj� b�d��anas takt�  
          if((chanGLedPos[chanGBreakPointAddrOne+1] == 1) && 
             (chanBLedPos[chanBBreakPointAddrFour] == 1) && (i == chanBBreakPointAddrFour))
          {}          
          if((chanFLedPos[chanFBreakPointAddrOne+1] == 1) && 
             (chanBLedPos[chanBBreakPointAddrEnd] == 1) && (i == chanBBreakPointAddrEnd))
          {}  
          else if(chanBLedPos[i] == 1){          
          chanBLedPos[i-1] = chanBLedPos[i];
          chanBLedPos[i] = 0;
          }
        }
 //LED s�kuma v�rt�bas (celi�a inicializ�cija)       
        if(chanGLedPos[chanGBreakPointAddrOne+1] == 1){
          chanBLedPos[chanBBreakPointAddrFour] = 1;
        }
        if(chanFLedPos[chanFBreakPointAddrOne+1] == 1){
          chanBLedPos[chanBBreakPointAddrEnd] = 1;
        }       
      }
      putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_B, 
                                chanBLedPos, colorRed, colorBlue, colorGreen);                
    break;   
    }       
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  
//                      F - 39 LED X8 (ch5) brd2    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *      
    case 5:{
    if (ledMoveFlag == 1){
      //LED neiet t�l�k par adresi "chanFBreakPointAddrOne".Liek s�kum�,
      //lai satiktos LED krustpunt�
      if(chanFLedPos[chanFBreakPointAddrOne] == 1){
        chanFLedPos[chanFBreakPointAddrOne] = 0;
      }
      if(chanFLedPos[chanFBreakPointAddrOne-1] == 1){
        chanFLedPos[chanFBreakPointAddrOne-1] = 0;
      } 
      
      for(i=0; i<LED_IN_STRIP_F; i++){
        
        chFLedPosOld[i] = chanFLedPos[i];
        
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� virtu�l� celi�a adres�. To b�da tikai n�kamaj� 
//b�d��anas takt�     
        if(chanFLedPos[i] == 1){//&&(i != LED_IN_STRIP_F-1)){
          if((chanFLedPos[chanFBreakPointAddrTwo+2] == 1) && 
             (chanFLedPos[chanFBreakPointAddrFour+1] == 1) && (i == chanFBreakPointAddrFour+1))
          {}
          if((chanFLedPos[chanFBreakPointAddrTwo+2] == 1) && 
             (chanFLedPos[chanFBreakPointAddrEnd] == 1) && (i == chanFBreakPointAddrEnd))
          {}           
     
//Pab�da diodes pa vienu pz�ciju pa labi (uz priek�u)        
          else if((chanFLedPos[i] == 1)&&(i < chanFBreakPointAddrOne) && (chFState == 0)){
            chFState = 1;
            chanFLedPos[i+1] = chanFLedPos[i];
            chanFLedPos[i] = 0;
          }  
          else if((chanFLedPos[i] == 1) && (i >= chanFBreakPointAddrOne)){
            chanFLedPos[i-1] = chanFLedPos[i];
            chanFLedPos[i] = 0;            
          }
        }  
      }
      
//ja ir B kan�l� skrieno�� diode, ieliek s�kuma st�vokl� diodi F kan�l�      
      if(chanBLedPos[chanBBreakPointAddrOne] == 1){
        chanFLedPos[chanFBreakPointAddrThree] = 1;
      }
      if(chanFLedPos[chanFBreakPointAddrTwo+2] == 1){
        chanFLedPos[chanFBreakPointAddrFour+1] = 1;
      } 
      if(chFLedPosOld[chanFBreakPointAddrFour] == 1){
        chanFLedPos[chanFBreakPointAddrFour-1] = 0;
      }
      //Uzst�da jaunu LED adres� "chanFBreakPointAddrFive+1" 36
      if(chanFLedPos[chanFBreakPointAddrOne+2] == 1){
        chanFLedPos[chanFBreakPointAddrFive+1] = 1;
      } 
      //LED neiet t�l�k par adresi "chanFBreakPointAddrFive"
      if(chFLedPosOld[chanFBreakPointAddrFive] == 1){
        chanFLedPos[chanFBreakPointAddrFive-1] = 0;
      }

      //Uzst�da jaunu LED adres� "chanFBreakPointAddrEnd" 38
      if(chanFLedPos[chanFBreakPointAddrTwo+2] == 1){
        chanFLedPos[chanFBreakPointAddrEnd] = 1;
      }
      if(chFLedPosOld[chanFBreakPointAddrEnd-1] == 1){
        chanFLedPos[chanFBreakPointAddrEnd-2] = 0;
      }
      //Uzst�da jaunu LED adres� "chanFBreakPointAddrStart" 0
      if(chFLedPosOld[chanFBreakPointAddrEnd-1] == 1){
        chanFLedPos[chanFBreakPointAddrStart] = 1;
        
      }
       chFState = 0;
    }
    putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_F, 
                                chanFLedPos, colorRed, colorBlue, colorGreen);
    break; 
    } 
    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  
//                      G - 39 LED X9 (ch6) brd2    
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *      
    case 6:{
    
    if (ledMoveFlag == 1){
//LED neiet t�l�k par adresi "chanFBreakPointAddrOne".Liek s�kum�,
      //lai satiktos LED krustpunt�
      if(chanGLedPos[chanGBreakPointAddrOne] == 1){
        chanGLedPos[chanGBreakPointAddrOne] = 0;
      }
      if(chanGLedPos[chanGBreakPointAddrOne-1] == 1){
        chanGLedPos[chanGBreakPointAddrOne-1] = 0;
      } 
      
      for(i=0; i<LED_IN_STRIP_G; i++){
        
        chGLedPosOld[i] = chanGLedPos[i];
        
//Ja pirm� adres� ir "1", �o poz�ciju neb�da uz priek�u, jo ielikta ir jauna
//skrieno�� diode pirmaj� virtu�l� celi�a adres�. To b�da tikai n�kamaj� 
//b�d��anas takt�     
        if(chanGLedPos[i] == 1){
          if((chanGLedPos[chanGBreakPointAddrTwo+2] == 1) && 
             (chanGLedPos[chanGBreakPointAddrFour+1] == 1) && (i == chanGBreakPointAddrFour+1))
          {}
          if((chanGLedPos[chanGBreakPointAddrTwo+2] == 1) && 
             (chanGLedPos[chanGBreakPointAddrEnd] == 1) && (i == chanGBreakPointAddrEnd))
          {}           
     
//Pab�da diodi pa vienu pz�ciju pa labi (uz priek�u)        
          else if((chanGLedPos[i] == 1)&&(i < chanGBreakPointAddrOne) && (chGState == 0)){
            chGState = 1;
            chanGLedPos[i+1] = chanGLedPos[i];
            chanGLedPos[i] = 0;
          }  
          else if((chanGLedPos[i] == 1) && (i >= chanGBreakPointAddrOne)){
            chanGLedPos[i-1] = chanGLedPos[i];
            chanGLedPos[i] = 0;            
          }
        }  
      }
      
//ja ir B kan�l� skrieno�� diode, ieliek s�kuma st�vokl� diodi F kan�l�      
      if(chanBLedPos[chanBBreakPointAddrOne] == 1){
        chanGLedPos[chanGBreakPointAddrThree] = 1;
      }
      if(chanGLedPos[chanGBreakPointAddrTwo+2] == 1){
        chanGLedPos[chanGBreakPointAddrFour+1] = 1;
      } 
      if(chGLedPosOld[chanGBreakPointAddrFour] == 1){
        chanGLedPos[chanGBreakPointAddrFour-1] = 0;
      }
      //Uzst�da jaunu LED adres� "chanFBreakPointAddrFive+1" 36
      if(chanGLedPos[chanGBreakPointAddrOne+2] == 1){
        chanGLedPos[chanGBreakPointAddrFive+1] = 1;
      } 
      //LED neiet t�l�k par adresi "chanFBreakPointAddrFive"
      if(chGLedPosOld[chanGBreakPointAddrFive] == 1){
        chanGLedPos[chanGBreakPointAddrFive-1] = 0;
      }

      //Uzst�da jaunu LED adres� "chanFBreakPointAddrEnd" 38
      if(chanGLedPos[chanGBreakPointAddrTwo+2] == 1){
        chanGLedPos[chanGBreakPointAddrEnd] = 1;
      }
      if(chGLedPosOld[chanGBreakPointAddrEnd-1] == 1){
        chanGLedPos[chanGBreakPointAddrEnd-2] = 0;
      }
      //Uzst�da jaunu LED adres� "chanFBreakPointAddrStart" 0
      if(chGLedPosOld[chanGBreakPointAddrEnd-1] == 1){
        chanGLedPos[chanGBreakPointAddrStart] = 1;        
      }
       chGState = 0;
    } 
    
    putLedStripDataInColorMap(colorMap, startAdress, LED_IN_STRIP_G, 
                                chanGLedPos, colorRed, colorBlue, colorGreen);
    break; 
    } 
    }
//*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  
      
  }  
  ledMoveFlag = 0;  
  }  
  
   runLedPeriodCtr++; 
  return colorMap;
}

//*****************************************************************************
// ***********************   INTERNAL FUNCTION DEFINITION   *******************
//*****************************************************************************

void putDisplayDataInColorMap(uint8_t *map, int32_t adress, int32_t i, 
                              int8_t r, int8_t b, int8_t g, 
                              int32_t *ptrToBubbleArr,
                              int32_t ledOnCount, double varLedValue){
                                
  int32_t   bubbleValRed,
            bubbleValGreen,
            bubbleValBlue;
  const int32_t dimmBacklightLed = 1;//ekrana backlight diozu skaits, kas ir izslegts vai blavaks

/*Ekr�na apgaismojumaizveide virs kr�su indik�cijas l�me�a*/  
    if((g == 0) && (b == 0) && (r == 0)){
/*vien�da spilgtuma ekr�na apgaismojuma diodes*/ 
      if(i > ledOnCount*3 + 3 + dimmBacklightLed*3){//solis pa 3
      *(map+adress + i) = (uint8_t)(colorRed/dyspBacklightDivid);
      *(map+adress + i+1) = (uint8_t)(colorBlue/dyspBacklightDividBlueCol);
      *(map+adress + i+2) = (uint8_t)(colorGreen/dyspBacklightDivid);
    }
/* pirm� ekr�na apgaismojuma diode virs kr�sainaj�m diod�m maina spo�umu 
atkar�b� no p�d�j�s kr�s�in�s diodes spo�uma intensit�tes. Ja p�d�j� kr�sain� 
diode ir ar maz�ku intensit�ti, ekr�na apgaismojuma pirm� diode ar� ir ar 
maz�ku intesnsit�ti. ja �is netiek iev�rots, ekr�n� redzams kr�su p�rb�des
l�ciens */    
      else if (i == ledOnCount*3 + 3 + dimmBacklightLed*3){
      *(map+adress + i) = (uint8_t)((1 - varLedValue) * colorRed/dyspBacklightDivid);
      *(map+adress + i+1) = (uint8_t)((1 - varLedValue) * colorBlue/dyspBacklightDividBlueCol);
      *(map+adress + i+2) = (uint8_t)((1 - varLedValue) * colorGreen/dyspBacklightDivid);
    }
  }
  else{
    if(g == 0){
      bubbleValGreen=0;
    } else {
      bubbleValGreen = *(ptrToBubbleArr + adress/3 + i/3);
    }
    if(r == 0){
      bubbleValRed=0;
    } else {
      bubbleValRed = *(ptrToBubbleArr + adress/3 + i/3);
    }
    if(b == 0){
      bubbleValBlue=0;
    } else {
      bubbleValBlue = *(ptrToBubbleArr + adress/3 + i/3);
    } 
    
    if(abs(bubbleValGreen) > g){
      if(bubbleValGreen>0){
         bubbleValGreen = 1+(int32_t)((bubbleValGreen * g) / (COLOR_DEVIATION/2));
      }else if(bubbleValGreen<0){
      bubbleValGreen = (int32_t)((bubbleValGreen * g) / (COLOR_DEVIATION/2));
     }     
    }
    if(abs(bubbleValRed) > r) {
      if(bubbleValRed>0){
         bubbleValRed = 1+(int32_t)((bubbleValRed * r) / (COLOR_DEVIATION/2));
      }else if(bubbleValRed<0) {
        bubbleValRed = (int32_t)((bubbleValRed * r) / (COLOR_DEVIATION/2)); 
      }     
    }      
    if(abs(bubbleValBlue) > b){
      if(bubbleValBlue>0){
         bubbleValBlue = 1+(int32_t)((bubbleValBlue * b) / (COLOR_DEVIATION/2));
      }else if(bubbleValBlue < 0){
        bubbleValBlue = (int32_t)((bubbleValBlue * b) / (COLOR_DEVIATION/2));  
      }    
    }  
    *(map+adress + i) =  (g + bubbleValGreen);
    *(map+adress + i+1) = (b + bubbleValBlue);
    *(map+adress + i+2) =  (r + bubbleValRed);
  }
}

//*****************************************************************************
//*****************************************************************************

void putLedStripDataInColorMap(uint8_t *map, int32_t adress, 
                               uint32_t ledInStrip, int32_t *chanXLedPos, 
                               uint8_t r, uint8_t b, uint8_t g){
  int32_t i;
                                 
  for(i=ledInStrip-1; i>=0; i--){
         
    if(*(chanXLedPos + i) == 1){
//Set runing LED color
      *(map+adress + i*3) =  g;//green
      *(map+adress + i*3 + 1) = b;//blue
      *(map+adress + i*3 + 2) = r; //red
    }
//Set LED strip color    
    if(*(chanXLedPos + i) == 0){
      *(map+adress + i*3) =  g/2;
      *(map+adress + i*3 + 1) = 0;
      *(map+adress + i*3 + 2) = 0;      
    }        
  }   
}

//*****************************************************************************
//*****************************************************************************