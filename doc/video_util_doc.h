/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __TIZEN_MEDIA_VIDEO_UTIL_DOC_H__
#define __TIZEN_MEDIA_VIDEO_UTIL_DOC_H__

/**
 * @file video_util_doc.h
 * @brief This file contains high level documentation of the Multimedia Service.
 */

/**
 * @defgroup CAPI_MEDIA_VIDEO_UTIL_MODULE Video Util
 * @brief  The @ref CAPI_MEDIA_VIDEO_UTIL_MODULE  APIs provides functions that transcode a media file.
 * @ingroup CAPI_MEDIA_FRAMEWORK
 *
 * @section CAPI_MEDIA_VIDEO_UTIL_MODULE_HEADER Required Header
 *   \#include <video_util.h>
 *
 * @section CAPI_MEDIA_VIDEO_UTIL_MODULE_OVERVIEW Overview
 *
 * Video Util API provides support for transcoding of a media file. Codec and the format that the API supported :
 *- Video : mpge4/h.263/h.264
 *- Audio : aac/amrnb
 *- Format : 3gp/mp4/max
 *
 * @section CAPI_MEDIA_VIDEO_UTIL_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/multimedia.transcoder\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/tools/native-tools/manifest-text-editor#feature"><b>Feature Element</b>.</a>
 *
 * @subsection CAPI_MEDIA_VIDEO_UTIL_FOREACH_OPERATIONS Foreach Operations
 * <div><table class="doxtable" >
 *     <tr>
 *        <th><b>FOREACH</b></th>
 *        <th><b>CALLBACK</b></th>
 *        <th><b>DESCRIPTION</b></th>
 *     </tr>
 *     <tr>
 *        <td>video_util_foreach_supported_file_format()</td>
 *        <td>video_util_supported_file_format_cb()</td>
 *        <td>Retrieves the list of supported media format</td>
 *     </tr>
 *     <tr>
 *        <td>video_util_foreach_supported_video_codec()</td>
 *        <td>video_util_supported_video_encoder_cb()</td>
 *        <td>Retrieves the list of supported video codec</td>
 *     </tr>
 *     <tr>
 *        <td>video_util_foreach_supported_audio_codec()</td>
 *        <td>video_util_supported_audio_encoder_cb()</td>
 *        <td>Retrieves the list of supported audio codec</td>
 *     </tr>
 *</table></div>
 */

#endif // __TIZEN_MEDIA_VIDEO_UTIL_DOC_H__
