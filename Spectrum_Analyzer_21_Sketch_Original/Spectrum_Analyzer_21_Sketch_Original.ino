/********************************************************************************************************************************
*
*  Project:  21 Band Spectrum Analyzer 
*  Platform: ESP32 DEV Board
*  Version:  8.0
*  
*  Dan Micu aka samm928
*  Senior PCB Designer
*  EasyEDA: https://oshwlab.com/samm928/21-Bands-Audio-Spectrum-Analyzer_copy
*  Youtube: https://www.youtube.com/watch?v=mA5JHc9urMM&t=4s
*  Redit:   https://www.github.com/user/samm928
*  -----------------------------------------------------------------------------------------------------------------------------
*  Current Version: 8.0  
*  - Changed hardware to ESP32 Platform for extra speed and additional memory.
*  - Upgraded hardware by adding an extra MSGEQ7 on the board for an additional 7-bands for a total of 21-Channels.
*  - Integrated additional Color Modes in main loop and extra Gradinet color Palletes and Patterns in settings.h
*  - Added MH-M18 Bluetooth module as an alternat audio line input option.
*
********************************************************************************************************************************
*  Libaries and external files
********************************************************************************************************************************/
#include <Arduino.h>
#include <Adafruit_SI5351.h>
#include <FastLED_NeoMatrix.h>
#include <EasyButton.h>
#include <Wire.h>
#include "hardwaretest.h"
#include "Settings.h"                              // External file with all changeable Settings
#include "debug.h"                                 // External file with debug subroutine
#include "fire.h"

// Peak related stuff we need
int Peakdelay;                                     // Delay before peak falls down to stack. Overruled by PEAKDELAY Potmeter
int PeakTimer[COLUMNS];                            // counter how many loops to stay floating before falling to stack
char PeakFlag[COLUMNS];                            // the top peak delay needs to have a flag because it has different timing while floating compared to falling to the stack

// Led matrix Arrays do not change unless you have more then 16 bands
byte peak[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};          // The length of these arrays must be >= COLUMNS
int oldBarHeights[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // So if you have more then 16 bands, you must add zero's to these arrays
int bandValues[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Misc. Stuff
int colorIndex = 0;                                // Yep, we need this to keep track of colors
uint8_t colorTimer = 0;                            // Needed to change color periodically
long LastDoNothingTime = 0;                        // only needed for screensaver
int DemoModeMem = 0;                               // to remember what mode we are in when going to demo, in order to restore it after wake up
bool AutoModeMem = false;                          // same story
bool DemoFlag = false;                             // we need to know if demo mode was manually selected or auto engadged. 
bool i2c_found;

// Button stuff
int buttonPushCounter = DefaultMode;               // Timer for Psuh Button
bool autoChangePatterns = false;                   // press the mode button 3 times within 2 seconds to auto change paterns.

// Defining some critical components
EasyButton modeBtn(Switch1);                       // The mode Button on A10 or pin64
Adafruit_SI5351 clockgen = Adafruit_SI5351();

/*******************************************************************************************************************************
*  FastLED_NeoMaxtrix - see https://github.com/marcmerlin/FastLED_NeoMatrix for Tiled Matrixes, Zig-Zag and so forth
*******************************************************************************************************************************/
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, kMatrixWidth, kMatrixHeight,
  NEO_MATRIX_BOTTOM        + NEO_MATRIX_LEFT +     // Use this to configure the array settings.
  NEO_MATRIX_COLUMNS       + NEO_MATRIX_ZIGZAG +   // Use Progressive if end-to end wiring was done to the array panel.
  NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS);   // I prefer lower frequencies to the left hand side.

/****************************************************************************************************************************
*  Setup routine
********************************************************************************************************************************/
void setup() {
  Serial.begin(115200);
  Serial.println ();     
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;
    Wire.begin();
  for (byte i = 1; i < 120; i++) {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0) {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      } // end of good response
     delay (5);  // give devices time to recover
  } // Serial Monitor to display devices found on I2C bus
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s)."); 
 
    //SI5351
    /* Initialise the sensor */
    if (clockgen.begin() != ERROR_NONE)
    {
      /* There was a problem detecting the IC ... check your connections */
      Serial.print("wiring or I2C ADDR!");
      while(1);
    }
if(COLUMNS==14){
    clockgen.setupPLL(SI5351_PLL_A, 36, 0, 1);// 25Mhz x 36 =900mhz
    clockgen.setupMultisynth(0, SI5351_PLL_A, 66, 964285, 1000000); //900hz / 8 =112.5hz//SI5351_MULTISYNTH_DIV_8/6/4//52, 83333, 1000000/135000//52, 83333, 1000000
    clockgen.setupRdiv(0, SI5351_R_DIV_128);//105000hz

    clockgen.setupPLL(SI5351_PLL_B, 36, 0, 1);
    clockgen.setupMultisynth(1, SI5351_PLL_B, 42, 613636, 1000000);//15-90//1-1048575//42, 0, 1//28, 968979, 1000000//245000//35, 156250, 1000000
    clockgen.setupRdiv(1, SI5351_R_DIV_128);//165000hz

    clockgen.setupMultisynth(2, SI5351_PLL_B, 28, 698979, 1000000);//SI5351_PLL_B/900
    clockgen.setupRdiv(2, SI5351_R_DIV_128);//165000hz
}

if(COLUMNS==21){
    clockgen.setupPLL(SI5351_PLL_A, 36, 0, 1);// 25Mhz x 36 =900mhz
    clockgen.setupMultisynth(0, SI5351_PLL_A, 66, 964285, 1000000); //900hz / 8 =112.5hz//SI5351_MULTISYNTH_DIV_8/6/4//52, 83333, 1000000/135000
    clockgen.setupRdiv(0, SI5351_R_DIV_128);//105000hz

    clockgen.setupPLL(SI5351_PLL_B, 36, 0, 1);
    clockgen.setupMultisynth(1, SI5351_PLL_B, 46, 106557, 1000000);//15-90//1-1048575//42, 0, 1//28, 968979, 1000000//245000
    clockgen.setupRdiv(1, SI5351_R_DIV_128);//135000hz

    clockgen.setupMultisynth(2, SI5351_PLL_B, 35, 156250, 1000000);//SI5351_PLL_B/900
    clockgen.setupRdiv(2, SI5351_R_DIV_128);//165000hz
}

  /* Enable the clocks */
  clockgen.enableOutputs(true);
  //SI5351
    
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.addLeds<CHIPSET, LOGO_PIN, COLOR_ORDER>(LogoLeds, NUM_LEDS_LOGO).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, MAX_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESSMARK);
  FastLED.clear();

  modeBtn.begin();
  modeBtn.onPressed(changeMode);                         // When mode key is pressed, call changeMode sub routine
  modeBtn.onPressedFor(LONG_PRESS_MS, Run_Diagnostics);  // when pressed for the set time( default 3 secs, can be changed in definition of LONG_PRESS_MS), run diagnstics
  modeBtn.onSequence(4, 2000, startAutoMode);            // enable automode if pressed 3 times, within 2 seconds

  dbgprint("Configuring datalines for MSGEQ7");
//Now let's configure the datalines for the MSGEQ7's and prepare them for running mode
  pinMode      (STROBE_PIN,    OUTPUT);                   // MSGEQ7 strobe pin configure as output pin
  pinMode      (RESET_PIN,     OUTPUT);                   // MSGEQ7 reset pin configure as output pin
  pinMode      (LED_PIN,       OUTPUT);                   // Connection to LEDSTRIP configure as output pin

  dbgprint("Init of MSGEQ7 IC's");
//Initialize the Analyzer Ic's
  digitalWrite (RESET_PIN,  LOW);
  digitalWrite (STROBE_PIN, LOW);
  delay        (1);
  digitalWrite (RESET_PIN,  HIGH);
  delay        (1);
  digitalWrite (RESET_PIN,  LOW);
  digitalWrite (STROBE_PIN, HIGH);
  delay        (1);
}
/********************************************************************************************************************************
*  END OF setup routine
********************************************************************************************************************************/

void changeMode() {
  dbgprint("Button pressed");
  if (FastLED.getBrightness() == 0) FastLED.setBrightness(BRIGHTNESSMARK);  //Re-enable if lights are "off"
  autoChangePatterns = false;
  buttonPushCounter = (buttonPushCounter + 1) % NumberOfModes; //%6
  dbgprint("mode: %d\n", buttonPushCounter);
   if(DemoFlag==true) {                // in case we are in demo mode ... and manual go out of it.
    dbgprint("demo is true");
    buttonPushCounter=DemoModeMem;     // go back to mode we had before demo mode
    LastDoNothingTime = millis();      // reset that timer to prevent going back to demo mode the same instance
    DemoFlag=false;
   }
 }

void startAutoMode() {
  autoChangePatterns = true;
  Matrix_Flag();                       // this is to show user that automode was engaged. It will show dutch flag for 2 seconds
  delay(2000);
  dbgprint(" Patterns will change after few seconds ");
  dbgprint(" You can reset by pressing the mode button again");
}

void brightnessOff() {
  FastLED.setBrightness(0);            // Lights out
}

/********************************************************************************************************************************
*  Main Loop
********************************************************************************************************************************/
void loop(){

   if (buttonPushCounter!=30)FastLED.clear();   // Not for demo mode
   rainbow_wave(10, 10);                        // Call subroutine for logo update

// GET OUR USER INPUTS   *****************************************************************************************************
   AMPLITUDE = map(analogRead(SENSITIVITYPOT), 0, 4095, 50, 4095);               // read sensitivity potmeter and update amplitude setting
   BRIGHTNESSMARK = map(analogRead(BRIGHTNESSPOT), 0, 4095, BRIGHTNESSMAX, 10);  // read brightness potmeter
   Peakdelay = map(analogRead(PEAKDELAYPOT), 0, 4095, 50, 1);                    // update the Peakdelay time with value from potmeter
   FastLED.setBrightness(BRIGHTNESSMARK);                                        // update the brightness
   modeBtn.read();                                                               // what the latest on our mode switch?
   
   for (int i = 0; i < COLUMNS; i++) bandValues[i] = 0;                          // Reset bandValues[ ]

// Now RESET the MSGEQ7's and use strobe to read out all current band values and store them in bandValues array *********
   digitalWrite(RESET_PIN, HIGH);
   delayMicroseconds(3000);
   digitalWrite(RESET_PIN, LOW);

// READ IN MGSEQ7 VALUES  ***********************************************************************************************
for (int i = 0; i < COLUMNS; i++) {
   digitalWrite(STROBE_PIN, LOW);
   delayMicroseconds(1000);
   bandValues[i] = analogRead(36) - NOISE;
     if (bandValues[i]<120)  bandValues[i]=0;
     bandValues[i] = constrain(bandValues[i], 0, AMPLITUDE);
     bandValues[i] = map(bandValues[i], 0, AMPLITUDE, 0, kMatrixHeight); i++;
   bandValues[i] = analogRead(39) - NOISE;
     if (bandValues[i]<120) bandValues[i]=0;
     bandValues[i] = constrain(bandValues[i], 0, AMPLITUDE);
     bandValues[i] = map(bandValues[i], 0, AMPLITUDE, 0, kMatrixHeight); i++;
   bandValues[i] = analogRead(34) - NOISE;
     if (bandValues[i]<120) bandValues[i]=0;
     bandValues[i] = constrain(bandValues[i], 0, AMPLITUDE);
   bandValues[i] = map(bandValues[i], 0, AMPLITUDE, 0, kMatrixHeight);
   if (bandValues[i] > DemoTreshold && i > 1) LastDoNothingTime = millis();   // if there is signal in any off the bands[>2] then no demo mode 
   digitalWrite(STROBE_PIN, HIGH);
}
   
// Collect and process data from BandValues and transform them into BAR HEIGHTS ***************************************
   for (byte band = 0; band < COLUMNS; band++) {                              // Scale the bars for the display
    int barHeight = bandValues[band];
    if (barHeight>TOP) barHeight = TOP;
    
    // Small amount of averaging between frames
       barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;               // Fast Filter, more rapid movement
    // barHeight = ((oldBarHeights[band] * 2) + barHeight) / 3;               // minimum filter makes changes more smooth

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min(TOP, barHeight);
      PeakFlag[band] = 1;
    }
    /*
       Mode 1: TriBar each Column is devided into 3 sections, Bottom,Middle and Top, Each section has different color
       Mode 2: Each Column different color, purple peaks
       Mode 3: Each Colomn has the same gradient from a color pattern, white peaks
       Mode 4: All is red color, blue peaks
       Mode 5: All is blue color, red peaks
       Mode 6: Center Bars following defined color pattern, Red White Yellow
       Mode 7: Center Bars following defined color pattern ---> White Red
       Mode 8: Center Bars following defined color pattern ---> Pink White Yellow
       Mode 9: Peaks only, color depends on level (height)
       Mode 10: Peaks only, blue color
       Mode 11: Peaks only, color depends on level(height), following the tribar pattern
       Mode 12: Fire, doesn't response to music
       Mode 0: Gradient mode, colomns all have the same gradient but gradient changes following a rainbow pattern
    */

   // if there hasn't been much of a input signal for a longer time (see settings) go to demo mode
   if ((millis() - LastDoNothingTime) > DemoAfterSec && DemoFlag==false)
   { dbgprint("In loop 1:  %d", millis() - LastDoNothingTime);
    DemoFlag=true;
    // First store current mode so we can go back to it after wake up
    DemoModeMem = buttonPushCounter;
    AutoModeMem = autoChangePatterns;
    autoChangePatterns = false;
    buttonPushCounter = 30;
    dbgprint("Automode is turned off because of demo");
   } 
// Wait,signal is back? then wakeup!     
    else if (DemoFlag==true && (millis() - LastDoNothingTime) < DemoAfterSec )   
    { dbgprint("In loop 2: %d", millis() - LastDoNothingTime);
      // while in demo the democounter was reset due to signal on one of the bars.
      // So we need to exit demo mode.
      buttonPushCounter = DemoModeMem;  // restore settings
      dbgprint ("automode setting restored to: %d",AutoModeMem);
      autoChangePatterns=AutoModeMem;   // restore settings
      DemoFlag=false;  
    }
    
// Now visualize those bar heights and add some MODES and PATERNS *******************************************************
    switch (buttonPushCounter) {
    case 0:
      BarsUp_Tub_01s(band, barHeight);              // Purple-Blue-Green-Red Gradient Stratic         *** DEFAULT START ***
      NormalPeaks(band, PeakColor_Red2);         // Magenta Peaks
      break;

    case 1:
      BarsUp_Tub_03s(band, barHeight);              // Red-Pink-Cyan Gradient Static                  *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Blue1);            // Blue Peaks
      break;

    case 2:
      BarsUp_Purple_04s(band, barHeight);           // Blue-Cyan-Green Gradient Static                *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Red);             // Red Peaks
      break;

    case 3:
      BarsUp_Purple_05s(band, barHeight);           // Blue-Cyan-Green Gradient Static                *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Blue);           // Blue Peaks
      break;

    case 4:
      BarsUp_Tub_02s(band, barHeight);              // Blue-Pink-Red-Yellow Gradient Static       
      NormalPeaks(band, PeakColor_Green);          // Green Peaks
      break;

    case 5:
      BarsUp_Purple_03s(band, barHeight);           // Gradient Static
      NormalPeaks(band, PeakColor_Orange2);         // Orange Peaks
      break;

    case 6:
      BarsUp_Tub_04s(band, barHeight);              // Blue-White-Pink-Yellow Gradient Static
      NormalPeaks(band, PeakColor_Orange1);         // Red Peaks
      break;

    case 7:
      BarsUp_Tub_05s(band, barHeight);              // Red-Pink-Green-Cyan Gradient Static
      NormalPeaks(band, PeakColor_Yellow);          // Orange Peaks
      break;

    case 8:
      BarsUp_Tub_06s(band, barHeight);              // Green-Cyan-Yellow-Purple Gradient Static
      NormalPeaks(band, PeakColor_Red);             // Red Peaks
      break;

    case 9:
      BarsUp_Tub_07s(band, barHeight);              // Red-Purple-White-Green Gradient Static         *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Blue1);           // Blue Peaks
      break;    

    case 10:
      BarsUp_Tub_08s(band, barHeight);              // Blue-White-Red Gradient Static                 *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Green);           // Green Peaks
      break;

    case 11:
      BarsUp_Tub_09s(band, barHeight);              // Green-White-Blue Gradient Static               *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Red2);            // Red Peaks
      break; 

    case 12:
      BarsUp_Heat_01s(band, barHeight);             // White-Yellow-Red-Yellow Gradient Static        *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Blue);            // Blue Peaks
      break;

    case 13:
      BarsUp_Heat_03s(band, barHeight);             // Gradient Static                                *** W O R K I N G ***         
      NormalPeaks(band, PeakColor_Green);           // Green Peaks
      break;

    case 14:
      BarsUp_Heat_06s(band, barHeight);             // Green-Cyan-Blue Gradient Static                *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Red);             // Red Peaks
      break;

    case 15:  
      BarsUp_Heat_03d(band, barHeight);             // Gradient Static                                *** W O R K I N G ***         
      NormalPeaks(band, PeakColor_Blue1);           // Green Peaks
      break;

    case 16:
      RainbowBarsA(band, barHeight);                // Rainbow-Horizontal Dynamic                     *** W O R K I N G ***
      // No Peaks
      break;
    
    case 17:
      BarsUp_Heat_06d(band, barHeight);             // Green-Cyan-Blue Gradient Static                *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Orange2);         // Orange Peaks
      break;

    case 18:
      CenterBars_RainbowDyV(band, barHeight);       // Purple peak colors
      // No Peaks
      break;

    case 19:
      ChangingBarsA(band, barHeight);               // Rainbow-Vertical Dynamic
      // No Peaks
      break;

    case 20:
      CenterBars_RainbowDyH(band, barHeight);       // Red-Yellow-Blue
      break;

    case 21:
      CenterBars_Youtub_02s(band, barHeight);       // Purple peak colors
      // No Bars 
      break;

    case 22:
      CenterBars_Heat_01s(band, barHeight);       // Purple peak colors
      // No Bars 
      break;
    
    case 23:
      CenterBars_Purple_02s(band, barHeight);       // Purple peak colors
      // No Bars 
      break;

    case 24:
      BarsUp_Heat_01d(band, barHeight);             // White-Yellow-Red-Yellow Gradient Static        *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Blue1);         // Orange Peaks
      break;

    case 25:
      BarsUp_Heat_03d(band, barHeight);             // Green-Cyan-Blue Gradient Static                *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Orange2);         // Orange Peaks
      break;
    
    case 26:
      BarsUp_Purple_03d(band, barHeight);             // Green-Cyan-Blue Gradient Static                *** W O R K I N G ***
      NormalPeaks(band, PeakColor_Green);         // Orange Peaks
      break;

    case 27:
      CenterBars_Heat_06s(band, barHeight);         // Purple peak colors
      break;

    case 28:
      CenterBars_Sunset_01s(band, barHeight);       // Purple peak colors
      break;

   case 29:
      CenterBars_Purple_03s(band, barHeight);       // Purple - Blue
      break;
    
    case 30:
      make_fire2(); // go to demo mode
      break;      
   }
    oldBarHeights[band] = barHeight;                // Save oldBarHeights for averaging later
 }

// Decay peak
  EVERY_N_MILLISECONDS(Fallingspeed) {
    for (byte band = 0; band < COLUMNS; band++) {
      if (PeakFlag[band] == 1) {
        PeakTimer[band]++;
        if (PeakTimer[band] > Peakdelay) {
          PeakTimer[band] = 0;
          PeakFlag[band] = 0;
        }
      } else if (peak[band] > 0) {
        peak[band] -= 1;
      }
    }
    colorTimer++;
  }
  EVERY_N_MILLISECONDS(10) colorTimer++;   // Used in some of the patterns
  EVERY_N_SECONDS(AutoChangetime) {
    if (autoChangePatterns) buttonPushCounter = (buttonPushCounter + 1) % (NumberOfModes - 1); //timer to autochange patterns when enabled but exclude demo mode
   dbgprint("Change=true?:%d Now in mode:%d",autoChangePatterns,buttonPushCounter);
  }
  FastLED.show();
}
/********************************************************************************************************************************
*  END SUB  Main Loop
********************************************************************************************************************************/

/********************************************************************************************************************************
*  SUB-Rountines related to Paterns and Peaks
********************************************************************************************************************************/
void ChangingBars(int band, int barHeight) {                             // Rainbow dynamic color Mode 0
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, CHSV(ChangingBars_Color));
    }
  }
}

void ChangingBarsA(int band, int barHeight) {                             // Rainbow dynamic color Mode 0
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, CHSV(ChangingBars_Color1));
    }
  }
}

void RainbowBars(int band, int barHeight) {                              // Rainbow Static with Peaks color Mode 2
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, CHSV(RainbowBars_Color));
    }
  }
}

void RainbowBarsA(int band, int barHeight) {                              // Rainbow Static with Peaks color Mode 2
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, CHSV(RainbowBars_Color1));
    }
  }
}


//********* Normal Bars Up Purple Gradiant STATIC *********************************************************************************

void BarsUp_Purple_01s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal1, y*11));//
    }
  }
}
void BarsUp_Purple_02s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal2, y*12.75));//
    }
  }
}
void BarsUp_Purple_03s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal3, y*11));//
    }
  }
}
void BarsUp_Purple_04s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal4, y*11));//
    }
  }
}
void BarsUp_Purple_05s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal5, y*11));//
    }
  }
}

//********* Normal Bars Up Purple Gradiant DYNAMIC *********************************************************************************

void BarsUp_Purple_01d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal1, y*12.75 + colorTimer));//
    }
  }
}
void BarsUp_Purple_02d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal2, y*12.75 + colorTimer));//
    }
  }
}
void BarsUp_Purple_03d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal3, y*12.75 + colorTimer));//
    }
  }
}
void BarsUp_Purple_04d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal4, y*12.75 + colorTimer));//
    }
  }
}
void BarsUp_Purple_05d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal5, y*12.75 + colorTimer));//
    }
  }
}
//********* Normal Bars Up YouTub Gradiant STATIC *********************************************************************************

void BarsUp_Tub_01s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub1, y*12.75));//
    }
  }
}
void BarsUp_Tub_02s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub2, y*12.75));//
    }
  }
}
void BarsUp_Tub_03s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub3, y*11));//
    }
  }
}
void BarsUp_Tub_04s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub4, y*12.75));//
    }
  }
}
void BarsUp_Tub_05s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub5, y*11));//
    }
  }
}
void BarsUp_Tub_06s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub6, y*11));//
    }
  }
}
void BarsUp_Tub_07s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub7, y*11));//
    }
  }
}
void BarsUp_Tub_08s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub8, y*12));//
    }
  }
}
void BarsUp_Tub_09s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub9, y*12));//
    }
  }
}

void BarsUp_Tub_11s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub11, y*12));//
    }
  }
}

void BarsUp_Tub_13s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub13, y*12));//
    }
  }
}
void BarsUp_Tub_14s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub14, y*12));//
    }
  }
}


//********* Normal Bars Up YouTub Gradiant DYNAMIC *********************************************************************************

void BarsUp_Tub_01d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub2, y*12.75 + colorTimer));//
    }
  }
}
void BarsUp_Tub_02d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub2, y*12.75 + colorTimer));//
    }
  }
}
void BarsUp_Tub_03d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub3, y*11 + colorTimer));//
    }
  }
}
void BarsUp_Tub_04d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub4, y*12.75 + colorTimer));//
    }
  }
}
void BarsUp_Tub_05d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub5, y*11 + colorTimer));//
    }
  }
}
void BarsUp_Tub_06d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub6, y*11 + colorTimer));//
    }
  }
}
void BarsUp_Tub_07d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub7, y*11 + colorTimer));//
    }
  }
}
void BarsUp_Tub_08d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub8, y*12 + colorTimer));//
    }
  }
}
void BarsUp_Tub_09d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub9, y*12 + colorTimer));//
    }
  }
}

void BarsUp_Tub_11d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub11, y*12 + colorTimer));//
    }
  }
}

void BarsUp_Tub_13d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub13, y*12 + colorTimer));//
    }
  }
}
void BarsUp_Tub_14d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(youtub14, y*12 + colorTimer));//
    }
  }
}


//********* Normal Bars Up HeatPal Gradiant STATIC *********************************************************************************

void BarsUp_Heat_01s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal1, y*12.75));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_02s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal2, y*12.75));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_03s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal3, y*12.75));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_04s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal4, y*12.75));//
    }
  }
}
void BarsUp_Heat_05s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal5, y*12.75));//
    }
  }
}
void BarsUp_Heat_06s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal6, y*12.75));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_07s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal7, y*12.75));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_08s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal8, y*12.75));//
    }
  }
}
void BarsUp_Heat_09s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal9, y*12.75));// 
    }
  }
}
void BarsUp_Heat_10s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal10, y*12.75));// 
    }
  }
}
void BarsUp_Heat_11s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal11, y*12.75));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_12s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal12, y*12.75));//
    }
  }
}
void BarsUp_Heat_13s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal13, y*12.75));//
    }
  }
}
void BarsUp_Heat_14s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal14, y*12.75));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_15s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal15, y*12.75));//
    }
  }
}


//********* Normal Bars Up HeatPal Gradiant DYNAMIC *********************************************************************************

void BarsUp_Heat_01d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal1, y*12 + colorTimer));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_02d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal2, y*12 + colorTimer));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_03d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal3, y*12 + colorTimer));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_04d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal4, y*12 + colorTimer));//
    }
  }
}
void BarsUp_Heat_05d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal5, y*12 + colorTimer));//
    }
  }
}
void BarsUp_Heat_06d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal5, y*12 + colorTimer));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_07d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal7, y*12 + colorTimer));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_08d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal8, y*12 + colorTimer));//
    }
  }
}
void BarsUp_Heat_09d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal9, y*12 + colorTimer));//
    }
  }
}
void BarsUp_Heat_10d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal10, y*12 + colorTimer));//
    }
  }
}
void BarsUp_Heat_11d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal11, y*12 + colorTimer));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_12d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal12, y*12 + colorTimer));//
    }
  }
}
void BarsUp_Heat_13d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal13, y*12 + colorTimer));//
    }
  }
}
void BarsUp_Heat_14d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal14, y*12 + colorTimer));// ****** W I N N E R *******
    }
  }
}
void BarsUp_Heat_15d(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = TOP; y >= TOP - barHeight; y--) {
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal15, y*12 + colorTimer));//
    }
  }
}


//********* Center Bars Youtub Gradiant Static *********************************************************************************

void CenterBars_Youtub_01s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(youtub1, colorIndex));
    }
  }
}
void CenterBars_Youtub_02s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(youtub2, colorIndex));
    }
  }
}
void CenterBars_Youtub_03s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(youtub3, colorIndex));
    }
  }
}
void CenterBars_Youtub_04s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(youtub4, colorIndex));
    }
  }
}
void CenterBars_Youtub_05s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(youtub5, colorIndex));
    }
  }
}
void CenterBars_Youtub_06s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(youtub6, colorIndex));
    }
  }
}
void CenterBars_Youtub_07s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(youtub7, colorIndex));
    }
  }
}
void CenterBars_Youtub_08s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(youtub8, colorIndex));
    }
  }
}

//********* Center Bars Purple Gradiant Static *********************************************************************************

void CenterBars_Purple_01s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal1, colorIndex));
    }
  }
}
void CenterBars_Purple_02s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal2, colorIndex));
    }
  }
}
void CenterBars_Purple_03s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal3, colorIndex));
    }
  }
}
void CenterBars_Purple_04s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal4, colorIndex));
    }
  }
}
void CenterBars_Purple_05s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal5, colorIndex));
    }
  }
}
void CenterBars_Purple_06s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal6, colorIndex));
    }
  }
}
void CenterBars_Purple_07s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal7, colorIndex));
    }
  }
}
void CenterBars_Purple_08s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(purplePal8, colorIndex));
    }
  }
}


//********* Center Bars Heat Gradiant Static *********************************************************************************

void CenterBars_Heat_01s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal1, colorIndex));
    }
  }
}
void CenterBars_Heat_02s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal2, colorIndex));
    }
  }
}
void CenterBars_Heat_03s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal3, colorIndex));
    }
  }
}
void CenterBars_Heat_04s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal4, colorIndex));
    }
  }
}
void CenterBars_Heat_05s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal5, colorIndex));
    }
  }
}
void CenterBars_Heat_06s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal6, colorIndex));
    }
  }
}
void CenterBars_Heat_07s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal7, colorIndex));
    }
  }
}
void CenterBars_Heat_08s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal8, colorIndex));
    }
  }
}
void CenterBars_Heat_09s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal9, colorIndex));
    }
  }
}
void CenterBars_Heat_10s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal10, colorIndex));
    }
  }
}
void CenterBars_Heat_11s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal11, colorIndex));
    }
  }
}
void CenterBars_Heat_12s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal12, colorIndex));
    }
  }
}
void CenterBars_Heat_13s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal13, colorIndex));
    }
  }
}
void CenterBars_Heat_14s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal14, colorIndex));
    }
  }
}
void CenterBars_Heat_15s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(heatPal15, colorIndex));
    }
  }
}
void CenterBars_Sunset_01s(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    if (barHeight < 0) barHeight = 1; // at least a white line in the middle is what we want
    int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, ColorFromPalette(sunset, colorIndex));
    }
  }
}
//********* Center Bars RAINBOW DYNAMIC & STATIC Bars *********************************************************************************

void CenterBars_RainbowDyV(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
      if (barHeight < 0) barHeight = 1;               // at least a line in the middle is what we want
      int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y,  CHSV(ChangingBars_Color));
    }
  }
}

void CenterBars_RainbowDyH(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
      if (barHeight < 0) barHeight = 1;               // at least a line in the middle is what we want
      int yStart = ((kMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix -> drawPixel(x, y, CHSV(RainbowBars_Color1));
    }
  }
}


//*************** Peak Modes *************************************************************************

void RainbowPeaksSt(int band) {                               // Rainbow Peaks Horizontal Static 
  int xStart = BAR_WIDTH * band;
  int peakHeight = TOP - peak[band] - 1;
    for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    matrix -> drawPixel(x, peakHeight, CHSV(RainbowBars_Color)); 
  }
}

void RainbowPeaksDy(int band) {                               // Rainbow Peaks Horizontal Dynamic
  int xStart = BAR_WIDTH * band;
  int peakHeight = TOP - peak[band] - 1;
    for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    matrix -> drawPixel(x, peakHeight, CHSV(RainbowBars_Color1)); 
  }
}

void OutrunPeaks(int band) {                                  // Rainbow Peaks Verical Dynamic
  int xStart = BAR_WIDTH * band;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    matrix -> drawPixel(x, peakHeight, ColorFromPalette(youtub1, peakHeight * (255 / kMatrixHeight)));
  }
}

void NormalPeaks(int band, int H, int S, int V) {            // Normal Peaks CHSV control
  int xStart = BAR_WIDTH * band;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
  matrix -> drawPixel(x, peakHeight, CHSV(H, S, V));
  }
}

void NormalPeaksA(int band, int R, int G, int B) {           // Normal peaks RGB control
  int xStart = BAR_WIDTH * band;
  int peakHeight = TOP - peak[band] - 1;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    matrix -> drawPixel(x, peakHeight, CRGB(R, G, B));
  }
}

/********************************************************************************************************************************
*  END SUB Rountines related to Paterns and Peaks                                                                               *
********************************************************************************************************************************/

/********************************************************************************************************************************
*  Sub Routine for Diagnostics                                                                                                  *
********************************************************************************************************************************/
void Run_Diagnostics() {
  delay(100);
  DEBUG = 1; //this is needed to fully use the dbgprint function
  dbgprint("**************************************************************************************");
  dbgprint("*   Diagnostic Mode                                                                  *");
  dbgprint("**************************************************************************************");
  dbgprint("*    Arduino Sketch Version %s                                                     *", version);
  dbgprint("*    Mark Donners, The Electronic Engineer                                           *");
  dbgprint("*    Website:   www.theelectronicengineer.nl                                         *");
  dbgprint("*    facebook:  https://www.facebook.com/TheelectronicEngineer                       *");
  dbgprint("*    youtube:   https://www.youtube.com/channel/UCm5wy-2RoXGjG2F9wpDFF3w             *");
  dbgprint("*    github:    https://github.com/donnersm                                          *");
  dbgprint("**************************************************************************************");
  dbgprint("\n");

  dbgprint("**************************************************************************************");
  dbgprint("* The Colors of the Dutch Flag will now alternate on all leds                        *");
  dbgprint("* Press the Mode button to exit                                                      *");
  dbgprint("**************************************************************************************");
  Matrix_Flag();
  WaitForKeyRelease();
  WaitforKeyPress();
  WaitForKeyRelease();
  dbgprint("\n");

  dbgprint("**************************************************************************************");
  dbgprint("* Now showing Rainbow mode                                                           *");
  dbgprint("* Press the Mode button to exit                                                      *");
  dbgprint("**************************************************************************************");
  dbgprint("\n\n");
  delay(500);

  while (1) {
    Matrix_Rainbow();
    if (digitalRead(Switch1) == LOW) break;
  }
  dbgprint("break detected");
  WaitForKeyRelease();

  dbgprint("**************************************************************************************");
  dbgprint("* Now Only the Logo Ledstrip will blink 3x in red color 1 sec on/ 1 sec off          *");

  FastLED.clear();
  for (int j = 0; j < 3; j++) {
    Logo_Blink();
    dbgprint("* Logo Blink test %d of 3 done                                                        *", j + 1);
  }
  dbgprint("* Testing of Logo Ledstrip is done                                                   *");
  dbgprint("**************************************************************************************");
  dbgprint("\n");
  WaitforKeyPress();
  WaitForKeyRelease();

  dbgprint("**************************** Diagnostic LED Test Finished  ***************************");
  dbgprint("* You where able to see the red, white and blue Flag? It's the Dutch Flag!           *");
  dbgprint("* All leds where on? No defective ones? Also, the Logo was blinking 9x in red, right?*");
  dbgprint("* Press the button again to continue                                                 *");
  dbgprint("**************************************************************************************");
  dbgprint("\n");
  WaitforKeyPress();
  WaitForKeyRelease();

  dbgprint("*********************** Frequency Board Test *****************************************");
  dbgprint("* To test the outputs of the frequency board Remove both MSGEQ7 Ic's from the socket *");
  dbgprint("* place a 1K resistor in each socket between pin 3(output) and pin 8(clock)          *");
  dbgprint("* Channel 0 should output a frequency around 5 Khz. while channel 1 will give 10Khz  *");
  dbgprint("* This is not accurate measurement and only a indication. Value +/- 500Hz is fine    *");
  dbgprint("* If a channel gives you a measurement of 0, it means it is not working              *");
  dbgprint("* To exit, press and hold the mode key for 3 seconds                                 *");
  dbgprint("************************ Frequency Board Test ****************************************");
  dbgprint("\n");

  WaitForKeyRelease();
  delay(500);
  dbgprint("****************************** Brightness gain test *********************************");
  dbgprint("* This will print both adc values until you press the mode button.                  *");
  dbgprint("* Press the mode button to begin                                                    *");
  dbgprint("******************************** ADC value test *************************************");
  dbgprint("\n");
  WaitforKeyPress();
  WaitForKeyRelease();
  while (digitalRead(27) == HIGH) {
  dbgprint("ADC value 0:  %d   ADC Value 1:  %d", analogRead(0), analogRead(1));
  };

  WaitForKeyRelease();
  delay(500);

  dbgprint("****************************** Potmeter test ****************************************");
  dbgprint("* This will print the mapped values potmeters until you press the mode button       *");
  dbgprint("* Sense: 50-4095 , Brightness: 10-Brightnessmax , Peak Delay: 1-150                 *");
  dbgprint("* Press the mode button to begin                                                    *");
  dbgprint("****************************** Potmeter test ****************************************");
  dbgprint("\n");
  WaitforKeyPress();
  WaitForKeyRelease();
  while (digitalRead(59) == HIGH) {
    AMPLITUDE = map(analogRead(SENSITIVITYPOT), 0, 4095, 4095, 50);                 // read sensitivity potmeter and update amplitude setting
    BRIGHTNESSMARK = map(analogRead(BRIGHTNESSPOT), 0, 4095, BRIGHTNESSMAX, 10);    // read brightness potmeter
    Peakdelay = map(analogRead(PEAKDELAYPOT), 0, 4095, 150, 1);                     // update the Peakdelay time with value from potmeter
    dbgprint("Sense (50-4095):  %d  -  Brightness(10-Brightnessmax):  %d  -  Peak Delay(1-150): %d", AMPLITUDE, BRIGHTNESSMARK, Peakdelay);
  };

  WaitForKeyRelease();
  delay(500);
  dbgprint("* When you press the mode button, the system will go to normal operation mode but   *");
  dbgprint("* with the debug feedback on.                                                       *");
  dbgprint("******************************** END of TEST ****************************************");
  WaitforKeyPress();

}
/********************************************************************************************************************************
 * ** END Sub Routine for Diagnostics                                                                                              **
 ********************************************************************************************************************************/

/********************************************************************************************************************************
 * ** sub function to make rainbowcolors on ledstrip                                                                                                   **
 ********************************************************************************************************************************/
void rainbow_wave(uint8_t thisSpeed, uint8_t deltaHue) {          // The fill_rainbow call doesn't support brightness levels.

  // uint8_t thisHue = beatsin8(thisSpeed,0,255);                 // A simple rainbow wave.
  uint8_t thisHue = beat8(thisSpeed, 255); // A simple rainbow march.
  fill_rainbow(LogoLeds, NUM_LEDS_LOGO, thisHue, deltaHue);       // Use FastLED's fill_rainbow routine.
} // rainbow_wave()

//--------------------------RED FLAMES---------------------------------
void make_fire() {
  uint16_t i, j;
  if (t > millis()) return;
  t = millis() + (1000 / FPS);
  // First, all existing heat will reach the display and fade
  for (i = rows - 1; i > 0; --i) {
    for (j = 0; j < cols; ++j) {
      uint8_t n = 0;
      if (pix[i - 1][j] > 0)
        n = pix[i - 1][j] - 1;
      pix[i][j] = n;
    }
  }
  // Heated bottom row
  for (j = 0; j < cols; ++j) {
    i = pix[0][j];
    if (i > 0) {
      pix[0][j] = random(NCOLORS - 6, NCOLORS - 2);
    }
  }
  // Flare
  for (i = 0; i < nflare; ++i) {
    int x = flare[i] & 0xff;
    int y = (flare[i] >> 8) & 0xff;
    int z = (flare[i] >> 16) & 0xff;
    glow(x, y, z);
    if (z > 1) {
      flare[i] = (flare[i] & 0xffff) | ((z - 1) << 16);
    } else {
      // This flare is out
      for (int j = i + 1; j < nflare; ++j) {
        flare[j - 1] = flare[j];
      }
      --nflare;
    }
  }
  newflare();
  // Setting and drawing
  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
      matrix -> drawPixel(j, rows - i, colors1[pix[i][j]]);
    }
  }
}

//--------------------------BLUE FLAMES---------------------------------
void make_fire1() {
  uint16_t i, j;
  if (t > millis()) return;
  t = millis() + (1000 / FPS);
  // First, all existing heat will reach the display and fade
  for (i = rows - 1; i > 0; --i) {
    for (j = 0; j < cols; ++j) {
      uint8_t n = 0;
      if (pix[i - 1][j] > 0)
        n = pix[i - 1][j] - 1;
      pix[i][j] = n;
    }
  }
  // Heated bottom row
  for (j = 0; j < cols; ++j) {
    i = pix[0][j];
    if (i > 0) {
      pix[0][j] = random(NCOLORS - 6, NCOLORS - 2);
    }
  }
  // Flare
  for (i = 0; i < nflare; ++i) {
    int x = flare[i] & 0xff;
    int y = (flare[i] >> 8) & 0xff;
    int z = (flare[i] >> 16) & 0xff;
    glow(x, y, z);
    if (z > 1) {
      flare[i] = (flare[i] & 0xffff) | ((z - 1) << 16);
    } else {
      // This flare is out
      for (int j = i + 1; j < nflare; ++j) {
        flare[j - 1] = flare[j];
      }
      --nflare;
    }
  }
  newflare();
  // Setting and drawing
  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
      matrix -> drawPixel(j, rows - i, colors0[pix[i][j]]);
    }
  }
}

//--------------------------GREEN FLAMES---------------------------------
void make_fire2() {
  uint16_t i, j;
  if (t > millis()) return;
  t = millis() + (1000 / FPS);

  // First, all existing heat will reach the display and fade
  for (i = rows - 1; i > 0; --i) {
    for (j = 0; j < cols; ++j) {
      uint8_t n = 0;
      if (pix[i - 1][j] > 0)
        n = pix[i - 1][j] - 1;
      pix[i][j] = n;
    }
  }
  // Heated bottom row
  for (j = 0; j < cols; ++j) {
    i = pix[0][j];
    if (i > 0) {
      pix[0][j] = random(NCOLORS - 6, NCOLORS - 2);
    }
  }
  // Flare
  for (i = 0; i < nflare; ++i) {
    int x = flare[i] & 0xff;
    int y = (flare[i] >> 8) & 0xff;
    int z = (flare[i] >> 16) & 0xff;
    glow(x, y, z);
    if (z > 1) {
      flare[i] = (flare[i] & 0xffff) | ((z - 1) << 16);
    } else {
      // This flare is out
      for (int j = i + 1; j < nflare; ++j) {
        flare[j - 1] = flare[j];
      }
      --nflare;
    }
  }
  newflare();
  // Setting and drawing
  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
      matrix -> drawPixel(j, rows - i, colors2[pix[i][j]]);
    }
  }
}