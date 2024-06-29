/*   ****   A simple Graphical User Interface   ****
  featuring : 
              SPI OLED 320x240
              4 Touch screen buttons
              activating different programs
              
    NOTE: the OLED screen works on 3V logic!! it is intollerant to 5V
    touch the screen to choose between Mode0 to mode3.
    touch the arrow in order to proceed to the chosen mode

    
    by: Gal Arbel, 2024
*/
#include <Wire.h>
#include <ButtonIRQ.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <URTouch.h>
#include <Fonts/FreeSans9pt7b.h>
#include <RTClib.h>
#include <SD.h>
#include <LCDWIKI_GUI.h> // Core graphics library
#include <LCDWIKI_KBV.h> // Hardware-specific library
#include <TouchScreen.h>         //Adafruit Library

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define FILE_NAME_SIZE_MAX 20
char p[]="225termi.bmp";//picture



//int XP = 6, YP = A1, XM = A2, YM = 7;  //most common configuration
//int XP = 7, YP = A2, XM = A1, YM = 6;  //next common configuration
int XP = 5, YP = A3, XM = A3, YM = 7; 
LCDWIKI_KBV my_lcd(ILI9486, A3, A2, A1, A0, A4); // model, cs, cd, wr, rd, reset
RTC_DS3231 rtc;
//URTouch  myTouch(7, 6, 5, 4, 3); //tclk,  tcs,  tdin,  dout,  irq
TouchScreen ts(XP, YP, XM, YM, 300);   //re-initialised after diagnose

bool togsw = false;
bool result;
char t[32];



int btn2pin = 2; //reserved for future use

int x, y;
int mode;

uint32_t bmp_offset = 0;
uint16_t s_width = my_lcd.Get_Display_Width();
uint16_t s_height = my_lcd.Get_Display_Height();

uint16_t read_16(File fp) {
    uint8_t low;
    uint16_t high;
    low = fp.read();
    high = fp.read();
    return (high << 8) | low;
}

uint32_t read_32(File fp) {
    uint16_t low;
    uint32_t high;
    low = read_16(fp);
    high = read_16(fp);
    return (high << 16) | low;   
}

bool analysis_bmp_header(File fp, uint32_t &bmp_width, uint32_t &bmp_height) {
    if (read_16(fp) != 0x4D42) {
        Serial.println("Not a BMP file");
        return false;
    }
    read_32(fp); // File size
    read_32(fp); // Creator bytes
    bmp_offset = read_32(fp); // Start of image data
    read_32(fp); // Header size
    bmp_width = read_32(fp);
    bmp_height = read_32(fp);
    if (read_16(fp) != 1) {
        Serial.println("BMP has incorrect number of planes");
        return false;
    }
    if (read_16(fp) != 24) {
        Serial.println("BMP is not 24-bit");
        return false;
    }
    if (read_32(fp) != 0) {
        Serial.println("BMP is compressed");
        return false;
    }
    return true;
}

void draw_bmp_picture(const char *filename) {
    File bmp_file = SD.open(filename);
    if (!bmp_file) {
        Serial.print("File not found: ");
        Serial.println(filename);
        return;
    }

    uint32_t bmp_width, bmp_height;
    if (!analysis_bmp_header(bmp_file, bmp_width, bmp_height)) {
        Serial.print("Invalid BMP file: ");
        Serial.println(filename);
        bmp_file.close();
        return;
    }

    bmp_file.seek(bmp_offset);
    float x_ratio = bmp_width / (float)s_width;
    float y_ratio = bmp_height / (float)s_height;
    uint8_t r, g, b;

    for (uint16_t y = 0; y < s_height; y++) {
        for (uint16_t x = 0; x < s_width; x++) {
            bmp_file.seek(bmp_offset + (int(y * y_ratio) * bmp_width + int(x * x_ratio)) * 3);
            b = bmp_file.read();
            g = bmp_file.read();
            r = bmp_file.read();
            my_lcd.Set_Draw_color(my_lcd.Color_To_565(r, g, b));
            my_lcd.Draw_Pixel(x, s_height - y - 1); // BMP files are stored upside-down
        }
    }

    bmp_file.close();
}

void setup(){
  Serial.begin(115200); 
  my_lcd.Init_LCD();
  Serial.println(my_lcd.Read_ID(), HEX);
  my_lcd.Fill_Screen(BLUE);

  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
      my_lcd.Set_Text_Back_colour(BLUE);
      my_lcd.Set_Text_colour(WHITE);
      my_lcd.Set_Text_Size(1);
      my_lcd.Print_String("SD Card Init fail!", 0, 0);
      while (1); // Stop if SD card initialization fails
  } else {
      Serial.println("SD Card initialized.");
  }
  Wire.begin();
  //tftwelcome();
  //myTouch.InitTouch();
  //myTouch.setPrecision(PREC_MEDIUM);
  //drawButtons();
  //while(!arrow()){
    //checkTouch();
  //}
  my_lcd.Fill_Rect(0, 0, 320, 480, BLACK);

}

void loop(){
  if (ts.pressure()>10){
    //Serial.print("x = "); Serial.println(ts.readTouchX());
    Serial.print("y = "); Serial.println(ts.readTouchY());
    delay(1000);
  }
  //draw_bmp_picture("480wlcm.bmp");
  /*
  switch (mode){
    case 0:
      Serial.println("program 0 running");
      break;
    case 1:
      Serial.println("program 1 running");
      break;
    case 2:
      Serial.println("program 2 running");
      break;
    case 3:
      Serial.println("program 3 running");
      break;
  }
  //Serial.print("mode "); Serial.print(mode); Serial.println(" selected");
        //my_lcd.fillScreen(ILI9341_BLACK);
*/
}


/*
void drawButtons(){
  my_lcd.Fill_Rect(0, 0, 480, 320, BLACK);
  my_lcd.setFont(&FreeSans9pt7b);
  my_lcd.setCursor(30, 50);
  my_lcd.setTextSize(2);
  my_lcd.setTextColor(WHITE);
  my_lcd.print("Choose Mode");
  for (x = 0; x < 4; x++)
  {
    my_lcd.setTextSize(1);
    my_lcd.setTextColor(GREEN);
    my_lcd.fillRect(30+(x*62), 100, 60, 60, WHITE - (x*1000));//x,y,w,h,col
    my_lcd.setCursor(37+(x*62), 120);
    my_lcd.print("mode ");
    my_lcd.setCursor(50+(x*62), 140);
    my_lcd.print(x);
  }
  //Draw Arrow
  my_lcd.fillRect(90, 190, 110, 15, MAGENTA);
  my_lcd.fillTriangle(200, 180, 230, 197, 200, 215, MAGENTA);
  my_lcd.setCursor(120, 202);
  my_lcd.setTextColor(YELLOW);
  my_lcd.print("proceed");
} 

void checkTouch(){
  if(myTouch.dataAvailable()){
    myTouch.read();
    x = myTouch.getX();
    y = myTouch.getY();
    //Serial.print("x = ");
    //Serial.println();
    //Serial.print("y = ");
    //Serial.println(y); 
    if(y > 100 && y < 160){
      if(x > 30 && x < 90){//mode 0 selected
        Serial.println("mode 0 Selected");
        drawButtons();
        buttonTouched(30, 100);
        mode = 0;
      } else if (x > 92 && x < 152){//mode 1 selected
          Serial.println("mode 1 Selected");
          drawButtons();
          buttonTouched(92, 100);
          mode = 1;
      } else if (x > 154 && x < 214){//mode 2 selected
          Serial.println("mode 2 Selected");
          drawButtons();
          buttonTouched(154, 100);
          mode = 2;
      }  else if (x > 216 && x < 276){//mode 3 selected
          Serial.println("mode 3 Selected");
          drawButtons();
          buttonTouched(216, 100);
          mode = 4;
      } 
    }          
  } 
  else {
    delay(100);
  }
}
void buttonTouched(int x, int y){
  int width = 60;
  my_lcd.drawRect(x, y, width, width, BLUE);
  my_lcd.drawRect(x+1, y+1, width-2, width-2, BLUE);
  my_lcd.drawRect(x+2, y+2, width-4, width-4, BLACK);

}
bool arrow(){
  if(myTouch.dataAvailable()){
    myTouch.read();
    x = myTouch.getX();
    y = myTouch.getY();
    if(x > 90 && x < 230){
      if(y > 190 && x < 210){
        return true;
      }
    }
    else return false;
  }
}

void sdbegin(){
  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  while (!SD.begin(SD_CS_PIN)) {//CS pin
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your module?");
  } 
  
    Serial.println("SD Wiring is correct and a card is present.");
    delay(2000); 
         
}
*/

void tftwelcome() {
  char p[20] = "480wlcm.bmp"; // Initial picture file name with enough space
  draw_bmp_picture(p);
  // Display the first image
  delay(2500);      // Wait for 2500 milliseconds (2.5 seconds)

   
}
