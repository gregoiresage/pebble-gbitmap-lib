#pragma once

#include <pebble.h>

typedef struct {
	Layer       *layer;

	Animation* animation;
	AnimationImplementation animImpl;

	GBitmap* up_image;
	GBitmap* down_image;
	GBitmap* up_anim_image;
	GBitmap* down_anim_image;
	GBitmap* anim_resized_image;
	uint8_t *resized_data;

	uint8_t anim_image_y;
	bool isAnimating;
	
	uint8_t next_Digit;
	uint8_t current_Digit;
} FlipLayer;

FlipLayer* flip_layer_create(GRect frame);

void flip_layer_destroy(FlipLayer *flip_layer);

Layer* flip_layer_get_layer(FlipLayer *flip_layer);

void flip_layer_animate_to(FlipLayer *text_layer, uint8_t next_value);

