#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "D3_main.h"



//*****************************************************************************
// *********************   EXTERNAL FUNCTION DECLARATION  *********************
//*****************************************************************************


//*****************************************************************************
// *********************   INTERNAL FUNCTION DECLARATION   ********************
//*****************************************************************************


//******************************************************************************
//******************************************************************************
//******************************************************************************

int32_t * CalculateEconomicValue(int32_t PositionDelayNormalized[], 
                                 int32_t *pointerToEncoderCalibrateZero,  
                                 int32_t *pointerToEncoderCalibrateMaxNorm){ 

  int32_t static economicValue[DISPLAY_COUNT];
  int32_t i,
          positionRelativeToZero[ENC_COUNT];
  
  const int32_t normalizeToScale = 127;
  
  for(i=0; i<ENC_COUNT; i++){
    if(PositionDelayNormalized[i] <= *(pointerToEncoderCalibrateZero + i)){
      positionRelativeToZero[i] = ((PositionDelayNormalized[i] - 
                                  *(pointerToEncoderCalibrateZero + i)) * normalizeToScale) /
                                ( *(pointerToEncoderCalibrateZero + i));
    }
    else if(PositionDelayNormalized[i] > *(pointerToEncoderCalibrateZero + i)){
      positionRelativeToZero[i] = ((PositionDelayNormalized[i] - 
                                 *(pointerToEncoderCalibrateZero + i)) * normalizeToScale) /
                                ((*(pointerToEncoderCalibrateMaxNorm + i) -
                                  *(pointerToEncoderCalibrateZero + i)));
    } 
  }  
  economicValue[0] = (positionRelativeToZero[0] * (1) + 
                      positionRelativeToZero[1] * (1) + 
                      positionRelativeToZero[2] * (-1)) + 
                      normalizeToScale * ENC_COUNT; 
    
  return economicValue; 
}

//*****************************************************************************
// ***********************   INTERNAL FUNCTION DEFINITION   *******************
//*****************************************************************************