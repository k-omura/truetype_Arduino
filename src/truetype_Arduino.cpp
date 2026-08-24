/*
  Read truetype(.ttf) from SD and generate bitmap.

  TrueType™ Reference Manual
  https://developer.apple.com/fonts/TrueType-Reference-Manual/
*/

#include "truetype_Arduino.h"

/* constructor */

truetypeClass::truetypeClass() : table(NULL), points(NULL), beginPoints(NULL), endPoints(NULL), userFrameBuffer(NULL)
{
  glyph.points = NULL;
  glyph.endPtsOfContours = NULL;
}

/* ----------------public---------------- */

void truetypeClass::end()
{
#ifdef ENABLE_INTEGRATED_TTF
  if (!this->useIntegratedTtf)
#endif
    file.close();
  free(this->table);
  this->table = NULL;
  this->numTables = 0;
  free(this->cmapSegments);
  this->cmapSegments = NULL;
  this->cmapSegmentCapacity = 0;
  free(this->edgeCrosses);
  this->edgeCrosses = NULL;
  free(this->edgeOrder);
  this->edgeOrder = NULL;
  free(this->fillCoverage);
  this->fillCoverage = NULL;
  free(this->points);
  this->points = NULL;
  free(this->beginPoints);
  this->beginPoints = NULL;
  free(this->endPoints);
  this->endPoints = NULL;
  this->edgeCrossesCapacity = 0;
  this->fillCoverageCapacity = 0;
  this->freePointsAll();
  this->freeGlyph();
  free(glyph.points);
  glyph.points = NULL;
  free(glyph.endPtsOfContours);
  glyph.endPtsOfContours = NULL;
  this->glyphPointsCapacity = 0;
  this->glyphContoursCapacity = 0;
  this->pointsCapacity = 0;
  this->beginPointsCapacity = 0;
  this->endPointsCapacity = 0;
}

/* initialize */
uint8_t truetypeClass::setTtfFile(File _file, uint8_t _checkCheckSum)
{
  if (_file == 0)
  {
    return 0;
  }

  this->end();
  this->file = _file;
#ifdef ENABLE_INTEGRATED_TTF
  this->useIntegratedTtf = false;
#endif
  this->cacheValid = false;
  this->filePosition = 0;
  if (this->readTableDirectory(_checkCheckSum) == 0)
  {
    free(this->table);
    this->table = NULL;
    file.close();
    return 0;
  }

  if (this->readCmap() == 0)
  {
    free(this->table);
    this->table = NULL;
    file.close();
    return 0;
  }

  if (this->readHMetric() == 0)
  {
    free(this->table);
    this->table = NULL;
    file.close();
    return 0;
  }

  this->readKern();
  this->readHeadTable();
  this->locaTableOffset = 0;
  this->glyfTableOffset = 0;
  for (uint16_t i = 0; i < this->numTables; i++)
  {
    if (strcmp(this->table[i].name, "loca") == 0)
      this->locaTableOffset = this->table[i].offset;
    if (strcmp(this->table[i].name, "glyf") == 0)
      this->glyfTableOffset = this->table[i].offset;
  }
  free(this->table);
  this->table = NULL;
  this->numTables = 0;
  return 1;
}

#ifdef ENABLE_INTEGRATED_TTF
uint8_t truetypeClass::setIntegratedTtf()
{
  if (integrated_ttf_size == 0)
  {
    return 0;
  }

  this->end();
  this->useIntegratedTtf = true;
  this->cacheValid = false;
  this->filePosition = 0;
  if (this->readTableDirectory(0) == 0 || this->readCmap() == 0 || this->readHMetric() == 0)
  {
    free(this->table);
    this->table = NULL;
    this->numTables = 0;
    return 0;
  }

  this->readKern();
  this->readHeadTable();
  this->locaTableOffset = 0;
  this->glyfTableOffset = 0;
  for (uint16_t i = 0; i < this->numTables; i++)
  {
    if (strcmp(this->table[i].name, "loca") == 0)
      this->locaTableOffset = this->table[i].offset;
    if (strcmp(this->table[i].name, "glyf") == 0)
      this->glyfTableOffset = this->table[i].offset;
  }
  free(this->table);
  this->table = NULL;
  this->numTables = 0;
  return 1;
}
#endif

void truetypeClass::setFramebuffer(uint16_t _framebufferWidth, uint16_t _framebufferHeight, uint16_t _framebuffer_bit, uint8_t _framebufferDirection, uint8_t *_framebuffer)
{
  this->displayWidth = _framebufferWidth;
  this->displayHeight = _framebufferHeight;
  this->framebufferBit = _framebuffer_bit;
  this->framebufferDirection = _framebufferDirection;
  this->userFrameBuffer = _framebuffer;

  if (this->framebufferDirection)
  {
    // Framebuffer bit direction: Vertical
  }
  else
  {
    // Framebuffer bit direction: Horizontal
    switch (this->framebufferBit)
    {
    case 8: // 8bit Horizontal
      this->displayWidthFrame = this->displayWidth;
      break;
    case 4: // 4bit Horizontal
      this->displayWidthFrame = (this->displayWidth % 2 == 0) ? (this->displayWidth / 2) : (this->displayWidth / 2 + 1);
      break;
    case 1: // 1bit Horizontal
    default:
      this->displayWidthFrame = (this->displayWidth % 8 == 0) ? (this->displayWidth / 8) : (this->displayWidth / 8 + 1);
      break;
    }
  }
  return;
}

void truetypeClass::setCharacterSize(uint16_t _characterSize)
{
  this->characterSize = _characterSize;
}

void truetypeClass::setCharacterSpacing(int16_t _characterSpace, uint8_t _kerning)
{
  this->characterSpace = _characterSpace;
  this->kerningOn = _kerning;
}

void truetypeClass::setTextBoundary(uint16_t _start_x, uint16_t _end_x, uint16_t _end_y)
{
  this->start_x = _start_x;
  this->end_x = _end_x;
  this->end_y = _end_y;
}

void truetypeClass::setTextColor(uint8_t _onLine, uint8_t _inside, uint8_t _fill)
{
  this->colorLine = _onLine;
  this->colorInside = _inside;
  this->fillInside = _fill;
}

void truetypeClass::setTextRotation(uint16_t _rotation)
{
  switch (_rotation)
  {
  case ROTATE_90:
  case 90:
    _rotation = 1;
    break;
  case ROTATE_180:
  case 180:
    _rotation = 2;
    break;
  case ROTATE_270:
  case 270:
    _rotation = 3;
    break;
  default:
    _rotation = 0;
    break;
  }
  this->stringRotation = _rotation;
}

/* ----------------private---------------- */
/* calculate checksum */
uint32_t truetypeClass::calculateCheckSum(uint32_t offset, uint32_t length)
{
  uint32_t checksum = 0;
  uint32_t fullWords = length / 4;
  uint32_t remainder = length & 3;
  this->seekFile(offset);

  while (fullWords-- > 0)
  {
    checksum += this->getUInt32t();
  }
  if (remainder != 0)
  {
    uint32_t lastWord = 0;
    for (uint32_t i = 0; i < remainder; i++)
    {
      lastWord |= (uint32_t)this->getUInt8t() << (24 - i * 8);
    }
    checksum += lastWord;
  }
  return checksum;
}

/* read table directory */
int truetypeClass::readTableDirectory(int checkCheckSum)
{
  this->seekFile(numTablesPos);
  numTables = this->getUInt16t();
  this->table = (ttTable_t *)malloc(sizeof(ttTable_t) * numTables);
  if (!this->table || numTables == 0)
  {
    this->table = NULL;
    this->numTables = 0;
    return 0;
  }

  this->seekFile(tablePos);
  // Serial.println("---table list---");
  for (int i = 0; i < numTables; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      this->table[i].name[j] = this->getUInt8t();
      // Serial.printf("%c", table[i].name[j]);
    }
    this->table[i].name[4] = '\0';
    this->table[i].checkSum = this->getUInt32t();
    this->table[i].offset = this->getUInt32t();
    this->table[i].length = this->getUInt32t();

    // Serial.printf("--%X", table[i].offset);
    // Serial.println();
  }

  if (checkCheckSum)
  {
    for (int i = 0; i < numTables; i++)
    {
      if (strcmp(this->table[i].name, "head") != 0)
      { /* checksum of "head" is invalid */
        uint32_t c = this->calculateCheckSum(this->table[i].offset, this->table[i].length);
        if (this->table[i].checkSum != c)
        {
          return 0;
        }
      }
    }
  }
  return 1;
}

/* read head table */
void truetypeClass::readHeadTable()
{
  for (int i = 0; i < numTables; i++)
  {
    if (strcmp(table[i].name, "head") == 0)
    {
      this->seekFile(table[i].offset);

      headTable.version = this->getUInt32t();
      headTable.revision = this->getUInt32t();
      headTable.checkSumAdjustment = this->getUInt32t();
      headTable.magicNumber = this->getUInt32t();
      headTable.flags = this->getUInt16t();
      headTable.unitsPerEm = this->getUInt16t();
      for (int j = 0; j < 8; j++)
      {
        headTable.created[j] = this->getUInt8t();
      }
      for (int j = 0; j < 8; j++)
      {
        headTable.modified[j] = this->getUInt8t();
      }
      xMin = headTable.xMin = this->getInt16t();
      yMin = headTable.yMin = this->getInt16t();
      xMax = headTable.xMax = this->getInt16t();
      yMax = headTable.yMax = this->getInt16t();
      if (this->fontAscender == 0 && this->fontDescender == 0)
      {
        this->fontAscender = yMax;
        this->fontDescender = yMin;
      }
      headTable.macStyle = this->getUInt16t();
      headTable.lowestRecPPEM = this->getUInt16t();
      headTable.fontDirectionHint = this->getInt16t();
      headTable.indexToLocFormat = this->getInt16t();
      headTable.glyphDataFormat = this->getInt16t();
    }
  }
}

/* cmap */
/* read cmap format 4 */
uint8_t truetypeClass::readCmapFormat4()
{
  this->seekFile(cmapFormat4.offset);
  if ((cmapFormat4.format = this->getUInt16t()) != 4)
  {
    return 0;
  }

  cmapFormat4.length = this->getUInt16t();
  if (cmapFormat4.length < 24 || cmapFormat4.offset > this->cmapTableEnd ||
      (uint32_t)cmapFormat4.length > this->cmapTableEnd - cmapFormat4.offset)
  {
    return 0;
  }
  cmapFormat4.language = this->getUInt16t();
  cmapFormat4.segCountX2 = this->getUInt16t();
  if (cmapFormat4.segCountX2 == 0 || (cmapFormat4.segCountX2 & 1) != 0 ||
      16UL + 4UL * cmapFormat4.segCountX2 > cmapFormat4.length)
  {
    return 0;
  }
  cmapFormat4.searchRange = this->getUInt16t();
  cmapFormat4.entrySelector = this->getUInt16t();
  cmapFormat4.rangeShift = this->getUInt16t();
  cmapFormat4.endCodeOffset = cmapFormat4.offset + 14;
  cmapFormat4.startCodeOffset = cmapFormat4.endCodeOffset + cmapFormat4.segCountX2 + 2;
  cmapFormat4.idDeltaOffset = cmapFormat4.startCodeOffset + cmapFormat4.segCountX2;
  cmapFormat4.idRangeOffsetOffset = cmapFormat4.idDeltaOffset + cmapFormat4.segCountX2;
  cmapFormat4.glyphIndexArrayOffset = cmapFormat4.idRangeOffsetOffset + cmapFormat4.segCountX2;

  uint16_t segmentCount = cmapFormat4.segCountX2 / 2;
  free(this->cmapSegments);
  this->cmapSegments = (ttCmapSegment_t *)malloc(sizeof(ttCmapSegment_t) * segmentCount);
  if (!this->cmapSegments)
  {
    this->cmapSegmentCapacity = 0;
    return 0;
  }
  this->cmapSegmentCapacity = segmentCount;
  for (uint16_t i = 0; i < segmentCount; i++)
  {
    this->seekFile(cmapFormat4.endCodeOffset + 2UL * i);
    this->cmapSegments[i].endCode = this->getUInt16t();
    this->seekFile(cmapFormat4.startCodeOffset + 2UL * i);
    this->cmapSegments[i].startCode = this->getUInt16t();
    this->seekFile(cmapFormat4.idDeltaOffset + 2UL * i);
    this->cmapSegments[i].idDelta = this->getInt16t();
    this->seekFile(cmapFormat4.idRangeOffsetOffset + 2UL * i);
    this->cmapSegments[i].idRangeOffset = this->getUInt16t();
  }

  return 1;
}

/* read cmap */
uint8_t truetypeClass::readCmap()
{
  uint16_t platformId, platformSpecificId;
  uint32_t cmapOffset, tableOffset;
  uint8_t foundMap = 0;

  if ((cmapOffset = this->seekToTable("cmap")) == 0)
  {
    return 0;
  }

  for (uint16_t i = 0; i < this->numTables; i++)
  {
    if (strcmp(this->table[i].name, "cmap") == 0)
    {
      this->cmapTableEnd = this->table[i].offset + this->table[i].length;
      break;
    }
  }

  cmapIndex.version = this->getUInt16t();
  cmapIndex.numberSubtables = this->getUInt16t();

  for (uint16_t i = 0; i < cmapIndex.numberSubtables; i++)
  {
    platformId = this->getUInt16t();
    platformSpecificId = this->getUInt16t();
    tableOffset = this->getUInt32t();
    if ((platformId == 3) && (platformSpecificId == 1))
    {
      cmapFormat4.offset = cmapOffset + tableOffset;
      foundMap = this->readCmapFormat4();
      break;
    }
  }

  if (foundMap == 0)
  {
    return 0;
  }

  return 1;
}

/* convert character code to glyph id */
uint16_t truetypeClass::codeToGlyphId(uint16_t _code)
{
  if (!this->cmapSegments || this->cmapSegmentCapacity == 0)
  {
    return 0;
  }
  int lo = 0, hi = this->cmapSegmentCapacity - 1, segment = -1;
  while (lo <= hi)
  {
    int mid = lo + (hi - lo) / 2;
    if (_code <= this->cmapSegments[mid].endCode)
    {
      segment = mid;
      hi = mid - 1;
    }
    else
    {
      lo = mid + 1;
    }
  }
  if (segment < 0 || _code < this->cmapSegments[segment].startCode)
  {
    return 0;
  }
  int16_t idDelta = this->cmapSegments[segment].idDelta;
  uint16_t idRangeOffset = this->cmapSegments[segment].idRangeOffset;
  if (idRangeOffset == 0)
  {
    return (uint16_t)((int32_t)_code + idDelta);
  }
  uint32_t rangeAddress = cmapFormat4.idRangeOffsetOffset + 2UL * segment;
  uint32_t glyphAddress = rangeAddress + idRangeOffset +
                          2UL * (_code - this->cmapSegments[segment].startCode);
  if (glyphAddress + 2 > cmapFormat4.offset + cmapFormat4.length)
  {
    return 0;
  }
  this->seekFile(glyphAddress);
  uint16_t glyphId = this->getUInt16t();
  return glyphId ? (uint16_t)((int32_t)glyphId + idDelta) : 0;
}

/* kerning */
/* read kerning table */
uint8_t truetypeClass::readKern()
{
  uint32_t nextTable;
  this->kernTablePos = 0;

  if (this->seekToTable("kern") == 0)
  {
    // Serial.println("kern not found");
    return 0;
  }

  kernHeader.version = this->getUInt16t();
  kernHeader.nTables = this->getUInt16t();

  // only support up to 32 sub-tables
  if (kernHeader.nTables > 32)
  {
    kernHeader.nTables = 32;
  }

  for (uint8_t i = 0; i < kernHeader.nTables; i++)
  {
    uint16_t format;

    uint32_t subtableStart = this->tellFile();
    kernSubtable.version = this->getUInt16t();
    kernSubtable.length = this->getUInt16t();
    nextTable = subtableStart + kernSubtable.length;
    kernSubtable.coverage = this->getUInt16t();

    if (kernSubtable.version != 0 || kernSubtable.length < 6)
    {
      this->seekFile(nextTable);
      continue;
    }

    format = (uint16_t)(kernSubtable.coverage >> 8);

    // only support format0
    if (format != 0)
    {
      this->seekFile(nextTable);
      continue;
    }

    // only use horizontal kerning tables
    if ((kernSubtable.coverage & 0x0003) != 0x0001)
    {
      this->seekFile(nextTable);
      continue;
    }

    // format0
    kernFormat0.nPairs = this->getUInt16t();
    kernFormat0.searchRange = this->getUInt16t();
    kernFormat0.entrySelector = this->getUInt16t();
    kernFormat0.rangeShift = this->getUInt16t();
    this->kernTablePos = this->tellFile();

    break;
  }

  return 1;
}

int16_t truetypeClass::getKerning(uint16_t _left_glyph, uint16_t _right_glyph)
{
  if (this->kernTablePos == 0 || this->kernFormat0.nPairs == 0)
  {
    return 0;
  }
  uint32_t key0 = ((uint32_t)(_left_glyph) << 16) | (_right_glyph);
  int lo = 0, hi = (int)this->kernFormat0.nPairs - 1;
  while (lo <= hi)
  {
    int mid = lo + (hi - lo) / 2;
    this->seekFile(this->kernTablePos + (uint32_t)mid * 6);
    uint32_t key1 = this->getUInt32t();
    if (key1 == key0)
    {
      return this->getInt16t();
    }
    if (key1 < key0)
      lo = mid + 1;
    else
      hi = mid - 1;
  }
  return 0;
}

// hmtx. metric information for the horizontal layout each of the glyphs
uint8_t truetypeClass::readHMetric()
{
  uint32_t hheaTablePos = this->seekToTable("hhea");
  if (hheaTablePos == 0)
  {
    return 0;
  }
  this->seekFile(hheaTablePos + 34);
  this->numberOfHMetrics = this->getUInt16t();
  if (this->numberOfHMetrics == 0 || this->seekToTable("hmtx") == 0)
  {
    // Serial.println("hmtx not found");
    return 0;
  }

  this->hmtxTablePos = this->tellFile();
  return 1;
}

ttHMetric_t truetypeClass::getHMetric(uint16_t _code)
{
  ttHMetric_t result;
  result.advanceWidth = 0;
  uint16_t metricIndex = (_code < this->numberOfHMetrics) ? _code : (this->numberOfHMetrics - 1);
  this->seekFile(this->hmtxTablePos + ((uint32_t)metricIndex * 4));
  result.advanceWidth = getUInt16t();
  if (_code < this->numberOfHMetrics)
  {
    result.leftSideBearing = getInt16t();
  }
  else
  {
    this->seekFile(this->hmtxTablePos + ((uint32_t)this->numberOfHMetrics * 4) +
                   ((uint32_t)_code - this->numberOfHMetrics) * 2);
    result.leftSideBearing = getInt16t();
  }

  result.advanceWidth = (uint16_t)this->fontUnitsToPixels(result.advanceWidth);
  result.leftSideBearing = (int16_t)this->fontUnitsToPixels(result.leftSideBearing);
  return result;
}

int32_t truetypeClass::fontUnitsToPixels(int32_t _value)
{
  if (this->headTable.unitsPerEm == 0)
    return 0;
  return (_value * (int32_t)this->characterSize) / this->headTable.unitsPerEm;
}

/* get glyph offset */
uint32_t truetypeClass::getGlyphOffset(uint16_t index)
{
  uint32_t offset = 0;

  if (this->locaTableOffset == 0 || this->glyfTableOffset == 0)
  {
    for (int i = 0; i < numTables; i++)
    {
      if (strcmp(table[i].name, "loca") == 0)
        this->locaTableOffset = table[i].offset;
      if (strcmp(table[i].name, "glyf") == 0)
        this->glyfTableOffset = table[i].offset;
    }
  }
  if (headTable.indexToLocFormat == 1)
  {
    this->seekFile(this->locaTableOffset + index * 4UL);
    offset = this->getUInt32t();
  }
  else
  {
    this->seekFile(this->locaTableOffset + index * 2UL);
    offset = this->getUInt16t() * 2UL;
  }
  return offset + this->glyfTableOffset;
}

/* read coords */
void truetypeClass::readCoords(char _xy, uint16_t _startPoint)
{
  int16_t value = 0;
  uint8_t shortFlag, sameFlag;

  if (_xy == 'x')
  {
    shortFlag = FLAG_XSHORT;
    sameFlag = FLAG_XSAME;
  }
  else
  {
    shortFlag = FLAG_YSHORT;
    sameFlag = FLAG_YSAME;
  }

  for (uint16_t i = _startPoint; i < glyph.numberOfPoints; i++)
  {
    if (glyph.points[i].flag & shortFlag)
    {
      if (glyph.points[i].flag & sameFlag)
      {
        value += this->getUInt8t();
      }
      else
      {
        value -= this->getUInt8t();
      }
    }
    else if (~glyph.points[i].flag & sameFlag)
    {
      value += this->getUInt16t();
    }

    if (_xy == 'x')
    {
      glyph.points[i].x = value;
    }
    else
    {
      int32_t rawX = glyph.points[i].x;
      int32_t rawY = value;
      glyph.points[i].x = (int16_t)(((this->glyphTransformation.a * rawX) +
                                     (this->glyphTransformation.c * rawY)) >>
                                    14) +
                          this->glyphTransformation.dx;
      glyph.points[i].y = (int16_t)(((this->glyphTransformation.b * rawX) +
                                     (this->glyphTransformation.d * rawY)) >>
                                    14) +
                          this->glyphTransformation.dy;
    }
  }
}

/* read simple glyph */
uint8_t truetypeClass::readSimpleGlyph(uint8_t _addGlyph)
{
  uint8_t repeatCount;
  uint8_t flag;
  static uint16_t counterContours;
  static uint16_t counterPoints;

  if (glyph.numberOfContours <= 0)
  {
    return 0;
  }

  if (!_addGlyph)
  {
    counterContours = 0;
    counterPoints = 0;
  }

  if (glyph.numberOfContours > this->glyphContoursCapacity)
  {
    uint16_t *tmp = (uint16_t *)realloc(glyph.endPtsOfContours, sizeof(uint16_t) * glyph.numberOfContours);
    if (!tmp)
      return 0;
    glyph.endPtsOfContours = tmp;
    this->glyphContoursCapacity = glyph.numberOfContours;
  }

  for (uint16_t i = counterContours; i < glyph.numberOfContours; i++)
  {
    glyph.endPtsOfContours[i] = counterPoints + this->getUInt16t();
  }

  uint16_t instructionLength = this->getUInt16t();
  this->seekFile(this->tellFile() + instructionLength);

  for (uint16_t i = counterContours; i < glyph.numberOfContours; i++)
  {
    if (glyph.endPtsOfContours[i] > glyph.numberOfPoints)
    {
      glyph.numberOfPoints = glyph.endPtsOfContours[i];
    }
  }
  glyph.numberOfPoints++;

  uint16_t requiredPoints = glyph.numberOfPoints + glyph.numberOfContours;
  if (requiredPoints > this->glyphPointsCapacity)
  {
    ttPoint_t *tmp = (ttPoint_t *)realloc(glyph.points, sizeof(ttPoint_t) * requiredPoints);
    if (!tmp)
      return 0;
    glyph.points = tmp;
    this->glyphPointsCapacity = requiredPoints;
  }

  for (uint16_t i = counterPoints; i < glyph.numberOfPoints; i++)
  {
    flag = this->getUInt8t();
    glyph.points[i].flag = flag;
    if (flag & FLAG_REPEAT)
    {
      repeatCount = this->getUInt8t();
      while (repeatCount-- && (uint16_t)(i + 1) < glyph.numberOfPoints)
      {
        glyph.points[++i].flag = flag;
      }
    }
  }

  this->readCoords('x', counterPoints);
  this->readCoords('y', counterPoints);

  counterContours = glyph.numberOfContours;
  counterPoints = glyph.numberOfPoints;

  return 1;
}

/* read Compound glyph */
uint8_t truetypeClass::readCompoundGlyph()
{
  uint16_t glyphIndex;
  uint16_t flags;
  uint8_t numberOfGlyphs = 0;
  uint32_t offset;
  int32_t arg1, arg2;

  int32_t parentA = this->glyphTransformation.a;
  int32_t parentB = this->glyphTransformation.b;
  int32_t parentC = this->glyphTransformation.c;
  int32_t parentD = this->glyphTransformation.d;
  int32_t parentDx = this->glyphTransformation.dx;
  int32_t parentDy = this->glyphTransformation.dy;
  if (this->compoundGlyphDepth == 0)
    glyph.numberOfContours = 0;
  if (this->compoundGlyphDepth++ >= 16)
    return 0;

  do
  {
    flags = this->getUInt16t();
    glyphIndex = this->getUInt16t();

    if (flags & 0b00000000001)
    {
      arg1 = this->getInt16t();
      arg2 = this->getInt16t();
    }
    else
    {
      arg1 = (int8_t)this->getUInt8t();
      arg2 = (int8_t)this->getUInt8t();
    }

    int32_t a = 1 << 14, b = 0, c = 0, d = 1 << 14;
    if (flags & 0x0008)
      a = d = this->getInt16t();
    else if (flags & 0x0040)
    {
      a = this->getInt16t();
      d = this->getInt16t();
    }
    else if (flags & 0x0080)
    {
      a = this->getInt16t();
      b = this->getInt16t();
      c = this->getInt16t();
      d = this->getInt16t();
    }

    int32_t componentDx = parentDx;
    int32_t componentDy = parentDy;
    if (flags & 0x0002)
    {
      componentDx += (parentA * arg1 + parentC * arg2) >> 14;
      componentDy += (parentB * arg1 + parentD * arg2) >> 14;
    }
    this->glyphTransformation.a = (parentA * a + parentC * b) >> 14;
    this->glyphTransformation.b = (parentB * a + parentD * b) >> 14;
    this->glyphTransformation.c = (parentA * c + parentC * d) >> 14;
    this->glyphTransformation.d = (parentB * c + parentD * d) >> 14;
    this->glyphTransformation.dx = componentDx;
    this->glyphTransformation.dy = componentDy;

    if (flags & 0b01000000000)
    {
      this->charCode = glyphIndex;
    }

    // Serial.printf("--%d: flag: 0x%04X index: %4d\n", numberOfGlyphs, flags, glyphIndex);
    // Serial.printf("dx: %3d, dy: %3d\n", this->glyphTransformation.dx, this->glyphTransformation.dy);
    // Serial.printf("Scaling: %d\n", this->glyphTransformation.enableScale);

    uint16_t componentStart = glyph.numberOfPoints;
    offset = this->tellFile();

    uint32_t glyphOffset = this->getGlyphOffset(glyphIndex);
    this->seekFile(glyphOffset);
    int16_t componentContours = this->getInt16t();
    this->seekFile(glyphOffset + 10);

    if (componentContours >= 0)
    {
      glyph.numberOfContours += componentContours;
      this->readSimpleGlyph(glyph.numberOfPoints != 0);
    }
    else if (numberOfGlyphs < 32)
    {
      this->readCompoundGlyph();
    }
    this->seekFile(offset);

    if (!(flags & 0x0002) && glyph.numberOfPoints > componentStart)
    {
      if (componentStart > 0 && (uint16_t)arg1 < componentStart && (uint16_t)arg2 < glyph.numberOfPoints - componentStart)
      {
        int32_t dx = glyph.points[(uint16_t)arg1].x - glyph.points[componentStart + (uint16_t)arg2].x;
        int32_t dy = glyph.points[(uint16_t)arg1].y - glyph.points[componentStart + (uint16_t)arg2].y;
        for (uint16_t i = componentStart; i < glyph.numberOfPoints; i++)
        {
          glyph.points[i].x += dx;
          glyph.points[i].y += dy;
        }
      }
    }

    numberOfGlyphs++;
    this->glyphTransformation = {(int16_t)parentDx, (int16_t)parentDy, parentA, parentB, parentC, parentD}; // init
  } while ((flags & 0b00000100000) && numberOfGlyphs < 32);

  if (flags & 0x0100)
  {
    uint16_t instructionLength = this->getUInt16t();
    this->seekFile(this->tellFile() + instructionLength);
  }
  this->glyphTransformation = {(int16_t)parentDx, (int16_t)parentDy, parentA, parentB, parentC, parentD};
  this->compoundGlyphDepth--;
  return 1;
}

/* read glyph */
uint8_t truetypeClass::readGlyph(uint16_t _code, uint8_t _justSize)
{
  uint32_t offset = this->getGlyphOffset(_code);
  this->seekFile(offset);
  glyph.numberOfContours = this->getInt16t();
  glyph.xMin = this->getInt16t();
  glyph.yMin = this->getInt16t();
  glyph.xMax = this->getInt16t();
  glyph.yMax = this->getInt16t();

  this->glyphTransformation = {0, 0, 1 << 14, 0, 0, 1 << 14}; // init
  this->compoundGlyphDepth = 0;

  if (_justSize)
  {
    return 0;
  }

  if (glyph.numberOfContours >= 0)
  {
    return this->readSimpleGlyph();
  }
  else
  {
    return this->readCompoundGlyph();
  }
  return 0;
}

/* free glyph */
void truetypeClass::freeGlyph()
{
  glyph.numberOfPoints = 0;
}

// generate Bitmap
void truetypeClass::generateOutline(int16_t _x, int16_t _y, uint16_t _width)
{
  this->freePointsAll();
  if (_width == 0 || this->characterSize == 0)
    return;
  int32_t xRange = glyph.xMax - glyph.xMin;
  int32_t yRange = this->yMax - this->yMin;
  this->outlineOriginX = _x;
  this->outlineOriginY = _y + this->characterSize - 1;
  this->outlineScaleX = xRange > 0 ? ((int32_t)(_width - 1) << 16) / xRange : 0;
  this->outlineScaleY = yRange > 0 ? ((int32_t)(this->characterSize - 1) << 16) / yRange : 0;

  uint16_t pointStart = 0;
  for (uint16_t contour = 0; contour < glyph.numberOfContours; contour++)
  {
    uint16_t last = glyph.endPtsOfContours[contour];
    uint16_t count = last - pointStart + 1;
    uint16_t startIndex = pointStart;
    bool hasOnCurveStart = false;
    for (uint16_t i = 0; i < count; i++)
    {
      if (glyph.points[pointStart + i].flag & FLAG_ONCURVE)
      {
        startIndex = pointStart + i;
        hasOnCurveStart = true;
        break;
      }
    }
    ttCoordinate_t startPoint;
    if (hasOnCurveStart)
    {
      startPoint = {glyph.points[startIndex].x, glyph.points[startIndex].y};
    }
    else
    {
      startPoint = {(int16_t)((glyph.points[pointStart].x + glyph.points[last].x) / 2),
                    (int16_t)((glyph.points[pointStart].y + glyph.points[last].y) / 2)};
    }
    ttCoordinate_t previous = startPoint;
    ttCoordinate_t pending = {0, 0};
    bool hasPending = false;
    uint16_t steps = hasOnCurveStart ? count - 1 : count;
    uint16_t index = hasOnCurveStart ? startIndex + 1 : startIndex;
    for (uint16_t step = 0; step < steps; step++)
    {
      uint16_t currentIndex = index > last ? pointStart : index;
      ttCoordinate_t current = {glyph.points[currentIndex].x, glyph.points[currentIndex].y};
      if (glyph.points[currentIndex].flag & FLAG_ONCURVE)
      {
        if (hasPending)
        {
          this->emitQuadratic(previous, pending, current);
          hasPending = false;
        }
        else
        {
          this->emitLine(previous, current);
        }
        previous = current;
      }
      else if (hasPending)
      {
        ttCoordinate_t midpoint = {(int16_t)((pending.x + current.x) / 2),
                                   (int16_t)((pending.y + current.y) / 2)};
        this->emitQuadratic(previous, pending, midpoint);
        previous = midpoint;
        pending = current;
      }
      else
      {
        pending = current;
        hasPending = true;
      }
      index++;
      if (index > last)
        index = pointStart;
    }
    if (hasPending)
      this->emitQuadratic(previous, pending, startPoint);
    else
      this->emitLine(previous, startPoint);
    this->addEndPoint(this->numPoints - 1);
    this->addBeginPoint(this->numPoints);
    pointStart = last + 1;
  }
  return;
}

/* Bresenham's line algorithm */
void truetypeClass::addLine(int16_t _x0, int16_t _y0, int16_t _x1, int16_t _y1)
{
  // Serial.printf("addLine(%3d, %3d) -> (%3d, %3d)\n", _x0, _y0, _x1, _y1);
  uint16_t dx = abs(_x1 - _x0);
  uint16_t dy = abs(_y1 - _y0);
  int16_t sx, sy, err, e2;

  if (_x0 < _x1)
  {
    sx = 1;
  }
  else
  {
    sx = -1;
  }
  if (_y0 < _y1)
  {
    sy = 1;
  }
  else
  {
    sy = -1;
  }
  err = dx - dy;

  while (1)
  {
    this->addPixel(_x0, _y0, this->colorLine);
    if ((_x0 == _x1) && (_y0 == _y1))
    {
      break;
    }
    e2 = 2 * err;
    if (e2 > -dy)
    {
      err -= dy;
      _x0 += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      _y0 += sy;
    }
  }
}

void truetypeClass::fillGlyph(int16_t _x_min, int16_t _y_min, uint16_t _width)
{
  if (this->numPoints == 0 || _width == 0)
    return;
  if (this->numPoints > this->edgeCrossesCapacity)
  {
    this->edgeCrosses = (ttEdgeCross_t *)realloc(this->edgeCrosses, sizeof(ttEdgeCross_t) * this->numPoints);
    this->edgeOrder = (uint16_t *)realloc(this->edgeOrder, sizeof(uint16_t) * this->numPoints);
    if (!this->edgeCrosses || !this->edgeOrder)
      return;
    this->edgeCrossesCapacity = this->numPoints;
  }
  if (this->displayWidth > this->fillCoverageCapacity)
  {
    this->fillCoverage = (uint8_t *)realloc(this->fillCoverage, this->displayWidth);
    if (!this->fillCoverage)
      return;
    this->fillCoverageCapacity = this->displayWidth;
  }
  int32_t xMinLimit = _x_min;
  int32_t xMaxLimit = (int32_t)_x_min + _width;
  int32_t coverageStart = xMinLimit > 0 ? xMinLimit : 0;
  int32_t coverageEnd = xMaxLimit < this->displayWidth ? xMaxLimit : this->displayWidth;
  if (coverageStart >= coverageEnd)
    return;

  uint16_t sortedEdgeCount = 0;
  uint16_t begin = 0;
  uint16_t end = 0;
  for (uint16_t i = 0; i < this->numPoints; i++)
  {
    uint16_t next = (i == this->endPoints[end]) ? this->beginPoints[begin] : (uint16_t)(i + 1);
    if (i == this->endPoints[end])
    {
      end++;
      begin++;
    }
    ttPixelPoint_t point1 = this->points[i];
    ttPixelPoint_t point2 = this->points[next];
    int32_t deltaY = point2.y - point1.y;
    if (deltaY == 0)
      continue;

    int32_t deltaX = point2.x - point1.x;
    int32_t startY = point1.y < point2.y ? point1.y : point2.y;
    ttEdgeCross_t edge;
    edge.startY = startY;
    edge.endY = point1.y > point2.y ? point1.y : point2.y;
    edge.dir = point1.y < point2.y ? 1 : -1;
    edge.step = (int32_t)(((int64_t)deltaX * 65536) / ((int64_t)deltaY * 4));
    edge.x = point1.x + (int32_t)(((int64_t)(startY - point1.y) * deltaX) / deltaY);
    this->edgeCrosses[i] = edge;

    uint16_t insert = sortedEdgeCount;
    while (insert > 0 && this->edgeCrosses[this->edgeOrder[insert - 1]].startY > startY)
    {
      this->edgeOrder[insert] = this->edgeOrder[insert - 1];
      insert--;
    }
    this->edgeOrder[insert] = i;
    sortedEdgeCount++;
  }

  uint16_t activeCount = 0;
  uint16_t nextEdge = 0;
  for (int16_t y = _y_min; y < (int16_t)(_y_min + this->characterSize); y++)
  {
    memset(&this->fillCoverage[coverageStart], 0, coverageEnd - coverageStart);
    for (uint8_t sample = 0; sample < 4; sample++)
    {
      int32_t sampleY = ((int32_t)y << 16) + sample * 16384 + 8192;
      while (nextEdge < sortedEdgeCount && this->edgeCrosses[this->edgeOrder[nextEdge]].startY <= sampleY)
      {
        uint16_t edgeIndex = this->edgeOrder[nextEdge++];
        ttEdgeCross_t *edge = &this->edgeCrosses[edgeIndex];
        if (edge->endY > sampleY)
        {
          edge->x += (int32_t)(((int64_t)(sampleY - edge->startY) * edge->step * 4) / 65536);
          uint16_t insert = activeCount;
          while (insert > 0 && this->edgeCrosses[this->edgeOrder[insert - 1]].x > edge->x)
          {
            this->edgeOrder[insert] = this->edgeOrder[insert - 1];
            insert--;
          }
          this->edgeOrder[insert] = edgeIndex;
          activeCount++;
        }
      }

      for (uint16_t i = 0; i < activeCount;)
      {
        if (this->edgeCrosses[this->edgeOrder[i]].endY <= sampleY)
        {
          for (uint16_t j = i + 1; j < activeCount; j++)
            this->edgeOrder[j - 1] = this->edgeOrder[j];
          activeCount--;
          continue;
        }
        i++;
      }

      int16_t winding = 0;
      int32_t spanStart = 0;
      for (uint16_t i = 0; i < activeCount; i++)
      {
        ttEdgeCross_t &cross = this->edgeCrosses[this->edgeOrder[i]];
        int16_t previous = winding;
        winding += cross.dir;
        if (previous == 0 && winding != 0)
          spanStart = cross.x;
        else if (previous != 0 && winding == 0)
          this->accumulateSpan(spanStart, cross.x, xMinLimit, xMaxLimit, this->fillCoverage);
        cross.x += cross.step;
      }
      for (uint16_t i = 1; i < activeCount; i++)
      {
        uint16_t order = this->edgeOrder[i];
        uint16_t insert = i;
        while (insert > 0 && this->edgeCrosses[this->edgeOrder[insert - 1]].x > this->edgeCrosses[order].x)
        {
          this->edgeOrder[insert] = this->edgeOrder[insert - 1];
          insert--;
        }
        this->edgeOrder[insert] = order;
      }
    }
    this->commitCoverage(y, this->fillCoverage, coverageStart, coverageEnd);
  }
  return;
}

void truetypeClass::addCoverage(uint8_t *_coverage, uint16_t _value)
{
  uint16_t total = (uint16_t)*_coverage + (uint16_t)((_value + 2) / 4);
  *_coverage = total > 255 ? 255 : (uint8_t)total;
}

void truetypeClass::accumulateSpan(int32_t _start, int32_t _end, int32_t _xMin, int32_t _xMax, uint8_t *_coverage)
{
  int64_t from = _start, to = _end;
  int64_t minX = (int64_t)_xMin << 16, maxX = (int64_t)_xMax << 16;
  if (from < minX)
    from = minX;
  if (to > maxX)
    to = maxX;
  if (from < 0)
    from = 0;
  if (to > ((int64_t)this->displayWidth << 16))
    to = (int64_t)this->displayWidth << 16;
  if (from >= to)
    return;
  int32_t first = from >> 16, last = (to - 1) >> 16;
  if (first == last)
  {
    this->addCoverage(&_coverage[first], (uint16_t)(((to - from) * 255 + 32768) >> 16));
    return;
  }
  this->addCoverage(&_coverage[first], (uint16_t)(((((int64_t)(first + 1) << 16) - from) * 255 + 32768) >> 16));
  for (int32_t x = first + 1; x < last; x++)
    this->addCoverage(&_coverage[x], 255);
  this->addCoverage(&_coverage[last], (uint16_t)(((to - ((int64_t)last << 16)) * 255 + 32768) >> 16));
}

void truetypeClass::commitCoverage(int16_t _y, uint8_t *_coverage, int32_t _xStart, int32_t _xEnd)
{
  for (int32_t x = _xStart; x < _xEnd; x++)
  {
    if (_coverage[x] == 0)
      continue;
    if (this->framebufferBit == 8 && !this->framebufferDirection)
    {
      this->addPixel(x, _y, this->blendCoverage(this->getPixel(x, _y), this->colorInside, _coverage[x]));
    }
    else if (_coverage[x] >= 128)
    {
      this->addPixel(x, _y, this->colorInside);
    }
  }
}

uint8_t truetypeClass::blendCoverage(uint8_t _background, uint8_t _foreground, uint8_t _coverage)
{
  uint16_t inverse = 255U - _coverage;
  uint16_t red = (((_foreground >> 5) & 0x07U) * _coverage + ((_background >> 5) & 0x07U) * inverse + 127U) / 255U;
  uint16_t green = (((_foreground >> 2) & 0x07U) * _coverage + ((_background >> 2) & 0x07U) * inverse + 127U) / 255U;
  uint16_t blue = ((_foreground & 0x03U) * _coverage + (_background & 0x03U) * inverse + 127U) / 255U;
  return (uint8_t)((red << 5) | (green << 2) | blue);
}

uint8_t truetypeClass::getPixel(int16_t _x, int16_t _y)
{
  if (!this->userFrameBuffer)
    return 0;
  uint16_t temp = _x;
  switch (this->stringRotation)
  {
  case ROTATE_270:
    _x = _y;
    _y = this->displayHeight - 1 - temp;
    break;
  case ROTATE_180:
    _x = this->displayWidth - 1 - _x;
    _y = this->displayHeight - 1 - _y;
    break;
  case ROTATE_90:
    _x = this->displayWidth - 1 - _y;
    _y = temp;
    break;
  default:
    break;
  }
  if (!this->userFrameBuffer || _x < 0 || _y < 0 || (uint16_t)_x >= this->displayWidth || (uint16_t)_y >= this->displayHeight)
    return 0;
  return this->userFrameBuffer[(uint16_t)_x + (uint16_t)_y * this->displayWidthFrame];
}

void truetypeClass::emitLine(ttCoordinate_t _p0, ttCoordinate_t _p1)
{
  this->addLineFixed(((int32_t)this->outlineOriginX << 16) + ((int32_t)_p0.x - this->glyph.xMin) * this->outlineScaleX,
                     ((int32_t)this->outlineOriginY << 16) - ((int32_t)_p0.y - this->yMin) * this->outlineScaleY,
                     ((int32_t)this->outlineOriginX << 16) + ((int32_t)_p1.x - this->glyph.xMin) * this->outlineScaleX,
                     ((int32_t)this->outlineOriginY << 16) - ((int32_t)_p1.y - this->yMin) * this->outlineScaleY);
}

void truetypeClass::addLineFixed(int32_t _x0, int32_t _y0, int32_t _x1, int32_t _y1)
{
  if (this->numPoints == 0)
  {
    this->addPoint(_x0, _y0);
    this->addBeginPoint(0);
  }
  this->addPoint(_x1, _y1);
  if (!this->fillInside || this->colorLine != this->colorInside)
  {
    this->addLine(_x0 >> 16, _y0 >> 16, _x1 >> 16, _y1 >> 16);
  }
}

void truetypeClass::emitQuadratic(ttCoordinate_t _p0, ttCoordinate_t _control, ttCoordinate_t _p1)
{
  const int64_t flatnessDelta = 1LL << 16;
  const uint8_t maxDepth = 4;
  uint8_t minimumDepth;
  if (this->characterSize <= 8)
  {
    minimumDepth = 4;
  }
  else if (this->characterSize <= 12)
  {
    minimumDepth = 3;
  }
  else if (this->characterSize <= 24)
  {
    minimumDepth = 2;
  }
  else
  {
    minimumDepth = 1;
  }

  ttQuadraticSegment_t stack[8];
  uint8_t stackCount = 1;
  stack[0] = {_p0, _control, _p1, 0};
  while (stackCount > 0)
  {
    ttQuadraticSegment_t segment = stack[--stackCount];
    int32_t secondX = (int32_t)segment.p0.x - 2 * (int32_t)segment.control.x + segment.p1.x;
    int32_t secondY = (int32_t)segment.p0.y - 2 * (int32_t)segment.control.y + segment.p1.y;
    int64_t scaledX = (int64_t)secondX * this->outlineScaleX;
    int64_t scaledY = (int64_t)secondY * this->outlineScaleY;
    bool flatEnough = scaledX * scaledX + scaledY * scaledY <= flatnessDelta * flatnessDelta;
    if ((segment.depth >= minimumDepth && flatEnough) || segment.depth >= maxDepth)
    {
      this->emitLine(segment.p0, segment.p1);
      continue;
    }

    ttCoordinate_t q0 = {(int16_t)(((int32_t)segment.p0.x + segment.control.x) / 2),
                         (int16_t)(((int32_t)segment.p0.y + segment.control.y) / 2)};
    ttCoordinate_t q1 = {(int16_t)(((int32_t)segment.control.x + segment.p1.x) / 2),
                         (int16_t)(((int32_t)segment.control.y + segment.p1.y) / 2)};
    ttCoordinate_t midpoint = {(int16_t)(((int32_t)q0.x + q1.x) / 2),
                               (int16_t)(((int32_t)q0.y + q1.y) / 2)};
    if (stackCount + 2 > (uint8_t)(sizeof(stack) / sizeof(stack[0])))
    {
      this->emitLine(segment.p0, segment.p1);
      continue;
    }
    stack[stackCount++] = {midpoint, q1, segment.p1, (uint8_t)(segment.depth + 1)};
    stack[stackCount++] = {segment.p0, q0, midpoint, (uint8_t)(segment.depth + 1)};
  }
}

void truetypeClass::textDraw(int16_t _x, int16_t _y, const wchar_t _character[])
{
  uint16_t c = 0;
  uint16_t prev_code = 0;

  while (_character[c] != '\0')
  {
    // space (half-width, full-width)
    if ((_character[c] == ' ') || (_character[c] == L'　'))
    {
      prev_code = 0;
      _x += this->characterSize / 4;
      c++;
      continue;
    }
    // Serial.printf("%c\n", _character[c]);

    this->charCode = this->codeToGlyphId(_character[c]);
    // Serial.printf("code:%4d\n", this->charCode);
    this->readGlyph(this->charCode);
    // Serial.println(glyph.numberOfContours);

    _x += this->characterSpace;
    if (prev_code != 0 && this->kerningOn)
    {
      int16_t kern = this->getKerning(prev_code, this->charCode); // space between charctor
      _x += this->fontUnitsToPixels(kern);
    }
    prev_code = this->charCode;

    ttHMetric_t hMetric = getHMetric(this->charCode);
    uint16_t width = (uint16_t)this->fontUnitsToPixels(glyph.xMax - glyph.xMin);

    // Line breaks when reaching the edge of the display
    if ((hMetric.leftSideBearing + width + _x) > this->end_x)
    {
      _x = this->start_x;
      _y += this->characterSize;
      if (_y > this->end_y)
      {
        break;
      }
    }

    // Line breaks with line feed code
    if (_character[c] == '\n')
    {
      _x = this->start_x;
      _y += this->characterSize;
      if (_y > this->end_y)
      {
        break;
      }
      continue;
    }

    if (glyph.numberOfContours >= 0)
    {
      // write framebuffer
      this->generateOutline(hMetric.leftSideBearing + _x, _y, width);

      if (this->fillInside)
      {
        this->fillGlyph(hMetric.leftSideBearing + _x, _y, width);
      }
    }
    this->freePointsAll();
    this->freeGlyph();

    // Serial.println("---done");
    _x += (hMetric.advanceWidth) ? (hMetric.advanceWidth) : (width);
    c++;
  }
}

void truetypeClass::textDraw(int16_t _x, int16_t _y, const char _character[])
{
  if (!_character)
    return;
  uint16_t length = 0;
  while (_character[length] != '\0')
  {
    length++;
  }
  wchar_t *wcharacter = (wchar_t *)calloc(sizeof(wchar_t), length + 1);
  if (!wcharacter)
    return;
  for (uint16_t i = 0; i < length; i++)
  {
    wcharacter[i] = _character[i];
  }
  this->textDraw(_x, _y, wcharacter);
  free(wcharacter);
}

void truetypeClass::textDraw(int16_t _x, int16_t _y, const String _string)
{
  uint16_t length = _string.length();
  wchar_t *wcharacter = (wchar_t *)calloc(sizeof(wchar_t), length + 1);
  if (!wcharacter)
    return;
  this->stringToWchar(_string, wcharacter);
  this->textDraw(_x, _y, wcharacter);
  free(wcharacter);
}

void truetypeClass::addPixel(int16_t _x, int16_t _y, uint8_t _colorCode)
{
  // Serial.printf("addPix(%3d, %3d)\n", _x, _y);
  uint8_t *buf_ptr;

  // limit to boundary co-ordinates the boundary is always in the same orientation as the string not the buffer
  if ((_x < this->start_x) || (_x >= this->end_x) || (_y >= this->end_y))
  {
    return;
  }

  // Rotate co-ordinates relative to the buffer
  uint16_t temp = _x;
  switch (this->stringRotation)
  {
  case ROTATE_270:
    _x = _y;
    _y = this->displayHeight - 1 - temp;
    break;
  case ROTATE_180:
    _x = this->displayWidth - 1 - _x;
    _y = this->displayHeight - 1 - _y;
    break;
  case ROTATE_90:
    _x = this->displayWidth - 1 - _y;
    _y = temp;
    break;
  case 0:
  default:
    break;
  }

  // out of range
  if ((_x < 0) || ((uint16_t)_x >= this->displayWidth) || ((uint16_t)_y >= this->displayHeight) || (_y < 0))
  {
    return;
  }

  if (this->framebufferDirection)
  {
    // Framebuffer bit direction: Vertical
  }
  else
  {
    // Framebuffer bit direction: Horizontal
    switch (this->framebufferBit)
    {
    case 8: // 8bit Horizontal
    {
      this->userFrameBuffer[(uint16_t)_x + (uint16_t)_y * this->displayWidthFrame] = _colorCode;
    }
    break;
    case 4: // 4bit Horizontal
    {
      buf_ptr = &this->userFrameBuffer[((uint16_t)_x / 2) + (uint16_t)_y * this->displayWidthFrame];
      _colorCode = _colorCode & 0b00001111;

      if ((uint16_t)_x % 2)
      {
        *buf_ptr = (*buf_ptr & 0b00001111) + (_colorCode << 4);
      }
      else
      {
        *buf_ptr = (*buf_ptr & 0b11110000) + _colorCode;
      }
    }
    break;
    case 1: // 1bit Horizontal
    default:
    {
      buf_ptr = &this->userFrameBuffer[((uint16_t)_x / 8) + (uint16_t)_y * this->displayWidthFrame];
      uint8_t bitMask = 0b10000000 >> ((uint16_t)_x % 8);
      uint8_t bit = (_colorCode) ? (bitMask) : (0b00000000);
      *buf_ptr = (*buf_ptr & ~bitMask) + bit;
    }
    break;
    }
  }
  return;
}

uint16_t truetypeClass::getStringWidth(const wchar_t _character[])
{
  uint16_t prev_code = 0;
  uint16_t c = 0;
  uint16_t output = 0;

  while (_character[c] != '\0')
  {
    // space (half-width, full-width)
    if ((_character[c] == ' ') || (_character[c] == L'　'))
    {
      prev_code = 0;
      output += this->characterSize / 4;
      c++;
      continue;
    }
    uint16_t code = this->codeToGlyphId(_character[c]);
    this->readGlyph(code, 1);

    output += this->characterSpace;
    if (prev_code != 0 && this->kerningOn)
    {
      int16_t kern = this->getKerning(prev_code, code); // space between charctor
      output += this->fontUnitsToPixels(kern);
    }
    prev_code = code;

    ttHMetric_t hMetric = getHMetric(code);
    uint16_t width = (uint16_t)this->fontUnitsToPixels(glyph.xMax - glyph.xMin);
    output += (hMetric.advanceWidth) ? (hMetric.advanceWidth) : (width);
    c++;
  }

  return output;
}

uint16_t truetypeClass::getStringWidth(const char _character[])
{
  if (!_character)
    return 0;
  uint16_t length = 0;
  uint16_t output = 0;
  while (_character[length] != '\0')
  {
    length++;
  }
  wchar_t *wcharacter = (wchar_t *)calloc(sizeof(wchar_t), length + 1);
  if (!wcharacter)
    return 0;
  for (uint16_t i = 0; i < length; i++)
  {
    wcharacter[i] = _character[i];
  }
  output = this->getStringWidth(wcharacter);
  free(wcharacter);
  return output;
}

uint16_t truetypeClass::getStringWidth(const String _string)
{
  uint16_t length = _string.length();
  uint16_t output = 0;

  wchar_t *wcharacter = (wchar_t *)calloc(sizeof(wchar_t), length + 1);
  if (!wcharacter)
    return 0;
  this->stringToWchar(_string, wcharacter);

  output = this->getStringWidth(wcharacter);
  free(wcharacter);
  return output;
}

/* Points*/
void truetypeClass::addPoint(int32_t _x, int32_t _y)
{
  if (this->numPoints >= this->pointsCapacity)
  {
    uint16_t newCapacity = this->pointsCapacity ? (uint16_t)(this->pointsCapacity * 2) : 16;
    ttPixelPoint_t *tmp = (ttPixelPoint_t *)realloc(this->points, sizeof(ttPixelPoint_t) * newCapacity);
    if (!tmp)
      return;
    this->points = tmp;
    this->pointsCapacity = newCapacity;
  }
  this->numPoints++;
  this->points[(this->numPoints - 1)].x = _x;
  this->points[(this->numPoints - 1)].y = _y;
}

void truetypeClass::addBeginPoint(uint16_t _bp)
{
  if (this->numBeginPoints >= this->beginPointsCapacity)
  {
    uint16_t newCapacity = this->beginPointsCapacity ? (uint16_t)(this->beginPointsCapacity * 2) : 8;
    uint16_t *tmp = (uint16_t *)realloc(this->beginPoints, sizeof(uint16_t) * newCapacity);
    if (!tmp)
      return;
    this->beginPoints = tmp;
    this->beginPointsCapacity = newCapacity;
  }
  this->numBeginPoints++;
  this->beginPoints[(this->numBeginPoints - 1)] = _bp;
}

void truetypeClass::addEndPoint(uint16_t _ep)
{
  if (this->numEndPoints >= this->endPointsCapacity)
  {
    uint16_t newCapacity = this->endPointsCapacity ? (uint16_t)(this->endPointsCapacity * 2) : 8;
    uint16_t *tmp = (uint16_t *)realloc(this->endPoints, sizeof(uint16_t) * newCapacity);
    if (!tmp)
      return;
    this->endPoints = tmp;
    this->endPointsCapacity = newCapacity;
  }
  this->numEndPoints++;
  this->endPoints[(this->numEndPoints - 1)] = _ep;
}

void truetypeClass::freePointsAll()
{
  this->freePoints();
  this->freeBeginPoints();
  this->freeEndPoints();
}

void truetypeClass::freePoints()
{
  this->numPoints = 0;
}

void truetypeClass::freeBeginPoints()
{
  this->numBeginPoints = 0;
}

void truetypeClass::freeEndPoints()
{
  this->numEndPoints = 0;
}

/* file */
/* seek to the first position of the specified table name */
uint32_t truetypeClass::seekToTable(const char *name)
{
  for (uint32_t i = 0; i < this->numTables; i++)
  {
    if (strcmp(table[i].name, name) == 0)
    {
      this->seekFile(table[i].offset);
      return table[i].offset;
    }
  }
  return 0;
}

void truetypeClass::seekFile(uint32_t _offset)
{
  this->filePosition = _offset;
}

uint32_t truetypeClass::tellFile() const
{
  return this->filePosition;
}

void truetypeClass::readFile(uint8_t *_destination, uint16_t _length)
{
#ifdef ENABLE_INTEGRATED_TTF
  if (this->useIntegratedTtf)
  {
    if (this->filePosition <= integrated_ttf_size && _length <= integrated_ttf_size - this->filePosition)
    {
      memcpy(_destination, &integrated_ttf[this->filePosition], _length);
    }
    else
    {
      memset(_destination, 0, _length);
    }
    this->filePosition += _length;
    return;
  }
#endif

  uint32_t position = this->filePosition;
  if (this->cacheValid && position >= this->cacheBase && position + _length <= this->cacheBase + this->cacheLength)
  {
    memcpy(_destination, this->cacheBuffer + position - this->cacheBase, _length);
    this->filePosition += _length;
    return;
  }
  if (_length <= TT_CACHE_SIZE)
  {
    this->cacheBase = position;
    this->file.seek(position);
    this->cacheLength = this->file.read(this->cacheBuffer, TT_CACHE_SIZE);
    this->cacheValid = this->cacheLength != 0;
    if (_length <= this->cacheLength)
    {
      memcpy(_destination, this->cacheBuffer, _length);
      this->filePosition += _length;
      return;
    }
  }
  this->cacheValid = false;
  this->file.seek(position);
  size_t bytesRead = this->file.read(_destination, _length);
  if (bytesRead < _length)
  {
    memset(_destination + bytesRead, 0, _length - bytesRead);
  }
  this->filePosition += _length;
}

/* calculate */
void truetypeClass::stringToWchar(String _string, wchar_t _charctor[])
{
  uint16_t s = 0;
  uint16_t c = 0;
  uint32_t codeu32;

  while (_string[s] != '\0')
  {
    int numBytes = GetU8ByteCount(_string[s]);
    switch (numBytes)
    {
    case 1:
      codeu32 = char32_t(uint8_t(_string[s]));
      s++;
      break;
    case 2:
      if (!IsU8LaterByte(_string[s + 1]))
      {
        s++;
        continue;
      }
      if ((uint8_t(_string[s]) & 0x1E) == 0)
      {
        s++;
        continue;
      }

      codeu32 = char32_t(_string[s] & 0x1F) << 6;
      codeu32 |= char32_t(_string[s + 1] & 0x3F);
      s += 2;
      break;
    case 3:
      if (!IsU8LaterByte(_string[s + 1]) || !IsU8LaterByte(_string[s + 2]))
      {
        s++;
        continue;
      }
      if ((uint8_t(_string[s]) & 0x0F) == 0 &&
          (uint8_t(_string[s + 1]) & 0x20) == 0)
      {
        s++;
        continue;
      }

      codeu32 = char32_t(_string[s] & 0x0F) << 12;
      codeu32 |= char32_t(_string[s + 1] & 0x3F) << 6;
      codeu32 |= char32_t(_string[s + 2] & 0x3F);
      s += 3;
      break;
    case 4:
      if (!IsU8LaterByte(_string[s + 1]) || !IsU8LaterByte(_string[s + 2]) ||
          !IsU8LaterByte(_string[s + 3]))
      {
        s++;
        continue;
      }
      if ((uint8_t(_string[s]) & 0x07) == 0 &&
          (uint8_t(_string[s + 1]) & 0x30) == 0)
      {
        s++;
        continue;
      }

      codeu32 = char32_t(_string[s] & 0x07) << 18;
      codeu32 |= char32_t(_string[s + 1] & 0x3F) << 12;
      codeu32 |= char32_t(_string[s + 2] & 0x3F) << 6;
      codeu32 |= char32_t(_string[s + 3] & 0x3F);
      s += 4;
      break;
    default:
      s++;
      continue;
    }

    if (codeu32 > 0x10FFFF)
    {
      continue;
    }

    if (codeu32 < 0x10000)
    {
      _charctor[c] = char16_t(codeu32);
    }
    else
    {
      uint32_t surrogate = codeu32 - 0x10000;
      _charctor[c] = (wchar_t)(0xD800 + (surrogate / 0x400));
      c++;
      _charctor[c] = (wchar_t)(0xDC00 + (surrogate % 0x400));
    }
    c++;
  }
  _charctor[c] = 0;
}

uint8_t truetypeClass::GetU8ByteCount(char _ch)
{
  if (0 <= uint8_t(_ch) && uint8_t(_ch) < 0x80)
  {
    return 1;
  }
  if (0xC2 <= uint8_t(_ch) && uint8_t(_ch) < 0xE0)
  {
    return 2;
  }
  if (0xE0 <= uint8_t(_ch) && uint8_t(_ch) < 0xF0)
  {
    return 3;
  }
  if (0xF0 <= uint8_t(_ch) && uint8_t(_ch) < 0xF8)
  {
    return 4;
  }
  return 0;
}

bool truetypeClass::IsU8LaterByte(char _ch)
{
  return 0x80 <= uint8_t(_ch) && uint8_t(_ch) < 0xC0;
}

/* get uint8_t at the current position */
uint8_t truetypeClass::getUInt8t()
{
  uint8_t x;

  this->readFile(&x, 1);
  return x;
}

/* get int16_t at the current position */
int16_t truetypeClass::getInt16t()
{
  byte x[2];

  this->readFile(x, 2);
  return (int16_t)((uint16_t)x[0] << 8 | x[1]);
}

/* get uint16_t at the current position */
uint16_t truetypeClass::getUInt16t()
{
  byte x[2];

  this->readFile(x, 2);
  return (x[0] << 8) | x[1];
}

/* get uint32_t at the current position */
uint32_t truetypeClass::getUInt32t()
{
  byte x[4];

  this->readFile(x, 4);
  return (x[0] << 24) | (x[1] << 16) | (x[2] << 8) | x[3];
}
