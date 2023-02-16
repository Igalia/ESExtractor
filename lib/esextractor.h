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

#include <iostream>
#include <fstream>
#include <istream>
#include <vector>

#ifdef _DEBUG
#   define DBG(FMT, ...) printf(FMT "\n", ##__VA_ARGS__)
#else
#   define DBG(FMT, ...)
#endif
#define INFO(FMT, ...) printf(FMT "\n", ##__VA_ARGS__)
#define ERR(FMT, ...) printf("ERROR: " FMT "\n", ##__VA_ARGS__)

#if defined _WIN32 || defined __CYGWIN__
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


typedef enum ESExtractorVideoCodec {
  ES_EXTRACTOR_VIDEO_CODEC_UNKNOWN = 0,
  ES_EXTRACTOR_VIDEO_CODEC_H264,
  ES_EXTRACTOR_VIDEO_CODEC_H265,
} ESExtractorVideoCodec;

typedef enum _ESExtractorResult
{
  /*< public >*/
  ES_EXTRACTOR_RESULT_NEW_PACKET = 0,
  ES_EXTRACTOR_RESULT_LAST_PACKET,
  ES_EXTRACTOR_RESULT_EOS,
  ES_EXTRACTOR_RESULT_NO_PACKET ,
  ES_EXTRACTOR_RESULT_ERROR,
} ESExtractorResult;

typedef enum _ESExtractorFrameState {
  ES_EXTRACTOR_FRAME_STATE_NONE = 0,
  ES_EXTRACTOR_FRAME_STATE_START,
  ES_EXTRACTOR_FRAME_STATE_END,
} ESExtractorFrameState;
class ESExtractor {
public:
  ESExtractor();
  ~ESExtractor();

  ESExtractorResult readStream();
  ESExtractorVideoCodec getCodec() { return m_codec;};
  bool openFile(const char* fileName);
  void reset ();
  std::vector<unsigned char>* getNextFrame() { return &m_nextFrame;}
  int frameCount() { return m_frameCount;}

private:
  std::vector<unsigned char> readFile(int32_t data_size, int32_t pos = 0, bool append = false );

  void printNalType(int nalUnitType);
  std::vector<unsigned char> prepareFrame(std::vector<unsigned char> buffer, uint32_t start, uint32_t end);
  int parseStream(int32_t start_position);
  void printBufferHex(std::vector<unsigned char> buffer);
  bool isH264(std::vector<unsigned char> buffer);
  bool isH265(std::vector<unsigned char> buffer);

  std::ifstream m_file;
  int32_t m_filePosition;
  int32_t m_fileSize;

  int32_t m_bufferPosition;
  int32_t m_frameStartPos;
  int32_t m_frameStartCodeLen;
  bool m_mpeg_detected;
  ESExtractorVideoCodec m_codec;
  ESExtractorFrameState m_frameState;
  uint32_t m_frameCount;
  int32_t m_readSize;
  bool m_eos;
  std::vector<unsigned char> m_buffer;
  std::vector<unsigned char> m_nextFrame;
};


#ifdef __cplusplus
extern "C" {
#endif

ES_EXTRACTOR_API
ESExtractor * es_extractor_new (const char * uri);

ES_EXTRACTOR_API
ESExtractorResult es_extractor_read_frame (ESExtractor * demuxer, uint8_t ** packet, int  * size);

ES_EXTRACTOR_API
ESExtractorVideoCodec es_extractor_video_codec(ESExtractor * demuxer);

ES_EXTRACTOR_API
int es_extractor_frame_count (ESExtractor * extractor);

ES_EXTRACTOR_API
void es_extractor_teardown (ESExtractor * demuxer);

#ifdef __cplusplus
}
#endif