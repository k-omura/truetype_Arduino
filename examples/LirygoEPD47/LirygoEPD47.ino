/*
  LilyGo T5 4.7 inch E-paper - ESP32
  based on https://github.com/Xinyuan-LilyGO/LilyGo-EPD47

  Settings
  PSRAM:enable
  Flash size:16MB(128Mb)
  Partition Scheme:16MB Flash (2MB APP/12.5MB FAT) (Set according to the font file and program size)

  "ESP32 Sketch Data Upload" before writing the program
  https://github.com/me-no-dev/arduino-esp32fs-plugin
*/

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include "epd_driver.h"
#include <FFat.h>

#include "truetype_Arduino.h"

#define BATT_PIN 36

//960x540
uint8_t *framebuffer;
bool EPD_done = LOW;
Rect_t area1 = {
  .x = 0,
  .y = 60,
  .width = EPD_WIDTH,
  .height =  EPD_HEIGHT - 60
};
Rect_t area2 = {
  .x = 0,
  .y = 0,
  .width = EPD_WIDTH,
  .height = 60
};

truetypeClass truetype = truetypeClass();

void setup() {
  Serial.begin(115200);

  //frameBuffer init
  framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!framebuffer) {
    Serial.println("alloc memory failed !!!");
    while (1);
  }
  memset(framebuffer, 0xff, EPD_WIDTH * EPD_HEIGHT / 2);

  //trueType
  if (!FFat.begin(true)) {
    Serial.println("Mount Failed");
    return;
  }
  Serial.println("File system mounted");

  File fontFile = FFat.open("/AlexBrush.ttf", "r");
  if (!fontFile) {
    Serial.println("Error opening the file");
    return;
  }
  Serial.println(fontFile.name());
  Serial.println(fontFile.size());
  if (!truetype.setTtfFile(fontFile)) {
    Serial.println("read ttf failed");
    return;
  }
  truetype.setFramebuffer(EPD_WIDTH, EPD_HEIGHT, 4, 0, framebuffer);
  truetype.setCharacterSize(100);
  truetype.setCharacterSpacing(0);
  truetype.setTextBoundary(10, EPD_WIDTH, EPD_HEIGHT);
  truetype.setTextColor(0x00, 0x00);

  truetype.textDraw(10, 100, "The quick brown fox jumps over the lazy dog");
  
  epd_init();
  epd_poweron();
  epd_clear();
  epd_draw_grayscale_image(epd_full_screen(), framebuffer);
  epd_poweroff();
}

void loop() {
}

void clearBitmap(uint16_t _x1, uint16_t _y1, uint16_t _x2, uint16_t _y2, uint8_t _colorCode, uint16_t _epd_width, uint8_t _bitmap[]) {
  for (uint16_t x = _x1; x <= _x2; x++) {
    for (uint16_t y = _y1; y <= _y2; y++) {
      bitmap_pixel(x, y, _colorCode, _epd_width, _bitmap);
    }
  }
}

void bitmap_pixel(uint16_t _x, uint16_t _y, uint16_t _colorCode, uint16_t _epd_width, uint8_t _bitmap[]) {
  uint8_t *buf_ptr = &_bitmap[(_x / 2) + _y * (_epd_width / 2)];
  _colorCode = _colorCode & 0b00001111;

  if (_x % 2) {
    *buf_ptr = (*buf_ptr & 0b00001111) + (_colorCode << 4);
  } else {
    *buf_ptr = (*buf_ptr & 0b11110000) + _colorCode;
  }
}
