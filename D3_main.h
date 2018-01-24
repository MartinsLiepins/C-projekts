
#ifndef _D3_MAIN_H
#define _D3_MAIN_H
#endif



#define LED_1            GPIO_PIN_1 //pin PD1 => LED_R on Board

#define ENC_CLOCK           GPIO_PIN_0 //pin PK0 => clk to ENC
#define ENC_DATA_ENABLE     GPIO_PIN_3 //pin PK3 => ENC clk Enable

#define ENC_DATA_IN_1         GPIO_PIN_5 //pin PA5 => Data from ENC 1 (X11 on "Bank_CPU V1" board)
#define ENC_DATA_IN_2         GPIO_PIN_2 //pin PA2 => Data from ENC 2 (X12 on "Bank_CPU V1" board) 
#define ENC_DATA_IN_3         GPIO_PIN_1 //pin PF1 => Data from ENC 3 (X13 on "Bank_CPU V1" board)


#define CALIBRATE_BUTTON_PORT        GPIO_PORTA_BASE // calibration button Port PA0
#define CALIBRATE_BUTTON_PIN         GPIO_PIN_0 //calibration button pin PA0

#define ENC_COUNT              3 //Enkoderu skaits
#define ENC_DATA_LENGHT        16//Enkodera izejas bitu skaits


#define UART_BAUD_RATE         2500000//2500000//250000// 115200//Baudrate to 6 UART board (6x90 LED) 1Mbps
#define LED_COUNT              54  //LED skaits vienâ kanâlâ 
#define DISPLAY_COUNT          1  //displeju skaits
#define DISPLAY_CHANNEL_COUNT  5 //Displeja kanâlu skaits (X4 - X8 porti uz RGB digital x6 v1.1. PCB), katrs darbina 2 displeja LED virknes
#define LED_STRIP_COUNT        7 //dioþu virkòu skaits, kas vadâs no atseviðíiem X4 - X9 portiem uz RGB digital x6 v1.1. PCB   

#define INTEGRATION_DELAY      50// integrçðans aizture 0,5s 

static unsigned char toggle_PIN_ENC_CLOCK = 0x01;

static uint32_t ReadDataFlag;

static uint32_t bubbleFreqCount = 0;

