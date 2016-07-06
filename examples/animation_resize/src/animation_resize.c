#include "pebble.h"
#include <pebble-gbitmap-lib/gbitmap_tools.h>

static Window *window = NULL;
static Layer *layer   = NULL;

static GBitmap *image         = NULL;
static GBitmap *resized_image = NULL;

static AnimationImplementation upAnimImpl;
static AnimationImplementation downAnimImpl;

#define NB_OF_IMAGES 5
static uint32_t IMAGE_REOURCE_ID[NB_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_CAT_PBI,
  RESOURCE_ID_IMAGE_CAT_1,
  RESOURCE_ID_IMAGE_CAT_2,
  RESOURCE_ID_IMAGE_CAT_4,
  RESOURCE_ID_IMAGE_CAT_8,
};

static void do_resize(int percent){
  if(resized_image){
    gbitmap_destroy(resized_image);
  }
  resized_image = scaleBitmap(image, percent, percent);
  layer_mark_dirty(layer);
}

static void upAnimationUpdate(struct Animation *animation, const AnimationProgress progress){
  int percent = progress * 200 / ANIMATION_NORMALIZED_MAX;
  do_resize(percent);
}

static void downAnimationUpdate(struct Animation *animation, const AnimationProgress progress){
  int percent = 200 - progress * 200 / ANIMATION_NORMALIZED_MAX;
  do_resize(percent);
}

static void layer_update_callback(Layer *me, GContext* ctx) {
  if(resized_image)
  {
    GRect bounds = gbitmap_get_bounds(resized_image);
    GRect layer_bounds = layer_get_bounds(me);
    graphics_draw_bitmap_in_rect(ctx, resized_image, (GRect){.origin=(GPoint){(layer_bounds.size.w - bounds.size.w)/2,(layer_bounds.size.h - bounds.size.h)/2},.size=bounds.size});
  }
}

static void animationStartedHandler(Animation *animation, void *context){
  int index = (uint32_t)context;
  if(image){
    gbitmap_destroy(image);
  }
  image = gbitmap_create_with_resource(IMAGE_REOURCE_ID[index]);
}

int main(void) {

  window = window_create();
  window_stack_push(window, true);

  // Init the layer for display the image
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_callback);
  layer_add_child(window_layer, layer);

  upAnimImpl.update = upAnimationUpdate;
  downAnimImpl.update = downAnimationUpdate;

  Animation **arr = (Animation**)malloc(2 * NB_OF_IMAGES * sizeof(Animation*));
  for(uint8_t i=0; i<NB_OF_IMAGES; i++){
    Animation *animation=animation_create();
    animation_set_duration(animation, 3000);
    animation_set_curve(animation, AnimationCurveEaseIn);
    animation_set_implementation(animation, &upAnimImpl);
    animation_set_handlers(animation,(AnimationHandlers){
      .started=animationStartedHandler
    }, (void*)(uint32_t)i);
    arr[2*i] = animation;

    animation=animation_create();
    animation_set_duration(animation, 2000);
    animation_set_curve(animation, AnimationCurveEaseOut);
    animation_set_implementation(animation, &downAnimImpl);
    arr[2*i+1] = animation;
  }

  // Create the sequence, set to loop forever
  Animation *sequence = animation_sequence_create_from_array(arr, 2 * NB_OF_IMAGES);
  animation_set_play_count(sequence, ANIMATION_DURATION_INFINITE);

  // Play the sequence
  animation_schedule(sequence);

  app_event_loop();

  if(image)
    gbitmap_destroy(image);
  if(resized_image)
    gbitmap_destroy(resized_image);
  
  window_destroy(window);
  layer_destroy(layer);

  for(uint8_t i=0; i<NB_OF_IMAGES; i++){
    animation_destroy(arr[2*i]);
    animation_destroy(arr[2*i+1]);
  }

  animation_destroy(sequence);
  
  // Destroy the array
  free(arr);
}
