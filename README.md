# Display truetype font for Arduino  
Read truetype(.ttf) from FS(ex. SD/SPIFFS/FATFS) and write framebuffer.  

- Read TrueType files ('cmap' format4).  
- Write any string to the user's framebuffer.  
- Set font size, position, color, and spacing.  
- Read the 'kern' table (format0) and kerning.  
- Beautifully arrange characters based on the 'hmtx' table.  
<img src="https://user-images.githubusercontent.com/26690530/116971484-99f68600-acf4-11eb-99c6-d5e29791b53f.JPG" width="500">

TrueType™ Reference Manual  
https://developer.apple.com/fonts/TrueType-Reference-Manual/  

# Standard code  
```
//Prepare a frame buffer
uint8_t *framebuffer; 
framebuffer = (uint8_t *)calloc(sizeof(uint8_t), FRAMEBUFFER_SIZE);

//Read TrueType file
//Example in SPIFFS
//I think that SD, FATFS and other codes will be almost the same
SPIFFS.begin(true);
File fontFile = SPIFFS.open("/FONTFILE.ttf", "r");

//TrueType class declaration
truetypeClass truetype = truetypeClass();

//Set framebuffer array in TrueType class
//Pay attention to the format of the framebuffer
truetype.setFramebuffer(DISPLAY_WIDTH, 4, framebuffer);

//Initial reading of ttf files
if (!truetype.setTtfFile(fontFile)) {
  Serial.println("read ttf failed");
  return;
}

//TrueType class string parameter settings
truetype.setCharacterSize(100);
truetype.setCharacterSpacing(0);
truetype.setStringWidth(10, DISPLAY_WIDTH, DISPLAY_HEIGHT);
truetype.setStringColor(0x00, 0x00);

//Write a string to the framebuffer
truetype.string(10, 10, L"epaperNotify");

//Export framebuffer to screen
FLASH_TO_SCREEN();

//end
truetype.end();
```

# API  
- uint8_t setTtfFile(File _file, uint8_t _checkCheckSum = 0);  
  - Set the ttf file read from SD, SPIFFS, FATFS, etc. 
  - File _file : ttf file.  
  - Return : 1 = read successful, 0 = read failure.  

- void setFramebuffer(uint16_t _framebufferWidth, uint16_t _framebuffer_bit, uint8_t *_framebuffer);  
  - Framebuffer settings.
  - uint16_t _framebufferWidth : Framebuffer width.  
  - uint16_t _framebuffer_bit : The number of bits per pixel. (1,4,8bit implemented)
  - uint8_t *_framebuffer : Framebuffer pointer.  
  - If you want it to correspond to your own framebuffer, edit the addPixel function. If you add code, please share it!

- void setCharacterSpacing(int16_t _characterSpace, uint8_t _kerning = 1);  
  - Setting the width between characters.  
  - int16_t _characterSpace : Width value between characters.
  - uint8_t _kerning : Read and use ttf 'kern' table. 1:'kern' + _characterSpace. 0: _characterSpace.

- void setCharacterSize(uint16_t _characterSize);  
  - Font size setting.  
  - uint16_t _characterSize : Character height.  

- void setStringWidth(uint16_t _start_x, uint16_t _end_x, uint16_t _end_y);
  - Setting the string range.  
  - uint16_t _start_x : The starting point x of the character string when a line break occurs.  
  - uint16_t _end_x : The final point x when breaking a line.  
  - uint16_t _end_y : The final point y when breaking a line.  

- void setStringColor(uint8_t _onLine, uint8_t _inside);  
  - Text color setting.  
  - uint8_t _onLine : Character outline color.  
  - uint8_t _inside : Text fill color.  

- void string(uint16_t _x, uint16_t _y, const wchar_t _character[]);  
  - Write a string to the framebuffer.  
  - uint16_t _x : String start point x.  
  - uint16_t _y : String start point y.  
  - const wchar_t _character[] : String pointer (double-byte character).  

- void string(uint16_t _x, uint16_t _y, const char _character[]);  
  - Write a string to the framebuffer.  
  - uint16_t _x : String start point x.  
  - uint16_t _y : String start point y.  
  - const char _character[] : String pointer (single-byte character).  
  - Under construction  

- void string(uint16_t _x, uint16_t _y, const String _string);  
  - Write a string to the framebuffer.  
  - uint16_t _x : String start point x.  
  - uint16_t _y : String start point y.  
  - const String _string : String pointer (String type).  

- void end();  
  - Close font file.  

# Originality  
- Fixed problem that some font files could not be read.  
- Handling Bezier curves.  
  - It supports up to 3rd order Bezier curves, but 4th order and above are drawn as straight lines.  
- Outline color and the fill color can be set individually.  
- Supports writing to arrays. Fonts are drawn in some form of framebuffer(uint8_t array).  
- Kerning by reading the 'kern' table.  
- Read 'hmtx' table and adjust layout  

# Future work (Issues)  
- Diversification of supported framebuffer formats.  
- Only support for 'cmap' format 4 and  'kern' format0 is supported.  
- Correction that some files can not be read.  
- Unable to read ttf file if file name is long(STM32F103).  
- Faster glyph reading and write framebuffer.  
- Decrease usage of SRAM.  
- Handling of Bezier curve(When exceeding 3 dimensions. Currently, provisional processing).  
- Make underline available.  
- Align text to the right.  

# Confirmed controller  
- ESP32([Board](https://github.com/espressif/arduino-esp32))  
- STM32F103C8T6 (SPI optimized) ([Board](https://github.com/stm32duino/Arduino_Core_STM32))  

# Demo  
- Full color
  - ILI9341 (SPI) [Demo](https://youtu.be/_-4tfssNTYE "ILI9341")    
  - SSD1331 (SPI) [Demo](https://youtu.be/wlubShLcMqE "SSD1331")  
  - (in progress)ST7735 (SPI)  
- Mono
  - SSD1306 (SPI) [Demo](https://youtu.be/WLiS6KDrS6Q "SSD1306")  
  - Waveshare e-Paper 2.9inch (SPI) [Demo](https://youtu.be/qs_nOYCx91o "e-Paper")  
  - Waveshare e-Paper 7.5inch(B) (Via bitmap array) [Demo](https://youtu.be/n9_DJ3ugalQ "e-Paper")  

# Note  
Feel free to post any bugs or ideas for fixes and improvements!  
Confirm the copyright of the font file. I did not distribute font files.  

It is based on the code by garretlab and changed.  
https://github.com/garretlab/truetype  
https://garretlab.web.fc2.com/arduino/lab/truetype/  
