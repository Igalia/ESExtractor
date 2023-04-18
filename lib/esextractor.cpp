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


ESExtractor::ESExtractor ():
m_stream (nullptr)
{
}

ESExtractor::~ESExtractor ()
{
  if (m_stream)
    delete m_stream;
}

ESEVideoFormat ESExtractor::format ()
{
  if (m_stream)
    return m_stream->format ();
  return ESE_VIDEO_FORMAT_UNKNOWN;
}

ESEVideoCodec ESExtractor::codec ()
{
  if (m_stream)
    return m_stream->codec ();
  return ESE_VIDEO_CODEC_UNKNOWN;
}

const char *
ESExtractor::codec_name ()
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

int
ESExtractor::packetCount ()
{
  if (m_stream)
    return m_stream->frameCount ();

  return 0;
}

ESEPacket *
ESExtractor::currentPacket ()
{
  if (m_stream)
    return m_stream->currentPacket ();

  return nullptr;
}

ESEResult ESExtractor::processToNextPacket ()
{
  if (m_stream)
    return m_stream->processToNextFrame ();

  return ESE_RESULT_ERROR;
}

bool
ESExtractor::prepare (const char *uri, const char *options)
{
  ESEVideoFormat format = ese_stream_probe_video_format (uri);
  m_stream = NULL;
  if (format == ESE_VIDEO_FORMAT_NAL) {
    m_stream = new ESENALStream ();
  } else if (format == ESE_VIDEO_FORMAT_IVF) {
    m_stream = new ESEIVFStream ();
  }
  if (m_stream && m_stream->prepare (uri, options)) {
    return (m_stream->processToNextFrame()  <= ESE_RESULT_ERROR);
  }
  return false;
}

//C API

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

ESEResult
es_extractor_read_packet (ESExtractor * extractor, ESEPacket ** packet)
{
  ESEResult res = ESE_RESULT_NEW_PACKET;
  res = extractor->processToNextPacket ();
  if(res <  ESE_RESULT_EOS)
    *packet = extractor->currentPacket ();
  else
    *packet = nullptr;

  return res;
}

ESEVideoCodec
es_extractor_video_codec (ESExtractor * extractor)
{
  return extractor->codec ();
}

ESEVideoFormat
es_extractor_video_format (ESExtractor * extractor)
{
  return extractor->format ();
}

const char *
es_extractor_video_codec_name (ESExtractor * extractor)
{
  return extractor->codec_name ();
}

int
es_extractor_packet_count (ESExtractor * extractor)
{
  return extractor->packetCount ();
}

void
es_extractor_clear_packet (ESEPacket * pkt)
{
  if (pkt)
    delete pkt;
}

void
es_extractor_teardown (ESExtractor * extractor)
{
  delete (extractor);
}

void
es_extractor_set_log_level (uint8_t level)
{
  Logger::instance ().setLogLevel (level);
}
