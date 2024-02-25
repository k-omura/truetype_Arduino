
<img src="https://github.com/jie326513988/truetype_Arduino/blob/master/1.jpeg" width="500">
[观看视频](www.bilibili.com/video/BV1dx421Z7rG)

### 2024-02-25
* 修复修复大部分字体标点符号异常，字符显示空白和位置异常的BUG，
  * fillGlyph()传入负值会导致系统崩溃
  * 以空格字符的2倍advanceWidth作为advanceWidthMAX值（从hmet表获取的advanceWidth异常时）
  * advanceWidth值异常时使用advanceWidthMAX值
  * 对leftSideBearing和glyph.xMin进行判断，以便能以正常的数值进行后续计算（从hmet表获取的leftSideBearing异常时）
* 修复检测到换行符无法换行的BUG
* 添加一个获取字符宽度的函数，就是在textDraw()中获取_x值，可以少计算一次宽度。
<img src="https://github.com/jie326513988/truetype_Arduino/blob/master/20240225-1.jpg" width="500">

#### 旧的消息
* ~修复原版ascii符号会导致系统崩溃的问题~
* 修复原版内存泄漏问题
* 添加与GXEPD2联动的例程，适用于库支持的墨水屏和esp8266&esp32
* ~目前已知BUG，部分字体无法正常显示，部分字体会导致系统崩溃~
# Display truetype font for Arduino  
Read truetype(.ttf) from FS(ex. SD/SPIFFS/FATFS) and write framebuffer.  

- Read TrueType files ('cmap' format4).  
- Write any string to the user's framebuffer.  
- Supports "Simple glyphs" and part of "Compound glyphs".
- Set font size, position, color, spacing, Rotate.  
- Centered and right aligned strings. By getting the length of the string.  
- Read the 'kern' table (format0) and kerning.  
- Beautifully arrange characters based on the 'hmtx' table.  
<img src="https://user-images.githubusercontent.com/26690530/116971484-99f68600-acf4-11eb-99c6-d5e29791b53f.JPG" width="500">
https://github.com/jie326513988/truetype_Arduino/blob/master/1.jpeg
TrueType™ Reference Manual  
https://developer.apple.com/fonts/TrueType-Reference-Manual/  

The native library for STM32 is [here](https://github.com/k-omura/STM32_UIKit).  

# Standard code  
```
//TrueType class declaration
truetypeClass truetype = truetypeClass();
uint8_t *framebuffer; 

void setup() {
  //Prepare a frame buffer
  framebuffer = (uint8_t *)calloc(sizeof(uint8_t), FRAMEBUFFER_SIZE);

  //Read TrueType file
  //Example in SPIFFS
  //I think that SD, FATFS and other codes will be almost the same
  SPIFFS.begin(true);
  File fontFile = SPIFFS.open("/FONTFILE.ttf", "r");

  //Set framebuffer array in TrueType class
  //Pay attention to the format of the framebuffer
  truetype.setFramebuffer(DISPLAY_WIDTH, DISPLAY_HEIGHT, 4, 0, framebuffer);

  //Initial reading of ttf files
  if (!truetype.setTtfFile(fontFile)) {
    Serial.println("read ttf failed");
    return;
  }

  //TrueType class string parameter settings
  truetype.setCharacterSize(100);
  truetype.setCharacterSpacing(0);
  truetype.setTextBoundary(10, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  truetype.setTextColor(0x00, 0x00);

  //Write a string to the framebuffer
  truetype.textDraw(10, 10, "The quick brown fox jumps over the lazy dog");

  //Export framebuffer to screen
  FLASH_TO_SCREEN();

  //end
  truetype.end();
}
```

# API  
- uint8_t setTtfFile(File _file, uint8_t _checkCheckSum = 0);  
  - Set the ttf file read from SD, SPIFFS, FATFS, etc. 
  - File _file : ttf file.  
  - Return : 1 = read successful, 0 = read failure.  

- void setFramebuffer(uint16_t _framebufferWidth, uint16_t _framebufferHeight, uint16_t _framebuffer_bit, uint8_t _framebufferDirection, uint8_t *_framebuffer);  
  - Framebuffer settings.
  - uint16_t _framebufferWidth : Framebuffer width.  
  - uint16_t _framebufferHeight : Framebuffer eight.
  - uint16_t _framebuffer_bit : The number of bits per pixel. (1,4,8bit implemented)
  - uint8_t _framebufferDirection : Bit orientation.
    - [See Framebuffer format](#Framebuffer-format)  
  - uint8_t *_framebuffer : Framebuffer pointer.  
  - If you want it to correspond to your own framebuffer, edit the addPixel function. If you add code, please share it!

- void setCharacterSpacing(int16_t _characterSpace, uint8_t _kerning = 1);  
  - Setting the width between characters.  
  - int16_t _characterSpace : Width value between characters.
  - uint8_t _kerning : Read and use ttf 'kern' table. 1:'kern' + _characterSpace. 0: _characterSpace.

- void setCharacterSize(uint16_t _characterSize);  
  - Font size setting.  
  - uint16_t _characterSize : Character height.  

- void setTextBoundary(uint16_t _start_x, uint16_t _end_x, uint16_t _end_y);
  - Setting the string range.  
    - Coordinate axes rotate with "setTextRotation"  
  - uint16_t _start_x : The starting point x of the character string when a line break occurs.  
  - uint16_t _end_x : The final point x when breaking a line.  
  - uint16_t _end_y : The final point y when breaking a line.  

- void setTextColor(uint8_t _onLine, uint8_t _inside);  
  - Text color setting.  
  - uint8_t _onLine : Character outline color.  
  - uint8_t _inside : Text fill color.  

- void setTextRotation(uint8_t _rotation);
  - Text rotation
    - Rotate along with the coordinate axes. The image is the following image.  
<img src="https://user-images.githubusercontent.com/26690530/117451729-d983d300-af7d-11eb-9914-bfaf59b01075.png" width="400">

  - uint8_t _rotation : rotation angle.
    - (ROTATE_0/ROTATE_90/ROTATE_180/ROTATE_270)  

- void textDraw(uint16_t _x, uint16_t _y, const wchar_t _character[]);  
  - Write a string to the framebuffer.  
  - uint16_t _x : String start point x.  
  - uint16_t _y : String start point y.  
  - const wchar_t _character[] : String pointer (double-byte character).  

- void textDraw(uint16_t _x, uint16_t _y, const char _character[]);  
  - Write a string to the framebuffer.  
  - uint16_t _x : String start point x.  
  - uint16_t _y : String start point y.  
  - const char _character[] : String pointer (single-byte character).  

- void textDraw(uint16_t _x, uint16_t _y, const String _string);  
  - Write a string to the framebuffer.  
  - uint16_t _x : String start point x.  
  - uint16_t _y : String start point y.  
  - const String _string : String pointer (String type).  

- uint16_t getStringWidth(const wchar_t _character[]);
  - const wchar_t _character[] : String pointer (double-byte character).  
  - Return : The length of the string. Automatic line breaks are not considered.
    - Can be used for text align center/right.  

- uint16_t getStringWidth(const char _character[]);
  - const char _character[] : String pointer (single-byte character).  
  - Return : The length of the string. Automatic line breaks are not considered.
    - Can be used for text align center/right.  

- uint16_t getStringWidth(const String _string);
  - const String _string : String pointer (String type).  
  - Return : The length of the string. Automatic line breaks are not considered.
    - Can be used for text align center/right.  

- void end();  
  - Close font file.  

# Framebuffer format
Bit orientation when storing information for multiple pixels per byte of the framebuffer.  
The types of framebuffers are broadly divided according to this direction.  
Currently supported: Horizontal - 1,4,8bit
## Horizontal
Example with 1bit / 1pixel  
<img src="https://user-images.githubusercontent.com/26690530/117164679-cf3bca80-adff-11eb-9a5b-11b3093502dd.png" width="300">

## Vertical
Example with 1bit / 1pixel  
<img src="https://user-images.githubusercontent.com/26690530/117164690-d2cf5180-adff-11eb-8bbf-10463ef52833.png" height="300">

# Originality  
- Fixed problem that some font files could not be read.  
- Handling Bezier curves.  
  - It supports up to 3rd order Bezier curves, but 4th order and above are drawn as straight lines.  
- Outline color and the fill color can be set individually.  
- Supports writing to arrays. Fonts are drawn in some form of framebuffer(uint8_t array).  
- Kerning by reading the 'kern' table.  
- Read 'hmtx' table and adjust layout.  
- Efficient filling process.  

# Future work  
## TrueType  
- Full support for "Compound glyphs"  
- Diversification of supported framebuffer formats.  
- Only support for 'cmap' format 4 and 'kern' format0 is supported.  
- Correction that some files can not be read.  
- Unable to read ttf file if file name is long(STM32F103).  
- Faster glyph reading.  
- Decrease usage of SRAM.  
- Handling of Bezier curve(When exceeding 3 dimensions. Currently, provisional processing).  
## Draw framebuffer  
- Text from the right
- Underline  

# Confirmed controller  
- ESP32([Board](https://github.com/espressif/arduino-esp32))  
- STM32F103C8T6([Board](https://github.com/stm32duino/Arduino_Core_STM32))  

# Demo  
- Full color
  - ILI9341 [Demo](https://youtu.be/_-4tfssNTYE "ILI9341")    
  - SSD1331 [Demo](https://youtu.be/wlubShLcMqE "SSD1331")  
- Mono
  - SSD1306 [Demo](https://youtu.be/WLiS6KDrS6Q "SSD1306")  
  - Waveshare e-Paper 2.9inch [Demo](https://youtu.be/qs_nOYCx91o "e-Paper")  
  - Waveshare e-Paper 7.5inch(B) [Demo](https://youtu.be/n9_DJ3ugalQ "e-Paper")  

# Note  
Feel free to post any bugs or ideas for fixes and improvements!  
Confirm the copyright of the font file. I did not distribute font files.  

It is based on the code by garretlab and changed.  
https://github.com/garretlab/truetype  
https://garretlab.web.fc2.com/arduino/lab/truetype/  
