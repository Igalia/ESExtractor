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

#include <stdarg.h>
#include <string.h>

enum {
  ES_LOG_LEVEL_NONE = 0,
  ES_LOG_LEVEL_ERROR,
  ES_LOG_LEVEL_INFO,
  ES_LOG_LEVEL_DEBUG,
  ES_LOG_LEVEL_MEMDUMP,
  ES_LOG_LEVEL_MAX
};

class Logger {
  public:
  Logger() {
    m_level = ES_LOG_LEVEL_ERROR;

  }
  static Logger& instance()
  {
    static Logger instance;
    return instance;
  }
  int level() { return m_level;}
  void setLogLevel(uint8_t level) {
    m_level = level;
    if (m_level > ES_LOG_LEVEL_MAX)
      m_level=ES_LOG_LEVEL_DEBUG;
  }
  void createLog(const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
  }

  void createLogData(const uint8_t* buffer, uint32_t length, const char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    for (uint32_t i = 0; i < length; i++) {
      fprintf(stdout, "0x%.2X ", buffer[i]);
    }
    fprintf(stdout, "\n");
  }

  private:
    uint8_t m_level;
};

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOGGER(LEVEL, PREFIX, FMT, SUFFIX, ...) if (LEVEL <= Logger::instance().level()) Logger::instance().createLog(PREFIX "%s:%d:%s:\t"  FMT SUFFIX,__FILENAME__,__LINE__,__FUNCTION__, ##__VA_ARGS__)
#define LOGGER_DATA(LEVEL, DATA, LENGTH, PREFIX, FMT, SUFFIX, ...) if (LEVEL <= Logger::instance().level()) Logger::instance().createLogData(DATA, LENGTH, PREFIX "%s:%d:%s:\t"  FMT SUFFIX,__FILENAME__,__LINE__,__FUNCTION__, ##__VA_ARGS__)
#define ERR(FMT, ...) LOGGER(ES_LOG_LEVEL_ERROR, "ESE_ERROR\t", FMT, "\n", ##__VA_ARGS__)
#define INFO(FMT, ...) LOGGER(ES_LOG_LEVEL_INFO, "ESE_INFO\t", FMT, "\n", ##__VA_ARGS__)

#define DBG(FMT, ...) LOGGER(ES_LOG_LEVEL_DEBUG, "ESE_DEBUG\t", FMT, "\n", ##__VA_ARGS__)
#define MEM_DUMP(DATA, LENGTH, FMT, ...) LOGGER_DATA(ES_LOG_LEVEL_MEMDUMP, DATA, LENGTH, "ESE_MEMDUMP\t", FMT, "", ##__VA_ARGS__)

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

typedef enum ESExtractorVideoCodec {
  ES_EXTRACTOR_VIDEO_CODEC_UNKNOWN = 0,
  ES_EXTRACTOR_VIDEO_CODEC_H264,
  ES_EXTRACTOR_VIDEO_CODEC_H265,
} ESExtractorVideoCodec;

typedef enum ESExtractorPacketAlignment {
  ES_EXTRACTOR_PACKET_ALIGNMENT_NAL = 0,
  ES_EXTRACTOR_PACKET_ALIGNMENT_AU,
} ESExtractorPacketAlignment;

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
  ES_EXTRACTOR_NAL_STATE_NONE = 0,
  ES_EXTRACTOR_NAL_STATE_START,
  ES_EXTRACTOR_NAL_STATE_END,
} ESExtractorFrameState;

typedef struct _ESEPacket {
    uint8_t * data;
    int32_t data_size;
    int32_t packet_number;
    int64_t pts;
    int64_t dts;
    int64_t duration;
} ESEPacket;

class ESExtractor {
public:
  ESExtractor();
  ~ESExtractor();
  /// @brief This method will build the next frame (NAL or AU) available.
  /// @return
  ESExtractorResult processToNextFrame();
  ESExtractorVideoCodec getCodec() { return m_codec;};
  bool openFile(const char* fileName);
  /// @brief Reset the extractor state
  void reset (bool full = true);
  std::vector<unsigned char>* getCurrentFrame() { return &m_currentFrame;}
  /// @brief Returns the NAL count.
  /// @return
  int nalCount() { return m_nalCount;}
  /// @brief Returns the frame count.
  /// @return
  int frameCount() { return m_frameCount;}
  /// @brief Set frame format, either NAL or a complete access unit.
  /// @param alignment
  void setAlignment(ESExtractorPacketAlignment alignment) {m_alignment = alignment;}


private:
  // Read data from the stream buffer or file if buffer is too small
  ESExtractorResult readStream();
  // Read data from the file
  std::vector<unsigned char> readFile(int32_t data_size, int32_t pos = 0, bool append = false );
  // Prepare the next frame available from the given buffer at given position.
  std::vector<unsigned char> prepareFrame(std::vector<unsigned char> buffer, uint32_t start, uint32_t end);
  // Parse the stream to find MPEG start code and find frame boundaries
  int parseStream(int32_t start_position);
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
  ESExtractorPacketAlignment m_alignment;
  uint32_t m_nalCount;
  int32_t m_readSize;
  bool m_eos;
  std::vector<unsigned char> m_buffer;
  std::vector<unsigned char> m_nextNAL;
  bool                       m_audNalDetected;
  std::vector<unsigned char> m_nextFrame;
  std::vector<unsigned char> m_currentFrame;
  uint32_t m_frameCount;
};


#ifdef __cplusplus
extern "C" {
#endif

ES_EXTRACTOR_API
ESExtractor * es_extractor_new (const char * uri, ESExtractorPacketAlignment alignment);

ES_EXTRACTOR_API
ESExtractorResult es_extractor_read_frame (ESExtractor * demuxer, ESEPacket ** pkt);

ES_EXTRACTOR_API
void es_extractor_clear_packet (ESEPacket * pkt);

ES_EXTRACTOR_API
ESExtractorVideoCodec es_extractor_video_codec(ESExtractor * demuxer);

ES_EXTRACTOR_API
int es_extractor_frame_count (ESExtractor * extractor);

ES_EXTRACTOR_API
void es_extractor_teardown (ESExtractor * demuxer);

ES_EXTRACTOR_API
void es_extractor_set_log_level (uint8_t level);

#ifdef __cplusplus
}
#endif