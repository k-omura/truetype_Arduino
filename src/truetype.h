/*
  Read truetype(.ttf) from SD and generate bitmap.

  TrueType™ Reference Manual
  https://developer.apple.com/fonts/TrueType-Reference-Manual/
*/

#define TRUETYPE_H

#if !defined __SD_H__
#include "SD.h"
#endif /*__SD_H__*/

#if !defined _SPI_H_INCLUDED
#include "SPI.h"
#endif /*_SPI_H_INCLUDED*/

#if defined ESP32
#include "FS.h"
#endif /*FS_H*/

#define FLAG_ONCURVE (1 << 0)
#define FLAG_XSHORT (1 << 1)
#define FLAG_YSHORT (1 << 2)
#define FLAG_REPEAT (1 << 3)
#define FLAG_XSAME (1 << 4)
#define FLAG_YSAME (1 << 5)

typedef struct {
  char name[5];
  uint32_t checkSum;
  uint32_t offset;
  uint32_t length;
} ttTable_t;

typedef struct {
  uint32_t version;
  uint32_t revision;
  uint32_t checkSumAdjustment;
  uint32_t magicNumber;
  uint16_t flags;
  uint16_t unitsPerEm;
  char     created[8];
  char     modified[8];
  int16_t  xMin;
  int16_t  yMin;
  int16_t  xMax;
  int16_t  yMax;
  uint16_t macStyle;
  uint16_t lowestRecPPEM;
  int16_t fontDirectionHint;
  int16_t indexToLocFormat;
  int16_t glyphDataFormat;
} ttHeadttTable_t;

typedef struct {
  uint8_t flag;
  int16_t x;
  int16_t y;
} ttPoint_t;

typedef struct {
  int16_t numberOfContours;
  int16_t xMin;
  int16_t yMin;
  int16_t xMax;
  int16_t yMax;
  uint16_t *endPtsOfContours;
  uint16_t numberOfPoints;
  ttPoint_t *points;
} ttGlyph_t;

typedef struct {
  uint16_t version;
  uint16_t numberSubtables;
} ttCmapIndex_t;

typedef struct {
  uint16_t platformId;
  uint16_t platformSpecificId;
  uint16_t offset;
} ttCmapEncoding_t;

typedef struct {
  uint16_t format;
  uint16_t length;
  uint16_t language;
  uint16_t segCountX2;
  uint16_t searchRange;
  uint16_t entrySelector;
  uint16_t rangeShift;
  uint32_t offset;
  uint32_t endCodeOffset;
  uint32_t startCodeOffset;
  uint32_t idDeltaOffset;
  uint32_t idRangeOffsetOffset;
  uint32_t glyphIndexArrayOffset;
} ttCmapFormat4_t;

typedef struct {
  int x;
  int y;
} ttCoordinate_t;

class truetypeClass {
  public:
    #if defined ESP32
    truetypeClass(SDFS *sd);
    #else
    truetypeClass(SDClass *sd);
    #endif

    int xMin, xMax, yMin, yMax;

    int begin(int cs, const char * path, int checkCheckSum = 0);
    void end();
    int readGlyph(uint16_t code);
    void adjustGlyph();
    void freeGlyph();

    //generate Bitmap
    ttGlyph_t glyph;
    uint8_t *bitmap;
    uint8_t generateBitmap(uint16_t height);
    void freeBitmap();
    int getPixel(int x0, int y0, int width);
    bool isInside(int x, int y);

  private:
    #if defined ESP32
    static SDFS *sd;
    #else
    static SDClass *sd;
    #endif

    const int numTablesPos = 4;
    const int tablePos = 12;

    uint16_t numTables;
    ttTable_t *table;
    File file;
    ttHeadttTable_t headTable;
    ttCmapIndex_t cmapIndex;
    ttCmapEncoding_t *cmapEncoding;
    ttCmapFormat4_t cmapFormat4;

    uint8_t getUInt8t();
    int16_t getInt16t();
    uint16_t getUInt16t();
    uint32_t getUInt32t();
    uint32_t calculateCheckSum(uint32_t offset, uint32_t length);
    uint32_t seekToTable(const char *name);
    int readTableDirectory(int checkCheckSum);
    void readHeadTable();
    uint32_t getGlyphOffset(uint16_t index);
    int readCmapFormat4();
    int readCmap();
    void readCoords(char xy);
    void insertGlyph(int contour, int position, int16_t x, int16_t y, uint8_t flag);
    uint16_t codeToGlyphId(uint16_t code);
    int readSimpleGlyph();

    //generate Bitmap
    ttCoordinate_t *points;
    int numPoints;
    int *beginPoints;
    int numBeginPoints;
    int *endPoints;
    int numEndPoints;

    void addPixel(int _x0, int _y0, int _width, int _height);
    void addLine(int _x0, int _y0, int _x1, int _y1, int _width, int _height);
    void addPoint(int _x, int _y);
    void freePoints();
    void addBeginPoint(int _bp);
    void freeBeginPoints();
    void addEndPoint(int _ep);
    void freeEndPoints();
    int isLeft(ttCoordinate_t &_p0, ttCoordinate_t &_p1, ttCoordinate_t &_point);
};
