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


#ifndef __TIZEN_MEDIA_VIDEO_UTIL_H__
#define __TIZEN_MEDIA_VIDEO_UTIL_H__


#include <video_util_type.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup CAPI_MEDIA_VIDEO_UTIL_MODULE
 * @{
 */

/**
 * @brief Creates a handle to video util.
 * @details This function creates a handle to video util.
 * @remarks The @a video util handle must be released with video_util_destroy() by you.
 * @param[out] handle A handle to video util
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #VIDEO_UTIL_ERROR_OUT_OF_MEMORY Out of memory
 * @see video_util_destroy()
 *
 */
int video_util_create(video_util_h *handle);

/**
 * @brief Destroys a handle to video util.
 * @details The function frees all resources related to the video util handle. The video util
 * handle no longer can be used to perform any operation. A new video util handle
 * has to be created before the next usage.
 *
 * @param[in] handle The handle to video util
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see video_util_create()
 *
 */
int video_util_destroy(video_util_h handle);

/**
 * @brief Sets the video util's file path.
 * @details This function sets the @a source path to transcode.
 * @param[in] handle The handle to video util
 * @param[in] path The source file path
 * @return return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #VIDEO_UTIL_ERROR_OUT_OF_MEMORY Out of memory
 * @see video_util_create()
 * @see video_util_destroy()
 */
int video_util_set_file_path(video_util_h handle, const char *path);

/**
 * @brief Sets the video util's accurate mode.
 * @remarks If you do not set, the default value is @c false.
 * @param[in] handle The handle to video util
 * @param [in] mode @c true, user can get an accurated frame for given the duration in video_util_start_transcoding()\n
 * @c false, user can only get the nearest i-frame
 * @return return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see video_util_create()
 * @see video_util_destroy()
 */
int video_util_set_accurate_mode(video_util_h handle, bool mode);


/**
 * @brief  Sets the video codec for encoding video stream.
 * @remarks You can get available list of video codec by using video_util_foreach_supported_video_codec().\n
 * If you do not set, the default codec is #VIDEO_UTIL_VIDEO_CODEC_MPEG4.
 * @param[in] handle The handle to video util
 * @param[in] codec The video codec
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see video_util_create()
 * @see video_util_destroy()
 * @see video_util_get_video_codec()
 * @see video_util_foreach_supported_video_codec()
 */
int video_util_set_video_codec(video_util_h handle, video_util_video_codec_e codec);

/**
 * @brief  Sets the audio codec for encoding audio stream.
 * @remarks You can get available list of audio codec by using video_util_foreach_supported_audio_codec().\n
 * If you do not set, the default codec is #VIDEO_UTIL_AUDIO_CODEC_AAC.
 * @param[in] handle The handle to video util
 * @param[in] codec The audio codec
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see video_util_create()
 * @see video_util_destroy()
 * @see video_util_get_audio_codec()
 * @see video_util_foreach_supported_audio_codec()
 */
int video_util_set_audio_codec(video_util_h handle, video_util_audio_codec_e codec);

/**
 * @brief  Sets the file format for transcoding media stream.
 * @remarks You can get available list of media format by using video_util_foreach_supported_file_format().\n
 * If you do not set, the default file format is #VIDEO_UTIL_FILE_FORMAT_3GP.
 * @param[in] handle The handle to video util
 * @param[in] format The media file format
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see video_util_create()
 * @see video_util_destroy()
 * @see video_util_get_file_format()
 * @see video_util_foreach_supported_file_format()
 */
int video_util_set_file_format(video_util_h handle, video_util_file_format_e format);

/**
 * @brief Sets the resolution(width and height).
 * @remarks If you do not set, the default resolution is original size.
 * @param[in] handle The handle to video util
 * @param[in] width The media's width, if the width is 0, it set original size.(minimum value is 128)
 * @param[in] height The media's height, if the height is 0, it set original size.(minimum value is 96)
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see video_util_create()
 * @see video_util_destroy()
 * @see video_util_get_resolution()
 */
int video_util_set_resolution(video_util_h handle, int width, int height);

/**
 * @brief Sets the frame rate.
 * @remarks  If  fps is set 0, the default is original fps from source.
 * @param[in] handle The handle to video util
 * @param[in] fps The frame rate(minimum value is 5, maximum value is 30)
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_EROR_NONE Successful
 * @retval #VIDEO_UTIL_EROR_INVALID_PARAMETER Invalid parameter
 * @see video_util_create()
 * @see video_util_destroy()
 */
int video_util_set_fps(video_util_h handle, int fps);

/**
 * @brief Transcodes the video for given video util handle.
 * @details This function starts the transcoding from start time and for given duration.\n
#video_util_transcoding_progress_cb() function is called during the video transcoding regularly after some interval.\n
#video_util_transcoding_completed_cb() function is called and the transcoded video will be saved at the given output path when transcoding is finished corresponding callback.
 * @remarks If there is already exists same file in file system, then old file will be overwritten.
 * @param[in] handle The handle to video util
 * @param[in] start The start position to transcode
 * @param[in] duration The duration is in seconds.\n
 If duration is 0, transcoding happens until end of the video.
 * @param[out] out_path The file path to save
 * @param[in] progress_cb The callback function to invoke
 * @param[in] completed_cb The callback function to invoke
 * @param[in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #VIDEO_UTIL_ERROR_INVALID_OPERATION Invalid operation
 * @see video_util_create()
 * @see video_util_destroy()
 * @see video_util_transcoding_progress_cb()
 * @see video_util_transcoding_completed_cb()
 * @see video_util_get_progress_transcoding()
 * @see video_util_cancel_transcoding()
 */
int video_util_start_transcoding(video_util_h handle, unsigned long start, unsigned long duration, const char *out_path, video_util_transcoding_progress_cb progress_cb, video_util_transcoding_completed_cb completed_cb,void *user_data);

/**
 * @brief Cancels to transcode the video for given video util handle.
 * @details This function cancels the transcoding for given video util handle.\n
 * @remarks If this function is invoked during transcoding, the data transcoded is discard.
 * @param[in] handle The handle to video util
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #VIDEO_UTIL_ERROR_INVALID_OPERATION Invalid operation
 * @see video_util_create()
 * @see video_util_destroy()
 * @see video_util_start_transcoding()
 */
int video_util_cancel_transcoding(video_util_h handle);

/**
 * @brief Retreives the current position and duration of the video transcoding.
 * @param[in] handle The handle to video util
 * @param[out] current_position The transcoded current position
 * @param[out] duration The duration is in seconds.\n
 If duration is 0, transcoding happens until end of the video.
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre video_util_start_transcoding()
 * @see video_util_create()
 * @see video_util_destroy()
 */
int video_util_get_progress_transcoding(video_util_h handle, unsigned long *current_position, unsigned long *duration);

/**
 * @brief  Retrieves all supported media format by invoking a specific callback for each supported media format.
 * @remarks The callback invocation depends on the codec.
 * @param[in] handle The handle to video util
 * @param[in] callback The callback function to invoke
 * @param[in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  video_util_supported_file_format_cb() will be invoked
 * @see video_util_get_file_format()
 * @see video_util_set_file_format()
 * @see video_util_supported_file_format_cb()
 */
int video_util_foreach_supported_file_format(video_util_h handle, video_util_supported_file_format_cb callback, void *user_data);

/**
 * @brief  Retrieves all supported video encoder by invoking a specific callback for each supported video encoder.
 * @remarks The callback invocation depends on the file format.
 * @param[in] handle The handle to video util
 * @param[in] callback The callback function to invoke
 * @param[in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  video_util_supported_video_encoder_cb() will be invoked
 * @see video_util_get_video_codec()
 * @see video_util_set_video_codec()
 * @see video_util_supported_video_encoder_cb()
 */
int video_util_foreach_supported_video_codec(video_util_h handle, video_util_supported_video_encoder_cb callback, void *user_data);

/**
 * @brief  Retrieves all supported audio encoder by invoking a specific callback for each supported audio encoder.
 * @remarks The callback invocation depends on the file format.
 * @param[in] handle The handle to video util
 * @param[in] callback The callback function to invoke
 * @param[in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #VIDEO_UTIL_ERROR_NONE Successful
 * @retval #VIDEO_UTIL_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  video_util_supported_audio_encoder_cb() will be invoked
 * @see video_util_get_audio_codec()
 * @see video_util_set_audio_codec()
 * @see video_util_supported_audio_encoder_cb()
 */
int video_util_foreach_supported_audio_codec(video_util_h handle, video_util_supported_audio_encoder_cb callback, void *user_data);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__TIZEN_MEDIA_VIDEO_UTIL_H__ */
