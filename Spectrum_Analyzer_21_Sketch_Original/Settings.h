/********************************************************************************************************************************
*
*  Project:  21 Band Spectrum Analyzer 
*  Target Platform: ESP32 38-PIN DEV Board
*  Version: 5.0
*  
********************************************************************************************************************************/

#pragma once
char version[]="5.0";                               // Define version number for reference only

// Debugging
#define DEBUG_BUFFER_SIZE 100                       // Debug buffer size
int  DEBUG = 1;                                     // When debug=1, extra information is printed to serial port. Turn of if not needed--> DEBUG=0

// Ledstrip Logo
#define LOGO_PIN         19                         // Second ledstrip for logo.
#define NUM_LEDS_LOGO    20                         // How many leds on your logo. You can define more leds then connected, that will result in wider gradient.
// Ledstrips/ matrix main display  **************************************************************************************
#define LED_PIN          18                         // This is the data pin of your led matrix, or ledstrips.
#define COLUMNS          21                         // Number of bands on display, this is not the same as display width...because display can be 28 ( double pixels per bar)

//const uint8_t                                     // if you have more then 16 bands, you will need to change the Led Matrix Arrays in the main file.
#define kMatrixWidth     21                         // Matrix width --> number of columns in your led matrix
#define kMatrixHeight    20                         // Matrix height --> number of leds per column   

// Some definitions for setting up the matrix  **************************************************************************
#define BAR_WIDTH   (kMatrixWidth / (COLUMNS -1))   // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define TOP         (kMatrixHeight - 0)             // Don't allow the bars to go offscreen
#define NUM_LEDS    (kMatrixWidth * kMatrixHeight)  // Total number of LEDs in the display array

// Ledstrips or pixelmatrix  ********************************************************************************************
#define CHIPSET      WS2812B                        // LED strip type -> Same for both ledstrip outputs( Matrix and logo)
#define BRIGHTNESSMAX    160                        // Max brightness of the leds...carefull...to bright might draw to much amps!
#define COLOR_ORDER      GRB                        // If colours look wrong, play with this
#define LED_VOLTS          5                        // Usually 5 or 12
#define MAX_MILLIAMPS   3000                        // Careful with the amount of power here if running off USB port, This will effect your brightnessmax. Currentlimit overrules it.
                                                    // If your power supply or usb can not handle the set current, arduino will freeze due to power drops.
// ADC Filter  **********************************************************************************************************
#define NOISE             20                        // Used as a crude noise filter on the adc input, values below this are ignored

//Controls  *************************************************************************************************************
#define BRIGHTNESSPOT     35                        // Potmeter for Peak Delay Time input 0...5V (0-3.3V on ESP32)
#define SENSITIVITYPOT    32                        // Potmeter for Brightness input 0...5V (0-3.3V on ESP32)
#define PEAKDELAYPOT      33                        // Potmeter for sensitivity input 0...5V (0-3.3V on ESP32)
#define Switch1            5                        // Connect a push button to this pin to change patterns
#define LONG_PRESS_MS   3000                        // Number of ms to count as a long press on the switch

// MSGEQ7 Pinout Connections  ******************************************************************************************
#define STROBE_PIN        17                        // MSGEQ7 strobe pin
#define RESET_PIN         16                        // MSGEQ7 reset pin

int BRIGHTNESSMARK =     100;                       // Default brightnetss, however, overruled by the Brightness potmeter
int AMPLITUDE      =    2000;                       // Depending on your audio source level, you may need to alter this value. it's controlled by the Sensitivity Potmeter

// Peak related stuff
#define Fallingspeed      30                        // This is the time it takes for peak tiels to fall to stack, this is not the extra time that you can add by using the potmeter
                                                    // for peakdelay. Because that is the extra time it levitates before falling to the stack    
#define AutoChangetime    10                        // If the time  in seconds between modes, when the patterns change automatically, if to fast, you can increase this number            

CRGB leds[NUM_LEDS];                                // Leds on the Ledstrips/Matrix of the actual Spectrum analyzer lights.
CRGB LogoLeds[NUM_LEDS_LOGO];                       // Leds on the ledstrip for the logo

#define NumberOfModes     31                        // The number of modes, remember it starts counting at 0,so if your last mode is 11 then the total number of modes is 12
#define DefaultMode        1                        // This is the mode it will start with after a reset or boot
#define DemoAfterSec    6000                        // if there is no input signal during this number of seconds, the unit will go to demo mode
#define DemoTreshold      10                        // this defines the treshold that will get the unit out of demo mode

/****************************************************************************************************
 * Colors of bars and peaks in different modes, changeable to your likings                          *
 ************************************************************************^^^^***********************/

// Static horizontal Rainbow ************************************************************************
#define RainbowBars_Color  (x / BAR_WIDTH) * (255 / COLUMNS), 255, 255

// Dynamic Horizontal Rainbow ***********************************************************************
#define RainbowBars_Color1  (x / BAR_WIDTH) * (255 / COLUMNS) + colorTimer, 255, 255

// Dynamic Vertical Rainbow *************************************************************************
#define ChangingBars_Color   y * (255 / kMatrixHeight), 255, 255

// Dynamic Vertical Rainbow *************************************************************************
#define ChangingBars_Color1   y * (255 / kMatrixHeight) + colorTimer, 255, 255

// ************************* YouTub1 Gradient pallete ***********************************************
DEFINE_GRADIENT_PALETTE( youtub1_gp ) {
  0, 255,   0,   0,  
 35, 255,   0,   0,
100,   0, 255,   0,
170,   0,   0, 200,
255, 255,   0, 255, };
CRGBPalette16 youtub1 = youtub1_gp;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( youtub2_gp ) {
  0,   0,   0, 255,  
 45,   0,   0, 255, 
 90, 255, 255,   0,
150, 255,   0,   0,
190, 255,   0, 255,
200,  85,   0, 255,
255,   0,   0, 255, };
CRGBPalette16 youtub2 = youtub2_gp;

DEFINE_GRADIENT_PALETTE( youtub3_gp ) {
  0, 180, 255, 255,  
 30,   0, 100, 255, 
 90, 162,   0, 200,
110, 255, 150, 218,
200, 230,   0,   0,
255, 255,   0,   0, };
CRGBPalette16 youtub3 = youtub3_gp;

DEFINE_GRADIENT_PALETTE( youtub4_gp ) {
  0, 150, 255,   0,  
 35, 150, 255,   0, 
 70, 255,   0, 150,
 90, 255,   0, 255,
150, 100, 200, 255,
255,   0,   0, 255, };  
CRGBPalette16 youtub4 = youtub4_gp;

DEFINE_GRADIENT_PALETTE( youtub5_gp ) {
  0,   0, 255, 255,  
 29,   0, 255, 255,
 30,   0, 255,   0, 
 80,   0, 255,   0,
100, 255, 110, 210,
200, 200,   5,   5,
255, 255,   0,   0, }; 
CRGBPalette16 youtub5 = youtub5_gp;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( youtub6_gp ) {
  0, 157,   0, 255,  
 30, 255,   0, 157,
 40, 255, 100,   0, 
 90, 255, 200,   0,
120, 100, 255, 255,
200,   5, 255,   5,
255,   0, 255,   0, }; 
CRGBPalette16 youtub6 = youtub6_gp;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( youtub7_gp ) {
  0,   0, 255,   0,  
 35,   0, 255,   0,
 90, 100, 255, 255,
130, 255, 100, 255,
160, 255,  10, 100,
190, 255,  15,   5,
255, 255,  10,   0, }; 
CRGBPalette16 youtub7 = youtub7_gp;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( youtub8_gp ) {
  0, 255,   0,   0,  
 50, 255,   0,  20,
110, 255, 255, 255,
130,  50, 255, 255,
160,   0, 150, 255,
190,   5,  15, 255,
255,   0,  10, 255, }; 
CRGBPalette16 youtub8 = youtub8_gp;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( youtub9_gp ) {
  0,   0,   0, 255,  
 50,   0,   0, 255,
110, 100,   0, 255,
//130, 100,   0, 255,
160,  60, 255,  60,
190,  10, 255,  25,
255,   0, 255,  10, }; 
CRGBPalette16 youtub9 = youtub9_gp;

DEFINE_GRADIENT_PALETTE( youtub10_gp ) {
  0,   0,   0, 255,  
130,  50, 255, 255,
170, 180,  50, 255,
200, 255,  50, 150,
255, 255,   0, 150, }; 
CRGBPalette16 youtub10 = youtub10_gp;

DEFINE_GRADIENT_PALETTE( youtub11_gp ) {
  0, 255,   0,   0,  
 40, 255,   0,   0,
150,  50, 255, 255,
190,   0, 255, 100,
210,   0, 255,  20,
255,   0, 255,   0, }; 
CRGBPalette16 youtub11 = youtub11_gp;

DEFINE_GRADIENT_PALETTE( youtub12_gp ) {
  0,   0, 255,   0,  
 60,   0, 255,   0,
130, 200, 150, 255,
160,  80, 100, 255,
210,   0,   0, 255,
255,   0,   0, 255, }; 
CRGBPalette16 youtub12 = youtub12_gp;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( youtub13_gp ) {
  0,   0,   0, 255,  
 20,   0,   0, 255,
 80,  55,   0, 255,
120, 255,  60,   0,
210,   0, 255,  20,
255,   0, 255,   0, }; 
CRGBPalette16 youtub13 = youtub13_gp;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( youtub14_gp ) {
  0,   0, 255,   0,  
 55,   0, 255,   0,
 85, 255, 150,   0,
160, 255,   0, 255,
210,  50,   0, 255,
255,  50,   0, 255, }; 
CRGBPalette16 youtub14 = youtub14_gp;

// ************************* Purple Gradient pallete **********************************************
DEFINE_GRADIENT_PALETTE( purple_gp ) {
  0,   0, 212, 255,      //Blue
255, 179,   0, 255 };    //Purple
CRGBPalette16 purplePal = purple_gp;

DEFINE_GRADIENT_PALETTE( purple_gp1 ) {
  0, 231,   0,   0,      //Red
255,   0, 212, 255 };    //Blue
CRGBPalette16 purplePal1 = purple_gp1;

DEFINE_GRADIENT_PALETTE( purple_gp2 ) {
 0,   255, 218,    0,    //Green
255, 179,   0, 255 };    //Purple
CRGBPalette16 purplePal2 = purple_gp2;

DEFINE_GRADIENT_PALETTE( purple_gp3 ) {
  0, 255,   0,   0,      //Red
255,   0,   0, 255 };    //Blue
CRGBPalette16 purplePal3 = purple_gp3;

DEFINE_GRADIENT_PALETTE( purple_gp4 ) {
  0,   0, 255,   0,      //Green 
255,   0,   0, 255 };    //Red
CRGBPalette16 purplePal4 = purple_gp4;

DEFINE_GRADIENT_PALETTE( purple_gp5 ) {
  0, 255,   0,   0,      //Red
127, 255,   0,   0,      //Green
255,   0,   0, 255 };    //Blue
CRGBPalette16 purplePal5 = purple_gp5;

DEFINE_GRADIENT_PALETTE( purple_gp6 ) {
  0, 246,  60, 255,      //
255,   9,  60, 169 };    //
CRGBPalette16 purplePal6 = purple_gp6;

DEFINE_GRADIENT_PALETTE( purple_gp7 ) {
  0, 243,  60, 100,      //
255, 120, 200,  66 };    //
CRGBPalette16 purplePal7 = purple_gp7;

DEFINE_GRADIENT_PALETTE( purple_gp8 ) {
  0, 252, 70,   80,      //
 85, 100, 226, 150,      //
170, 249, 140,  50,      //
255,  20, 231, 100 };    //
CRGBPalette16 purplePal8 = purple_gp8;

DEFINE_GRADIENT_PALETTE( purple_gp9 ) {
  0, 201,  66,  62,      //
255, 101, 255,  98 };    //
CRGBPalette16 purplePal9 = purple_gp9;

DEFINE_GRADIENT_PALETTE( purple_gp10 ) {
 0,   28, 64,    150,    // 
255, 246,  125, 32 };    //
CRGBPalette16 purplePal10 = purple_gp10;

DEFINE_GRADIENT_PALETTE( purple_gp11 ) {
 0,   2, 183,    204,    // 
255, 233,   80, 22 };    //
CRGBPalette16 purplePal11 = purple_gp11;

DEFINE_GRADIENT_PALETTE( purple_gp12 ) {
  0, 150,  40, 255,      // 
255, 100, 255, 120 };    //
CRGBPalette16 purplePal12 = purple_gp12;

DEFINE_GRADIENT_PALETTE( purple_gp13 ) {
  0, 238,  20,  50,      // 
255,  50,  20, 200 };    //
CRGBPalette16 purplePal13 = purple_gp13;

DEFINE_GRADIENT_PALETTE( purple_gp14 ) {
  0,  60, 130, 255,      //
255,  58, 200,  47 };    //
CRGBPalette16 purplePal14 = purple_gp14;

// ************************* Heat Gradient pallete **********************************************
DEFINE_GRADIENT_PALETTE( redyellow1_gp ) {
  0,   200, 200,  200,   //White
 64,   255, 218,    0,   //Green
128,   231,   0,    0,   //Red
192,   255, 218,    0,   //Green
255,   200, 200,  200 }; //White
CRGBPalette16 heatPal1 = redyellow1_gp;    // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( redyellow_gp2 ) {
  0,   0, 212, 255,   
127, 179,   0, 255, 
255,  0, 212, 255}; 
CRGBPalette16 heatPal2 = redyellow_gp2;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( redyellow_gp3 ) {
  0, 231,   0,   0,      //Red
127,   0, 212, 255,      //Blue
255, 231,   0,   0};     //Red
CRGBPalette16 heatPal3 = redyellow_gp3;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( redyellow_gp4 ) {
  0, 255, 218,   0,      //Green
127, 179,   0, 255,      //Purple
255, 255, 218,   0};     //Green
CRGBPalette16 heatPal4 = redyellow_gp4;

DEFINE_GRADIENT_PALETTE( redyellow_gp5 ) {
  0, 246, 156, 145,      //
127,   9,  60, 169,      //
255, 246, 156, 145};     //
CRGBPalette16 heatPal5 = redyellow_gp5;

DEFINE_GRADIENT_PALETTE( redyellow_gp6 ) {
  0,   0, 255,   0,      //Green 
127,   0,   0, 255,      //Red
255,   0, 255,   0};     //Green 
CRGBPalette16 heatPal6 = redyellow_gp6;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( redyellow_gp7 ) {
  0, 246, 156, 145,      //
127,   9,  60, 169,      //
255, 246, 156, 145};     //
CRGBPalette16 heatPal7 = redyellow_gp7;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( redyellow_gp8 ) {
  0, 243,  60, 100,      //
127, 120, 200,  66,      //
255, 243,  60, 100};     //
CRGBPalette16 heatPal8 = redyellow_gp8;

DEFINE_GRADIENT_PALETTE( redyellow_gp9 ) {
  0, 252,  70,  80,      //
 85, 100, 226, 150,      //
127, 249, 140,  50,      //
169, 100, 226, 150,      //
255, 252,  70,  80};     //
CRGBPalette16 heatPal9 = redyellow_gp9;

DEFINE_GRADIENT_PALETTE( redyellow_gp10 ) {
  0, 201,  66,  62,      //
127, 101, 255,  98,      //
255, 201,  66,  62};     //
CRGBPalette16 heatPal10 = redyellow_gp10;

DEFINE_GRADIENT_PALETTE( redyellow_gp11 ) {
  0,  28,  64, 150,      // 
127, 246, 125,  32,      //
255,  28,  64, 150};     // 
CRGBPalette16 heatPal11 = redyellow_gp11;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( redyellow_gp12 ) {
  0, 150,  40, 255,      // 
127, 100, 255, 120,      //
255, 150,  40, 255 };    // 
CRGBPalette16 heatPal12 = redyellow_gp12;

DEFINE_GRADIENT_PALETTE( redyellow_gp13 ) {  
  0, 150,  40, 255,      // 
127, 100, 255, 120,      //
255, 150,  40, 255 };    // 
CRGBPalette16 heatPal13 = redyellow_gp13;

DEFINE_GRADIENT_PALETTE( redyellow_gp14 ) {  
  0, 238,  50, 100,      // 
127,  70,  40, 200 ,     //
255, 238,  50, 100};     // 
CRGBPalette16 heatPal14 = redyellow_gp14;   // ****** W I N N E R *******

DEFINE_GRADIENT_PALETTE( redyellow_gp15 ) {  
 0,  152, 111, 255,      //
127,  58, 130,  47 ,     //
255, 152, 111, 255};     //
CRGBPalette16 heatPal15 = redyellow_gp15;

DEFINE_GRADIENT_PALETTE( Sunset_Real_gp ) {
    0, 120,  0,   0,
   22, 179, 22,   0,
   51, 255,104,   0,
   85, 167, 22,  18,
  135, 100,  0, 103,
  198,  16,  0, 130,
  255,   0,  0, 160};
CRGBPalette16 sunset = Sunset_Real_gp;

// ********* TriBarA Original ***************************************************
 #define TriBars_Color_TopA             0, 255, 255     // Orange-Red
 #define TriBars_Color_MiddleA         20, 255, 255     // Yellow-Orange
 #define TriBars_Color_BottomA         90, 255, 255     // Green- Yellow

 #define TriBars_Color_Top_PeaksA        0, 255, 255    // Pink
 #define TriBars_Color_Middle_PeaksA   208, 255, 255    // Blue
 #define TriBars_Color_Bottom_PeaksA   172, 255, 255    // Purple

// ********* TriBarB  Original **************************************************
 #define TriBars_Color_TopB             2, 255, 225     // Red-Orange
 #define TriBars_Color_MiddleB         25, 255, 225     // Orange-Yellow
 #define TriBars_Color_BottomB        160, 255, 255     // Purple-Blue

 #define TriBars_Color_Top_PeaksB      245, 215, 255    // Purple
 #define TriBars_Color_Middle_PeaksB    90, 255, 255    // Green
 #define TriBars_Color_Bottom_PeaksB     4, 255, 255    // Orange

// SOLID COLOR BARS  **************************************************************
 #define BarsColor_Red           0, 255, 255            // Red Bars
 #define BarsColor_Green        94, 255, 255            // Green Bars
 #define BarsColor_Blue        175, 255, 255            // Blue Bars

// ADDITIONAL PEAK COLOR MODES  ***************************************************
#define PeakColor_Red1           2, 255, 255            // Red1 peaks
#define PeakColor_Red2           4, 225, 255            // Red2 peaks
#define PeakColor_Orange1        8, 255, 255            // Orange peaks
#define PeakColor_Orange2       12, 245, 255            // Orange peaks
#define PeakColor_Yellow        35, 255, 255            // Green Peaks
#define PeakColor_Green         95, 255, 255            // Green Peaks
#define PeakColor_Blue         160, 255, 255            // Blue peaks
#define PeakColor_Blue1        170, 255, 255            // Blue peaks
#define PeakColor_Blue2        180, 255, 255            // Blue peaks
#define PeakColor_Violet       200, 255, 255            // Purple peaks
#define PeakColor_Purple       215, 245, 255            // Orange peaks
#define PeakColor_Pink         245, 210, 255            // Hot Pink peaks
#define PeakColor_Red          255, 255, 255            // Red peaks
 
/***********************************************************************************
* Setting below are only related to the demo Fire mode                             *
************************************************************************************/

#define FPS 30              /* Refresh rate 15 looks good*/

//Flare constants
const uint8_t flarerows   =    10;  //2  /* number of rows (from bottom) allowed to flare */
const uint8_t maxflare    =     5;  //8  /* max number of simultaneous flares */
const uint8_t flarechance =    50;       /* chance (%) of a new flare (if there's room) */
const uint8_t flaredecay  =    14;       /* decay rate of flare radiation; 14 is good */

// This is the map of colors from coolest (black) to hottest. Want blue flames? Go for it ! 
uint32_t colors0[] = {
  //Blue Flame
  0x000000,
  0x000010,
  0x000030,
  0x000060,
  0x000080,
  0x0000A0,
  0x0020C0,
  0x0040c0,
  0x0060c0,
  0x0080c0,
  0x807080
};
uint32_t colors1[] = {
  //Red Flame
  0x000000,
  0x100000,
  0x300000,
  0x600000,
  0x800000,
  0xA00000,
  0xC02000,
  0xC04000,
  0xC06000,
  0xC08000,
  0x807080
};
uint32_t colors2[] = {
  //Green Flame
  0x000000,
  0x001000,
  0x003000,
  0x006000,
  0x008000,
  0x00a000,
  0x20c000,
  0x40c000,
  0x60c000,
  0x80c000,
  0x807080
};
// ******************************* T H E  E N D ************************************
