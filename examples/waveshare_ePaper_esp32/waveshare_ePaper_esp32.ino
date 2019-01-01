/*
  output to e-Paper Module sample
  2.9inch
*/

#include <FS.h>
#include <SPI.h>

//#define PORTRAIT
#define LANDSCAPE
#include <waveshare_ePaper_spi_ttf.h>

#define DRIVER_OUTPUT_CONTROL 0x01
#define BOOSTER_SOFT_START_CONTROL 0x0C
#define GATE_SCAN_START_POSITION 0x0F
#define DEEP_SLEEP_MODE 0x10
#define DATA_ENTRY_MODE_SETTING 0x11
#define SW_RESET 0x12
#define TEMPERATURE_SENSOR_CONTROL 0x1A
#define MASTER_ACTIVATION 0x20
#define DISPLAY_UPDATE_CONTROL_1 0x21
#define DISPLAY_UPDATE_CONTROL_2 0x22
#define WRITE_RAM 0x24
#define WRITE_VCOM_REGISTER 0x2C
#define WRITE_LUT_REGISTER 0x32
#define SET_DUMMY_LINE_PERIOD 0x3A
#define SET_GATE_TIME 0x3B
#define BORDER_WAVEFORM_CONTROL 0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION 0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION 0x45
#define SET_RAM_X_ADDRESS_COUNTER 0x4E
#define SET_RAM_Y_ADDRESS_COUNTER 0x4F
#define TERMINATE_FRAME_READ_WRITE 0xFF

#define ePaper_WHITE 0xFF
#define ePaper_BLACK 0x00

#if defined(PORTRAIT)
#define initPortrait true
#else
#define initPortrait false
#endif

//SPI pin settings
SPIClass ePaperspi = SPIClass(VSPI);
waveshare_ePaper_spi_ttf ttfout = waveshare_ePaper_spi_ttf(&ePaperspi);

enum {
  ePaper_CS = 27,
  ePaper_RESET = 26,
  ePaper_DC = 25,
  ePaper_BUSY = 35,
  SD_CS = 5
};
//SPI pin settings end

truetypeClass hiragino = truetypeClass(&SD);
//truetypeClass genshin = truetypeClass(&SD);
truetypeClass helvetica = truetypeClass(&SD);
//truetypeClass Avenir = truetypeClass(&SD);
//truetypeClass TNR = truetypeClass(&SD);
//truetypeClass Myriad = truetypeClass(&SD);
truetypeClass comic = truetypeClass(&SD);
//const char *fontFile = "/fonts/ipag.ttf";
//const char *fontFile = "/fonts/ipam.ttf";
const char *fontHiragino = "/fonts/hiraginog.ttf";
//const char *fontGenshin = "/fonts/gennokaku/GenShinGothic-Medium.ttf";
const char *fontHelvetica = "/fonts/helvetica.ttf";
//const char *fontAvenir = "/fonts/Avenir.ttf";
//const char *fontTNR = "/fonts/TimesNewRoman.ttf";
//const char *fontMyriad = "/fonts/myriad.ttf";
const char *fontComic = "/fonts/ComicSans.ttf";
//CheekFont-Regular
//e-Paper ttf font end

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for native USB port only
  delay(200);

  //e-Paper pin settings
  pinMode(ePaper_CS, OUTPUT);
  pinMode(ePaper_RESET, OUTPUT);
  pinMode(ePaper_DC, OUTPUT);
  pinMode(ePaper_BUSY, INPUT);

  digitalWrite(ePaper_CS, HIGH);
  digitalWrite(ePaper_RESET, HIGH);
  digitalWrite(ePaper_DC, HIGH);
  delay(200);
  //e-Paper pin settings end

  //SPI initilizetion
  ttfout.setSPIpin(ePaper_CS, ePaper_RESET, ePaper_DC, ePaper_BUSY); //set SPI for epaper pin (CS, RESET, DC, BUSY)
  ePaperspi.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  ePaperspi.begin();
  Serial.println("SPI init");
  //SPI initilizetion end

  //e-Paper initilizetion
  ttfout.init();
  Serial.println("ePaper init");
  Serial.print("width: ");
  Serial.println(ePaperWidth + 1);

  /*
     there are 2 memory areas embedded in the e-paper display
     and once the display is refreshed, the memory area will be auto-toggled,
     i.e. the next action of SetFrameMemory will set the other memory area
     therefore you have to clear the frame memory twice.
  */
  fill_rect(0, ePaperWidth, 0, ePaperHeight, HIGH); // HIGH:black, LOW:white
  displayFrame();
  fill_rect(0, ePaperWidth, 0, ePaperHeight, HIGH);
  displayFrame();
  delay(1000);
  fill_rect(0, ePaperWidth, 0, ePaperHeight, LOW);
  displayFrame();
  fill_rect(0, ePaperWidth, 0, ePaperHeight, LOW);
  displayFrame();
  delay(1000);
  //e-Paper initilizetion end

  //font begin
  hiragino.begin(SD_CS, fontHiragino);
  //genshin.begin(SD_CS, fontGenshin); //error
  comic.begin(SD_CS, fontComic); //error
  Serial.println("read fonts");

  //font output to e-Paper initialize
  ttfout.setTruetype(&comic); //set font. use with "truetype.h"
  ttfout.setColor(HIGH, HIGH, LOW); //set color (inside, outline, background)
  Serial.println("output ttf initialized");

  //font output to e-Paper
  ttfout.displayString(90, 0, "Hello", 80, 5); //output string (start_x, start_y, string(char), height of charctor, inter-character space)
  comic.end();

  fill_rect(95, 96, 90, 120, HIGH);
  helvetica.begin(SD_CS, fontHelvetica);
  ttfout.setTruetype(&helvetica); //set font. use with "truetype.h"
  ttfout.displayString(100, 90, "e-Paper", 30, 2); //output string (start_x, start_y, string(wchar_t, 2byte), height of charctor, inter-character space)
  helvetica.end();
  displayFrame();
  delay(3000);

  //display clear
  fill_rect(0, ePaperWidth, 0, ePaperHeight, LOW);
  displayFrame();
  fill_rect(0, ePaperWidth, 0, ePaperHeight, LOW);
  displayFrame();

  ttfout.setTruetype(&hiragino); //set font. use with "truetype.h"

  //display clear
  fill_rect(0, ePaperWidth, 0, ePaperHeight, LOW);
  displayFrame();
  fill_rect(0, ePaperWidth, 0, ePaperHeight, LOW);
  displayFrame();

  //count up
  ttfout.setColor(LOW, HIGH, LOW); //set color (inside, outline, background)
  for (int i = 0; i <= 30; i++) {
    char number[4];
    sprintf(number, "%03d", i);
    ttfout.displayMonospaced(80, 5, number, 60, 40); //output string (start_x, start_y, string(char), height of charctor, Monospace value)
    displayFrame();
  }
  ttfout.setColor(HIGH, HIGH, LOW); //set color (inside, outline, background)

  //display clear
  fill_rect(0, ePaperWidth, 0, ePaperHeight, LOW);
  displayFrame();
  fill_rect(0, ePaperWidth, 0, ePaperHeight, LOW);
  displayFrame();
}

//unicode start bit
uint32_t i = 0x4e9c;
//0x21 number
//0x4e9c //kanji in Japanese fonts.
wchar_t charctor[2] = {i, 0};

void loop() {
  //display all unicode charactor
  ttfout.displayString(110, 5, charctor, 100, 10);
  displayFrame();
  charctor[0]++;

  delay(10);
}

//-----------------------------------------------------------------------------------------------
//epaper fill rect (drawing in page units)
void fill_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2, bool _reversal) {
  uint16_t seekColumn;
  int8_t seekPage;
  uint8_t firstPageBit, lastPageBit;
  uint8_t fillData, firstPageData, lastPageData;

  if (initPortrait) {
    Serial.println("working...");
  } else {
    seekColumn = _x2 - _x1 + 1;
    seekPage = (uint8_t)(_y2 / 8) - (uint8_t)(_y1 / 8) - 1;
    firstPageBit = (_y1 % 8);
    lastPageBit = (_y2 % 8);

    fillData = 0b11111111;
    firstPageData = fillData >> (firstPageBit);
    lastPageData = fillData << (7 - lastPageBit);
  }

  if (_reversal) {
    fillData = ~fillData;
    firstPageData = ~firstPageData;
    lastPageData = ~lastPageData;
  }

  digitalWrite(ePaper_CS, LOW);

  if (initPortrait) {
    set_rect(_x1, _x2, _y1, _y2);
    setMemoryPointer(_x1, _y1);
  } else {
    set_rect(_y1, _y2, ePaperWidth - _x1, ePaperWidth - _x2);
    setMemoryPointer(_y1, ePaperWidth - _x1);
  }

  digitalWrite(ePaper_DC, LOW);
  ePaperspi.transfer(WRITE_RAM);
  digitalWrite(ePaper_DC, HIGH);

  if (initPortrait) {
    for (uint16_t i = 0; i < (_x2 - _x1 + 1) / 8 * (_y2 - _y1 + 1); i++) {
      ePaperspi.transfer(0xff);
    }
  } else {
    if (seekPage >= 0) {
      for (uint16_t col = 0; col < seekColumn; col++) {
        ePaperspi.transfer(firstPageData);
      }

      while (seekPage-- > 0 ) {
        for (uint16_t col = 0; col < seekColumn; col++) {
          ePaperspi.transfer(fillData);
        }
      }

      for (uint16_t col = 0; col < seekColumn; col++) {
        ePaperspi.transfer(lastPageData);
      }
    } else {
      fillData = firstPageData & lastPageData;

      for (uint16_t col = 0; col < seekColumn; col++) {
        ePaperspi.transfer(fillData);
      }
    }
  }

  digitalWrite(ePaper_CS, HIGH);
  wait_ePaper();

  return;
}

void set_rect(uint16_t _x1, uint16_t _x2, uint16_t _y1, uint16_t _y2) {
  digitalWrite(ePaper_DC, LOW);
  ePaperspi.transfer(SET_RAM_X_ADDRESS_START_END_POSITION);
  digitalWrite(ePaper_DC, HIGH);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  ePaperspi.transfer((_x1 >> 3) & 0xFF);
  ePaperspi.transfer((_x2 >> 3) & 0xFF);

  digitalWrite(ePaper_DC, LOW);
  ePaperspi.transfer(SET_RAM_Y_ADDRESS_START_END_POSITION);
  digitalWrite(ePaper_DC, HIGH);
  ePaperspi.transfer(_y1 & 0xFF);
  ePaperspi.transfer((_y1 >> 8) & 0xFF);
  ePaperspi.transfer(_y2 & 0xFF);
  ePaperspi.transfer((_y2 >> 8) & 0xFF);
}

void setMemoryPointer(int _x, int _y) {
  digitalWrite(ePaper_DC, LOW);
  ePaperspi.transfer(SET_RAM_X_ADDRESS_COUNTER);
  digitalWrite(ePaper_DC, HIGH);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  ePaperspi.transfer((_x >> 3) & 0xFF);

  digitalWrite(ePaper_DC, LOW);
  ePaperspi.transfer(SET_RAM_Y_ADDRESS_COUNTER);
  digitalWrite(ePaper_DC, HIGH);
  ePaperspi.transfer(_y & 0xFF);
  ePaperspi.transfer((_y >> 8) & 0xFF);

  wait_ePaper();
}

//flash display
void displayFrame() {
  digitalWrite(ePaper_CS, LOW);

  digitalWrite(ePaper_DC, LOW);
  ePaperspi.transfer(DISPLAY_UPDATE_CONTROL_2);
  digitalWrite(ePaper_DC, HIGH);
  ePaperspi.transfer(0xC4);

  digitalWrite(ePaper_DC, LOW);
  ePaperspi.transfer(MASTER_ACTIVATION);
  ePaperspi.transfer(TERMINATE_FRAME_READ_WRITE);
  digitalWrite(ePaper_DC, HIGH);

  digitalWrite(ePaper_CS, HIGH);

  wait_ePaper();
}

void sleep() {
  digitalWrite(ePaper_CS, LOW);
  digitalWrite(ePaper_DC, LOW);
  ePaperspi.transfer(DEEP_SLEEP_MODE);
  digitalWrite(ePaper_DC, HIGH);
  digitalWrite(ePaper_CS, HIGH);
  wait_ePaper();
}

void wait_ePaper() {
  while (digitalRead(ePaper_BUSY) == HIGH) { //LOW: idle, HIGH: busy
    delay(100);
  }
}
