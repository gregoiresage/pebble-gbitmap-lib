#pragma once

#include "pebble.h"

//! Creates a new resized GBitmap on the heap based on an existing GBitmap.
//! The resulting GBitmap must be destroyed using gbitmap_destroy.
//! @param src The source bitmap to be resized
//! @param ratio_width_percent The width resize ratio
//! @param ratio_height_percent The height resize ratio
//! @return A pointer to the GBitmap. `NULL` if the GBitmap could not be created
GBitmap* scaleBitmap(GBitmap* src, uint16_t ratio_width_percent, uint16_t ratio_height_percent);
