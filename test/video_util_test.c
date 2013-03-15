/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
*
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
#include <glib.h>
#include <video_util.h>

//#define CANCEL_TEST

typedef struct
{
	video_util_h video_h;
	int	idx;
	int start_time;
	int duration;
}test_util_s;

GMainLoop *g_loop = NULL;
int g_make_video_cnt = 5;
int g_make_video_interval = 10000;
int g_duration = 5000;

static int test_transcode_do(test_util_s *_util_s);

void test_transcode_completed_cb(video_util_error_e error, void *user_data)
{
	int idx = 0;
	int start_position = 0;

	test_util_s *_util_s = (test_util_s *)user_data;
	printf("transcode_completed_cb============= [%2d][%d]\n", _util_s->idx, error);

	if(_util_s->idx == (g_make_video_cnt-1))
	{
		g_main_loop_quit(g_loop);
		return;
	}

	idx = _util_s->idx + 1;
	start_position = _util_s->start_time + g_make_video_interval;
	_util_s->idx = idx;
	_util_s->start_time = start_position;

	test_transcode_do(_util_s);

#ifdef CANCEL_TEST
	int ret = VIDEO_UTIL_ERROR_NONE;
	if(_util_s->idx == 3)
	{
		printf("Try cancel============= [%2d]]\n", _util_s->idx);
		ret = video_util_cancel_transcoding(_util_s->video_h);
		if(ret != VIDEO_UTIL_ERROR_NONE)
		{
			printf("[%d]error video_util_cancel_transcoding [%d]\n", __LINE__, ret);
			g_main_loop_quit(g_loop);
			return;
		}

		_util_s->idx = 4;
		test_transcode_do(_util_s);
		return;
	}
#endif
	return;
}

void test_transcode_progress_cb(unsigned long current_position,  unsigned long duration, void *user_data)
{
	test_util_s *_util_s = (test_util_s *)user_data;

	printf("transcode_progress_cb-------------- [%2d][%ld][%ld]\n", _util_s->idx, current_position, duration);

#if 0
	unsigned long pos = 0;
	unsigned long dur = 0;
	video_util_get_progress_transcoding(_util_s->video_h, &pos, &dur);
	printf("transcode_progress_cb-------------- [%2d][%ld][%ld]\n", _util_s->idx, pos, dur);
#endif
	return;
}

bool test_transcode_spec_cb(int value, void *user_data)
{
	if(user_data != NULL)
		printf("[%s]-----------", (char*)user_data);
	printf("[%d] \n", value);

	return true;
}

bool supported_spec_check(video_util_h handle)
{
	int ret = 0;
	ret = video_util_foreach_supported_file_format(handle, (video_util_supported_file_format_cb)test_transcode_spec_cb, "format_check");
	printf("[%d] video_util_foreach_supported_file_format [%d]\n", __LINE__, ret);
	ret = video_util_foreach_supported_video_codec(handle, (video_util_supported_video_encoder_cb)test_transcode_spec_cb, "video_codec_check");
	printf("[%d] video_util_foreach_supported_video_codec [%d]\n", __LINE__, ret);
	ret = video_util_foreach_supported_audio_codec(handle, (video_util_supported_audio_encoder_cb)test_transcode_spec_cb, "audio_codec_check");
	printf("[%d] video_util_foreach_supported_audio_codec [%d]\n", __LINE__, ret);

	return true;
}

static int test_transcode_do(test_util_s *util_s)
{
	int ret = 0;
	char test_output_file_path[128] = {0, };

	memset(test_output_file_path, 0x00, sizeof(test_output_file_path));
	snprintf(test_output_file_path, sizeof(test_output_file_path), "/opt/usr/media/Videos/transcode_test_%d.mp4", util_s->idx);

	printf("g_start_time[%d] duration[%d] [%s]\n", util_s->start_time, util_s->duration, test_output_file_path);

	ret = video_util_start_transcoding(util_s->video_h, util_s->start_time, util_s->duration, test_output_file_path, test_transcode_progress_cb, test_transcode_completed_cb, util_s);
	if(ret != VIDEO_UTIL_ERROR_NONE)
	{
		printf("[%d]error video_util_start_transcoding [%d]\n", __LINE__, ret);
		g_main_loop_quit(g_loop);
		return ret;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_h video_h = NULL;
	test_util_s *_util_s = NULL;
	int cnt = argc -1;

	if(cnt < 1)
	{
		printf("type file path plz. [%d]\n", cnt);
		return 0;
	}

	ret = video_util_create(&video_h);
	if(ret != VIDEO_UTIL_ERROR_NONE)
	{
		printf("[%d]error video_util_create [%d]\n", __LINE__, ret);
	}

#if 0
	supported_spec_check(video_h);
#endif

	ret = video_util_set_file_path(video_h,  argv[1]);
	ret = video_util_set_file_format(video_h, VIDEO_UTIL_FILE_FORMAT_3GP);
	ret = video_util_set_video_codec(video_h, VIDEO_UTIL_VIDEO_CODEC_MPEG4);
	ret = video_util_set_audio_codec(video_h, VIDEO_UTIL_AUDIO_CODEC_AAC);
	ret = video_util_set_accurate_mode(video_h, 0);
	ret = video_util_set_resolution(video_h, 176, 144);
	ret = video_util_set_fps(video_h, 10);

	if(ret != VIDEO_UTIL_ERROR_NONE)
	{
		printf("[%d]error video_util_set condition [%d]\n", __LINE__, ret);
		return 0;
	}

	_util_s = (test_util_s*)calloc(1,sizeof(test_util_s));
	if(_util_s == NULL)
	{
		printf("[%d]error calloc\n", __LINE__);
		return 0;
	}

	_util_s->video_h = video_h;
	_util_s->idx = 0;
	_util_s->start_time = 0;
	_util_s->duration = g_duration;

	test_transcode_do(_util_s);

	g_loop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(g_loop);
	g_main_loop_unref(g_loop);

	ret = video_util_destroy(video_h);	//destory handle in cb
	if(ret != VIDEO_UTIL_ERROR_NONE)
	{
		printf("[%d]error video_util_destroy [%d]\n", __LINE__, ret);
	}
	else
	{
		printf("[%d]Success video_util_destroy [%d]\n", __LINE__, ret);
	}

	if(_util_s != NULL)
		free(_util_s);

	return 0;
}
