#include <pebble.h>

#include "flip_layer.h"
#include <pebble-gbitmap-lib/gbitmap_tools.h>

static void layer_update_callback(Layer *me, GContext* ctx) {
	FlipLayer* flip_layer = *(FlipLayer**)(layer_get_data(me));
	GRect layer_bounds = layer_get_bounds(me);

	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, layer_bounds, 7, GCornersAll);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_round_rect(ctx, layer_bounds, 7);

	graphics_context_set_compositing_mode(ctx,GCompOpAssign);

	if(flip_layer->up_image){
		GRect bounds = gbitmap_get_bounds(flip_layer->up_image);
		GPoint origin;
		origin.x = (layer_bounds.size.w - bounds.size.w) / 2;
		origin.y = (layer_bounds.size.h - 2*bounds.size.h) / 2;
		graphics_draw_bitmap_in_rect(ctx, flip_layer->up_image, (GRect) { .origin = origin, .size = bounds.size });
	}
	if(flip_layer->down_image){
		GRect bounds = gbitmap_get_bounds(flip_layer->down_image);
		GPoint origin;
		origin.x = (layer_bounds.size.w - bounds.size.w) / 2;
		origin.y = layer_bounds.size.h / 2;
		graphics_draw_bitmap_in_rect(ctx, flip_layer->down_image, (GRect) { .origin = origin, .size = bounds.size });
	}

	// graphics_context_set_compositing_mode(ctx,GCompOpAssign);

	if(flip_layer->anim_resized_image){
		GRect bounds = gbitmap_get_bounds(flip_layer->anim_resized_image);
		GPoint origin;
		origin.x = (layer_bounds.size.w - bounds.size.w) / 2;
		origin.y = flip_layer->anim_image_y;
		graphics_fill_rect(ctx, (GRect) { .origin = origin, .size = bounds.size }, 0, GCornersAll);
		graphics_draw_bitmap_in_rect(ctx, flip_layer->anim_resized_image, (GRect) { .origin = origin, .size = bounds.size });
		graphics_draw_rect(ctx, (GRect) { .origin = { 0, flip_layer->anim_image_y }, .size = { layer_bounds.size.w, bounds.size.h } });
	}
	
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, GRect(0,layer_bounds.size.h/2 - 1,layer_bounds.size.w,3), 0, 0);

	graphics_fill_rect(ctx, GRect(2,layer_bounds.size.h/2-4,2,9), 0, GCornersAll);
	graphics_fill_rect(ctx, GRect(layer_bounds.size.w - 2 - 2,layer_bounds.size.h/2-4,2,9), 0, GCornersAll);
}

Layer* flip_layer_get_layer(FlipLayer *flip_layer){
	return flip_layer->layer;
}

static void animationUpdate(Animation *animation, const AnimationProgress progress) {
	FlipLayer *flip_layer = (FlipLayer *)animation_get_context(animation);

	GRect layer_bounds = layer_get_bounds(flip_layer->layer);

	int percent = progress * 100 / ANIMATION_NORMALIZED_MAX;
	if(percent < 50){
		if(flip_layer->anim_resized_image){
			gbitmap_destroy(flip_layer->anim_resized_image);
			flip_layer->anim_resized_image = NULL;
		}
		flip_layer->anim_resized_image = scaleBitmap(flip_layer->up_anim_image, 100, 100 - 2 * percent);
		if(flip_layer->anim_resized_image){
			GRect bounds = gbitmap_get_bounds(flip_layer->anim_resized_image);
			flip_layer->anim_image_y = layer_bounds.size.h/2 - bounds.size.h;
		}
	}
	else {
		if(flip_layer->anim_resized_image){
			gbitmap_destroy(flip_layer->anim_resized_image);
			flip_layer->anim_resized_image = NULL;
		}
		flip_layer->anim_resized_image = scaleBitmap(flip_layer->down_anim_image, 100, 2 * (percent - 50));
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

	if(flip_layer->nb_of_images > 0){
		flip_layer->up_image = gbitmap_create_with_resource(flip_layer->up_images[flip_layer->next_Digit]);
		flip_layer->down_image = gbitmap_create_with_resource(flip_layer->down_images[flip_layer->current_Digit]);
	
		flip_layer->up_anim_image = gbitmap_create_with_resource(flip_layer->up_images[flip_layer->current_Digit]);
		flip_layer->down_anim_image = gbitmap_create_with_resource(flip_layer->down_images[flip_layer->next_Digit]);
	
		layer_mark_dirty(flip_layer->layer);
	}
}

void animation_stopped(Animation *animation, bool finished, void *data) {
	FlipLayer *flip_layer = (FlipLayer *)data;
	flip_layer->current_Digit = flip_layer->next_Digit;

	if(flip_layer->anim_resized_image){
		gbitmap_destroy(flip_layer->anim_resized_image);
		flip_layer->anim_resized_image = NULL;
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

	flip_layer->down_image = gbitmap_create_with_resource(flip_layer->down_images[flip_layer->current_Digit]);

	flip_layer->isAnimating = false;

	layer_mark_dirty(flip_layer->layer);
}

void flip_layer_animate_to(FlipLayer *flip_layer, uint8_t next_value){
	if(!flip_layer->isAnimating && flip_layer->next_Digit != (next_value % flip_layer->nb_of_images)){
		flip_layer->next_Digit = next_value % flip_layer->nb_of_images;
		animation_schedule(flip_layer->animation);
	}
}

FlipLayer* flip_layer_create(GRect frame){
	FlipLayer* flip_layer = malloc(sizeof(FlipLayer));

	flip_layer->layer = layer_create_with_data(frame, sizeof(FlipLayer*));
	layer_set_update_proc(flip_layer->layer, layer_update_callback);
	memcpy(layer_get_data(flip_layer->layer), &flip_layer, sizeof(FlipLayer*));

	flip_layer->animation = animation_create();
	static const AnimationImplementation implementation = {
	  .update = animationUpdate
	};
	animation_set_handlers(flip_layer->animation, (AnimationHandlers) {
		.started = (AnimationStartedHandler) animation_started,
		.stopped = (AnimationStoppedHandler) animation_stopped,
	}, flip_layer);
	animation_set_duration(flip_layer->animation, 1400);
	animation_set_implementation(flip_layer->animation, &implementation);

	flip_layer->current_Digit = 0;
	flip_layer->up_image = NULL;
	flip_layer->down_image = NULL;
	flip_layer->up_anim_image = NULL;
	flip_layer->down_anim_image = NULL;

	flip_layer->up_images = NULL;
	flip_layer->down_images = NULL;
	flip_layer->nb_of_images = 0;

	return flip_layer;
}

void flip_layer_destroy(FlipLayer *flip_layer){
	if(flip_layer->animation){
		animation_unschedule(flip_layer->animation);
		// animation_destroy(flip_layer->animation);
	}
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
	free(flip_layer);
}

void flip_layer_set_images(FlipLayer *flip_layer, int *up_images, int *down_images, int nb_of_images){
	flip_layer->up_images 		= up_images;
	flip_layer->down_images 	= down_images;
	flip_layer->nb_of_images 	= nb_of_images;

	if(nb_of_images > 0){
		flip_layer->up_image = gbitmap_create_with_resource(up_images[0]);
		flip_layer->down_image = gbitmap_create_with_resource(down_images[0]);
	}
}
