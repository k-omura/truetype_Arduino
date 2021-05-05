#include "FS.h"
#include "SPIFFS.h"
#include "truetype_Arduino.h"
/*
   this example simply creates a bitmap with your text written on it using the ttf file you supply
   it the prints the output to Serial

   dont forget to
   create a folder in your sketch folder called data
   add the ttf to a folder called data in your sketch folder and call it by a short name SPIFFS has name length limitations
   then upload it to SPIFFS

*/

// just to be clear
#define WIDTH_BYTES 50
#define HEIGHT_PIXELS 40
#define DISPLAY_HEIGHT HEIGHT_PIXELS
#define FRAMEBUFFER_SIZE (WIDTH_BYTES * HEIGHT_PIXELS)
#define BITS_PER_PIXEL 1 // either 1, 4, or 8

#define DISPLAY_WIDTH (WIDTH_BYTES * (8 / BITS_PER_PIXEL))
#define MY_TTF "/FONTFILE.ttf"

//TrueType class declaration
truetypeClass truetype = truetypeClass();

void print_bitmap(uint8_t *framebuffer, uint16_t width_in_bytes, uint16_t height_in_pixels) {
  for (int i = 0; i < (width_in_bytes * height_in_pixels); i++) {
    if ((i % width_in_bytes) == 0)
      Serial.println();
    for (uint8_t bits = 8; bits > 0; bits--) {
      if (_BV(bits - 1) & framebuffer[i])
        Serial.print(" ");
      else
        Serial.print("*");
    }
    // Serial.print(" "); // uncomment to show individual bytes
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  //Prepare a frame buffer
  uint8_t *framebuffer;
  framebuffer = (uint8_t *)calloc(sizeof(uint8_t), FRAMEBUFFER_SIZE);
  if (!framebuffer) {
    Serial.println("alloc memory failed !!!");
    while (1);
  }

  Serial.println(FRAMEBUFFER_SIZE);
  //Read TrueType file
  //Example in SPIFFS
  //I think that SD, FATFS and other codes will be almost the same
  SPIFFS.begin(true);
  File fontFile = SPIFFS.open(MY_TTF, "r");
  Serial.println(fontFile.name());
  Serial.println(fontFile.size());

  //Set framebuffer array in TrueType class
  //Pay attention to the format of the framebuffer
  truetype.setFramebuffer(DISPLAY_WIDTH, BITS_PER_PIXEL, framebuffer);

  //Initial reading of ttf files
  if (!truetype.setTtfFile(fontFile)) {
    Serial.println("read ttf failed");
  } else {
    //TrueType class string parameter settings
    truetype.setCharacterSize(20);
    truetype.setCharacterSpacing(0);
    truetype.setStringWidth(10, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    truetype.setStringColor(0x01, 0x01);

    //Write a string to the framebuffer
    truetype.string(10, 10, L"Hello");
    //truetype.string(10, 10, L"L'année 1866 fut marquée par un événement bizarre,");
    //Export framebuffer to screen
    //FLASH_TO_SCREEN();
    print_bitmap(framebuffer, WIDTH_BYTES, HEIGHT_PIXELS);

    //end
    truetype.end();
  }
}

void loop() {
}
