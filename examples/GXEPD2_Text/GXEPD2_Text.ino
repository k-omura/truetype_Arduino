// 修复原版ascii符号会导致系统崩溃的问题
// 修复原版内存泄漏问题
// 添加与GXEPD2联动的例程，适用于墨水屏和esp8266
// 目前已知BUG，部分字体无法正常显示，部分字体会导致系统崩溃
// 目前测试成功的中文字体有造字工房系列、方正兰亭细黑、苹方黑体等

#include <SPI.h>
#include <LittleFS.h> // 官方要求的新文件系统库  #include "FS.h"未来将不会得官方支持，已弃用
#include <SD.h>
#define SPI_SPEED SD_SCK_MHZ(20) // SD卡频率
#define SD_CS     5              // SD卡片选开关
#define SD_POW    10             // SD卡电源开关 高电平打开低电平关闭

#include "SdFat.h"
SdFat32 sd;
cid_t m_cid;
csd_t m_csd;
uint32_t m_eraseSize;
uint32_t m_ocr;
//#define SD_CONFIG SdSpiConfig(SD_CS, SHARED_SPI, SPI_SPEED)  //DEDICATED_SPI(专用) SHARED_SPI（共享）
SPISettings spi_settings(10000000, MSBFIRST, SPI_MODE0);


#define ENABLE_GxEPD2_GFX 1
#include <GxEPD2_BW.h>
//#include <GxEPD2_3C.h>
//#include <GxEPD2_7C.h>
#include <U8g2_for_Adafruit_GFX.h>

//GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS*/ 15, /*DC*/ 0, /*RST*/ 2, /*BUSY*/ 4));                         // GDEM029A01
GxEPD2_BW<GxEPD2_154, GxEPD2_154::HEIGHT> display(GxEPD2_154(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); // GDEP015OC1 200x200, IL3829, no longer available
//GxEPD2_BW<GxEPD2_420_M01, GxEPD2_420_M01::HEIGHT> display(GxEPD2_420_M01(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); // GDEW042M01
//GxEPD2_BW<GxEPD2_420_Z96, GxEPD2_420_Z96::HEIGHT> display(GxEPD2_420_Z96 (/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); // GDEH042Z96

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
#define baise  GxEPD_WHITE  //白色
#define heise  GxEPD_BLACK  //黑色

#include "truetype_Arduino.h"
//#define MY_TTF "方正中等线_GBK.ttf"       //简繁都有 但太粗
//#define MY_TTF "方正黑体_GBK.ttf"         //简繁都有 但太粗
//#define MY_TTF "方正细等线_GBK.ttf"       //无繁体
//#define MY_TTF "4110_方正细黑一_GBK.ttf"  // 复杂字不行
//#define MY_TTF "文泉驿等宽正黑.ttf"        // 标点符号不行
//#define MY_TTF "造字工房典黑超细体.ttf"     // 无繁体
//#define MY_TTF "文鼎PL细上海宋.ttf"        // 复杂字不行
//#define MY_TTF "等线细.ttf"              // 较粗 渲染快
//#define MY_TTF "苹方黑体.ttf"            // 适中 渲染最快
//#define MY_TTF "华康金刚黑极细.ttf"       // 最细 渲染慢 无繁体
#define MY_TTF "方正兰亭细黑.ttf"          // 最好

//TrueType类声明
truetypeClass truetype = truetypeClass();

void setup()
{
  Serial.begin(74880);

  //屏幕初始化
  //display.init();
  display.init(0, 0, 10, 1);            // 串口使能 初始化完全刷新使能 复位时间 ret上拉使能
  u8g2Fonts.begin(display);             // 将u8g2过程连接到Adafruit GFX
  u8g2Fonts.setFontMode(1);             // 使用u8g2透明模式（这是默认设置）
  u8g2Fonts.setFontDirection(0);        // 从左到右（这是默认设置）
  u8g2Fonts.setForegroundColor(heise);  // 设置前景色
  u8g2Fonts.setBackgroundColor(baise);  // 设置背景色
  display.setRotation(1);               // 设置方向

  Serial.println("");

  /*
    if (!LittleFS.begin()) {
    Serial.println("LittleFS挂载失败");
    }
    else Serial.println("LittleFS挂载成功");*/

  //SD卡挂载检查
  if (!sdBeginCheck()) ESP.deepSleep(0);

  clearScreen();

  display.init(0, 0, 10, 1); // 串口使能 初始化完全刷新使能 复位时间 ret上拉使能
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  uint16_t ttf_y = 0;

  for (uint16_t i = 24; i <= 30; i++)
  {
    uint32_t time1 = millis();
    String cs[7] = {"你，好 好",
                    "He，ll o",
                    "よし，よ し",
                    "你，好",
                    "He，llo",
                    "よし，よし",
                    "你，好",
                   };
    GXEPD2_truetype(0, ttf_y + i, String(i) + cs[i - 24], i);
    ttf_y = ttf_y + i + 0;
    Serial.println(millis() - time1);
  }

  /*String cs[2] = {"二愣子睁大着双眼，直直望着茅草和烂泥糊成的黑屋顶，身上盖着的旧棉被，已呈",
                  "深黄色，看不出原来的本来面目，还若有若无的散发着淡淡的霉味123456。"
                 };
    uint32_t time1 = millis();
    GXEPD2_truetype(0, 25, cs[0], 25);
    GXEPD2_truetype(0, 125, cs[1], 25);
    Serial.println(millis() - time1);
    Serial.print("中文大小："); Serial.println(getTTFWidth(25, "甘", MY_TTF));*/
  display.nextPage();
}

void loop()
{
  esp_sleep(0);
}

void GXEPD2_truetype(uint16_t x, uint16_t y, String zf, uint8_t fontSize)
{
  //***** 检查有几个字符
  uint16_t zf_count;
  uint16_t zf_length = zf.length();
  for (uint16_t i = 0; i < zf_length; i++)
  {
    if ((zf[i]& B11100000) == B11100000) {
      zf_count++;
      i += 2;
    }
    else if ((zf[i]& B11000000) == B11000000) {
      zf_count++;
      i += 1;
    }
    else zf_count++;
  }

  //Serial.println(" ");
  //Serial.print("显示字符："); Serial.println(zf);
  //Serial.print("字符数量："); Serial.println(zf_count);
  //Serial.print("字号大小："); Serial.println(fontSize);

  //***** ttf文件位置
  //File fontFile = LittleFS.open(MY_TTF, "r");
  File fontFile = SDFS.open(MY_TTF, "r");
  //Serial.print("字体名称："); Serial.println(fontFile.name());
  //Serial.print("字体大小："); Serial.println(fontFile.size());

  //在TrueType类中设置帧缓冲区数组
  //请注意帧缓冲区的格式
  //ttf文件的初始读取
  if (!truetype.setTtfFile(fontFile)) Serial.println("读取ttf失败");
  else
  {
    // 字体大小、边界范围等参数
    truetype.setCharacterSize(fontSize); //设置字号大小
    truetype.setCharacterSpacing(0);     //设置间距
    truetype.setTextColor(0x01, 0x01); //设置文本颜色
    truetype.setTextBoundary(0, display.width(), 1000); //设置字符边界
    uint8_t BITS_PER_PIXEL = 1; // 任何一个 1, 4, or 8 //每像素位
    //uint16_t DISPLAY_WIDTH = zf_count * fontSize; // 显示宽度 快速
    uint16_t DISPLAY_WIDTH = truetype.getStringWidth(zf); // 显示宽度 慢速
    uint16_t DISPLAY_HEIGHT = fontSize; // 显示高度 高度像素
    
    //display.width(), display.height()
    //Serial.print("getStringWidth:"); Serial.println(truetype.getStringWidth(zf));
    //Serial.print("DISPLAY_WIDTH:"); Serial.println(DISPLAY_WIDTH);
    //Serial.print("DISPLAY_HEIGHT:"); Serial.println(DISPLAY_HEIGHT);

    //***** 准备一个帧缓冲区
    uint32_t FRAMEBUFFER_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT; //帧缓冲区大小
    uint8_t *framebuffer;
    framebuffer = (uint8_t *)calloc(sizeof(uint8_t), 20000);

    //Serial.print("可用堆大小1："); Serial.println(ESP.getFreeHeap());
    if (!framebuffer)
    {
      Serial.println("分配内存失败");
      truetype.end();
      return;
    }
    //Serial.print("分配内存成功："); Serial.println(FRAMEBUFFER_SIZE);
    truetype.setFramebuffer(DISPLAY_WIDTH, DISPLAY_HEIGHT, BITS_PER_PIXEL, 0, framebuffer);

    //将字符串写入帧缓冲区
    truetype.textDraw(0, 0, zf);

    //显示至屏幕
    display.drawBitmap(x, y - fontSize, framebuffer, DISPLAY_WIDTH, DISPLAY_HEIGHT, heise);
    truetype.end();
    
    //一定要释放内存
    delete framebuffer;
    framebuffer = NULL;
    
    //Serial.print("可用堆大小2："); Serial.println(ESP.getFreeHeap());
  }
}

//SD卡挂载失败会导致软看门狗重启
boolean sdBeginCheck() //SD挂载检查
{
  //#define SdSpiConfig(SD_CS, SHARED_SPI, sdFrequency) //DEDICATED_SPI(专用) SHARED_SPI（共享）

  pinMode(SD_POW, OUTPUT);
  digitalWrite(SD_POW, 1);
  delay(1);

  SDFS.setConfig(SDFSConfig(SD_CS, SPI_SPEED));

  SDFS.end(); //SDFS.end();
  ESP.wdtDisable(); //ESP.wdtEnable(8000);
  //yield();

  if (sd.cardBegin(SdSpiConfig(SD_CS, SHARED_SPI, SPI_SPEED))) //DEDICATED_SPI(专用) SHARED_SPI（共享）
  {
    SDFS.begin();
    //sd.cardBegin(SD_CONFIG);
    Serial.println("SD OK");
    ESP.wdtEnable(8000);
    return 1;
  }
  else
  {
    Serial.println("SD fail");
    ESP.wdtEnable(8000);
    return 0;
  }
  return 0;
}

uint16_t getTTFWidth(uint8_t fontSize, String zf, String myttf)
{
  File fontFile = SDFS.open(myttf, "r");
  //Serial.print("字体名称："); Serial.println(fontFile.name());
  uint16_t zf_width;
  //ttf文件的初始读取
  if (!truetype.setTtfFile(fontFile)) return 0; //读取ttf文件失败
  else
  {
    // 字体大小、边界范围等参数
    truetype.setCharacterSize(fontSize); //设置字号大小
    truetype.setCharacterSpacing(0);     //设置间距
    return truetype.getStringWidth(zf);
  }
  return 0;
}
