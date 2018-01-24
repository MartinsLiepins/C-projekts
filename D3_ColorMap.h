#ifndef _D3_COLORMAP_H
#define _D3_COLORMAP_H
#endif

double convertToDouble = 2.5,
       convertToDbl = 1.00;
  
#define DELTA_RAND_NUM 6
#define COLOR_DEVIATION 30//"v�r��an�s efekta kr�su nob�dei no nulles st�vok�a"
#define DELTA_CTR_MAX 512//254/max skait�t�ja v�rt�ba, kuru sasniedzot LED ir max spilgtums (v�r��an�s frekvence)

const uint8_t  blueLedStopLimit = 7, /*uz ekr�na palieko�o zilo dio�u skaits, 
kas nesamazin�s, mainoties enkoderu st�voklim.*/

                ledChannelCount = 5, //paral�lo kan�lu skaits displejam
                ledPerPixel = 3, //pikse�u (RGB) skaits uz diodi 
                bubbleStopLimit = 10,//LED skaits, pie kura apst�jas "v�r��an�s efekts"
                bubbleFreqMax = 5,//max v�r��an�s v�rt�ba (saskait�mais, palielinot v�r��an�s frekvence samazin�s)
                BubbleFreqMin = 80;//min v�r��an�s v�rt�ba (reizin�t�js, palielinot v�r��an�s frekvencei samazin�s)

const int32_t bubbleDecreaseIntensityValue = 250; //v�rt�ba pie kuras samazin�s v�r��an�s efekta burbu�u intensit�te (0-762) 

const uint8_t colorGreen = 25, //za��s diodes spo�ums
              colorRed = 25,   //sarkan�s diodes spo�ums
              colorBlue = 25;  //zil�s diodes spo�ums

const uint8_t dyspBacklightDivid = 2,//dal�t�js krasam R,G ekr�na izgaismojumam
              dyspBacklightDividBlueCol = 6; //dal�t�js zilai krasai, liel�ks, jo savadak zils tonis

//Skrieno�o LED atk�rto�an�s bie�ums
const int32_t runLedStep = 9;

const int32_t runLedFullPeriod = 40,//dio�u virknes LED skrie�anas �trums (skait�t�js l�dz)
              runLedSpeedCorection = 4,//dio�u virknes maksim�lais �trums (skait�t�js l�dz)
              zeroPoint = 30,//nobide no videjas vertibas (fullScale/2 => 762/2=381), pie kura skrien�� LED sak virzities pa kreisi vai pa labi
              runLedZeroSpeed = 40;//di�u skrie�anas �trums nulles punkt�

//Kan�lu kritiskie punkti skrieno�o dio�u virkn�m(sazarojuma punkti viena kan�la ietvaros)
const int32_t chanABreakPointAddrStart = 0,
              chanABreakPointAddrOne = 1,
              chanABreakPointAddrTwo = 2,
              chanABreakPointAddrThree = 3,
              chanABreakPointAddrFour = 4,
              chanABreakPointAddrFive = 14,
              chanABreakPointAddrEnd = 23,
        
              chanCBreakPointAddrStart = 0,
              chanCBreakPointAddrOne = 1,
              chanCBreakPointAddrTwo = 2,
              chanCBreakPointAddrThree = 3,
              chanCBreakPointAddrFour = 4,
              chanCBreakPointAddrFive = 14,
              chanCBreakPointAddrEnd = 23,
              
              chanDBreakPointAddrStart = 0,
              chanDBreakPointAddrOne = 2,
              chanDBreakPointAddrTwo = 12,
              chanDBreakPointAddrEnd = 34,
              
              chanEBreakPointAddrStart = 0,
              chanEBreakPointAddrOne = 2,
              chanEBreakPointAddrTwo = 12,
              chanEBreakPointAddrEnd = 34,        
        
              chanBBreakPointAddrStart = 0,
              chanBBreakPointAddrOne = 14, 
              chanBBreakPointAddrTwo = 17,
              chanBBreakPointAddrThree = 18,
              chanBBreakPointAddrFour = 31,
              chanBBreakPointAddrFive = 32,
              chanBBreakPointAddrEnd = 45,
        
              chanFBreakPointAddrStart = 0,
              chanFBreakPointAddrOne = 10, 
              chanFBreakPointAddrTwo = 20,
              chanFBreakPointAddrThree = 32,
              chanFBreakPointAddrFour = 33,
              chanFBreakPointAddrFive = 35,
              chanFBreakPointAddrSix = 37,
              chanFBreakPointAddrEnd = 38,
        
              chanGBreakPointAddrStart = 0,
              chanGBreakPointAddrOne = 10, 
              chanGBreakPointAddrTwo = 20,
              chanGBreakPointAddrThree = 32,
              chanGBreakPointAddrFour = 33,
              chanGBreakPointAddrFive = 35,
              chanGBreakPointAddrSix = 37,
              chanGBreakPointAddrEnd = 38;
  
//LED skaits katr� skrieno�o dio�u kan�l� (7 kan�li) 
#define LED_IN_STRIP_A  24
#define LED_IN_STRIP_C  24
#define LED_IN_STRIP_D  35
#define LED_IN_STRIP_E  35     
#define LED_IN_STRIP_B  46
#define LED_IN_STRIP_F  39
#define LED_IN_STRIP_G  39