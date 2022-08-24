/*
obs-plugin-countdown-timer
Copyright (C) <2022> <Dmitrii Naidolinskii> <horisontdawn@yandex.ru>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/
#include <obs/obs.h>
#include <obs/obs-module.h>
#include <obs/graphics/image-file.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>

struct counter {
	int width;
	int height;

	pthread_mutex_t lock;

	gs_image_file3_t img[2];

	obs_source_t *source;
	obs_properties_t *props;

	uint8_t bm[8];
	uint8_t bs[8];

	uint64_t last_time;
	uint64_t cur_time;

	int8_t min;
	int8_t sec;
};

OBS_DECLARE_MODULE ()

OBS_MODULE_AUTHOR ("xverizex")


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
	.output_flags = OBS_SOURCE_VIDEO,
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
	UNUSED_PARAMETER (data);
}

static void *cm_create (obs_data_t *settings, obs_source_t *source) {
	struct counter *counter = calloc (1, sizeof (struct counter));
	counter->source = source;

	pthread_mutex_init (&counter->lock, NULL);

	counter->width = 32;
	counter->height = 32;

	cm_update (counter, settings);

	return counter;
}
static void cm_destroy (void *data) {
	struct counter *counter = (struct counter *) data;

	gs_image_file3_free (&counter->img[0]);
	gs_image_file3_free (&counter->img[1]);
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
	UNUSED_PARAMETER (settings);
}

static obs_properties_t *cm_props (void *data) {
	struct counter *counter = (struct counter *) data;

	obs_properties_t *props = obs_properties_create ();
	obs_property_t *prop;

	prop = obs_properties_add_int (props, "minutes", "Minutes", 1, 60, 5);
	obs_property_int_set_suffix (prop, " minutes");
	prop = obs_properties_add_int (props, "seconds", "Seconds", 1, 60, 10);
	obs_property_int_set_suffix (prop, " seconds");
	prop = obs_properties_add_path (props, "bit_0", "bit_0", OBS_PATH_FILE, "*.*", "");
	prop = obs_properties_add_path (props, "bit_1", "bit_1", OBS_PATH_FILE, "*.*", "");

	counter->props = props;

	return props;
}
static void cm_update (void *data, obs_data_t *settings) {
	struct counter *counter = (struct counter *) data;

	const char *bit_0 = obs_data_get_string (settings, "bit_0");
	const char *bit_1 = obs_data_get_string (settings, "bit_1");
	counter->min = obs_data_get_int (settings, "minutes");
	counter->sec = obs_data_get_int (settings, "seconds");

	obs_enter_graphics ();
	gs_image_file3_init (&counter->img[0], bit_0, GS_IMAGE_ALPHA_PREMULTIPLY);
	gs_image_file3_init (&counter->img[1], bit_1, GS_IMAGE_ALPHA_PREMULTIPLY);
	gs_image_file3_init_texture (&counter->img[0]);
	gs_image_file3_init_texture (&counter->img[1]);
	obs_leave_graphics ();

	counter->width = counter->img[0].image2.image.cx * 8;
	counter->height = counter->img[0].image2.image.cy + counter->img[1].image2.image.cy;
}
static void cm_video_tick (void *data, float seconds) {
	UNUSED_PARAMETER (seconds);

	struct counter *counter = (struct counter *) data;

	struct timeval tv;
	gettimeofday (&tv, NULL);

	uint64_t cur_time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

	if (counter->last_time == 0L) counter->last_time = cur_time;

	uint64_t diff = cur_time - counter->last_time;

	counter->cur_time += diff;

	if (counter->cur_time >= 1000) {
		counter->cur_time = 0L;
		counter->sec--;
		if (counter->sec < 0) {
			counter->min--;
			counter->sec = 60;
			if (counter->min < 0) counter->min = 0;
		}
	}

	counter->last_time = cur_time;

}
static void cm_video_render (void *data, gs_effect_t *effect) {
	struct counter *counter = (struct counter *) data;

	gs_eparam_t *const param = gs_effect_get_param_by_name (effect, "image");

	gs_matrix_push ();
	gs_matrix_push ();
	int pos = 1 << 7;
	for (int i = 0; i < 8; i++) {
		int index = counter->min & pos ? 1 : 0;
		gs_effect_set_texture (param, counter->img[index].image2.image.texture);
		gs_draw_sprite (counter->img[index].image2.image.texture, 0, 
				counter->img[index].image2.image.cx,
				counter->img[index].image2.image.cy
				);
		gs_matrix_translate3f (counter->img[index].image2.image.cx, 0.f, 0.f);
		pos >>= 1;
	}
	gs_matrix_pop ();

	gs_matrix_translate3f (0.f, counter->img[0].image2.image.cy, 0.f);

	gs_matrix_push ();
	pos = 1 << 7;
	for (int i = 0; i < 8; i++) {
		int index = counter->sec & pos ? 1 : 0;
		gs_effect_set_texture (param, counter->img[index].image2.image.texture);
		gs_draw_sprite (counter->img[index].image2.image.texture, 0, 
				counter->img[index].image2.image.cx,
				counter->img[index].image2.image.cy
				);
		gs_matrix_translate3f (counter->img[index].image2.image.cx, 0.f, 0.f);
		pos >>= 1;
	}
	gs_matrix_pop ();

	gs_matrix_pop ();
}
