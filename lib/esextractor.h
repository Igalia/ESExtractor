/* ESExtractor
 * Copyright (C) 2022 Igalia, S.L.
 *     Author: Stephane Cerveau <scerveau@igalia.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You
 * may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#pragma once

#include <cstdint>

#if (defined _WIN32 || defined __CYGWIN__) && !defined(ES_STATIC_COMPILATION)
  #ifdef BUILDING_ES_EXTRACTOR
    #define ES_EXTRACTOR_API __declspec(dllexport)
  #else
    #define ES_EXTRACTOR_API __declspec(dllimport)
  #endif
#else
  #ifdef BUILDING_ES_EXTRACTOR
      #define ES_EXTRACTOR_API __attribute__ ((visibility ("default")))
  #else
      #define ES_EXTRACTOR_API
  #endif
#endif

struct ESExtractor;

struct ESEPacket {
  uint8_t *data;
  int32_t data_size;
  int32_t packet_number;
  uint64_t pts;
  uint64_t dts;
  uint64_t duration;
};

typedef enum ESEResult {
  /*< public >*/
  ESE_RESULT_NEW_PACKET = 0,
  ESE_RESULT_LAST_PACKET,
  ESE_RESULT_EOS,
  ESE_RESULT_NO_PACKET,
  ESE_RESULT_ERROR,
} ESEResult;

typedef enum ESEVideoCodec {
  ESE_VIDEO_CODEC_UNKNOWN = 0,
  ESE_VIDEO_CODEC_H264,
  ESE_VIDEO_CODEC_H265,
  ESE_VIDEO_CODEC_VP8,
  ESE_VIDEO_CODEC_VP9,
  ESE_VIDEO_CODEC_AV1,
} ESEVideoCodec;

#ifdef __cplusplus
extern "C" {
#endif

ES_EXTRACTOR_API
ESExtractor * es_extractor_new (const char * uri, const char* options);

ES_EXTRACTOR_API
ESEResult es_extractor_read_packet (ESExtractor * demuxer, ESEPacket ** pkt);

ES_EXTRACTOR_API
void es_extractor_clear_packet (ESEPacket * pkt);

ES_EXTRACTOR_API
ESEVideoCodec es_extractor_video_codec (ESExtractor * extractor);

ES_EXTRACTOR_API
const char* es_extractor_video_codec_name (ESExtractor * extractor);

ES_EXTRACTOR_API
int es_extractor_packet_count (ESExtractor * extractor);

ES_EXTRACTOR_API
void es_extractor_teardown (ESExtractor * demuxer);

ES_EXTRACTOR_API
void es_extractor_set_log_level (uint8_t level);

#ifdef __cplusplus
}
#endif
