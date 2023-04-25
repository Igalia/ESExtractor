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

#include "esefilereader.h"
#include "esestream.h"
#include "eseivfstream.h"
#include "eselogger.h"
#include "eseutils.h"

#define MPEG_HEADER_SIZE 3
#define MAX_SEARCH_SIZE 5

ESEVideoFormat
ese_stream_probe_video_format (const char *uri)
{
  ESEVideoFormat format = ESE_VIDEO_FORMAT_UNKNOWN;
  ESEStream stream;
  ESEBuffer buffer;
  if (stream.prepare (uri)) {
    if (stream.probeIVF () != -1)
      format = ESE_VIDEO_FORMAT_IVF;
    else if (stream.probeH26x () != -1)
      format = ESE_VIDEO_FORMAT_NAL;
  }
  DBG ("Found a format %d", format);
  return format;
}

ESEStream::ESEStream (ESEVideoFormat format)
: m_format (format)
, m_currentPacket (nullptr)
, m_nextPacket (nullptr)
{
  reset ();
}

ESEStream::~ESEStream ()
{
  DBG ("Found %u frame and read %d of %d", m_frameCount, m_reader->readSize (),
    m_reader->streamSize ());
}

void
ESEStream::reset ()
{
  m_eos = false;
  m_bufferPosition = 0;
  m_frameCount = 0;
  m_currentPacket = nullptr;
  if (m_nextPacket)
    delete m_nextPacket;
  m_nextPacket = nullptr;
  m_codec = ESE_VIDEO_CODEC_UNKNOWN;
  m_buffer = ESEBuffer ();
  m_currentFrame = ESEBuffer ();
  if (m_reader)
    m_reader->reset ();
}

bool
ESEStream::prepare (const char *uri, const char *options)
{
  parseOptions (options);
  m_reader = make_unique<ESEFileReader>(uri);
  if (!m_reader->prepare ())
    return false;
  return (processToNextFrame () <= ESE_RESULT_ERROR);
}

void
ESEStream::setOptions (const char *options)
{
  parseOptions (options);
}

ESEBuffer
ESEStream::prepareFrame (ESEBuffer buffer, uint32_t start,
  uint32_t end)
{
  uint32_t frame_start = start;
  if (start > buffer.size () || end > buffer.size ()) {
    throw std::out_of_range ("start and end positions must be within the buffer size");
  }
  if (start > end) {
    throw std::invalid_argument ("start position must be less than end position");
  }
  ESEBuffer frame = ESEBuffer (buffer.begin () + frame_start,
    buffer.begin () + end);

  ESEBuffer start_code = getStartCode ();
  frame.insert (frame.begin (), start_code.begin (), start_code.end ());

  return frame;
}

ESEPacket *
ESEStream::prepareNextPacket (uint64_t pts, uint64_t dts, uint64_t duration)
{
  m_nextPacket = new ESEPacket ();
  m_nextPacket->data = static_cast<std::uint8_t *> (std::malloc (m_currentFrame.size ()));
  std::memcpy (m_nextPacket->data, m_currentFrame.data (), m_currentFrame.size ());
  m_nextPacket->data_size = m_currentFrame.size ();
  m_nextPacket->pts = pts;
  m_nextPacket->dts = dts;
  m_nextPacket->duration = duration;
  m_frameCount++;
  return m_nextPacket;
}

ESEPacket *
ESEStream::currentPacket ()
{
  m_currentPacket = m_nextPacket;
  m_nextPacket = nullptr;
  return m_currentPacket;
}

int32_t
ESEStream::scanMPEGHeader (ESEBuffer buffer, int32_t pos)
{
  for (uint32_t i = pos; i < buffer.size () - 3; i++) {
    bool found = (buffer[i] == 0x00 && buffer[i + 1] == 0x00
      && buffer[i + 2] == 0x01);
    if (found)
      return i;
  }
  return -1;
}

bool
ESEStream::isH265 (ESEBuffer buffer)
{
  int nut;
  bool found = false;

  if (buffer.size () < MAX_SEARCH_SIZE)
    return false;

  /* forbiden_zero_bit | nal_unit_type */
  nut = buffer[0] & 0xfe;

  /* if forbidden bit is different to 0 won't be h265 */
  if (nut > 0x7e) {
    found = false;
  }
  nut = nut >> 1;

  /* if nuh_layer_id is not zero or nuh_temporal_id_plus1 is zero then
   * it won't be h265 */
  if ((buffer[3] & 0x01) || (buffer[4] & 0xf8) || !(buffer[4] & 0x07)) {
    found = false;
  }
  if ((nut >= 0 && nut <= 9) || (nut >= 16 && nut <= 21) || (nut >= 32 && nut <= 40)) {
    found = true;
  }

  if (found) {
    m_codec = ESE_VIDEO_CODEC_H265;
    DBG ("Found h265");
  }
  return found;
}

bool
ESEStream::isH264 (ESEBuffer buffer)
{
  int nut, ref;
  bool found = false;

  if (buffer.size () < MAX_SEARCH_SIZE)
    return found;

  nut = buffer[0] & 0x9f; /* forbiden_zero_bit | nal_unit_type */
  ref = buffer[0] & 0x60; /* nal_ref_idc */

  /* if forbidden bit is different to 0 won't be h264 */
  if (nut > 0x1f) {
    found = false;
  }

  if ((nut >= 1 && nut <= 13) || nut == 19) {
    if ((nut == 5 && ref == 0) || ((nut == 6 || (nut >= 9 && nut <= 12)) && ref != 0)) {
      found = false;
    } else {
      found = true;
    }
  } else if (nut >= 14 && nut <= 33) {
    if (nut == 14 || nut == 15 || nut == 20) {
      found = true;
    }
  }

  if (found) {
    m_codec = ESE_VIDEO_CODEC_H264;
    DBG ("Found h264");
  }

  return found;
}

int32_t
ESEStream::probeH26x ()
{
  int32_t offset;
  m_reader->reset ();
  m_buffer = m_reader->getBuffer (BUFFER_MAX_PROBE_LENGTH);

  offset = scanMPEGHeader (m_buffer, 0);
  if (offset > 0) {
    /* start code might have 2 or 3 0-bytes */
    offset += 3;
    ESEBuffer buffer = subVector (m_buffer, offset, MAX_SEARCH_SIZE);
    if (isH264 (buffer) || isH265 (buffer))
      return offset;
  }
  return -1;
}

int32_t
ESEStream::probeIVF ()
{
  IVFHeader ivf_header;
  m_reader->reset ();
  m_buffer = m_reader->getBuffer (sizeof (IVFHeader));
  std::memcpy (&ivf_header, m_buffer.data (), sizeof (IVFHeader));
  if (ivf_header.signature == ESE_MAKE_FOURCC ('D', 'K', 'I', 'F'))
    return 0;
  return -1;
}

void
ESEStream::parseOptions (const char *options)
{
  char *token;
  if (options == nullptr)
    return;
  token = strtok ((char *)options, "\n");
  while (token != NULL) {
    std::string s (token);
    size_t pos = s.find (":");
    m_options[s.substr (0, pos)] = s.substr (pos + 1, std::string::npos);
    token = strtok (NULL, "\n");
  }
}