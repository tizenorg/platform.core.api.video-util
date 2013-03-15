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


#ifndef __TIZEN_VIDEO_UTIL_PRIVATE_H__
#define __TIZEN_VIDEO_UTIL_PRIVATE_H__

#include <stdbool.h>
#include <video_util_type.h>
#include <mm_types.h>

typedef bool (*video_util_supported_type_cb)(int type, void *user_data);

typedef struct
{
	char *input_path;
	bool accurate_mode;
	video_util_video_codec_e video_codec;
	video_util_audio_codec_e audio_codec;
	video_util_file_format_e file_format;
	int width;
	int height;
	int fps;
	MMHandleType transcode_h;
}video_util_s;

typedef struct
{
	void *user_data;
	video_util_transcoding_progress_cb transcode_progress_cb;
	video_util_transcoding_completed_cb transcode_completed_cb;
}video_util_cb_s;

typedef struct
{
	void *user_data;
	video_util_supported_type_cb supported_type_cb;
}video_util_type_cb_s;

typedef enum
{
	VIDEO_UTIL_TYPE_FORMAT	= 0,
	VIDEO_UTIL_TYPE_VIDEO_ENC,
	VIDEO_UTIL_TYPE_AUDIO_ENC
}video_util_type_e;

#endif /*__TIZEN_VIDEO_UTIL_PRIVATE_H__*/
