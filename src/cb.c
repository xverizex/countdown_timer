#include <obs/obs.h>
#include <obs/obs-module.h>
#include <obs/graphics/image-file.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include "log.h"

struct counter {
	int width;
	int height;

	gs_texture_t *bit[2];

	pthread_mutex_t lock;
	uint8_t *data[2];

	gs_image_file_t img;
};

OBS_MODULE_AUTHOR ("xverizex")

OBS_DECLARE_MODULE ()

static const char *cm_get_name (void *data);
static void *cm_create (obs_data_t *settings, obs_source_t *source);
static void cm_destroy (void *data);
static uint32_t cm_get_width (void *data);
static uint32_t cm_get_height (void *data);
static void cm_get_defaults (obs_data_t *settings);
static obs_properties_t *cm_props (void *data);
static void cm_update (void *data, obs_data_t *settings);
static void cm_video_tick (void *data, float seconds);
static void cm_video_render (void *data, gs_effect_t *effect);
static void cm_show (void *data);

static struct obs_source_info sinfo = {
	.id = "countdown_binary",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB,
	.get_name = cm_get_name,
	.create = cm_create,
	.destroy = cm_destroy,
	.get_properties = cm_props,
	.get_defaults = cm_get_defaults,
	.update = cm_update,
	.show = cm_show,
	.video_tick = cm_video_tick,
	.video_render = cm_video_render,
	.get_width = cm_get_width,
	.get_height = cm_get_height,
	.icon_type = OBS_ICON_TYPE_WINDOW_CAPTURE,
};

static const char *cm_get_name (void *data) {
	UNUSED_PARAMETER (data);
	return "countdown_timer";
}

const char *obs_module_name () {
	return "countdown_timer";
}

const char *obs_module_description () {
	return "set timer for display when you to start streaming";
}


bool obs_module_load () {
	obs_register_source (&sinfo);
	return true;
}

static void cm_show (void *data) {
}

static void *cm_create (obs_data_t *settings, obs_source_t *source) {
	struct counter *counter = calloc (1, sizeof (struct counter));

	pthread_mutex_init (&counter->lock, NULL);

	counter->width = 32;
	counter->height = 32;

	counter->data[0] = &bit_0[0];
	counter->data[1] = &bit_1[0];

#if 0
	obs_enter_graphics ();
	gs_image_file_init (&counter->img, "/home/cf/Pictures/binary_timer/countdown_1.png");
	gs_image_file_init_texture (&counter->img);
	obs_leave_graphics ();
#endif
#if 0
	obs_enter_graphics ();
	pthread_mutex_lock (&counter->lock);
	counter->bit[0] = gs_texture_create (32, 32, GS_RGBA, 1, (const uint8_t **) &counter->data[0], GS_RENDER_TARGET);
	counter->bit[1] = gs_texture_create (32, 32, GS_RGBA, 1, (const uint8_t **) &counter->data[1], GS_RENDER_TARGET);
	pthread_mutex_unlock (&counter->lock);
	obs_leave_graphics ();
#endif
	obs_enter_graphics ();
	counter->bit[0] = gs_texture_create_from_file ("/home/cf/Pictures/binary_timer/countdown_1.png");
	counter->bit[1] = gs_texture_create_from_file ("/home/cf/Pictures/binary_timer/countdown_2.png");

	obs_leave_graphics ();
	return counter;
}
static void cm_destroy (void *data) {
}

static uint32_t cm_get_width (void *data) {
	struct counter *counter = (struct counter *) data;
	return counter->width;
}
static uint32_t cm_get_height (void *data) {
	struct counter *counter = (struct counter *) data;
	return counter->height;
}
static void cm_get_defaults (obs_data_t *settings) {
}

static obs_properties_t *cm_props (void *data) {
	UNUSED_PARAMETER (data);

	obs_properties_t *props = obs_properties_create ();
	obs_property_t *prop;

	prop = obs_properties_add_int (props, "minutes", "Minutes", 1, 60, 5);
	obs_property_int_set_suffix (prop, " minutes");
	prop = obs_properties_add_int (props, "seconds", "Seconds", 1, 60, 10);
	obs_property_int_set_suffix (prop, " seconds");

	return props;
}
static void cm_update (void *data, obs_data_t *settings) {
	struct counter *counter = (struct counter *) data;
#if 0
	obs_enter_graphics ();
	pthread_mutex_lock (&counter->lock);
	pthread_mutex_unlock (&counter->lock);
	obs_leave_graphics ();
#endif
}
static void cm_video_tick (void *data, float seconds) {
	struct counter *counter = (struct counter *) data;

#if 0
	obs_enter_graphics ();
	pthread_mutex_lock (&counter->lock);

	pthread_mutex_unlock (&counter->lock);
	obs_leave_graphics ();
#endif
}
static void cm_video_render (void *data, gs_effect_t *effect) {
	struct counter *counter = (struct counter *) data;

	obs_enter_graphics ();
	pthread_mutex_lock (&counter->lock);


#if 1
	gs_enable_framebuffer_srgb (true);
	gs_blend_state_push();
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
#endif

	gs_draw_sprite (counter->bit[0], 0, 32, 32);
	//gs_draw_sprite (counter->img.texture, 0, counter->img.cx, counter->img.cy);


#if 1
	gs_blend_state_pop();
#endif
	gs_enable_framebuffer_srgb (false);

	pthread_mutex_unlock (&counter->lock);
	obs_leave_graphics ();
}
