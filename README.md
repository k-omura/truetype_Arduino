# Display truetype font for Arduino  
Read truetype(.ttf) from FS(ex. SD/SPIFFS) and generate bitmap. And draw the generated bitmap on the display.  

TrueType™ Reference Manual  
https://developer.apple.com/fonts/TrueType-Reference-Manual/  


# Originality  
## truetype  
- Separate programs in terms of the "generating a font bitmap" and the "displaying on the screen".  
- Fixed problem that some font files could not be read.  
- Handling Bezier curves.  
- Faster filling process.  

I divided the reading of the font file and the display on the screen. Which allows expansion to various displays.  
In the original, an error occurred except for the font file verified by the author, so we made a correction.  
Also, in the original, it corresponds to the second-order Bezier curve, and an error occurs depending on the font file. Therefore, this code incorporates a provisional solution to higher-order Bezier curve.  

## display  
- Displays without depending on libraries such as Adafruit.  
- Outline color and the fill color can be set individually.  
- Supports writing to arrays. Fonts can be drawn in monochrome for the uint8_t array.  

# Future work (Issues)  
## truetype  
- Correction that some files can not be read.  
- Unable to read ttf file if file name is long(STM32F103).  
- Faster glyph reading and bitmap generation.  
- Decrease usage of SRAM.  
- Handling of Bezier curve(When exceeding 3 dimensions. Currently, provisional processing).  

## display  
- Support for more various displays.  
- Faster display of fonts (SPI optimization is required).  
- Make underline available.  
- Align text to the right.  
- Mono display: Text indication is not independent.  
  - Because it writes directly to the display, it will erase the display nearby due to its structure. It can be solved by passing through a buffer.  
- Mono display: Support Landscape only  

# Confirmed controller  
- ESP32([Board](https://github.com/espressif/arduino-esp32))  
- STM32F103C8T6 (SPI optimized) ([Board](https://github.com/stm32duino/Arduino_Core_STM32))  

# Supported display  
- Full color
  - ILI9341 (SPI) [Demo](https://youtu.be/_-4tfssNTYE "ILI9341")    
  - SSD1331 (SPI) [Demo](https://youtu.be/wlubShLcMqE "SSD1331")  
  - (in progress)ST7735 (SPI)  
- Mono
  - SSD1306 (SPI) [Demo](https://youtu.be/WLiS6KDrS6Q "SSD1306")  
  - Waveshare e-Paper 2.9inch (SPI) [Demo](https://youtu.be/qs_nOYCx91o "e-Paper")  

See also the ILI9341 ESP32's exclusive code for any display.  
The font file is read in the same way as ILI9341, but the example code on other displays has not been updated yet.  
It is common to use the SPI for loading the SD card. For that reason, the work of the I2C display is not prioritized.  

# Note  
Feel free to post any bugs or ideas for fixes and improvements!  
Confirm the copyright of the font file. I did not distribute font files.  

It is based on the code by garretlab and changed.  
https://github.com/garretlab/truetype  
https://garretlab.web.fc2.com/arduino/lab/truetype/  
