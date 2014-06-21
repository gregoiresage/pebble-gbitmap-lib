#include <pebble.h>

#include "flip_layer.h"
#include "gbitmap_tools.h"

#define NUMBER_OF_IMAGES 10

const int IMAGE_RESOURCE_UP_IDS[NUMBER_OF_IMAGES] = {
	RESOURCE_ID_IMAGE_0_UP,
	RESOURCE_ID_IMAGE_1_UP,
	RESOURCE_ID_IMAGE_2_UP,
	RESOURCE_ID_IMAGE_3_UP,
	RESOURCE_ID_IMAGE_4_UP,
	RESOURCE_ID_IMAGE_5_UP,
	RESOURCE_ID_IMAGE_6_UP,
	RESOURCE_ID_IMAGE_7_UP,
	RESOURCE_ID_IMAGE_8_UP,
	RESOURCE_ID_IMAGE_9_UP,
};

const int IMAGE_RESOURCE_DOWN_IDS[NUMBER_OF_IMAGES] = {
	RESOURCE_ID_IMAGE_0_DOWN,
	RESOURCE_ID_IMAGE_1_DOWN,
	RESOURCE_ID_IMAGE_2_DOWN,
	RESOURCE_ID_IMAGE_3_DOWN,
	RESOURCE_ID_IMAGE_4_DOWN,
	RESOURCE_ID_IMAGE_5_DOWN,
	RESOURCE_ID_IMAGE_6_DOWN,
	RESOURCE_ID_IMAGE_7_DOWN,
	RESOURCE_ID_IMAGE_8_DOWN,
	RESOURCE_ID_IMAGE_9_DOWN,
};

static void layer_update_callback(Layer *me, GContext* ctx) {
	graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
	
	FlipLayer* flip_layer = *(FlipLayer**)(layer_get_data(me));
	GRect layer_bounds = layer_get_bounds(me);
	if(flip_layer->up_image){
		GRect bounds = flip_layer->up_image->bounds;
		GPoint origin;
		origin.x = (layer_bounds.size.w - bounds.size.w) / 2;
		origin.y = (layer_bounds.size.h - 2*bounds.size.h) / 2;
		graphics_draw_bitmap_in_rect(ctx, flip_layer->up_image, (GRect) { .origin = origin, .size = bounds.size });
	}
	if(flip_layer->down_image){
		GRect bounds = flip_layer->down_image->bounds;
		GPoint origin;
		origin.x = (layer_bounds.size.w - bounds.size.w) / 2;
		origin.y = layer_bounds.size.h / 2;
		graphics_draw_bitmap_in_rect(ctx, flip_layer->down_image, (GRect) { .origin = origin, .size = bounds.size });
	}
	if(flip_layer->anim_resized_image){
		GRect bounds = flip_layer->anim_resized_image->bounds;
		GPoint origin;
		origin.x = (layer_bounds.size.w - bounds.size.w) / 2;
		origin.y = flip_layer->anim_image_y;
		graphics_draw_bitmap_in_rect(ctx, flip_layer->anim_resized_image, (GRect) { .origin = origin, .size = bounds.size });
		graphics_draw_rect(ctx, (GRect) { .origin = { 0, flip_layer->anim_image_y }, .size = { layer_bounds.size.w, bounds.size.h } });
	}
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_round_rect(ctx, GRect(0,0,layer_bounds.size.w,layer_bounds.size.h), 7);
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, GPoint(1, layer_bounds.size.h/2 - 1), GPoint(layer_bounds.size.w-1, layer_bounds.size.h/2 - 1));
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_line(ctx, GPoint(1, layer_bounds.size.h/2), GPoint(layer_bounds.size.w-1, layer_bounds.size.h/2));
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, GPoint(1, layer_bounds.size.h/2 + 1), GPoint(layer_bounds.size.w-1, layer_bounds.size.h/2 + 1));
}

Layer* flip_layer_get_layer(FlipLayer *flip_layer){
	return flip_layer->layer;
}

void animationUpdate(struct Animation *animation, const uint32_t time_normalized){
	FlipLayer *flip_layer = (FlipLayer *)animation_get_context(animation);

	GRect layer_bounds = layer_get_bounds(flip_layer->layer);

	int percent = time_normalized * 100 / ANIMATION_NORMALIZED_MAX;
	if(percent < 50){
		if(flip_layer->anim_resized_image){
			gbitmap_destroy(flip_layer->anim_resized_image);
			flip_layer->anim_resized_image = NULL;
		}
		flip_layer->anim_resized_image = scaleBitmap(flip_layer->up_anim_image, 100, 100 - 2 * percent, flip_layer->resized_data);
		GRect bounds = flip_layer->anim_resized_image->bounds;
		flip_layer->anim_image_y = layer_bounds.size.h/2 - bounds.size.h;
	}
	else {
		if(flip_layer->anim_resized_image){
			gbitmap_destroy(flip_layer->anim_resized_image);
			flip_layer->anim_resized_image = NULL;
		}
		flip_layer->anim_resized_image = scaleBitmap(flip_layer->down_anim_image, 100, 2 * (percent - 50), flip_layer->resized_data);
		flip_layer->anim_image_y = layer_bounds.size.h/2;
	}

	layer_mark_dirty(flip_layer->layer);
}

static void animation_started(Animation *animation, void *data) {
	FlipLayer *flip_layer = (FlipLayer *)data;
	if(flip_layer->up_image){
		gbitmap_destroy(flip_layer->up_image);
	}
	if(flip_layer->down_image){
		gbitmap_destroy(flip_layer->down_image);
	}
	if(flip_layer->up_anim_image){
		gbitmap_destroy(flip_layer->up_anim_image);
	}
	if(flip_layer->down_anim_image){
		gbitmap_destroy(flip_layer->down_anim_image);
	}

	flip_layer->up_image = gbitmap_create_with_resource(IMAGE_RESOURCE_UP_IDS[flip_layer->next_Digit]);
	flip_layer->down_image = gbitmap_create_with_resource(IMAGE_RESOURCE_DOWN_IDS[flip_layer->current_Digit]);

	flip_layer->up_anim_image = gbitmap_create_with_resource(IMAGE_RESOURCE_UP_IDS[flip_layer->current_Digit]);
	flip_layer->down_anim_image = gbitmap_create_with_resource(IMAGE_RESOURCE_DOWN_IDS[flip_layer->next_Digit]);

	layer_mark_dirty(flip_layer->layer);
}

void animation_stopped(Animation *animation, bool finished, void *data) {
	FlipLayer *flip_layer = (FlipLayer *)data;
	flip_layer->current_Digit = flip_layer->next_Digit;

	if(flip_layer->anim_resized_image){
	}
	if(flip_layer->up_anim_image){
		gbitmap_destroy(flip_layer->up_anim_image);
		flip_layer->up_anim_image = NULL;
	}
	if(flip_layer->down_anim_image){
		gbitmap_destroy(flip_layer->down_anim_image);
		flip_layer->down_anim_image = NULL;
	}
	if(flip_layer->anim_resized_image){
		gbitmap_destroy(flip_layer->anim_resized_image);
		flip_layer->anim_resized_image = NULL;
	}
	if(flip_layer->down_image){
		gbitmap_destroy(flip_layer->down_image);
	}

	flip_layer->down_image = gbitmap_create_with_resource(IMAGE_RESOURCE_DOWN_IDS[flip_layer->current_Digit]);

	flip_layer->isAnimating = false;

	layer_mark_dirty(flip_layer->layer);
}

void flip_layer_animate_to(FlipLayer *flip_layer, uint8_t next_value){
	if(!flip_layer->isAnimating && flip_layer->next_Digit != (next_value % 10)){
		flip_layer->next_Digit = next_value % 10;
		animation_schedule(flip_layer->animation);
	}
}

FlipLayer* flip_layer_create(GRect frame){
	FlipLayer* flip_layer = malloc(sizeof(FlipLayer));

	flip_layer->layer = layer_create_with_data(frame, sizeof(FlipLayer*));
	layer_set_update_proc(flip_layer->layer, layer_update_callback);
	memcpy(layer_get_data(flip_layer->layer), &flip_layer, sizeof(FlipLayer*));

	flip_layer->animation = animation_create();
	flip_layer->animImpl.update = animationUpdate;
	animation_set_handlers(flip_layer->animation, (AnimationHandlers) {
		.started = (AnimationStartedHandler) animation_started,
		.stopped = (AnimationStoppedHandler) animation_stopped,
	}, flip_layer);

	animation_set_duration(flip_layer->animation, 700);
	animation_set_implementation(flip_layer->animation, &(flip_layer->animImpl));

	flip_layer->resized_data = malloc((40* 40) * sizeof(uint8_t));

	flip_layer->current_Digit = 0;
	flip_layer->up_image = gbitmap_create_with_resource(IMAGE_RESOURCE_UP_IDS[flip_layer->current_Digit]);
	flip_layer->down_image = gbitmap_create_with_resource(IMAGE_RESOURCE_DOWN_IDS[flip_layer->current_Digit]);

	return flip_layer;
}

void flip_layer_destroy(FlipLayer *flip_layer){
	layer_destroy(flip_layer->layer);
	if(flip_layer->up_image){
		gbitmap_destroy(flip_layer->up_image);
	}
	if(flip_layer->down_image){
		gbitmap_destroy(flip_layer->down_image);
	}
	if(flip_layer->up_anim_image){
		gbitmap_destroy(flip_layer->up_anim_image);
	}
	if(flip_layer->down_anim_image){
		gbitmap_destroy(flip_layer->down_anim_image);
	}
	if(flip_layer->anim_resized_image){
		gbitmap_destroy(flip_layer->anim_resized_image);
	}
	if(flip_layer->animation){
		animation_destroy(flip_layer->animation);
	}

	free(flip_layer->resized_data);
	free(flip_layer);
}
