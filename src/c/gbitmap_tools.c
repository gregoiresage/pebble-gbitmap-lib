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

  uint8_t mask = ((~0) & 0xFF) >> (8 - num_bits_per_pixel);

  while (tgtPixels < tgtWidth) {
    uint8_t srcVal = (*source >> (8 - num_bits_per_pixel * (1 + bitIndexInByte))) & mask;
    *target |= srcVal << (8 - num_bits_per_pixel * (1 + tgtPixels % nb_pixels_per_byte));

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
    *target |= ((*source >> bitIndexInByte) & 1) << tgtPixels % 8;

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

static uint8_t getNumColorsForFormat(GBitmapFormat format){
  switch (format) {
    case GBitmapFormat1BitPalette:  return 2;
#ifdef PBL_COLOR
    case GBitmapFormat2BitPalette:  return 4;
    case GBitmapFormat4BitPalette:  return 16;
#endif
    default: return 0;
  }
}

static uint8_t getNumBitsPerPixelForFormat(GBitmapFormat format){
  switch (format) {
    case GBitmapFormat1Bit: 
    case GBitmapFormat1BitPalette: return 1;
#ifdef PBL_COLOR
    case GBitmapFormat2BitPalette: return 2;
    case GBitmapFormat4BitPalette: return 4;
    case GBitmapFormat8Bit:        return 8;
#endif
    default: return 0;
  }
}

GBitmap* scaleBitmap(GBitmap* src, uint16_t ratio_width_percent, uint16_t ratio_height_percent){

  GBitmap* tgt = NULL;
  if(ratio_width_percent > 0 && ratio_height_percent > 0){

    GRect src_bounds = gbitmap_get_bounds(src);
    uint16_t src_bytes_per_row = gbitmap_get_bytes_per_row(src);

    uint16_t srcHeight = src_bounds.size.h;
    uint16_t srcWidth = src_bounds.size.w;
    uint16_t tgtHeight = srcHeight * ratio_height_percent / 100;
    uint16_t tgtWidth = srcWidth * ratio_width_percent / 100;

    GBitmapFormat format = gbitmap_get_format(src);
    uint8_t num_colors = getNumColorsForFormat(format);
    if(num_colors > 0){
      GColor *palette = gbitmap_get_palette(src);
      GColor *palette_copy = malloc(sizeof(GColor) * num_colors);
      memcpy(palette_copy, palette, sizeof(GColor) * num_colors);
      tgt = gbitmap_create_blank_with_palette((GSize){tgtWidth, tgtHeight}, format, palette_copy, true);
    }
    else {
      tgt = gbitmap_create_blank((GSize){tgtWidth, tgtHeight}, format);
    }

    if(tgt == NULL)
      return NULL;

    if(tgtHeight && tgtWidth){
      uint16_t target_bytes_per_row = gbitmap_get_bytes_per_row(tgt);
      uint8_t num_bits_per_pixel = getNumBitsPerPixelForFormat(format);

      int NumPixels = tgtHeight;
      int intPart = (srcHeight / tgtHeight) * src_bytes_per_row;
      int fractPart = srcHeight % tgtHeight;
      int E = 0;
      uint8_t *source = gbitmap_get_data(src) + src_bounds.origin.y * src_bytes_per_row;
      uint8_t *target = gbitmap_get_data(tgt);

      while (NumPixels-- > 0) {
        if(format == GBitmapFormat1Bit){
          scaleRowPBI(target, source, srcWidth, tgtWidth, src_bounds.origin.x);
        }
        else {
          scaleRow(target, source, srcWidth, tgtWidth, src_bounds.origin.x, num_bits_per_pixel);
        }
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
