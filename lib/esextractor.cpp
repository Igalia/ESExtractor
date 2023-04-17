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

#include "esextractor.h"
#include "eseivfstream.h"
#include "esenalstream.h"
#include "eselogger.h"

#include <memory>
#include <cassert>

struct ESExtractor {
  ESExtractor (const char *uri) {
    switch (ese_stream_probe_video_format (uri)) {
    case ESE_VIDEO_FORMAT_NAL:
      m_stream = std::make_unique<ESENALStream>();
      break;
    case ESE_VIDEO_FORMAT_IVF:
      m_stream = std::make_unique<ESEIVFStream>();
      break;
    default:
      assert(true);
    }
  }

  bool prepare(const char *uri, const char *options) { return m_stream->prepare(uri, options); }
  /// @brief This method will build the next frame (NAL or AU) available.
  /// @return
  ESEResult processToNextPacket() { return m_stream->processToNextFrame(); }
  ESEVideoCodec codec() { return m_stream->codec(); }

  const char *codec_name() {
    switch (codec()) {
    case ESE_VIDEO_CODEC_H264:
      return "h264";
    case ESE_VIDEO_CODEC_H265:
      return "h265";
    case ESE_VIDEO_CODEC_VP8:
      return "vp8";
    case ESE_VIDEO_CODEC_VP9:
      return "vp9";
    case ESE_VIDEO_CODEC_AV1:
      return "av1";
    default:
      return "unknown";
    };
  }
  /// @brief Reset the extractor state
  ESEPacket *currentPacket() { return m_stream->currentPacket(); }
  /// @brief Returns the packet count.
  /// @return
  int packetCount() { return m_stream->frameCount(); }

  std::unique_ptr<ESEStream> m_stream;
};

//C API

ESExtractor *
es_extractor_new (const char *uri, const char *options)
{
  ESExtractor *extractor = new ESExtractor (uri);
  if (extractor->prepare (uri, options))
    return extractor;

  es_extractor_teardown (extractor);
  return NULL;
}

ESEResult
es_extractor_read_packet (ESExtractor *extractor, ESEPacket ** packet)
{
  ESEResult res = extractor->processToNextPacket ();
  if(res <  ESE_RESULT_EOS)
    *packet = extractor->currentPacket ();
  else
    *packet = nullptr;

  return res;
}

ESEVideoCodec
es_extractor_video_codec (ESExtractor *extractor)
{
  return extractor->codec ();
}

const char *
es_extractor_video_codec_name (ESExtractor *extractor)
{
  return extractor->codec_name ();
}

int
es_extractor_packet_count (ESExtractor *extractor)
{
  return extractor->packetCount ();
}

void
es_extractor_clear_packet (ESEPacket *pkt)
{
  if (pkt)
    delete pkt;
}

void
es_extractor_teardown (ESExtractor *extractor)
{
  delete extractor;
}

void
es_extractor_set_log_level (uint8_t level)
{
  Logger::instance ().setLogLevel (level);
}
