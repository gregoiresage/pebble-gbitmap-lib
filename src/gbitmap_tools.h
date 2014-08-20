#pragma once

#include "pebble.h"

//! Creates a new resized GBitmap on the heap based on an existing GBitmap.
//! The resulting GBitmap must be destroyed using gbitmap_destroy.
//! @param src The source bitmap to be resized
//! @param ratio_width_percent The width resize ratio (between 0 and 100)
//! @param ratio_height_percent The height resize ratio (between 0 and 100)
//! @return A pointer to the GBitmap. `NULL` if the GBitmap could not be created
GBitmap* scaleBitmap(GBitmap* src, uint8_t ratio_width_percent, uint8_t ratio_height_percent);

//TODO improve the poor documentation of this function (my english is so bad...)
//! Creates a intermediate bitmap between source and dest.
//! The size of source and dest must be equals (width/height/row_size_bytes).
//! @param source The source bitmap to be resized
//! @param dest The final bitmap
//! @param result The intermediate bitmap to compute
//! @param remainingSteps The number of steps remaining until the result equals dest 
void computeMorphingBitmap(GBitmap* source, GBitmap* dest, GBitmap* result, uint8_t remainingSteps) ;