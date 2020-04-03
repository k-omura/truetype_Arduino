/*
  Read truetype(.ttf) from SD and generate bitmap.

  TrueType™ Reference Manual
  https://developer.apple.com/fonts/TrueType-Reference-Manual/
*/

#include "truetype_Arduino.h"

#if defined ESP32
SDFS *truetypeClass::sd;
#else
SDClass *truetypeClass::sd;
#endif

/* constructor */

#if defined ESP32
truetypeClass::truetypeClass(SDFS *sd) {
  this->sd = sd;
}
#else
truetypeClass::truetypeClass(SDClass *sd) {
  this->sd = sd;
}
#endif

/* get uint8_t at the current position */
uint8_t truetypeClass::getUInt8t() {
  uint8_t x;

  file.read(&x, 1);
  return x;
}

/* get int16_t at the current position */
int16_t truetypeClass::getInt16t() {
  byte x[2];

  file.read(x, 2);
  return (x[0] << 8) | x[1];
}

/* get uint16_t at the current position */
uint16_t truetypeClass::getUInt16t() {
  byte x[2];

  file.read(x, 2);
  return (x[0] << 8) | x[1];
}

/* get uint32_t at the current position */
uint32_t truetypeClass::getUInt32t() {
  byte x[4];

  file.read(x, 4);
  return (x[0] << 24) | (x[1] << 16) | (x[2] << 8) | x[3];
}

/* calculate checksum */
uint32_t truetypeClass::calculateCheckSum(uint32_t offset, uint32_t length) {
  uint32_t checksum = 0L;

  length = (length + 3) / 4;
  file.seek(offset);

  while (length-- > 0) {
    checksum += getUInt32t();
  }
  return checksum;
}

/* seek to the first position of the specified table name */
uint32_t truetypeClass::seekToTable(const char *name) {
  for (int i = 0; i < numTables; i++) {
    if (strcmp(table[i].name, name) == 0) {
      file.seek(table[i].offset);
      return table[i].offset;
    }
  }
  return 0;
}

/* read table directory */
int truetypeClass::readTableDirectory(int checkCheckSum) {
  file.seek(numTablesPos);
  numTables = getUInt16t();
  table = (ttTable_t *)malloc(sizeof(ttTable_t) * numTables);

  file.seek(tablePos);
  for (int i = 0; i < numTables; i++) {
    for (int j = 0; j < 4; j++) {
      table[i].name[j] = getUInt8t();
    }
    table[i].name[4] = '\0';
    table[i].checkSum = getUInt32t();
    table[i].offset = getUInt32t();
    table[i].length = getUInt32t();
  }

  if (checkCheckSum) {
    for (int i = 0; i < numTables; i++) {
      if (strcmp(table[i].name, "head") != 0) { /* checksum of "head" is invalid */
        uint32_t c = calculateCheckSum(table[i].offset, table[i].length);
        if (table[i].checkSum != c) {
          return 0;
        }
      }
    }
  }
  return 1;
}

/* read head table */
void truetypeClass::readHeadTable() {
  for (int i = 0; i < numTables; i++) {
    if (strcmp(table[i].name, "head") == 0) {
      file.seek(table[i].offset);

      headTable.version = getUInt32t();
      headTable.revision = getUInt32t();
      headTable.checkSumAdjustment = getUInt32t();
      headTable.magicNumber = getUInt32t();
      headTable.flags = getUInt16t();
      headTable.unitsPerEm = getUInt16t();
      for (int j = 0; j < 8; j++) {
        headTable.created[i] = getUInt8t();
      }
      for (int j = 0; j < 8; j++) {
        headTable.modified[i] = getUInt8t();
      }
      xMin = headTable.xMin = getInt16t();
      yMin = headTable.yMin = getInt16t();
      xMax = headTable.xMax = getInt16t();
      yMax = headTable.yMax = getInt16t();
      headTable.macStyle = getUInt16t();
      headTable.lowestRecPPEM = getUInt16t();
      headTable.fontDirectionHint = getInt16t();
      headTable.indexToLocFormat = getInt16t();
      headTable.glyphDataFormat = getInt16t();
    }
  }
}

/* get glyph offset */
uint32_t truetypeClass::getGlyphOffset(uint16_t index) {
  uint32_t offset = 0;

  for (int i = 0; i < numTables; i++) {
    if (strcmp(table[i].name, "loca") == 0) {
      if (headTable.indexToLocFormat == 1) {
        file.seek(table[i].offset + index * 4);
        offset = getUInt32t();
      } else {
        file.seek(table[i].offset + index * 2);
        offset = getUInt16t() * 2;
      }
      break;
    }
  }

  for (int i = 0; i < numTables; i++) {
    if (strcmp(table[i].name, "glyf") == 0) {
      return (offset + table[i].offset);
    }
  }

  return 0;
}

/* read cmap format 4 */
int truetypeClass::readCmapFormat4() {
  file.seek(cmapFormat4.offset);
  if ((cmapFormat4.format = getUInt16t()) != 4) {
    return 0;
  }

  cmapFormat4.length = getUInt16t();
  cmapFormat4.language = getUInt16t();
  cmapFormat4.segCountX2 = getUInt16t();
  cmapFormat4.searchRange = getUInt16t();
  cmapFormat4.entrySelector = getUInt16t();
  cmapFormat4.rangeShift = getUInt16t();
  cmapFormat4.endCodeOffset = cmapFormat4.offset + 14;
  cmapFormat4.startCodeOffset = cmapFormat4.endCodeOffset + cmapFormat4.segCountX2 + 2;
  cmapFormat4.idDeltaOffset = cmapFormat4.startCodeOffset + cmapFormat4.segCountX2;
  cmapFormat4.idRangeOffsetOffset = cmapFormat4.idDeltaOffset + cmapFormat4.segCountX2;
  cmapFormat4.glyphIndexArrayOffset = cmapFormat4.idRangeOffsetOffset + cmapFormat4.segCountX2;

  return 1;
}

/* read cmap */
int truetypeClass::readCmap() {
  uint16_t platformId, platformSpecificId;
  uint32_t cmapOffset, tableOffset;
  int foundMap = 0;

  if ((cmapOffset = seekToTable("cmap")) == 0) {
    return 0;
  }

  cmapIndex.version = getUInt16t();
  cmapIndex.numberSubtables = getUInt16t();

  for (int i = 0; i < cmapIndex.numberSubtables; i++) {
    platformId = getUInt16t();
    platformSpecificId = getUInt16t();
    tableOffset = getUInt32t();
    if ((platformId == 3) && (platformSpecificId == 1)) {
      cmapFormat4.offset = cmapOffset + tableOffset;
      readCmapFormat4();
      foundMap = 1;
      break;
    }
  }

  if (foundMap == 0) {
    return 0;
  }

  return 1;
}

/* convert character code to glyph id */
uint16_t truetypeClass::codeToGlyphId(uint16_t code) {
  uint16_t start, end, idRangeOffset;
  int16_t idDelta;
  int found = 0;
  uint16_t offset, glyphId;

  for (int i = 0; i < cmapFormat4.segCountX2 / 2; i++) {
    file.seek(cmapFormat4.endCodeOffset + 2 * i);
    end = getUInt16t();
    if (code <= end) {
      file.seek(cmapFormat4.startCodeOffset + 2 * i);
      start = getUInt16t();
      if (code >= start) {
        file.seek(cmapFormat4.idDeltaOffset + 2 * i);
        idDelta = getInt16t();
        file.seek(cmapFormat4.idRangeOffsetOffset + 2 * i);
        idRangeOffset = getUInt16t();
        if (idRangeOffset == 0) {
          glyphId = (idDelta + code) % 65536;
        } else {
          offset = (idRangeOffset / 2 + i + code - start - cmapFormat4.segCountX2 / 2) * 2;
          file.seek(cmapFormat4.glyphIndexArrayOffset + offset);
          glyphId = getUInt16t();
        }

        found = 1;
        break;
      }
    }
  }
  if (!found) {
    return 0;
  }
  return glyphId;
}

/* initialize */
int truetypeClass::begin(int cs, const char * path, int checkCheckSum) {
  if (!sd->begin(cs)) {
    return 0;
  }

  if ((file = sd->open(path)) == 0) {
    return 0;
  }

  if (readTableDirectory(checkCheckSum) == 0) {
    file.close();
    return 0;
  }

  if (readCmap() == 0) {
    file.close();
    return 0;
  }

  readHeadTable();

  return 1;
}

void truetypeClass::end() {
  file.close();
}

/* read coords */
void truetypeClass::readCoords(char xy) {
  int value = 0;
  int shortFlag, sameFlag;

  if (xy == 'x') {
    shortFlag = FLAG_XSHORT;
    sameFlag = FLAG_XSAME;
  } else {
    shortFlag = FLAG_YSHORT;
    sameFlag = FLAG_YSAME;
  }

  for (int i = 0; i < glyph.numberOfPoints; i++) {
    if (glyph.points[i].flag & shortFlag) {
      if (glyph.points[i].flag & sameFlag) {
        value += getUInt8t();
      } else {
        value -= getUInt8t();
      }
    } else if (~glyph.points[i].flag & sameFlag) {
      value += getUInt16t();
    }

    if (xy == 'x') {
      glyph.points[i].x = value;
    } else {
      glyph.points[i].y = value;
    }
  }
}

/* insert a point at the specified position */
void truetypeClass::insertGlyph(int contour, int position, int16_t x, int16_t y, uint8_t flag) {
  glyph.numberOfPoints++;
  glyph.points = (ttPoint_t *)realloc(glyph.points, sizeof(ttPoint_t) * glyph.numberOfPoints);

  for (int k = contour; k < glyph.numberOfContours; k++) {
    glyph.endPtsOfContours[k]++;
  }

  for (int i = glyph.numberOfPoints - 1; i >= position; i--) {
    glyph.points[i] = glyph.points[i - 1];
  }
  glyph.points[position].flag = flag;
  glyph.points[position].x = x;
  glyph.points[position].y = y;
}

/* treat consecutive off curves */
void truetypeClass::adjustGlyph() {
  //insert each start point at the end of the contour to close the figure
  for (int i = 0, j = 0; i < glyph.numberOfContours; i++) {
    insertGlyph(i, glyph.endPtsOfContours[i] + 1, glyph.points[j].x, glyph.points[j].y, glyph.points[j].flag);
    j = glyph.endPtsOfContours[i] + 1;
  }
}

/* read simple glyph */
int truetypeClass::readSimpleGlyph() {
  int repeatCount;
  uint8_t flag;

  glyph.endPtsOfContours = (uint16_t *)malloc((sizeof(uint16_t) * glyph.numberOfContours));

  for (int i = 0; i < glyph.numberOfContours; i++) {
    glyph.endPtsOfContours[i] = getUInt16t();
  }

  #if defined ESP32
  file.seek(getUInt16t(), SeekCur);
  #else
  file.seek(getUInt16t() + file.position());
  #endif

  if (glyph.numberOfContours == 0) {
    return 0;
  }

  glyph.numberOfPoints = 0;
  for (int i = 0; i < glyph.numberOfContours; i++) {
    if (glyph.endPtsOfContours[i] > glyph.numberOfPoints) {
      glyph.numberOfPoints = glyph.endPtsOfContours[i];
    }
  }
  glyph.numberOfPoints++;

  glyph.points = (ttPoint_t *)malloc(sizeof(ttPoint_t) * (glyph.numberOfPoints + glyph.numberOfContours));

  for (int i = 0; i < glyph.numberOfPoints; i++) {
    flag = getUInt8t();
    glyph.points[i].flag = flag;
    if (flag & FLAG_REPEAT) {
      repeatCount = getUInt8t();
      while (repeatCount--) {
        glyph.points[++i].flag = flag;
      }
    }
  }

  readCoords('x');
  readCoords('y');

  return 1;
}

/* read glyph */
int truetypeClass::readGlyph(uint16_t _code) {
  uint32_t offset = getGlyphOffset(codeToGlyphId(_code));
  file.seek(offset);
  glyph.numberOfContours = getInt16t();
  glyph.xMin = getInt16t();
  glyph.yMin = getInt16t();
  glyph.xMax = getInt16t();
  glyph.yMax = getInt16t();

  if (glyph.numberOfContours != -1) {
    readSimpleGlyph();

    return 1;
  }
  return 0;
}

/* free glyph */
void truetypeClass::freeGlyph() {
  free(glyph.points);
  free(glyph.endPtsOfContours);
}

//generate Bitmap
uint8_t truetypeClass::generateBitmap(uint16_t _height) {
  uint8_t width = _height * (glyph.xMax - glyph.xMin) / (this->yMax - this->yMin);
  uint16_t byteLength = (_height * (width + 7) / 8);

  this->bitmap = (uint8_t *)calloc(byteLength, sizeof(uint8_t));
  this->points = NULL;
  this->numPoints = 0;
  this->numBeginPoints = 0;
  this->numEndPoints = 0;

  for (uint16_t i = 0, j = 0; i < glyph.numberOfContours; i++, j++) {
    uint8_t lastPoint;
    uint8_t firstPoint;

    if ((glyph.points[lastPoint - 1].flag & FLAG_ONCURVE) && !(glyph.points[j].flag & FLAG_ONCURVE) && (glyph.points[j + 1].flag & FLAG_ONCURVE)) {
      //Bezier curve at the special case
      j++;
    }

    firstPoint = j;
    lastPoint = glyph.endPtsOfContours[i];

    while (j < lastPoint) {
      int16_t x0, y0, x1, y1;
      uint8_t thirdPointOfCurve, fourthPointOfCurve;

      thirdPointOfCurve = ((j + 2) > lastPoint) ? (firstPoint) : (j + 2);
      fourthPointOfCurve = ((j + 3) > lastPoint) ? (firstPoint + 1) : (j + 3);

      if ((glyph.points[j].flag & FLAG_ONCURVE) && (glyph.points[j + 1].flag & FLAG_ONCURVE)) {
        //straight line

        addLine(map(glyph.points[j].x, glyph.xMin, glyph.xMax, 0, width - 1),
                map(glyph.points[j].y, this->yMin, this->yMax, _height - 1, 0),
                map(glyph.points[j + 1].x, glyph.xMin, glyph.xMax, 0, width - 1),
                map(glyph.points[j + 1].y, this->yMin, this->yMax, _height - 1, 0),
                width, _height);
      } else if (!(glyph.points[j + 1].flag & FLAG_ONCURVE) && (glyph.points[thirdPointOfCurve].flag & FLAG_ONCURVE)) {
        //Bezier curve (Quadratic)

        x0 = glyph.points[j].x;
        y0 = glyph.points[j].y;

        for (float t = 0; t <= 1; t += 0.2) {
          x1 = (1 - t) * (1 - t) * glyph.points[j].x + 2 * t * (1 - t) * glyph.points[j + 1].x + t * t * glyph.points[thirdPointOfCurve].x;
          y1 = (1 - t) * (1 - t) * glyph.points[j].y + 2 * t * (1 - t) * glyph.points[j + 1].y + t * t * glyph.points[thirdPointOfCurve].y;

          addLine(map(x0, glyph.xMin, glyph.xMax, 0, width - 1),
                  map(y0, this->yMin, this->yMax, _height - 1, 0),
                  map(x1, glyph.xMin, glyph.xMax, 0, width - 1),
                  map(y1, this->yMin, this->yMax, _height - 1, 0),
                  width, _height);
          x0 = x1;
          y0 = y1;
        }

        if (thirdPointOfCurve != firstPoint) {
          j++;
        }
      } else {
        //Bezier curve (More than three cubic)
        //Decompose into n three-dimensional and two-dimensional (Interim solution. future work)

        x0 = glyph.points[j].x;
        y0 = glyph.points[j].y;

        for (float t = 0; t <= 1; t += 0.2) {
          x1 = (1 - t) * (1 - t) * (1 - t) * glyph.points[j].x + 3 * (1 - t) * (1 - t) * t * glyph.points[j + 1].x + 3 * (1 - t) * t * t * glyph.points[thirdPointOfCurve].x + t * t * t * glyph.points[fourthPointOfCurve].x;
          y1 = (1 - t) * (1 - t) * (1 - t) * glyph.points[j].y + 3 * (1 - t) * (1 - t) * t * glyph.points[j + 1].y + 3 * (1 - t) * t * t * glyph.points[thirdPointOfCurve].y + t * t * t * glyph.points[fourthPointOfCurve].y;

          addLine(map(x0, glyph.xMin, glyph.xMax, 0, width - 1),
                  map(y0, this->yMin, this->yMax, _height - 1, 0),
                  map(x1, glyph.xMin, glyph.xMax, 0, width - 1),
                  map(y1, this->yMin, this->yMax, _height - 1, 0),
                  width, _height);
          x0 = x1;
          y0 = y1;
        }

        if (thirdPointOfCurve != firstPoint) {
          j += 2;
        }
      }

      j++;
    }

    addEndPoint(this->numPoints - 1);
    addBeginPoint(this->numPoints);
  }

  return width;
}


void truetypeClass::freeBitmap() {
  free(this->bitmap);
  freePoints();
  freeBeginPoints();
  freeEndPoints();
}

void truetypeClass::addPixel(int _x0, int _y0, int _width, int _height) {
  this->bitmap[(_x0 / 8) + (((_width + 7) / 8) * _y0)] |= (1 << (7 - _x0 % 8));
}

/* Bresenham's line algorithm */
void truetypeClass::addLine(int _x0, int _y0, int _x1, int _y1, int _width, int _height) {
  int dx = abs(_x1 - _x0);
  int dy = abs(_y1 - _y0);
  int sx, sy, err, e2;

  if (numPoints == 0) {
    addPoint(_x0, _y0);
    addBeginPoint(0);
  }
  addPoint(_x1, _y1);

  if (_x0 < _x1) {
    sx = 1;
  } else {
    sx = -1;
  }
  if (_y0 < _y1) {
    sy = 1;
  } else {
    sy = -1;
  }
  err = dx - dy;

  while (1) {
    addPixel(_x0, _y0, _width, _height);
    if ((_x0 == _x1) && (_y0 == _y1)) {
      break;
    }
    e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      _x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      _y0 += sy;
    }
  }
}

void truetypeClass::addPoint(int _x, int _y) {
  this->numPoints++;
  this->points = (ttCoordinate_t *)realloc(this->points, sizeof(ttCoordinate_t) * this->numPoints);
  this->points[(this->numPoints - 1)].x = _x;
  this->points[(this->numPoints - 1)].y = _y;
}

void truetypeClass::freePoints() {
  free(this->points);
  this->points = NULL;
  this->numPoints = 0;
}

void truetypeClass::addBeginPoint(int _bp) {
  this->numBeginPoints++;
  this->beginPoints = (int *)realloc(this->beginPoints, sizeof(int) * this->numBeginPoints);
  this->beginPoints[(this->numBeginPoints - 1)] = _bp;
}

void truetypeClass::freeBeginPoints() {
  free(this->beginPoints);
  this->beginPoints = NULL;
  this->numBeginPoints = 0;
}

void truetypeClass::addEndPoint(int _ep) {
  this->numEndPoints++;
  this->endPoints = (int *)realloc(this->endPoints, sizeof(int) * this->numEndPoints);
  this->endPoints[(this->numEndPoints - 1)] = _ep;
}

void truetypeClass::freeEndPoints() {
  free(this->endPoints);
  this->endPoints = NULL;
  this->numEndPoints = 0;
}

bool truetypeClass::isInside(int _x, int _y) {
  int windingNumber = 0;
  int bpCounter = 0, epCounter = 0;
  ttCoordinate_t point = {_x, _y};
  ttCoordinate_t point1;
  ttCoordinate_t point2;

  for (int i = 0; i < this->numPoints; i++) {
    point1 = this->points[i];
    // Wrap?
    if (i == this->endPoints[epCounter]) {
      point2 = this->points[this->beginPoints[bpCounter]];
      epCounter++;
      bpCounter++;
    } else {
      point2 = this->points[i + 1];
    }

    if (point1.y <= point.y) {
      if (point2.y > point.y) {
        if (isLeft(point1, point2, point) > 0) {
          windingNumber++;
        }
      }
    } else {
      // start y > point.y (no test needed)
      if (point2.y <= point.y) {
        if (isLeft(point1, point2, point) < 0) {
          windingNumber--;
        }
      }
    }
  }

  return (windingNumber != 0);
}

int truetypeClass::isLeft(ttCoordinate_t &_p0, ttCoordinate_t &_p1, ttCoordinate_t &_point) {
  return ((_p1.x - _p0.x) * (_point.y - _p0.y) - (_point.x - _p0.x) * (_p1.y - _p0.y));
}

int truetypeClass::getPixel(int _x0, int _y0, int _width) {
  return (this->bitmap[(_x0 / 8) + (((_width + 7) / 8) * _y0)]) & (1 << (7 - _x0 % 8));
}
