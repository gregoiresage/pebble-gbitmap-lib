#include <pebble.h>
#include "flip_layer.h"

static Window *window;
#define LAYER_NUMBER 4
static FlipLayer *layer[LAYER_NUMBER];


static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  flip_layer_animate_to(layer[0], tick_time->tm_hour / 10);
  flip_layer_animate_to(layer[1], tick_time->tm_hour % 10);
  flip_layer_animate_to(layer[2], tick_time->tm_min / 10);
  flip_layer_animate_to(layer[3], tick_time->tm_min % 10);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  for(int i=0; i<LAYER_NUMBER; i++){
    layer[i] = flip_layer_create(GRect(36 * i, (168-58)/2, 36, 58));
    layer_add_child(window_layer, flip_layer_get_layer(layer[i]));
  }
}

static void window_unload(Window *window) {
  for(int i=0; i<LAYER_NUMBER; i++){
    flip_layer_destroy(layer[i]);
  }
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_background_color(window, GColorBlack);
  window_stack_push(window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
