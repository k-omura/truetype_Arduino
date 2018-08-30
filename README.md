# Display truetype font for Arduino  
Read truetype(.ttf) from SD and generate bitmap.

TrueType™ Reference Manual  
https://developer.apple.com/fonts/TrueType-Reference-Manual/  

Works on ESP32 only.  

# Originality  
## truetype  
- Separate programs in terms of the "generating a font bitmap" and the "displaying on the screen"  
- Fixed problem that some font files could not be read  
- Handling Bezier curves  
- Changes for expansion to other than ESP32  

I divided the reading of the font file and the display on the screen. Which allows expansion to various displays.  
In the original, an error occurred except for the font file verified by the author, so we made a correction.  
Also, in the original, it corresponds to the second-order Bezier curve, and an error occurs depending on the font file. Therefore, this code incorporates a provisional solution to higher-order Bezier curve.  

## display  
- Displays without depending on libraries such as Adafruit  
- Outline color and the fill color can be set individually  

# Future work (Issues)  
## truetype  
- Correction that some files can not be read  
- Faster display of fonts  
- Decrease usage of SRAM(Only works with esp32)  
- Handling of Bezier curve(Currently, provisional processing)  

## display  
- Support for more various displays  
- Faster display of fonts (especially complex characters such as kanji).  
- Align text to the right.  
- SSD1306: Text indication is not independent.  
  - Because it writes directly to the display, it will erase the display nearby due to its structure. It can be solved by passing through a buffer.  
- SSD1306: Noises rarely occur at the bottom of letters.  

# Supported display  
- ILI9341 (SPI)  
- SSD1331 (SPI)  
- SSD1306 (SPI)  

# Demo  
- [ILI9341](https://youtu.be/_-4tfssNTYE "ILI9341")  
- [SSD1331](https://youtu.be/wlubShLcMqE "SSD1331")    
- [SSD1306](https://youtu.be/WLiS6KDrS6Q "SSD1306")  

# Note  
Confirm the copyright of the font file. I did not distribute font files.  

It is based on the code by garretlab and changed.  
https://github.com/garretlab/truetype  
https://garretlab.web.fc2.com/arduino/lab/esp32_truetype  
