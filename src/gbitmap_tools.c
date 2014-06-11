#include "pebble.h"

/*
* Algorithm from http://www.compuphase.com/graphic/scale.htm and adapted for Pebble GBitmap
*/

static void scaleRow(uint8_t *target, uint8_t *source, int srcWidth, int tgtWidth)
{
  int tgtPixels = 0;
  int intPart = srcWidth / tgtWidth;
  int fractPart = srcWidth % tgtWidth;
  int E = 0;
  int srcIndex = 0;

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

GBitmap* scaleBitmap(GBitmap* src, uint8_t ratio_width_percent, uint8_t ratio_height_percent, uint8_t *resized_data){

  GBitmap* tgt = NULL;
  if(ratio_width_percent <= 100 && ratio_height_percent <= 100){
    tgt = malloc(sizeof(GBitmap));

    tgt->row_size_bytes = src->row_size_bytes;
    tgt->bounds = src->bounds;
    tgt->bounds.size.h = tgt->bounds.size.h * ratio_height_percent / 100;
    tgt->bounds.size.w = tgt->bounds.size.w * ratio_width_percent / 100;
    tgt->addr = resized_data;
    memset(tgt->addr, 0, (tgt->bounds.size.h * tgt->row_size_bytes) * sizeof(uint8_t));

    int srcHeight = src->bounds.size.h;
    int srcWidth = src->bounds.size.w;

    int tgtHeight = tgt->bounds.size.h;
    int tgtWidth = srcWidth * ratio_width_percent / 100;

    if(tgtHeight != 0 && tgtWidth != 0){

      int NumPixels = tgtHeight;
      int intPart = (srcHeight / tgtHeight) * src->row_size_bytes;
      int fractPart = srcHeight % tgtHeight;
      int E = 0;
      uint8_t *source = src->addr;
      uint8_t *target = tgt->addr;

      while (NumPixels-- > 0) {
        scaleRow(target, source, srcWidth, tgtWidth);
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