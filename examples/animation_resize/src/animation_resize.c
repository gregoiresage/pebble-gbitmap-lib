#include "pebble.h"
#include "gbitmap_tools.h"

static Window *window;
static Layer *layer;

static GBitmap *image;
static GBitmap *resized_image;

static Animation* animation;
static AnimationImplementation animImpl;


void animationUpdate(struct Animation *animation, const uint32_t time_normalized){
  int percent = time_normalized * 100 / ANIMATION_NORMALIZED_MAX;
  if(percent > 0){
    if(resized_image){
      gbitmap_destroy(resized_image);
    }
    resized_image = scaleBitmap(image, percent, percent);
    layer_mark_dirty(layer);
  }
}

static void layer_update_callback(Layer *me, GContext* ctx) {
  if(resized_image){
    GRect bounds = resized_image->bounds;
    graphics_draw_bitmap_in_rect(ctx, resized_image, (GRect) { .origin = { 5, 5 }, .size = bounds.size });
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(!animation_is_scheduled(animation)){
    animation_schedule(animation);
  }
}

static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

int main(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_click_config_provider(window, config_provider);

  // Init the layer for display the image
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_callback);
  layer_add_child(window_layer, layer);

  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NO_LITTER);
  resized_image = NULL;

  animation = animation_create();
  animImpl.update = animationUpdate;
  animation_set_duration(animation, 1000);
  animation_set_curve(animation, AnimationCurveEaseInOut);
  animation_set_implementation(animation, &animImpl);

  app_event_loop();

  animation_destroy(animation);

  gbitmap_destroy(image);
  if(resized_image){
    gbitmap_destroy(resized_image);
  }

  window_destroy(window);
  layer_destroy(layer);
}
