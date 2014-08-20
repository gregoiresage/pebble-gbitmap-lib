#include "pebble.h"

/*
* Algorithm from http://www.compuphase.com/graphic/scale.htm and adapted for Pebble GBitmap
*/

static void scaleRow(uint8_t *target, uint8_t *source, int srcWidth, int tgtWidth, int srcOrigX)
{
  int tgtPixels = 0;
  int intPart = srcWidth / tgtWidth;
  int fractPart = srcWidth % tgtWidth;
  int E = 0;
  int srcIndex = srcOrigX % 8;

  source += srcOrigX / 8;

  while (tgtPixels < tgtWidth) {
    *target  |= ((*source >> srcIndex) & 1) << (tgtPixels % 8);
    srcIndex += intPart;
    
    E += fractPart;
    if (E >= tgtWidth) {
      E -= tgtWidth;
      srcIndex++;
    } 

    if(srcIndex >= 8){
      source += srcIndex / 8;
      srcIndex = srcIndex % 8;
    }

    tgtPixels++;
    if(tgtPixels % 8 == 0){
     target++;
    }
  } 
}

GBitmap* scaleBitmap(GBitmap* src, uint8_t ratio_width_percent, uint8_t ratio_height_percent){

  GBitmap* tgt = NULL;
  if(ratio_width_percent <= 100 && ratio_height_percent <= 100){

    int srcHeight = src->bounds.size.h;
    int srcWidth = src->bounds.size.w;
    int tgtHeight = srcHeight * ratio_height_percent / 100;
    int tgtWidth = srcWidth * ratio_width_percent / 100;
    
    tgt = gbitmap_create_blank((GSize){tgtWidth, tgtHeight});

    if(tgt == NULL)
      return NULL;

    // tgt->bounds = src->bounds;
    // tgt->bounds.origin.x = 0;
    // tgt->bounds.origin.y = 0;
    // tgt->bounds.size.h = src->bounds.size.h * ratio_height_percent / 100;
    // tgt->bounds.size.w = src->bounds.size.w * ratio_width_percent / 100;
    // tgt->row_size_bytes = tgt->bounds.size.w % 32 == 0 ? tgt->bounds.size.w / 4 : 4 * (tgt->bounds.size.w / 32) + 4;
    // tgt->addr = resized_data;
    // memset(tgt->addr, 0, (tgt->bounds.size.h * tgt->row_size_bytes) * sizeof(uint8_t));

    if(tgtHeight != 0 && tgtWidth != 0){

      int NumPixels = tgtHeight;
      int intPart = (srcHeight / tgtHeight) * src->row_size_bytes;
      int fractPart = srcHeight % tgtHeight;
      int E = 0;
      uint8_t *source = src->addr + src->bounds.origin.y * src->row_size_bytes;
      uint8_t *target = tgt->addr;

      while (NumPixels-- > 0) {
        scaleRow(target, source, srcWidth, tgtWidth, src->bounds.origin.x);
        target += tgt->row_size_bytes;
        source += intPart;
        E += fractPart;
        if (E >= tgtHeight) {
          E -= tgtHeight;
          source += src->row_size_bytes;;
        } 
      } 
    }
  }

  return tgt;
}





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