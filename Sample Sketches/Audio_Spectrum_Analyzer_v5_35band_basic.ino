//   Project: 14 Band Spectrum Analyzer using WS2812B/SK6812
//   Target Platform: Arduino Mega2560 or Mega2560 PRO MINI  
//   The original code has been modified by PLATINUM to allow a scalable platform with many more bands.
//   It is not possible to run modified code on a UNO,NANO or PRO MINI. due to memory limitations.
//   The library Si5351mcu is being utilized for programming masterclock IC frequencies. 
//   Special thanks to Pavel Milanes for his outstanding work. https://github.com/pavelmc/Si5351mcu
//   Analog reading of MSGEQ7 IC1 and IC2 use pin A0 and A1.
//   Clock pin from MSGEQ7 IC1 and IC2 to Si5351mcu board clock 0 and 1
//   Si5351mcu SCL and SDA use pin 20 and 21
//   See the Schematic Diagram for more info
//   Programmed and tested by PLATINUM
//   Version 1.0    
//***************************************************************************************************

#include <Adafruit_NeoPixel.h>
#include <Adafruit_SI5351.h>
#define NOISE         50
#define ROWS          20  //num of row MAX=20
#define COLUMNS       35  //num of column
#define DATA_PIN     18   //led data pin
#define STROBE_PIN   17   //MSGEQ7 strobe pin
#define RESET_PIN    16   //MSGEQ7 reset pin
#define NUMPIXELS    ROWS * COLUMNS
Adafruit_SI5351 clockgen1;
Adafruit_SI5351 clockgen2;
TwoWire I2C_1 = TwoWire(0);
TwoWire I2C_2 = TwoWire(1);
#define SDA_1 21
#define SCL_1 22
#define SDA_2 23
#define SCL_2 19
#define SMOOTHING_FACTOR .8  // Define smoothing factor
int smoothedSpectrumValue[COLUMNS] = {0};  // Store smoothed values
struct Point{
char x, y;
char  r,g,b;
bool active;
};
struct TopPoint{
int position;
int peakpause;
};
Point spectrum[ROWS][COLUMNS];
TopPoint peakhold[COLUMNS];
int spectrumValue[COLUMNS];
long int counter = 0;
int long pwmpulse = 0;
bool toggle = false;
int long time_change = 0;
int effect = 0;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);
  void setup() 
  {
  Serial.begin(115200);
  I2C_1.begin(SDA_1, SCL_1, 100000);
  if (clockgen1.begin(&I2C_1) == ERROR_NONE) {  // Pass as pointer
  Serial.println("Si5351 #1 Initialized Successfully");
  } else {
  Serial.println("Si5351 #1 Initialization Failed");
  }
  I2C_2.begin(SDA_2, SCL_2, 100000);
  if (clockgen2.begin(&I2C_2) == ERROR_NONE) {  // Pass as pointer
  Serial.println("Si5351 #2 Initialized Successfully");
  } else {
  Serial.println("Si5351 #2 Initialization Failed");
  }
  clockgen1.setupPLL(SI5351_PLL_A, 36, 0, 1);
  clockgen1.setupMultisynth(0, SI5351_PLL_A, 1277, 2, 11);
  clockgen1.setupRdiv(0, SI5351_R_DIV_8);
  clockgen1.setupPLL(SI5351_PLL_B, 36, 0, 1);
  clockgen1.setupMultisynth(1, SI5351_PLL_B, 1050, 42, 107);
  clockgen1.setupRdiv(1, SI5351_R_DIV_8);
  clockgen1.setupMultisynth(2, SI5351_PLL_B, 1784, 1, 1);
  clockgen1.setupRdiv(2, SI5351_R_DIV_4);
  clockgen1.enableOutputs(true);
  clockgen2.setupPLL(SI5351_PLL_A, 36, 0, 1);
  clockgen2.setupMultisynth(0, SI5351_PLL_A, 1480, 5, 19); 
  clockgen2.setupRdiv(0, SI5351_R_DIV_4);
  clockgen2.setupPLL(SI5351_PLL_B, 36, 0, 1);
  clockgen2.setupMultisynth(1, SI5351_PLL_B, 1250, 1, 1);
  clockgen2.setupRdiv(1, SI5351_R_DIV_4);
  clockgen2.enableOutputs(true);
  Serial.println("Si5351 devices initialized and configured.");
  pinMode      (STROBE_PIN,    OUTPUT);
  pinMode      (RESET_PIN,     OUTPUT);
  pinMode      (DATA_PIN,      OUTPUT);
  pixels.setBrightness(20); //set Brightness
  pixels.begin();
  pixels.show();
  pinMode      (STROBE_PIN, OUTPUT);
  pinMode      (RESET_PIN,  OUTPUT);  
  digitalWrite (RESET_PIN,  LOW);
  digitalWrite (STROBE_PIN, LOW);
  delay        (1);  
  digitalWrite (RESET_PIN,  HIGH);
  delay        (1);
  digitalWrite (RESET_PIN,  LOW);
  digitalWrite (STROBE_PIN, HIGH);
  delay        (1);
  }
  void loop() 
  {    
  counter++;   
  clearspectrum(); 
  if (millis() - pwmpulse > 3000){
  toggle = !toggle;
  pwmpulse = millis();
  }
  digitalWrite(RESET_PIN, HIGH);
  delayMicroseconds(3000);
  digitalWrite(RESET_PIN, LOW);
  for(int i=0; i < COLUMNS; i++){ 
  digitalWrite(STROBE_PIN, LOW);
  delayMicroseconds(1000);
  int newReading = analogRead(36);
  if(newReading < 300) newReading = 0;
  newReading = constrain(newReading, 0, 1023);
  newReading = map(newReading, 0, 1023, 0, ROWS);
  spectrumValue[i] = (SMOOTHING_FACTOR * newReading) + ((1 - SMOOTHING_FACTOR) * spectrumValue[i]);  
  i++;
  newReading = analogRead(39);
  if(newReading < 300) newReading = 0;
  newReading = constrain(newReading, 0, 1023);
  newReading = map(newReading, 0, 1023, 0, ROWS);
  spectrumValue[i] = (SMOOTHING_FACTOR * newReading) + ((1 - SMOOTHING_FACTOR) * spectrumValue[i]);  
  i++;
  newReading = analogRead(34);
  if(newReading < 300) newReading = 0;
  newReading = constrain(newReading, 0, 1023);
  newReading = map(newReading, 0, 1023, 0, ROWS);
  spectrumValue[i] = (SMOOTHING_FACTOR * newReading) + ((1 - SMOOTHING_FACTOR) * spectrumValue[i]);  
  i++;
  newReading = analogRead(35);
  if(newReading < 300) newReading = 0;
  newReading = constrain(newReading, 0, 1023);
  newReading = map(newReading, 0, 1023, 0, ROWS);
  spectrumValue[i] = (SMOOTHING_FACTOR * newReading) + ((1 - SMOOTHING_FACTOR) * spectrumValue[i]);  
  i++;
  newReading = analogRead(32);
  if(newReading < 300) newReading = 0;
  newReading = constrain(newReading, 0, 1023);
  newReading = map(newReading, 0, 1023, 0, ROWS);
  spectrumValue[i] = (SMOOTHING_FACTOR * newReading) + ((1 - SMOOTHING_FACTOR) * spectrumValue[i]);
  digitalWrite(STROBE_PIN, HIGH);  
  }
  for(int j = 0; j < COLUMNS; j++){
  for(int i = 0; i < spectrumValue[j]; i++){ 
  spectrum[i][COLUMNS - 1 - j].active = 1;
  spectrum[i][COLUMNS - 1 - j].r = 0;   // Column Color red
  spectrum[i][COLUMNS - 1 - j].g = 255; // Column Color green
  spectrum[i][COLUMNS - 1 - j].b = 0;   // Column Color blue
  }
  if (spectrumValue[j] - 1 > peakhold[j].position) {
  peakhold[j].position = spectrumValue[j] - 1;
  peakhold[j].peakpause = 10; // Reset peak hold pause
  } else if (peakhold[j].peakpause > 0) {
  peakhold[j].peakpause--;  // Wait before dropping
  } else if (peakhold[j].position > 0) {
  peakhold[j].position--; // Slowly decay the peak
  }
  if (peakhold[j].position >= 0) {
  spectrum[peakhold[j].position][COLUMNS - 1 - j].active = 1;
  spectrum[peakhold[j].position][COLUMNS - 1 - j].r = 255;  // Peak Color red
  spectrum[peakhold[j].position][COLUMNS - 1 - j].g = 255;  // Peak Color green
  spectrum[peakhold[j].position][COLUMNS - 1 - j].b = 0;    // Peak Color blue
  }
  }
  flushMatrix();
  }
  void topSinking()
  {
  for(int j = 0; j < ROWS; j++)
  {
  if(peakhold[j].position > 0 && peakhold[j].peakpause <= 0) peakhold[j].position--;
  else if(peakhold[j].peakpause > 0) peakhold[j].peakpause--;       
  } 
  }
  void clearspectrum()
  {
  for(int i = 0; i < ROWS; i++)
  {
  for(int j = 0; j < COLUMNS; j++)
  {
  spectrum[i][j].active = false;  
  } 
  }
  }
  void flushMatrix()
  {
  for(int j = 0; j < COLUMNS; j++)
  {
  if( j % 2 != 0)
  {
  for(int i = 0; i < ROWS; i++)
  {
  if(spectrum[ROWS - 1 - i][j].active)
  {
  pixels.setPixelColor(j * ROWS + i, pixels.Color(
  spectrum[ROWS - 1 - i][j].r, 
  spectrum[ROWS - 1 - i][j].g, 
  spectrum[ROWS - 1 - i][j].b));         
  }
  else
  {
  pixels.setPixelColor( j * ROWS + i, 0, 0, 0);  
  } 
  }
  }
  else
  {
  for(int i = 0; i < ROWS; i++)
  {
  if(spectrum[i][j].active)
  {
  pixels.setPixelColor(j * ROWS + i, pixels.Color(
  spectrum[i][j].r, 
  spectrum[i][j].g, 
  spectrum[i][j].b));     
  }
  else
  {
  pixels.setPixelColor( j * ROWS + i, 0, 0, 0);  
  }
  }      
  } 
  }
  pixels.show();
  }
  
