#include <pebble.h>
#include "flip_layer.h"

static Window *window;
#define LAYER_NUMBER 6
static FlipLayer *layer_number[LAYER_NUMBER];
static FlipLayer *layer_month;

#define NUMBER_IMAGE_COUNT 10

int NUMBER_IMAGE_RESOURCE_UP_IDS[NUMBER_IMAGE_COUNT] = {
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

int NUMBER_IMAGE_RESOURCE_DOWN_IDS[NUMBER_IMAGE_COUNT] = {
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

#define MONTH_IMAGE_COUNT 12

int MONTH_IMAGE_RESOURCE_UP_IDS[MONTH_IMAGE_COUNT] = {
  RESOURCE_ID_IMAGE_JAN_UP,
  RESOURCE_ID_IMAGE_FEB_UP,
  RESOURCE_ID_IMAGE_MAR_UP,
  RESOURCE_ID_IMAGE_APR_UP,
  RESOURCE_ID_IMAGE_MAY_UP,
  RESOURCE_ID_IMAGE_JUN_UP,
  RESOURCE_ID_IMAGE_JUL_UP,
  RESOURCE_ID_IMAGE_AUG_UP,
  RESOURCE_ID_IMAGE_SEP_UP,
  RESOURCE_ID_IMAGE_OCT_UP,
  RESOURCE_ID_IMAGE_NOV_UP,
  RESOURCE_ID_IMAGE_DEC_UP,
};

int MONTH_IMAGE_RESOURCE_DOWN_IDS[MONTH_IMAGE_COUNT] = {
  RESOURCE_ID_IMAGE_JAN_DOWN,
  RESOURCE_ID_IMAGE_FEB_DOWN,
  RESOURCE_ID_IMAGE_MAR_DOWN,
  RESOURCE_ID_IMAGE_APR_DOWN,
  RESOURCE_ID_IMAGE_MAY_DOWN,
  RESOURCE_ID_IMAGE_JUN_DOWN,
  RESOURCE_ID_IMAGE_JUL_DOWN,
  RESOURCE_ID_IMAGE_AUG_DOWN,
  RESOURCE_ID_IMAGE_SEP_DOWN,
  RESOURCE_ID_IMAGE_OCT_DOWN,
  RESOURCE_ID_IMAGE_NOV_DOWN,
  RESOURCE_ID_IMAGE_DEC_DOWN,
};


static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  flip_layer_animate_to(layer_number[0], tick_time->tm_mday / 10);
  flip_layer_animate_to(layer_number[1], tick_time->tm_mday % 10);
  flip_layer_animate_to(layer_number[2], tick_time->tm_hour / 10);
  flip_layer_animate_to(layer_number[3], tick_time->tm_hour % 10);
  flip_layer_animate_to(layer_number[4], tick_time->tm_min / 10);
  flip_layer_animate_to(layer_number[5], tick_time->tm_min % 10);

  // flip_layer_animate_to(layer_month, tick_time->tm_mon);
}

static void window_appear(Window *window) {
  for(int i=0; i<LAYER_NUMBER; i++){
    flip_layer_set_images(layer_number[i], NUMBER_IMAGE_RESOURCE_UP_IDS, NUMBER_IMAGE_RESOURCE_DOWN_IDS, NUMBER_IMAGE_COUNT);
  }
  flip_layer_set_images(layer_month, MONTH_IMAGE_RESOURCE_UP_IDS, MONTH_IMAGE_RESOURCE_DOWN_IDS, MONTH_IMAGE_COUNT);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  for(int i=0; i<2; i++){
    layer_number[i] = flip_layer_create(GRect(144/4 * i, 0, 144/4, 168/2 - 2));
  }
  for(int i=2; i<6; i++){
    layer_number[i] = flip_layer_create(GRect(144/4 * (i-2), 168/2 + 2, 144/4, 168/2 - 2));
  }
  for(int i=0; i<LAYER_NUMBER; i++){
    // flip_layer_set_images(layer_number[i], NUMBER_IMAGE_RESOURCE_UP_IDS, NUMBER_IMAGE_RESOURCE_DOWN_IDS, NUMBER_IMAGE_COUNT);
    layer_add_child(window_layer, flip_layer_get_layer(layer_number[i]));
  }
  layer_month = flip_layer_create(GRect(144/2, 0, 144/2, 168/2 - 2)); 
  // flip_layer_set_images(layer_month, MONTH_IMAGE_RESOURCE_UP_IDS, MONTH_IMAGE_RESOURCE_DOWN_IDS, MONTH_IMAGE_COUNT);
  layer_add_child(window_layer, flip_layer_get_layer(layer_month));
  APP_LOG(0,"window_load");
}

static void window_unload(Window *window) {
  // for(int i=0; i<LAYER_NUMBER; i++){
  //   flip_layer_destroy(layer_number[i]);
  // }
  // flip_layer_destroy(layer_month);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .appear = window_appear,
    .unload = window_unload,
  });
  window_set_background_color(window, GColorBlack);
  window_stack_push(window, true);

  
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
