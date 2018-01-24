#ifndef _D3_COLORMAP_H
#define _D3_COLORMAP_H
#endif

double convertToDouble = 2.5,
       convertToDbl = 1.00;
  
#define DELTA_RAND_NUM 6
#define COLOR_DEVIATION 30//"vârîðanâs efekta krâsu nobîdei no nulles stâvokïa"
#define DELTA_CTR_MAX 512//254/max skaitîtâja vçrtîba, kuru sasniedzot LED ir max spilgtums (vârîðanâs frekvence)

const uint8_t  blueLedStopLimit = 7, /*uz ekrâna paliekoðo zilo dioþu skaits, 
kas nesamazinâs, mainoties enkoderu stâvoklim.*/

                ledChannelCount = 5, //paralçlo kanâlu skaits displejam
                ledPerPixel = 3, //pikseïu (RGB) skaits uz diodi 
                bubbleStopLimit = 10,//LED skaits, pie kura apstâjas "vârîðanâs efekts"
                bubbleFreqMax = 5,//max vârîðanâs vçrtîba (saskaitâmais, palielinot vârîðanâs frekvence samazinâs)
                BubbleFreqMin = 80;//min vârîðanâs vçrtîba (reizinâtâjs, palielinot vârîðanâs frekvencei samazinâs)

const int32_t bubbleDecreaseIntensityValue = 250; //vçrtîba pie kuras samazinâs vârîðanâs efekta burbuïu intensitâte (0-762) 

const uint8_t colorGreen = 25, //zaïâs diodes spoþums
              colorRed = 25,   //sarkanâs diodes spoþums
              colorBlue = 25;  //zilâs diodes spoþums

const uint8_t dyspBacklightDivid = 2,//dalîtâjs krasam R,G ekrâna izgaismojumam
              dyspBacklightDividBlueCol = 6; //dalîtâjs zilai krasai, lielâks, jo savadak zils tonis

//Skrienoðo LED atkârtoðanâs bieþums
const int32_t runLedStep = 9;

const int32_t runLedFullPeriod = 40,//dioþu virknes LED skrieðanas âtrums (skaitîtâjs lîdz)
              runLedSpeedCorection = 4,//dioþu virknes maksimâlais âtrums (skaitîtâjs lîdz)
              zeroPoint = 30,//nobide no videjas vertibas (fullScale/2 => 762/2=381), pie kura skrienðâ LED sak virzities pa kreisi vai pa labi
              runLedZeroSpeed = 40;//diþu skrieðanas âtrums nulles punktâ

//Kanâlu kritiskie punkti skrienoðo dioþu virknçm(sazarojuma punkti viena kanâla ietvaros)
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
  
//LED skaits katrâ skrienoðo dioþu kanâlâ (7 kanâli) 
#define LED_IN_STRIP_A  24
#define LED_IN_STRIP_C  24
#define LED_IN_STRIP_D  35
#define LED_IN_STRIP_E  35     
#define LED_IN_STRIP_B  46
#define LED_IN_STRIP_F  39
#define LED_IN_STRIP_G  39