#include <pebble.h>

#include "gbitmap_tools.h"

#define NUMBER_OF_IMAGES_IN_ANIMATION 10

static Window *window;
static Layer *layer;

static AppTimer *timer;

static bool isAnimating = false;
static int currentAnimationStep = NUMBER_OF_IMAGES_IN_ANIMATION;

static GBitmap* from_image;
static GBitmap* dest_image;
static GBitmap* tmp_image;
static uint8_t* from_addr;
static GRect displayBounds;

static void timer_callback(void *data) {
  currentAnimationStep--;

  if(currentAnimationStep < 0){
    isAnimating = false;
    from_image->addr = from_addr;
    currentAnimationStep = NUMBER_OF_IMAGES_IN_ANIMATION;
  }
  else {
    time_t now = time(NULL);
    uint16_t ms = 0;
    time_ms (&now,&ms); 
    uint32_t totaltime = now * 1000 + ms;
    computeMorphingBitmap(from_image, dest_image, tmp_image, currentAnimationStep);
    time_ms (&now,&ms); 
    totaltime = now * 1000 + ms - totaltime;
    APP_LOG(APP_LOG_LEVEL_INFO, "Image %d/%d computed in %ld ms", (NUMBER_OF_IMAGES_IN_ANIMATION-currentAnimationStep), NUMBER_OF_IMAGES_IN_ANIMATION, totaltime);
    uint8_t* addr = from_image->addr;
    from_image->addr = tmp_image->addr;
    tmp_image->addr = addr;
  }
  layer_mark_dirty(layer);
}

static void layer_update_callback(Layer *me, GContext* ctx) {
  if(isAnimating){
    graphics_draw_bitmap_in_rect(ctx, tmp_image, displayBounds);
  }
  else {
    graphics_draw_bitmap_in_rect(ctx, from_image, displayBounds);
  }

  if(isAnimating){
    timer = app_timer_register(50 /* milliseconds */, timer_callback, NULL);
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(currentAnimationStep != NUMBER_OF_IMAGES_IN_ANIMATION)
    return;

  if(from_image){
    from_image->addr = from_addr;
    gbitmap_destroy(from_image);
  }
  from_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_START);
  from_addr = from_image->addr;

  isAnimating = true;
  timer = app_timer_register(100 /* milliseconds */, timer_callback, NULL);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_callback);
  layer_add_child(window_layer, layer);

  from_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_START);
  from_addr = from_image->addr;
  dest_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_END);
  tmp_image = malloc(sizeof(GBitmap));
  memcpy(tmp_image, dest_image, sizeof(GBitmap));
  tmp_image->addr = malloc(sizeof(uint8_t) * dest_image->row_size_bytes * dest_image->bounds.size.h);

  displayBounds = GRect(
    (bounds.size.w - from_image->bounds.size.w)/2,
    (bounds.size.h - from_image->bounds.size.h)/2,
    from_image->bounds.size.w, 
    from_image->bounds.size.h);
}

static void window_unload(Window *window) {
  layer_destroy(layer);
  if(from_image){
    from_image->addr = from_addr;
    gbitmap_destroy(from_image);
  }
  gbitmap_destroy(dest_image);
  free(tmp_image->addr);
  free(tmp_image);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}