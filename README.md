# Display truetype font for Arduino  
Read truetype(.ttf) from SD and generate bitmap.

TrueType™ Reference Manual  
https://developer.apple.com/fonts/TrueType-Reference-Manual/  

Works on ESP32 only.  

# Originality  
- Separate programs in terms of the "generating a font bitmap" and the "displaying on the screen"  
- Fixed problem that some font files could not be read  
- Handling Bezier curves  
- Changes for expansion to other than ESP32  

I divided the reading of the font file and the display on the screen. Which allows expansion to various displays.  
In the original, an error occurred except for the font file verified by the author, so we made a correction.  
Also, in the original, it corresponds to the second-order Bezier curve, and an error occurs depending on the font file. Therefore, this code incorporates a provisional solution to higher-order Bezier curve.  

# Future work
- Correction that some files can not be read  
- Faster display of fonts  
- Decrease usage of SRAM(Only works with esp32)  
- Support for more various displays  

# Display  
- ILI9341 https://github.com/k-omura/ILI9341_ttf  
- SSD1331 https://github.com/k-omura/SSD1331_ttf  

# Note  
It is based on the code by garretlab and changed.  
https://github.com/garretlab/truetype  
https://garretlab.web.fc2.com/arduino/lab/esp32_truetype  
