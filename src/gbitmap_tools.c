#include "pebble.h"

/*
* Algorithm from http://www.compuphase.com/graphic/scale.htm and adapted for Pebble GBitmap
*/

// This is the scaling algorithm for palettized format
// GBitmapFormat1BitPalette pixels are stored like that : 0 1 2 3 4 5 6 7   8 9 10 11 12 13 14 15 .... (1 bit = 1 pixel)
// GBitmapFormat2BitPalette pixels are stored like that : 0 0 1 1 2 2 3 3   4 4 5 5 6 6 7 7 .... (2 bit = 1 pixel)
// GBitmapFormat4BitPalette pixels are stored like that : 0 0 0 0 1 1 1 1   2 2 2 2 3 3 3 3 .... (4 bit = 1 pixel)
// GBitmapFormat8Bit pixels are stored like that :        0 0 0 0 0 0 0 0   1 1 1 1 1 1 1 1 .... (8 bit = 1 pixel)
static void scaleRow(uint8_t *target, uint8_t *source, int srcWidth, int tgtWidth, int srcOrigX, uint8_t num_bits_per_pixel)
{
  int tgtPixels = 0;
  int intPart = srcWidth / tgtWidth;
  int fractPart = srcWidth % tgtWidth;
  int E = 0;

  uint8_t nb_pixels_per_byte = 8 / num_bits_per_pixel;
  int bitIndexInByte = srcOrigX % nb_pixels_per_byte;

  source += srcOrigX / nb_pixels_per_byte;

  uint8_t mask = (~0) >> (8 - num_bits_per_pixel);

  while (tgtPixels < tgtWidth) {
    uint8_t srcVal = (*source >> (8 - num_bits_per_pixel * (1 + bitIndexInByte))) & mask;
    *target  |= srcVal << (8 - num_bits_per_pixel * (1 + tgtPixels % nb_pixels_per_byte));

    bitIndexInByte += intPart;
    
    E += fractPart;
    if (E >= tgtWidth) {
      E -= tgtWidth;
      bitIndexInByte ++;
    } 

    if(bitIndexInByte >= nb_pixels_per_byte){
      source += bitIndexInByte / nb_pixels_per_byte;
      bitIndexInByte = bitIndexInByte % nb_pixels_per_byte;
    }

    tgtPixels++;
    if((tgtPixels * num_bits_per_pixel) % 8 == 0){
     target++;
    }
  } 
}

// This is the scaling algorithm for PBI format (1 bit = 1 pixel)
// Pixels are stored like that : 7 6 5 4 3 2 1 0  15 14 13 12 11 10 9 8 ....
static void scaleRowPBI(uint8_t *target, uint8_t *source, int srcWidth, int tgtWidth, int srcOrigX)
{
  int tgtPixels = 0;
  int intPart = srcWidth / tgtWidth;
  int fractPart = srcWidth % tgtWidth;
  int E = 0;

  int bitIndexInByte = srcOrigX % 8;

  source += srcOrigX / 8;

  while (tgtPixels < tgtWidth) {
    *target  |= ((*source >> bitIndexInByte) & 1) << tgtPixels % 8;

    bitIndexInByte += intPart;
    
    E += fractPart;
    if (E >= tgtWidth) {
      E -= tgtWidth;
      bitIndexInByte ++;
    } 

    if(bitIndexInByte >= 8){
      source += bitIndexInByte / 8;
      bitIndexInByte = bitIndexInByte % 8;
    }

    tgtPixels++;
    if(tgtPixels % 8 == 0){
     target++;
    }
  } 
}

#ifdef PBL_SDK_3
static uint8_t getNumColorsForFormat(GBitmapFormat format){
  switch (format) {
    case GBitmapFormat1BitPalette:  return 2;
    case GBitmapFormat2BitPalette:  return 4;
    case GBitmapFormat4BitPalette:  return 16;      
    default: return 0;
  }
}

static uint8_t getNumBitsPerPixelForFormat(GBitmapFormat format){
  switch (format) {
    case GBitmapFormat1Bit: 
    case GBitmapFormat1BitPalette: return 1;
    case GBitmapFormat2BitPalette: return 2;
    case GBitmapFormat4BitPalette: return 4;
    case GBitmapFormat8Bit:        return 8;
    default: return 0;
  }
}
#endif 

GBitmap* scaleBitmap(GBitmap* src, uint8_t ratio_width_percent, uint8_t ratio_height_percent){

  GBitmap* tgt = NULL;
  if(ratio_width_percent <= 100 && ratio_height_percent <= 100){

    GRect src_bounds = gbitmap_get_bounds(src);
    uint16_t src_bytes_per_row = gbitmap_get_bytes_per_row(src);

    int srcHeight = src_bounds.size.h;
    int srcWidth = src_bounds.size.w;
    int tgtHeight = srcHeight * ratio_height_percent / 100;
    int tgtWidth = srcWidth * ratio_width_percent / 100;

#ifdef PBL_SDK_2
    tgt = __gbitmap_create_blank((GSize){tgtWidth, tgtHeight});
#elif defined(PBL_SDK_3)
    GColor *palette = gbitmap_get_palette(src);
    GBitmapFormat format = gbitmap_get_format(src);
    uint8_t num_colors = getNumColorsForFormat(format);
    GColor *palette_copy = NULL;
    if(num_colors > 0){
      palette_copy = malloc(sizeof(GColor) * num_colors);
      memcpy(palette_copy, palette, sizeof(GColor) * num_colors);
    }
    tgt = gbitmap_create_blank_with_palette((GSize){tgtWidth, tgtHeight}, format, palette_copy, true);
#endif 

    if(tgt == NULL)
      return NULL;

    uint16_t target_bytes_per_row = gbitmap_get_bytes_per_row(tgt);
#ifdef PBL_SDK_3
    uint8_t num_bits_per_pixel = 1;
    num_bits_per_pixel = getNumBitsPerPixelForFormat(format);
#endif

    if(tgtHeight != 0 && tgtWidth != 0){

      int NumPixels = tgtHeight;
      int intPart = (srcHeight / tgtHeight) * src_bytes_per_row;
      int fractPart = srcHeight % tgtHeight;
      int E = 0;
      uint8_t *source = gbitmap_get_data(src) + src_bounds.origin.y * src_bytes_per_row;
      uint8_t *target = gbitmap_get_data(tgt);

      while (NumPixels-- > 0) {
#ifdef PBL_SDK_2
        scaleRowPBI(target, source, srcWidth, tgtWidth, src_bounds.origin.x);
#else 
        if(format == GBitmapFormat1Bit){
          scaleRowPBI(target, source, srcWidth, tgtWidth, src_bounds.origin.x);
        }
        else {
          scaleRow(target, source, srcWidth, tgtWidth, src_bounds.origin.x, num_bits_per_pixel);
        }
        
#endif
        target += target_bytes_per_row;
        source += intPart;
        E += fractPart;
        if (E >= tgtHeight) {
          E -= tgtHeight;
          source += src_bytes_per_row;
        } 
      } 
    }
  }

  return tgt;
}


#ifdef PBL_SDK_2

#define setBlackPixel(bmp, x, y) ((((uint8_t *)bmp->addr)[(y) * bmp->row_size_bytes + (x) / 8] &= ~(0x01 << ((x)%8))))
#define setWhitePixel(bmp, x, y) ((((uint8_t *)bmp->addr)[y * bmp->row_size_bytes + x / 8] |= (0x01 << (x%8))))
#define getPixel(bmp, x, y)\
(((x) >= bmp->bounds.size.w || (y) >= bmp->bounds.size.h || (x) < 0 || (y) < 0) ?\
 -1 :\
 ((((uint8_t *)bmp->addr)[(y)*bmp->row_size_bytes + (x)/8] & (1<<((x)%8))) != 0))


// findNearestPixel : algorithm adapted from Jnm code : https://github.com/Jnmattern/Minimalist_2.0/blob/master/src/bitmap.h#L336
static inline GPoint findNearestPixel(GBitmap* bmp, int px, int py, GColor color) {
  int maximum_radius_square = bmp->bounds.size.h * bmp->bounds.size.h + bmp->bounds.size.w * bmp->bounds.size.w;
  int radius = 1;

  while(radius * radius < maximum_radius_square){
    int x = 0, y = radius, d = radius-1;
    while (y >= x) {
      if (getPixel(bmp, px+x, py+y) == color) return GPoint(px+x, py+y);
      if (getPixel(bmp, px+y, py+x) == color) return GPoint(px+y, py+x);
      if (getPixel(bmp, px-x, py+y) == color) return GPoint(px-x, py+y);
      if (getPixel(bmp, px-y, py+x) == color) return GPoint(px-y, py+x);
      if (getPixel(bmp, px+x, py-y) == color) return GPoint(px+x, py-y);
      if (getPixel(bmp, px+y, py-x) == color) return GPoint(px+y, py-x);
      if (getPixel(bmp, px-x, py-y) == color) return GPoint(px-x, py-y);
      if (getPixel(bmp, px-y, py-x) == color) return GPoint(px-y, py-x);

      if (d >= 2*x-2) {
        d = d-2*x;
        x++;
      } else if (d <= 2*radius - 2*y) {
        d = d+2*y-1;
        y--;
      } else {
        d = d + 2*y - 2*x - 2;
        y--;
        x++;
      }
    }
    radius++;
  }
  return GPoint(0, 0);
}

#define signum(x) (x < 0 ? -1 : 1)

void computeMorphingBitmap(GBitmap* source, GBitmap* dest, GBitmap* result, uint8_t remainingSteps) {
  if(source->row_size_bytes != dest->row_size_bytes || source->bounds.size.w != dest->bounds.size.w || source->bounds.size.h != dest->bounds.size.h) {
    return;
  }

  memset(result->addr, 0xFF, sizeof(uint8_t) * source->row_size_bytes * source->bounds.size.h);

  GColor sourceColor, destColor;
  int diffX,diffY;

  int width   = source->bounds.size.w;
  int height  = source->bounds.size.h;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      sourceColor = getPixel(source,  x, y);
      destColor   = getPixel(dest,  x, y);
      if (sourceColor == GColorBlack && destColor == GColorBlack) {
        setBlackPixel(result, x, y);
      } else if (sourceColor == GColorBlack) {
        GPoint p = findNearestPixel(dest, x, y, GColorBlack);
        diffX = (p.x - x) / remainingSteps;
        diffY = (p.y - y) / remainingSteps;
        if(diffX == 0 && diffY == 0){
          if(abs(p.x - x) > abs(p.y - y)){
            diffX = signum(p.x - x);
          }
          else {
            diffY = signum(p.y - y);
          }
        }
        setBlackPixel(result, x + diffX, y + diffY);
      } else if (destColor == GColorBlack) {
        GPoint p = findNearestPixel(source, x, y, GColorBlack);
        diffX = (x - p.x) / remainingSteps;
        diffY = (y - p.y) / remainingSteps;
        if(diffX == 0 && diffY == 0){
          if(abs(p.x - x) > abs(p.y - y)){
            diffX = signum(x - p.x);
          }
          else {
            diffY = signum(y - p.y);
          }
        }
        setBlackPixel(result, p.x + diffX, p.y + diffY);
      }
    }
  }
}

#endif
