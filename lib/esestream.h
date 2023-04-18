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

#include "esereader.h"
#include "esextractor.h"

#include <vector>
#include <cstring>
#include <string>
#include <map>

#define ESE_MAKE_FOURCC(a,b,c,d) \
  ( (uint32_t)(a) | ((uint32_t) (b)) << 8  | ((uint32_t) (c)) << 16 | ((uint32_t) (d)) << 24 )

#define BUFFER_MAX_PROBE_LENGTH (128 * 1024)

ESEVideoFormat ese_stream_probe_video_format (const char* uri);

class ESEStream {
public:
  ESEStream (ESEVideoFormat format = ESE_VIDEO_FORMAT_UNKNOWN);
  virtual ~ESEStream ();

  virtual void reset();

  bool prepare (const char* uri, const char* options = nullptr);
  void setOptions (const char *options);
  virtual void parseOptions(const char* options);
  /// @brief This method will build the next frame (NAL or AU) available.
  /// @return
  virtual ESEResult processToNextFrame() {return ESE_RESULT_NO_PACKET;};
  
  int32_t scanMPEGHeader (ESEBuffer buffer, int32_t pos = 0);
  int32_t probeH26x ();
  int32_t probeIVF ();
  bool isH264 (ESEBuffer buffer);
  bool isH265 (ESEBuffer buffer);

  ESEVideoCodec codec() { return m_codec;}
  ESEVideoFormat format() { return m_format;}
  ESEBuffer * currentFrame() { return &m_currentFrame;}
  ESEPacket*                  currentPacket();

  /// @brief Returns the frame count.
  /// @return
  int frameCount() { return m_frameCount;}

protected:
  ESEReader                   m_reader;

  virtual ESEBuffer  getStartCode() {return {};}

  // Prepare the next frame available from the given buffer at given position.
  ESEBuffer  prepareFrame(ESEBuffer  buffer, uint32_t start, uint32_t end);
  ESEPacket* prepareNextPacket (uint64_t pts = 0, uint64_t dts = 0, uint64_t duration = 0);

  ESEVideoCodec                       m_codec;
  ESEVideoFormat                      m_format;
  std::map<std::string, std::string>  m_options;
  
  bool                                m_eos;
  ESEBuffer                           m_buffer;
  uint32_t                            m_bufferPosition;

  ESEBuffer                           m_currentFrame;
  uint32_t                            m_frameCount;
  ESEPacket*                          m_currentPacket;
  ESEPacket*                          m_nextPacket;
};
