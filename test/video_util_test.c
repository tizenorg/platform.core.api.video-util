/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <dlog.h>
#include <Elementary.h>
#include <appcore-efl.h>
#include <video_util.h>

#define PACKAGE			"video_util_test"
#define MAX_STRING_LEN	256

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG			"VIDEO_UTIL_TEST"

enum {
	CURRENT_STATUS_MAINMENU,
	CURRENT_STATUS_FILENAME,
	CURRENT_STATUS_SET_FORMAT,
	CURRENT_STATUS_SET_VIDEO_CODEC,
	CURRENT_STATUS_SET_AUDIO_CODEC,
	CURRENT_STATUS_SET_ACCURATE_MODE,
	CURRENT_STATUS_SET_RESOLUTION,
	CURRENT_STATUS_SET_FPS,
	CURRENT_STATUS_SET_TIME,
	CURRENT_STATUS_SET_OUTFILENAME,
	CURRENT_STATUS_MAX
};

static video_util_h video_h = NULL;
char g_uri[MAX_STRING_LEN] = { 0, };

int g_menu_state = CURRENT_STATUS_MAINMENU;
int g_handle_num = 1;
int format = VIDEO_UTIL_FILE_FORMAT_3GP;
int video_codec = VIDEO_UTIL_VIDEO_CODEC_MPEG4;
int audio_codec = VIDEO_UTIL_AUDIO_CODEC_AAC;
int accurate_mode = 0;
int width = 176;
int height = 144;
int fps = 10;
unsigned long start_position = 0;
unsigned long duration = 5000;
int make_video_cnt = 1;
char g_out[MAX_STRING_LEN] = { 0, };

typedef struct {
	video_util_h video_h;
	int idx;
	unsigned long start_time;
	unsigned long duration;
} test_util_s;

static void display_sub_basic();

static int _create_app(void *data)
{
	LOGD("My app is going alive!\n");
	return 0;
}

static int _terminate_app(void *data)
{
	LOGD("My app is going gone!\n");
	return 0;
}

struct appcore_ops ops = {
	.create = _create_app,
	.terminate = _terminate_app,
};

void _quit_program(void)
{
	elm_exit();
}

bool test_transcode_spec_cb(int value, void *user_data)
{
	if (!user_data) {
		LOGE("user_data is NULL");
		return FALSE;
	}

	if (!strcmp(user_data, "format_check")) {
		switch (value) {
		case 0:
			LOGI("[%s] --- [3gp]", user_data);
			break;
		case 1:
			LOGI("[%s] --- [mp4]", user_data);
			break;
		default:
			break;
		}
	} else if (!strcmp(user_data, "video_codec_check")) {
		switch (value) {
		case 0:
			LOGI("[%s] --- [m4v]", user_data);
			break;
		case 1:
			LOGI("[%s] --- [h263]", user_data);
			break;
		case 2:
			LOGI("[%s] --- [h264]", user_data);
			break;
		default:
			break;
		}
	} else if (!strcmp(user_data, "audio_codec_check")) {
		switch (value) {
		case 0:
			LOGI("[%s] --- [aac]", user_data);
			break;
		case 1:
			LOGI("[%s] --- [amrnb]", user_data);
			break;
		default:
			break;
		}
	}

	return true;
}

bool _supported_spec_check(video_util_h handle)
{
	int ret = 0;
	ret = video_util_foreach_supported_file_format(handle, (video_util_supported_file_format_cb)test_transcode_spec_cb, "format_check");
	printf("video_util_foreach_supported_file_format [%d]\n", ret);
	ret = video_util_foreach_supported_video_codec(handle, (video_util_supported_video_encoder_cb)test_transcode_spec_cb, "video_codec_check");
	printf("video_util_foreach_supported_video_codec [%d]\n", ret);
	ret = video_util_foreach_supported_audio_codec(handle, (video_util_supported_audio_encoder_cb)test_transcode_spec_cb, "audio_codec_check");
	printf("video_util_foreach_supported_audio_codec [%d]\n", ret);

	return true;
}

void _transcode_completed_cb(video_util_error_e error, void *user_data)
{
	int idx = 0;
	unsigned long ntn_start_position = 0;

	test_util_s *_util_s = (test_util_s *)user_data;

	LOGI("transcode_completed_cb============= [%2d / %2d][%d]\n", _util_s->idx, make_video_cnt, error);
	printf("transcode_completed_cb============= [%2d / %2d][%d]\n", _util_s->idx, make_video_cnt, error);

	if (_util_s->idx == (make_video_cnt - 1)) {
		LOGI("End trascoding");
		video_util_destroy(_util_s->video_h);
		_util_s->video_h = NULL;
		video_h = NULL;
		return;
	}

	idx = _util_s->idx + 1;
	ntn_start_position = _util_s->start_time + duration;
	_util_s->idx = idx;
	_util_s->start_time = ntn_start_position;

	_video_util_start_transcoding((test_util_s *)_util_s);

	return;
}

void _transcode_progress_cb(unsigned long current_position, unsigned long duration, void *user_data)
{
	test_util_s *_util_s = (test_util_s *)user_data;

	LOGD("transcode_progress_cb-------------- [%2d][%ld][%ld]\n", _util_s->idx, current_position, duration);

	return;
}

void _video_util_start_transcoding(test_util_s *util_s)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	char output_file_path[MAX_STRING_LEN] = { 0, };

	if (!video_h) {
		LOGE("video_util handle is NULL, please set format after create");
		return;
	}

	LOGI("video_util set below");
	LOGI("format: %d, video codec: %d, audio codec: %d, accurate mode: %d", format, video_codec, audio_codec, accurate_mode);
	LOGI("width: %d, height: %d, fps: %d", width, height, fps);
	LOGI("start time: %lu, durtation: %lu", start_position, duration);

	ret = video_util_set_file_format(video_h, format);
	if (ret != VIDEO_UTIL_ERROR_NONE) {
		LOGE("video_util_set_file_format is failed (%d)", ret);
		return;
	}

	ret = video_util_set_video_codec(video_h, video_codec);
	if (ret != VIDEO_UTIL_ERROR_NONE) {
		LOGE("video_util_set_video_codec is failed (%d)", ret);
		return;
	}

	ret = video_util_set_audio_codec(video_h, audio_codec);
	if (ret != VIDEO_UTIL_ERROR_NONE) {
		LOGE("video_util_set_audio_codec is failed (%d)", ret);
		return;
	}

	ret = video_util_set_accurate_mode(video_h, accurate_mode);
	if (ret != VIDEO_UTIL_ERROR_NONE) {
		LOGE("video_util_set_accurate_mode is failed (%d)", ret);
		return;
	}

	ret = video_util_set_resolution(video_h, width, height);
	if (ret != VIDEO_UTIL_ERROR_NONE) {
		LOGE("video_util_set_resolution is failed (%d)", ret);
		return;
	}

	ret = video_util_set_fps(video_h, fps);
	if (ret != VIDEO_UTIL_ERROR_NONE) {
		LOGE("video_util_set_fps is failed (%d)", ret);
		return;
	}

	memset(output_file_path, 0x00, MAX_STRING_LEN);

	snprintf(output_file_path, MAX_STRING_LEN, "%s_%d.%s", g_out, util_s->idx, format ? "mp4" : "3gp");

	LOGI("input start_time: %lu, duration: %lu, output_file_path: %s", util_s->start_time, util_s->duration, output_file_path);
	ret = video_util_start_transcoding(util_s->video_h, util_s->start_time, util_s->duration, output_file_path, _transcode_progress_cb, _transcode_completed_cb, util_s);

	if (ret != VIDEO_UTIL_ERROR_NONE) {
		LOGE("video_util_start_transcoding is failed (%d)", ret);
		return;
	}
}

void _reset_var()
{
	if (video_h) {
		video_util_destroy(video_h);
		video_h = NULL;
	}
	memset(g_uri, 0x00, MAX_STRING_LEN);
	memset(g_out, 0x00, MAX_STRING_LEN);
	g_menu_state = CURRENT_STATUS_MAINMENU;
	g_handle_num = 1;
	format = VIDEO_UTIL_FILE_FORMAT_3GP;
	video_codec = VIDEO_UTIL_VIDEO_CODEC_MPEG4;
	audio_codec = VIDEO_UTIL_AUDIO_CODEC_AAC;
	accurate_mode = 0;
	width = 176;
	height = 144;
	fps = 25;
	start_position = 5000;
	duration = 5000;
	make_video_cnt = 3;
}

static void input_filename(char *filename)
{
	int len = strlen(filename);
	int i = 0;
	int ret = VIDEO_UTIL_ERROR_NONE;

	if (len < 0 || len > MAX_STRING_LEN) {
		LOGE("Input file name is wrong");
		return;
	}

	_reset_var();

	if (video_h) {
		ret = video_util_cancel_transcoding(video_h);
		ret = video_util_destroy(video_h);
	}

	video_h = NULL;

	ret = video_util_create(&video_h);

	if (ret != VIDEO_UTIL_ERROR_NONE) {
		LOGE("video_util create is failed (%d)", ret);
		return;
	}

	strncpy(g_uri, filename, len);

	ret = video_util_set_file_path(video_h, g_uri);

	if (ret != VIDEO_UTIL_ERROR_NONE) {
		LOGE("video_util_set_file_path is failed");
		return;
	}

	_supported_spec_check(video_h);
}

void reset_menu_state()
{
	g_menu_state = CURRENT_STATUS_MAINMENU;
	return;
}

void _interpret_main_menu(char *cmd)
{
	int len = strlen(cmd);

	if (len == 1) {
		if (strncmp(cmd, "a", 1) == 0) {
			g_menu_state = CURRENT_STATUS_FILENAME;
		} else if (strncmp(cmd, "s", 1) == 0) {

			test_util_s *_util_s = (test_util_s *)calloc(1, sizeof(test_util_s));
			if (!_util_s) {
				g_print("test util calloc failed");
				return;
			}

			_util_s->video_h = video_h;
			_util_s->idx = 0;
			_util_s->start_time = start_position;
			_util_s->duration = duration;

			_video_util_start_transcoding(_util_s);

		} else if (strncmp(cmd, "c", 1) == 0) {
			int ret = VIDEO_UTIL_ERROR_NONE;
			if (!video_h) {
				g_print("video_util handle is NULL, please set format after create");
				return;;
			}
			ret = video_util_cancel_transcoding(video_h);
			if (ret != VIDEO_UTIL_ERROR_NONE) {
				g_print("video_util_cancel_transcoding is failed (%d)", ret);
				return;
			}
		} else if (strncmp(cmd, "f", 1) == 0) {
			g_menu_state = CURRENT_STATUS_SET_FORMAT;
		} else if (strncmp(cmd, "m", 1) == 0) {
			g_menu_state = CURRENT_STATUS_SET_ACCURATE_MODE;
		} else if (strncmp(cmd, "t", 1) == 0) {
			g_menu_state = CURRENT_STATUS_SET_TIME;
		} else if (strncmp(cmd, "o", 1) == 0) {
			g_menu_state = CURRENT_STATUS_SET_OUTFILENAME;
		} else if (strncmp(cmd, "q", 1) == 0) {
			_quit_program();
		} else {
			g_print("unknown menu \n");
		}
	} else if (len == 2) {
		if (strncmp(cmd, "vc", 2) == 0)
			g_menu_state = CURRENT_STATUS_SET_VIDEO_CODEC;
		else if (strncmp(cmd, "ac", 2) == 0)
			g_menu_state = CURRENT_STATUS_SET_AUDIO_CODEC;
		else if (strncmp(cmd, "vr", 2) == 0)
			g_menu_state = CURRENT_STATUS_SET_RESOLUTION;
		else if (strncmp(cmd, "vf", 2) == 0)
			g_menu_state = CURRENT_STATUS_SET_FPS;
		else
			g_print("unknown menu \n");
	} else {
		g_print("unknown menu \n");
	}
	return;
}

static void displaymenu(void)
{
	if (g_menu_state == CURRENT_STATUS_MAINMENU) {
		display_sub_basic();
	} else if (g_menu_state == CURRENT_STATUS_FILENAME) {
		g_print("*** input mediapath.\n");
	} else if (g_menu_state == CURRENT_STATUS_SET_FORMAT) {
		g_print("*** input file format.(0:3gp, 1:mp4)\n");
	} else if (g_menu_state == CURRENT_STATUS_SET_VIDEO_CODEC) {
		g_print("*** input video codec.(0:m4v, 1:h263, 2:h264)\n");
	} else if (g_menu_state == CURRENT_STATUS_SET_AUDIO_CODEC) {
		g_print("*** input audio codec.(0:aac, 1:amrnb)\n");
	} else if (g_menu_state == CURRENT_STATUS_SET_ACCURATE_MODE) {
		g_print("*** input accurate mode.(0: OFF, 1: ON)\n");
	} else if (g_menu_state == CURRENT_STATUS_SET_RESOLUTION) {
		g_print("*** input video resolution.(width, height)\n");
	} else if (g_menu_state == CURRENT_STATUS_SET_FPS) {
		g_print("*** input video fps.(5<=fps<=30)\n");
	} else if (g_menu_state == CURRENT_STATUS_SET_FPS) {
		g_print("*** input video fps.(5<=fps<=30)\n");
	} else if (g_menu_state == CURRENT_STATUS_SET_TIME) {
		g_print("*** input transcode start/duration time(ms), run nth.(start time, duration, n)\n");
	} else if (g_menu_state == CURRENT_STATUS_SET_OUTFILENAME) {
		g_print("*** input output filename.(defaunt path /opt/usr/media/)\n");
	} else {
		g_print("*** unknown status.\n");
		_quit_program();
	}
	g_print(" >>> ");
}

gboolean timeout_menu_display(void *data)
{
	displaymenu();
	return FALSE;
}

static void interpret(char *cmd)
{
	switch (g_menu_state) {

	case CURRENT_STATUS_MAINMENU:
		{
			_interpret_main_menu(cmd);
			break;
		}
	case CURRENT_STATUS_FILENAME:
		{
			input_filename(cmd);
			reset_menu_state();
			break;
		}
	case CURRENT_STATUS_SET_FORMAT:
		{
			int ret = VIDEO_UTIL_ERROR_NONE;
			format = atoi(cmd);
			if (format < 0 || format >= VIDEO_UTIL_FILE_FORMAT_MAX) {
				LOGE("input cmd is out of range.");
				reset_menu_state();
				break;
			}
			if (!video_h) {
				LOGE("video_util handle is NULL, please set format after create");
				reset_menu_state();
				break;
			}
			ret = video_util_set_file_format(video_h, format);
			if (ret != VIDEO_UTIL_ERROR_NONE) {
				LOGE("video_util_set_file_format is failed (%d)", ret);
				reset_menu_state();
				break;
			}

			reset_menu_state();
			break;
		}
	case CURRENT_STATUS_SET_VIDEO_CODEC:
		{
			int ret = VIDEO_UTIL_ERROR_NONE;
			video_codec = atoi(cmd);
			if (video_codec < 0 || video_codec >= VIDEO_UTIL_VIDEO_CODEC_NONE) {
				LOGE("input cmd is out of range");
				reset_menu_state();
				break;
			}
			if (!video_h) {
				LOGE("video_util handle is NULL, please set format after create");
				reset_menu_state();
				break;
			}
			ret = video_util_set_video_codec(video_h, video_codec);
			if (ret != VIDEO_UTIL_ERROR_NONE) {
				LOGE("video_util_set_video_codec is failed (%d)", ret);
				reset_menu_state();
				break;
			}

			reset_menu_state();
			break;
		}
	case CURRENT_STATUS_SET_AUDIO_CODEC:
		{
			int ret = VIDEO_UTIL_ERROR_NONE;
			audio_codec = atoi(cmd);
			if (audio_codec < 0 || audio_codec >= VIDEO_UTIL_VIDEO_CODEC_NONE) {
				LOGE("input cmd is out of range");
				reset_menu_state();
				break;
			}
			if (!video_h) {
				LOGE("video_util handle is NULL, please set format after create");
				reset_menu_state();
				break;
			}
			ret = video_util_set_video_codec(video_h, audio_codec);
			if (ret != VIDEO_UTIL_ERROR_NONE) {
				LOGE("video_util_set_video_codec is failed (%d)", ret);
				reset_menu_state();
				break;
			}

			reset_menu_state();
			break;
		}
	case CURRENT_STATUS_SET_ACCURATE_MODE:
		{
			int ret = VIDEO_UTIL_ERROR_NONE;
			accurate_mode = atoi(cmd);

			if (!video_h) {
				LOGE("video_util handle is NULL, please set format after create");
				reset_menu_state();
				break;
			}
			ret = video_util_set_accurate_mode(video_h, (bool)accurate_mode);
			if (ret != VIDEO_UTIL_ERROR_NONE) {
				LOGE("video_util_set_video_codec is failed (%d)", ret);
				reset_menu_state();
				break;
			}

			reset_menu_state();
			break;
		}
	case CURRENT_STATUS_SET_RESOLUTION:
		{
			int ret = VIDEO_UTIL_ERROR_NONE;
			int value = atoi(cmd);
			static int resolution_cnt = 0;

			if (!video_h) {
				LOGE("video_util handle is NULL, please set format after create");
				reset_menu_state();
				break;
			}

			switch (resolution_cnt) {
			case 0:
				width = value;
				resolution_cnt++;
				break;
			case 1:
				resolution_cnt = 0;
				height = value;

				ret = video_util_set_resolution(video_h, width, height);
				if (ret != VIDEO_UTIL_ERROR_NONE) {
					LOGE("video_util_set_resolution is failed (%d)", ret);
					reset_menu_state();
					break;
				}

				reset_menu_state();
				break;
			}
			break;
		}
	case CURRENT_STATUS_SET_FPS:
		{
			int ret = VIDEO_UTIL_ERROR_NONE;
			fps = atoi(cmd);

			if (fps < 5 || fps > 30) {
				LOGE("input cmd is out of range");
				reset_menu_state();
				break;
			}
			if (!video_h) {
				LOGE("video_util handle is NULL, please set format after create");
				reset_menu_state();
				break;
			}
			ret = video_util_set_fps(video_h, fps);
			if (ret != VIDEO_UTIL_ERROR_NONE) {
				LOGE("video_util_set_fps is failed (%d)", ret);
				reset_menu_state();
				break;
			}

			reset_menu_state();
			break;
		}
	case CURRENT_STATUS_SET_TIME:
		{
			int ret = VIDEO_UTIL_ERROR_NONE;
			int value = atoi(cmd);
			static int set_time_cnt = 0;

			switch (set_time_cnt) {
			case 0:
				start_position = value;
				set_time_cnt++;
				break;
			case 1:
				duration = value;
				set_time_cnt++;
				break;
			case 2:
				set_time_cnt = 0;
				make_video_cnt = value;
				reset_menu_state();
				break;

			}
			break;
		}
	case CURRENT_STATUS_SET_OUTFILENAME:
		{
			LOGI("output file is %s", g_out);
			snprintf(g_out, MAX_STRING_LEN, "/opt/usr/media/%s", cmd);
			g_out[MAX_STRING_LEN - 1] = '\0';
			LOGI("output file is %s", g_out);
			reset_menu_state();
			break;
		}
	default:
		break;
	}

	g_timeout_add(100, timeout_menu_display, 0);
}

static void display_sub_basic()
{
	g_print("\n");
	g_print("=========================================================================================\n");
	g_print("                                    video_util_test test\n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print(" a. Create \t\t\t");
	g_print(" s. Start transcoding \t");
	g_print(" c. Cancel transcoding \n");
	g_print(" f. Set file format\t\t");
	g_print("vc. Set video codec\t");
	g_print("ac. Set audio codec\n");
	g_print(" m. Set accurate mode\t\t");
	g_print("vr. Set resolution\t");
	g_print("vf. Set video fps\n");
	g_print(" t. Set start/duration time \t");
	g_print(" o. Set output filename\t");
	g_print(" q. quite test suite\t");
	g_print("\n");
	g_print("=========================================================================================\n");
}

gboolean input(GIOChannel *channel)
{
	gchar buf[MAX_STRING_LEN];
	gsize read;
	GError *error = NULL;

	g_io_channel_read_chars(channel, buf, MAX_STRING_LEN, &read, &error);
	buf[read] = '\0';
	g_strstrip(buf);
	interpret(buf);

	return TRUE;
}

int main(int argc, char *argv[])
{
	GIOChannel *stdin_channel;
	stdin_channel = g_io_channel_unix_new(0);
	g_io_channel_set_flags(stdin_channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)input, NULL);

	displaymenu();

	ops.data = NULL;

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);

}
