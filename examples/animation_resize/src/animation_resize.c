#include "pebble.h"
#include "gbitmap_tools.h"

static Window *window = NULL;
static Layer *layer    = NULL;

static GBitmap *image         = NULL;
static GBitmap *resized_image = NULL;

static Animation* animation = NULL;
static AnimationImplementation animImpl;

static int percent = 50;

#define NB_OF_IMAGES 5
static uint32_t IMAGE_REOURCE_ID[NB_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_CAT_PBI,
  RESOURCE_ID_IMAGE_CAT_2,
  RESOURCE_ID_IMAGE_CAT_4,
  RESOURCE_ID_IMAGE_CAT_16,
  RESOURCE_ID_IMAGE_CAT,
};

static uint8_t selected_image = 0;


static void do_resize(int percent){
  if(resized_image){
    gbitmap_destroy(resized_image);
  }
  resized_image = scaleBitmap(image, percent, percent);
  layer_mark_dirty(layer);
}

static void animationUpdate(struct Animation *animation, const AnimationProgress progress){
  int percent = progress * 100 / ANIMATION_NORMALIZED_MAX;
  do_resize(percent);
}

static void layer_update_callback(Layer *me, GContext* ctx) {
  if(resized_image){
    GRect bounds = gbitmap_get_bounds(resized_image);
    graphics_draw_bitmap_in_rect(ctx, resized_image, (GRect){.origin=(GPoint){(144 - bounds.size.w)/2,(168 - bounds.size.h)/2},.size=bounds.size});
  }

}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  animation = animation_create();
  animImpl.update = animationUpdate;
  animation_set_duration(animation, 1000);
  animation_set_curve(animation, AnimationCurveEaseInOut);
  animation_set_implementation(animation, &animImpl);
  animation_schedule(animation);

  // if(percent < 100)
  //   percent++;
  // do_resize(percent);
}

static void load_image(){
  percent = 50;
  if(image)
    gbitmap_destroy(image);
  image = gbitmap_create_with_resource(IMAGE_REOURCE_ID[selected_image]);
  do_resize(100);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(selected_image == 0){
    selected_image = NB_OF_IMAGES;
  }
  selected_image--;
  load_image();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  selected_image = (selected_image + 1) % NB_OF_IMAGES;
  load_image();
}

static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP,     up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN,   down_click_handler);
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

  load_image();

  app_event_loop();

  if(image)
    gbitmap_destroy(image);
  if(resized_image)
    gbitmap_destroy(resized_image);
  
  window_destroy(window);
  layer_destroy(layer);
}
