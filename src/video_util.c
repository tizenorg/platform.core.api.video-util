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

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dlog.h>
#include <mm_transcode.h>
#include <mm_error.h>
#include <video_util.h>
#include <video_util_private.h>
#include <system_info.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_MEDIA_VIDEO_UTIL"

#define UTIL_SAFE_FREE(src)	\
	do {					\
		if (src) {			\
			free(src);		\
			src = NULL;		\
		}					\
	} while (0)

#define UTIL_STRING_VALID(str) \
	((str != NULL && strlen(str) > 0) ? true : false)

#define VIDEO_UTIL_MINIMUM_WIDTH			128
#define VIDEO_UTIL_MINIMUM_HEIGHT		96
#define VIDEO_UTIL_MINIMUM_DURATION		1000	/* 1 sec */
#define VIDEO_UTIL_MINIMUM_FPS			5
#define VIDEO_UTIL_MAXIMUM_FPS			30
#define VIDEO_UTIL_MAXIMUM_BPS			4294967295UL

static int __video_util_create_transcode_handle(video_util_s *handle);
static int __video_util_destroy_transcode_handle(video_util_s *handle);
static int __video_util_check_transcode_is_busy(video_util_s * handle, bool *is_busy);
static bool __video_util_check_video_codec(video_util_video_codec_e video_codec);
static bool __video_util_check_audio_codec(video_util_audio_codec_e audio_codec);
static bool __video_util_check_file_format(video_util_file_format_e file_format);
static bool __video_util_check_resolution(int width, int height);
static bool __video_util_check_duration(int duration);
static bool __video_util_check_fps(int fps);
static video_util_error_e __video_util_error_convert(int error);
static void __video_util_transcode_progress_cb(unsigned long current_position, unsigned long duration, void *user_data);
static void __video_util_transcode_completed_cb(int error, void *user_data);
static bool __video_util_type_callback(int codec_type, void *user_data);
static int __video_util_foreach_supported_type(video_util_type_e type, video_util_supported_type_cb callback, void *user_data);

static int __video_util_create_transcode_handle(video_util_s *handle)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	MMHandleType transcode_h = 0;

	ret = mm_transcode_create(&transcode_h);
	if (ret != MM_ERROR_NONE) {
		if (ret == MM_ERROR_TRANSCODE_INVALID_VALUE) {
			video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
			return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
		} else if (ret == MM_ERROR_TRANSCODE_NOT_SUPPORT_FORMAT) {
			video_util_error("NOT_SUPPORTED_FORMAT(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED_FORMAT);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED_FORMAT;
		} else {
			video_util_error("INVALID_OPERATION(0x%08x)", ret);
			return VIDEO_UTIL_ERROR_INVALID_OPERATION;
		}
	}

	ret = mm_transcode_prepare(transcode_h, handle->input_path, handle->file_format, handle->video_codec, handle->audio_codec);
	if (ret != MM_ERROR_NONE) {
		if (ret == MM_ERROR_TRANSCODE_INVALID_VALUE) {
			video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
			ret = VIDEO_UTIL_ERROR_INVALID_PARAMETER;
		} else if (ret == MM_ERROR_TRANSCODE_NOT_SUPPORT_FORMAT) {
			video_util_error("NOT_SUPPORTED_FORMAT(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED_FORMAT);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED_FORMAT;
		} else {
			video_util_error("INVALID_OPERATION(0x%08x)", ret);
			ret = VIDEO_UTIL_ERROR_INVALID_OPERATION;
		}
	}

	if (ret == MM_ERROR_NONE)
		handle->transcode_h = transcode_h;
	else
		__video_util_destroy_transcode_handle(handle);

	return ret;
}

/* when should make new trandcode handle? input_filepath or file_format or video_codec or audio_codec is modified */
static int __video_util_destroy_transcode_handle(video_util_s *handle)
{
	int ret = VIDEO_UTIL_ERROR_NONE;

	if (handle->transcode_h) {
		ret = mm_transcode_destroy(handle->transcode_h);
		if (ret != MM_ERROR_NONE) {
			video_util_error("INVALID_OPERATION (0x%08x)", ret);
			return VIDEO_UTIL_ERROR_INVALID_OPERATION;
		}
	}

	handle->transcode_h = 0;

	return ret;
}

int __video_util_check_transcode_is_busy(video_util_s *handle, bool *is_busy)
{
	int ret = VIDEO_UTIL_ERROR_NONE;

	if (handle->transcode_h) {
		ret = mm_transcode_is_busy(handle->transcode_h, is_busy);
		if (ret != MM_ERROR_NONE) {
			video_util_error("INVALID_OPERATION(0x%08x)", ret);
			return VIDEO_UTIL_ERROR_INVALID_OPERATION;
		}
	} else {
		*is_busy = false;
	}

	return ret;
}

static bool __video_util_check_video_codec(video_util_video_codec_e video_codec)
{
	if ((video_codec < 0) || (video_codec > VIDEO_UTIL_VIDEO_CODEC_NONE)) {
		video_util_error("invalid video_codec [%d]", video_codec);
		return false;
	}

	return true;
}

static bool __video_util_check_audio_codec(video_util_audio_codec_e audio_codec)
{
	if ((audio_codec < 0) || (audio_codec > VIDEO_UTIL_AUDIO_CODEC_NONE)) {
		video_util_error("invalid audio_codec [%d]", audio_codec);
		return false;
	}

	return true;
}

static bool __video_util_check_file_format(video_util_file_format_e file_format)
{
	if ((file_format < 0) || (file_format >= VIDEO_UTIL_FILE_FORMAT_MAX)) {
		video_util_error("invalid file_format [%d]", file_format);
		return false;
	}

	return true;
}

static bool __video_util_check_resolution(int width, int height)
{
	if (((width > 0) && (width < VIDEO_UTIL_MINIMUM_WIDTH)) || (width < 0)) {
		video_util_error("invalid width [%d]", width);
		return false;
	}

	if (((height > 0) && (height < VIDEO_UTIL_MINIMUM_HEIGHT)) || (height < 0)) {
		video_util_error("invalid height [%d]", height);
		return false;
	}

	return true;
}

static bool __video_util_check_duration(int duration)
{
	if (((duration > 0) && (duration < VIDEO_UTIL_MINIMUM_DURATION)) || (duration < 0)) {
		video_util_error("invalid duration [%d]", duration);
		return false;
	}

	return true;
}

static bool __video_util_check_fps(int fps)
{
	if ((fps < 0) || ((fps > 0) && (fps < VIDEO_UTIL_MINIMUM_FPS)) || (fps > VIDEO_UTIL_MAXIMUM_FPS)) {
		video_util_error("invalid fps [%d]", fps);
		return false;
	}

	return true;
}

static video_util_error_e __video_util_error_convert(int error)
{
	if (error == MM_ERROR_NONE) {
		video_util_debug("Error None");
		return VIDEO_UTIL_ERROR_NONE;
	} else if (error == MM_ERROR_TRANSCODE_INVALID_VALUE) {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	} else if (error == MM_ERROR_TRANSCODE_NOT_SUPPORT_FORMAT) {
		video_util_error("NOT_SUPPORTED_FORMAT(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED_FORMAT);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED_FORMAT;
	} else {
		video_util_error("INVALID_OPERATION(0x%08x)", error);
		return VIDEO_UTIL_ERROR_INVALID_OPERATION;
	}
}

static void __video_util_transcode_progress_cb(unsigned long current_position, unsigned long duration, void *user_data)
{
	video_util_cb_s *_util_cb = (video_util_cb_s *)user_data;

	if ((_util_cb != NULL) && (_util_cb->transcode_progress_cb != NULL))
		_util_cb->transcode_progress_cb(current_position, duration, _util_cb->user_data);

	return;
}

static void __video_util_transcode_completed_cb(int error, void *user_data)
{
	int error_value = VIDEO_UTIL_ERROR_NONE;
	video_util_cb_s *_util_cb = (video_util_cb_s *)user_data;

	if ((_util_cb != NULL) && (_util_cb->transcode_completed_cb != NULL)) {
		error_value = __video_util_error_convert(error);
		_util_cb->transcode_completed_cb(error_value, _util_cb->user_data);
	}

	UTIL_SAFE_FREE(_util_cb);

	return;
}

static bool __video_util_type_callback(int codec_type, void *user_data)
{
	video_util_type_cb_s *codec_cb = (video_util_type_cb_s *)user_data;

	if (codec_cb != NULL) {
		if (codec_cb->supported_type_cb)
			codec_cb->supported_type_cb(codec_type, codec_cb->user_data);
	}

	return true;
}

static int __video_util_foreach_supported_type(video_util_type_e type, video_util_supported_type_cb callback, void *user_data)
{
	int ret = VIDEO_UTIL_ERROR_NONE;

	video_util_type_cb_s *codec_cb = (video_util_type_cb_s *)calloc(1, sizeof(video_util_type_cb_s));
	codec_cb->user_data = user_data;
	codec_cb->supported_type_cb = (video_util_supported_type_cb)callback;

	if (type == VIDEO_UTIL_TYPE_FORMAT)
		ret = mm_transcode_get_supported_container_format((mm_transcode_support_type_callback)__video_util_type_callback, (void *)codec_cb);
	else if (type == VIDEO_UTIL_TYPE_VIDEO_ENC)
		ret = mm_transcode_get_supported_video_encoder((mm_transcode_support_type_callback)__video_util_type_callback, (void *)codec_cb);
	else if (type == VIDEO_UTIL_TYPE_AUDIO_ENC)
		ret = mm_transcode_get_supported_audio_encoder((mm_transcode_support_type_callback)__video_util_type_callback, (void *)codec_cb);
	else {
		video_util_error("Invalid type (%d)", type);
		ret = VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	UTIL_SAFE_FREE(codec_cb);

	return ret;
}

int video_util_create(video_util_h *handle)
{
	int ret = VIDEO_UTIL_ERROR_NONE;

	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	if (handle == NULL) {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	video_util_s *_handle = (video_util_s *)calloc(1, sizeof(video_util_s));
	if (_handle == NULL) {
		video_util_error("OUT_OF_MEMORY(0x%08x)", VIDEO_UTIL_ERROR_OUT_OF_MEMORY);
		return VIDEO_UTIL_ERROR_OUT_OF_MEMORY;
	}

	_handle->input_path = NULL;
	_handle->accurate_mode = false;
	_handle->video_codec = 0;
	_handle->audio_codec = 0;
	_handle->file_format = 0;
	_handle->width = 0;
	_handle->height = 0;
	_handle->fps = 0;
	_handle->transcode_h = 0;
	_handle->_util_cb = NULL;

	*handle = (video_util_h)_handle;

	return ret;
}

int video_util_destroy(video_util_h handle)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if (_handle == NULL) {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	ret = __video_util_destroy_transcode_handle(_handle);

	UTIL_SAFE_FREE(_handle->_util_cb);
	video_util_debug("FREE(_handle->_util_cb)");
	UTIL_SAFE_FREE(_handle->input_path);
	UTIL_SAFE_FREE(_handle);

	return ret;
}

int video_util_set_file_path(video_util_h handle, const char *file_path)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool is_busy = false;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if ((_handle != NULL) && (UTIL_STRING_VALID(file_path))) {
		video_util_secure_info("file_path [%s]\n", file_path);

		if (_handle->input_path != NULL) {
			ret = __video_util_check_transcode_is_busy(_handle, &is_busy);
			if (ret != VIDEO_UTIL_ERROR_NONE)
				return ret;

			if (is_busy) {
				video_util_error("BUSY!! Transcoding is already running.\n");
				return VIDEO_UTIL_ERROR_BUSY;
			} else {
				ret = __video_util_destroy_transcode_handle(_handle);
				if (ret != VIDEO_UTIL_ERROR_NONE)
					return ret;

				UTIL_SAFE_FREE(_handle->input_path);
			}
		}

		_handle->input_path = strdup(file_path);
		if (_handle->input_path == NULL) {
			video_util_error("OUT_OF_MEMORY(0x%08x)", VIDEO_UTIL_ERROR_OUT_OF_MEMORY);
			return VIDEO_UTIL_ERROR_OUT_OF_MEMORY;
		}
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_set_accurate_mode(video_util_h handle, bool accurate_mode)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if (_handle != NULL) {
		_handle->accurate_mode = accurate_mode;
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_set_video_codec(video_util_h handle, video_util_video_codec_e codec)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool is_busy = false;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if ((_handle != NULL) && (__video_util_check_video_codec(codec))) {
		ret = __video_util_check_transcode_is_busy(_handle, &is_busy);
		if (ret != VIDEO_UTIL_ERROR_NONE)
			return ret;

		if (is_busy) {
			video_util_error("BUSY!! Transcoding is already running.\n");
			return VIDEO_UTIL_ERROR_BUSY;
		} else {
			ret = __video_util_destroy_transcode_handle(_handle);
			if (ret != VIDEO_UTIL_ERROR_NONE)
				return ret;
		}

		_handle->video_codec = codec;
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_set_audio_codec(video_util_h handle, video_util_audio_codec_e codec)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool is_busy = false;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if ((_handle != NULL) && (__video_util_check_audio_codec(codec))) {
		ret = __video_util_check_transcode_is_busy(_handle, &is_busy);
		if (ret != VIDEO_UTIL_ERROR_NONE)
			return ret;

		if (is_busy) {
			video_util_error("BUSY!! Transcoding is already running.\n");
			return VIDEO_UTIL_ERROR_BUSY;
		} else {
			ret = __video_util_destroy_transcode_handle(_handle);
			if (ret != VIDEO_UTIL_ERROR_NONE)
				return ret;
		}

		_handle->audio_codec = codec;
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_set_file_format(video_util_h handle, video_util_file_format_e format)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool is_busy = false;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if ((_handle != NULL) && (__video_util_check_file_format(format))) {
		ret = __video_util_check_transcode_is_busy(_handle, &is_busy);
		if (ret != VIDEO_UTIL_ERROR_NONE)
			return ret;

		if (is_busy) {
			video_util_error("BUSY!! Transcoding is already running.\n");
			return VIDEO_UTIL_ERROR_BUSY;
		} else {
			ret = __video_util_destroy_transcode_handle(_handle);
			if (ret != VIDEO_UTIL_ERROR_NONE)
				return ret;
		}

		_handle->file_format = format;
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_set_resolution(video_util_h handle, int width, int height)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if ((_handle != NULL) && (__video_util_check_resolution(width, height))) {
		_handle->width = width;
		_handle->height = height;
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_set_fps(video_util_h handle, int fps)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if (_handle && __video_util_check_fps(fps)) {
		_handle->fps = fps;
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_start_transcoding(video_util_h handle, unsigned long start, unsigned long duration, const char *out_path, video_util_transcoding_progress_cb progress_cb, video_util_transcoding_completed_cb completed_cb, void *user_data)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	mm_seek_mode_e accurate_mode = MM_SEEK_NUM;
	bool is_busy = false;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	video_util_debug("start [%d] duration [%d]\n", start, duration);

	if (_handle && (__video_util_check_duration(duration)) && (UTIL_STRING_VALID(_handle->input_path)) && (UTIL_STRING_VALID(out_path)) && completed_cb) {
		if (!_handle->transcode_h) {
			ret = __video_util_create_transcode_handle(_handle);
			if (ret != MM_ERROR_NONE)
				return ret;

			if (!_handle->transcode_h) {
				video_util_error("INVALID_OPERATION(0x%08x)", VIDEO_UTIL_ERROR_INVALID_OPERATION);
				return VIDEO_UTIL_ERROR_INVALID_OPERATION;
			}
		} else {
			ret = __video_util_check_transcode_is_busy(_handle, &is_busy);
			if (ret != VIDEO_UTIL_ERROR_NONE)
				return ret;

			if (is_busy) {
				video_util_error("BUSY!! Transcoding is already running.\n");
				return VIDEO_UTIL_ERROR_BUSY;
			}
		}

		video_util_debug("width [%d] height [%d] fps [%d]v_codec [%d] a_codec [%d] file_format [%d] accurate [%d]\n", _handle->width, _handle->height, _handle->fps, _handle->video_codec, _handle->audio_codec, _handle->file_format, _handle->accurate_mode);

		_handle->_util_cb = (video_util_cb_s *)calloc(1, sizeof(video_util_cb_s));
		if (!_handle->_util_cb) {
			video_util_error("Error _util_cb");
			return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
		}
		_handle->_util_cb->user_data = user_data;
		_handle->_util_cb->transcode_completed_cb = completed_cb;
		_handle->_util_cb->transcode_progress_cb = progress_cb;

		if (_handle->accurate_mode)
			accurate_mode = MM_SEEK_ACCURATE;
		else
			accurate_mode = MM_SEEK_INACCURATE;

		ret = mm_transcode(_handle->transcode_h, _handle->width, _handle->height, _handle->fps, start, duration, accurate_mode, out_path, (mm_transcode_progress_callback)__video_util_transcode_progress_cb, (mm_transcode_completed_callback)__video_util_transcode_completed_cb, (void *)_handle->_util_cb);

		if (ret != MM_ERROR_NONE) {
			UTIL_SAFE_FREE(_handle->_util_cb);

			if (ret == MM_ERROR_TRANSCODE_INVALID_VALUE) {
				video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
				return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
			} else if (ret == MM_ERROR_TRANSCODE_NOT_SUPPORT_FORMAT) {
				video_util_error("NOT_SUPPORTED_FORMAT(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED_FORMAT);
				return VIDEO_UTIL_ERROR_NOT_SUPPORTED_FORMAT;
			} else {
				video_util_error("INVALID_OPERATION(0x%08x)", ret);
				return VIDEO_UTIL_ERROR_INVALID_OPERATION;
			}
		}
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_cancel_transcoding(video_util_h handle)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if (_handle && (_handle->transcode_h)) {
		ret = mm_transcode_cancel(_handle->transcode_h);
		if (ret != MM_ERROR_NONE) {
			video_util_error("INVALID_OPERATION(0x%08x)", ret);
			return VIDEO_UTIL_ERROR_INVALID_OPERATION;
		}
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	UTIL_SAFE_FREE(_handle->_util_cb);
	video_util_debug("FREE(_handle->_util_cb)");

	return ret;
}

int video_util_get_progress_transcoding(video_util_h handle, unsigned long *current_position, unsigned long *duration)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	int value = 0;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if (_handle && current_position && duration) {
		if (_handle->transcode_h) {
			ret = mm_transcode_get_attrs(_handle->transcode_h, (mm_containerformat_e *)&value, (mm_videoencoder_e *)&value, (mm_audioencoder_e *)&value, current_position, duration, (unsigned int *)&value, (unsigned int *)&value);
			if (ret != MM_ERROR_NONE) {
				video_util_error("INVALID_OPERATION(0x%08x)", ret);
				return VIDEO_UTIL_ERROR_INVALID_OPERATION;
			}
		} else {
			video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
			return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
		}
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_foreach_supported_file_format(video_util_h handle, video_util_supported_file_format_cb callback, void *user_data)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if (_handle && callback) {
		ret = __video_util_foreach_supported_type(VIDEO_UTIL_TYPE_FORMAT, (video_util_supported_type_cb)callback, user_data);
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_foreach_supported_video_codec(video_util_h handle, video_util_supported_video_encoder_cb callback, void *user_data)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	}
	_handle = (video_util_s *)handle;
	if (_handle && callback) {
		ret = __video_util_foreach_supported_type(VIDEO_UTIL_TYPE_VIDEO_ENC, (video_util_supported_type_cb)callback, user_data);
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}

int video_util_foreach_supported_audio_codec(video_util_h handle, video_util_supported_audio_encoder_cb callback, void *user_data)
{
	int ret = VIDEO_UTIL_ERROR_NONE;
	video_util_s *_handle = NULL;
	bool transcoder_supported = false;
	if (0 == system_info_get_platform_bool(TRANSCODER_FEATURE_PATH, &transcoder_supported)) {
		if (false == transcoder_supported) {
			video_util_error("VIDEO_UTIL_ERROR_NOT_SUPPORTED(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
			return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
		}
	} else {
		video_util_error("Error - System Info of Platform(0x%08x)", VIDEO_UTIL_ERROR_NOT_SUPPORTED);
		return VIDEO_UTIL_ERROR_NOT_SUPPORTED;
	}

	_handle = (video_util_s *)handle;
	if (_handle && callback) {
		ret = __video_util_foreach_supported_type(VIDEO_UTIL_TYPE_AUDIO_ENC, (video_util_supported_type_cb)callback, user_data);
	} else {
		video_util_error("INVALID_PARAMETER(0x%08x)", VIDEO_UTIL_ERROR_INVALID_PARAMETER);
		return VIDEO_UTIL_ERROR_INVALID_PARAMETER;
	}

	return ret;
}
