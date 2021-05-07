#include "FS.h"
#include "SPIFFS.h"
#include "truetype_Arduino.h"
/*
 * This example simply creates a bitmap 
 * with your text written on it 
 * using the ttf file you supply
 * This is a good routine to test fonts
 * for compatiblity.
 * It is easier to see if you use 1 bit per pixel 
 * however it works just the same for 4 and 8
 * 
 * It rotates the text in all four directions 
 * 
 * Then the prints the buffer contents to Serial
 * 
 * dont forget to upload your ttf to SPIFFS
 * Warning some fonts are not supported!
 * 
 * SPIFFS upload
   * create a sub folder in your sketch folder called data
   * add the ttf to that folder 
   * for this example rename it to FONTFILE.ttf
   * nb. SPIFFS has name length limitations
   * upload it to SPIFFS 
   * upload will not work while Serial Terminal is connected

*/

// just to be clear
#define WIDTH_PIXELS 80
#define HEIGHT_PIXELS 80
#define DISPLAY_HEIGHT HEIGHT_PIXELS
#define DISPLAY_WIDTH WIDTH_PIXELS
#define BITS_PER_PIXEL 8 // either 1, 4, or 8

#define WIDTH_BYTES WIDTH_PIXELS / (8 / BITS_PER_PIXEL)
#define FRAMEBUFFER_SIZE ( WIDTH_BYTES * HEIGHT_PIXELS)

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
        Serial.print("#");
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
  truetype.setFramebuffer(DISPLAY_WIDTH, DISPLAY_HEIGHT, BITS_PER_PIXEL, ROTATE_0, framebuffer);
  //Initial reading of ttf files
  if (!truetype.setTtfFile(fontFile)) {
    Serial.println("read ttf failed");
  } else {
    //TrueType class string parameter settings
    truetype.setCharacterSize(40);
    truetype.setCharacterSpacing(0);
    truetype.setTextBoundary(10, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    truetype.setTextColor(0xFF, 0xFF); // in 1 bit only the 1st bit is used 
    //Write a string to the framebuffer
    String test = "0";
    truetype.textDraw(10, 0, test);
    test = "90";
    truetype.setTextRotation(ROTATE_90);
    truetype.textDraw(10, 0, test);
    test = "180";
    truetype.setTextRotation(ROTATE_180);
    truetype.textDraw(10, 0, test);
    test = "270";
    truetype.setTextRotation(ROTATE_270);
    truetype.textDraw(10, 0, test);
    //Export framebuffer to screen
    print_bitmap(framebuffer, WIDTH_BYTES, HEIGHT_PIXELS);
    //end
    truetype.end();
  }
}

void loop() {
}
