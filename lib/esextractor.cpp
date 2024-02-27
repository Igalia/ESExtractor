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
#include <cassert>

#include "eseivfstream.h"
#include "eselogger.h"
#include "esenalstream.h"
#include "eseutils.h"
#include "esextractor.h"

struct ESExtractor {

  ESExtractor ()
  {
  }

  ESEVideoFormat format ()
  {
    return m_stream->format ();
  }

  ESEVideoCodec codec ()
  {
    return m_stream->codec ();
  }

  const char *codec_name ()
  {
    switch (codec ()) {
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

  int packetCount ()
  {
    return m_stream->frameCount ();
  }

  ESEPacket *currentPacket ()
  {
    return m_stream->currentPacket ();
  }

  ESEResult processToNextPacket ()
  {
    return m_stream->processToNextFrame ();
  }

  bool prepare (const char *uri, const char *options)
  {
    ESEVideoFormat format = ESE_VIDEO_FORMAT_UNKNOWN;

    m_stream = make_unique<ESEStream> ();
    if (m_stream->prepare (uri, options))
      format = ese_stream_probe_video_format (m_stream.get ());

    m_stream = nullptr;
    if (format == ESE_VIDEO_FORMAT_NAL) {
      m_stream = make_unique<ESENALStream> ();
    } else if (format == ESE_VIDEO_FORMAT_IVF) {
      m_stream = make_unique<ESEIVFStream> ();
    }
    if (m_stream && m_stream->prepare (uri, options)) {
      return (m_stream->processToNextFrame () <= ESE_RESULT_ERROR);
    }
    return false;
  }

  bool prepare_data (ese_read_buffer_func func, void *data, const char *options)
  {
    ESEVideoFormat format = ESE_VIDEO_FORMAT_UNKNOWN;

    m_stream = make_unique<ESEStream> ();
    if (m_stream->prepare (func, data, options))
      format = ese_stream_probe_video_format (m_stream.get ());
    m_stream = nullptr;
    if (format == ESE_VIDEO_FORMAT_NAL) {
      m_stream = make_unique<ESENALStream> ();
    } else if (format == ESE_VIDEO_FORMAT_IVF) {
      m_stream = make_unique<ESEIVFStream> ();
    }
    if (m_stream && m_stream->prepare (func, data, options)) {
      return (m_stream->processToNextFrame () <= ESE_RESULT_ERROR);
    }
    return false;
  }

  void setOptions (const char *options)
  {
    m_stream->reset ();
    m_stream->setOptions (options);
    m_stream->processToNextFrame ();
  }

  void setBufferReadLength (size_t len)
  {
    m_stream->setBufferReadLength (len);
  }

  std::unique_ptr<ESEStream> m_stream;
};

ESExtractor *
es_extractor_new (const char *uri, const char *options)
{
  ESExtractor *extractor = new ESExtractor ();
  if (extractor->prepare (uri, options)) {
    return extractor;
  }

  es_extractor_teardown (extractor);
  return NULL;
}

ESExtractor *
es_extractor_new_with_read_func (ese_read_buffer_func func, void *data, const char *options)
{
  ESExtractor *extractor = new ESExtractor ();
  if (extractor->prepare_data (func, data, options)) {
    return extractor;
  }

  es_extractor_teardown (extractor);
  return NULL;
}

void
es_extractor_set_options (ESExtractor *extractor, const char *options)
{
  ESE_CHECK_VOID (extractor != NULL);
  extractor->setOptions (options);
}

ESEResult
es_extractor_read_packet (ESExtractor *extractor, ESEPacket **packet)
{
  ESE_CHECK (extractor != NULL, ESE_RESULT_ERROR);
  ESEResult res = extractor->processToNextPacket ();
  if (res < ESE_RESULT_EOS)
    *packet = extractor->currentPacket ();
  else
    *packet = nullptr;

  return res;
}

void
es_extractor_set_buffer_read_length (ESExtractor *extractor, size_t buffer_read_length)
{
  ESE_CHECK_VOID (extractor != NULL);
  extractor->setBufferReadLength (buffer_read_length);
}

ESEVideoCodec
es_extractor_video_codec (ESExtractor *extractor)
{
  ESE_CHECK (extractor != NULL, ESE_VIDEO_CODEC_UNKNOWN);
  return extractor->codec ();
}

ESEVideoFormat
es_extractor_video_format (ESExtractor *extractor)
{
  ESE_CHECK (extractor != NULL, ESE_VIDEO_FORMAT_UNKNOWN);
  return extractor->format ();
}

const char *
es_extractor_video_codec_name (ESExtractor *extractor)
{
  ESE_CHECK (extractor != NULL, nullptr);
  return extractor->codec_name ();
}

int
es_extractor_packet_count (ESExtractor *extractor)
{
  ESE_CHECK (extractor != NULL, -1);
  return extractor->packetCount ();
}

void
es_extractor_clear_packet (ESEPacket *pkt)
{
  if (pkt) {
    std::free (pkt->data);
    delete pkt;
  }
}

void
es_extractor_teardown (ESExtractor *extractor)
{
  ESE_CHECK_VOID (extractor != NULL);
  delete (extractor);
}

void
es_extractor_set_log_level (uint8_t level)
{
  Logger::instance ().setLogLevel (level);
}
