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

#ifndef __ESE_EXTRACTOR_H__
#define __ESE_EXTRACTOR_H__

#include "esestream.h"


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

class ESExtractor {
public:
  ESExtractor();
  ~ESExtractor();
  bool prepare(const char *uri, const char* options);
  /// @brief This method will build the next frame (NAL or AU) available.
  /// @return
  ESEResult processToNextFrame();
  ESEVideoCodec codec();
  const char* codec_name();

  /// @brief Reset the extractor state
  ESEPacket* currentPacket();
  /// @brief Returns the frame count.
  /// @return
  int frameCount();

  bool openFile (const char *uri);

private:

  ESEStream* m_stream;
};


#ifdef __cplusplus
extern "C" {
#endif

ES_EXTRACTOR_API
ESExtractor * es_extractor_new (const char * uri, const char* options);

ES_EXTRACTOR_API
ESEResult es_extractor_read_frame (ESExtractor * demuxer, ESEPacket ** pkt);

ES_EXTRACTOR_API
void es_extractor_clear_packet (ESEPacket * pkt);

ES_EXTRACTOR_API
ESEVideoCodec es_extractor_video_codec(ESExtractor * extractor);

ES_EXTRACTOR_API
const char* es_extractor_video_codec_name(ESExtractor * extractor);

ES_EXTRACTOR_API
int es_extractor_frame_count (ESExtractor * extractor);

ES_EXTRACTOR_API
void es_extractor_teardown (ESExtractor * demuxer);

ES_EXTRACTOR_API
void es_extractor_set_log_level (uint8_t level);

#ifdef __cplusplus
}
#endif
#endif //__ESE_EXTRACTOR_H__