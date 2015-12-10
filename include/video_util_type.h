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

#ifndef __TIZEN_MEDIA_VIDEO_UTIL_TYPE_H__
#define __TIZEN_MEDIA_VIDEO_UTIL_TYPE_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef DEPRECATED_API
#define DEPRECATED_API __attribute__ ((deprecated))
#endif

#define VIDEO_UTIL_ERROR_CLASS TIZEN_ERROR_VIDEO_UTIL
#define TRANSCODER_FEATURE_PATH "http://tizen.org/feature/multimedia.transcoder"
/**
 * @file video_util_type.h
 */

/**
* @addtogroup CAPI_MEDIA_VIDEO_UTIL_MODULE
* @{
*/

/**
 * @ingroup CAPI_MEDIA_VIDEO_UTIL_MODULE
 * @brief Enumeration for errors of a video util.
 * @since_tizen 2.3
 */
typedef enum {
	VIDEO_UTIL_ERROR_NONE = TIZEN_ERROR_NONE,							/**< Successful */
	VIDEO_UTIL_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,		/**< Invalid parameter */
	VIDEO_UTIL_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY,				/**< Out of memory */
	VIDEO_UTIL_ERROR_INVALID_OPERATION = TIZEN_ERROR_INVALID_OPERATION,			/**< Invalid operation */
	VIDEO_UTIL_ERROR_BUSY = TIZEN_ERROR_RESOURCE_BUSY,						/**< Device or resource busy */
	VIDEO_UTIL_ERROR_NOT_SUPPORTED_FORMAT = TIZEN_ERROR_VIDEO_UTIL | 0x01,	/**< Not supported format */
	VIDEO_UTIL_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED,			/**< Permission denied  */
	VIDEO_UTIL_ERROR_NOT_SUPPORTED = TIZEN_ERROR_NOT_SUPPORTED		/**< Not supported */
} video_util_error_e;

/**
 * @ingroup CAPI_MEDIA_VIDEO_UTIL_MODULE
 * @brief Enumeration for video codec.
 * @since_tizen 2.3
 */
typedef enum {
	VIDEO_UTIL_VIDEO_CODEC_MPEG4 = 0,	/**< MPEG4 */
	VIDEO_UTIL_VIDEO_CODEC_H263,		/**< H.263 */
	VIDEO_UTIL_VIDEO_CODEC_H264,		/**< H.264 */
	VIDEO_UTIL_VIDEO_CODEC_NONE		/**< No transcoding for video */
} video_util_video_codec_e;

/**
 * @ingroup CAPI_MEDIA_VIDEO_UTIL_MODULE
 * @brief Enumeration for audio codec.
 * @since_tizen 2.3
 */
typedef enum {
	VIDEO_UTIL_AUDIO_CODEC_AAC = 0,		/**< AAC */
	VIDEO_UTIL_AUDIO_CODEC_AMRNB,	/**< AMRNB */
	VIDEO_UTIL_AUDIO_CODEC_NONE		/**< No transcoding for audio */
} video_util_audio_codec_e;

/**
 * @ingroup CAPI_MEDIA_VIDEO_UTIL_MODULE
 * @brief Enumeration for formats of a media.
 * @since_tizen 2.3
 */
typedef enum {
	VIDEO_UTIL_FILE_FORMAT_3GP = 0,		/**< 3GP */
	VIDEO_UTIL_FILE_FORMAT_MP4,		/**< MP4 */
	VIDEO_UTIL_FILE_FORMAT_MAX		/**< MAX */
} video_util_file_format_e;

/**
 * @ingroup CAPI_MEDIA_VIDEO_UTIL_MODULE
 * @brief Video util handle.
 * @since_tizen 2.3
 */
typedef struct video_util_s *video_util_h;

/**
 * @ingroup CAPI_MEDIA_VIDEO_UTIL_MODULE
 * @brief Called regularly during the video transcoding.
 * @since_tizen 2.3
 *
 * @details The interval of callback invocation depends on the framework.
 *
 * @param[in] current_position The current position in milliseconds
 * @param[in] duration The duration in seconds
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre This callback function is invoked if you register this callback using video_util_set_progress_transcoding_cb().
 *
 * @see video_util_start_transcoding()
 */
typedef void (*video_util_transcoding_progress_cb)(unsigned long current_position, unsigned long duration, void *user_data);

/**
 * @ingroup CAPI_MEDIA_VIDEO_UTIL_MODULE
 * @brief Called when transcoding is finished just before storing in the file.
 * @since_tizen 2.3
 *
 * @param[in] error_code The error code of video util
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre video_util_start_transcoding() will invoke this function.
 */
typedef void (*video_util_transcoding_completed_cb)(video_util_error_e error_code, void *user_data);

/**
 * @brief Called iteratively to notify you about the supported file formats.
 * @since_tizen 2.3
 *
 * @param[in] format The format of media files
 * @param[in] user_data The user data passed from the foreach function
 *
 * @return @c true to continue with the next iteration of the loop,
 *               otherwise @c false to break out of the loop
 *
 * @pre video_util_foreach_supported_file_format() will invoke this callback.
 */
typedef bool (*video_util_supported_file_format_cb)(video_util_file_format_e format, void *user_data);

/**
 * @brief Called iteratively to notify you of supported video codec.
 * @since_tizen 2.3
 *
 * @param[in] format The codec of video
 * @param[in] user_data The user data passed from the foreach function
 *
 * @return @c true to continue with the next iteration of the loop,
 *               otherwise @c false to break out of the loop
 *
 * @pre video_util_foreach_supported_video_codec() will invoke this callback.
 */
typedef bool (*video_util_supported_video_encoder_cb)(video_util_video_codec_e codec, void *user_data);

/**
 * @brief Called iteratively to notify you about the supported audio codec.
 * @since_tizen 2.3
 *
 * @param[in] format The codec of audio
 * @param[in] user_data The user data passed from the foreach function
 *
 * @return @c true to continue with the next iteration of the loop,
 *               otherwise @c false to break out of the loop
 *
 * @pre video_util_foreach_supported_audio_codec() will invoke this callback.
 */
typedef bool (*video_util_supported_audio_encoder_cb)(video_util_audio_codec_e codec, void *user_data);

__attribute__ ((deprecated)) typedef bool (*video_util_progress_transcoding_cb)(video_util_error_e error, unsigned long current_position, unsigned long duration, void *user_data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__TIZEN_MEDIA_VIDEO_UTIL_TYPE_H__*/
