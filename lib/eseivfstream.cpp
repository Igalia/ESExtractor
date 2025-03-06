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

#include "eseivfstream.h"
#include "eselogger.h"
#include "esereader.h"

#pragma pack(push, 1)
struct IVFFrameHeader {
  uint32_t frame_size;
  uint64_t timestamp;
};
#pragma pack(pop)

ESEIVFStream::ESEIVFStream ()
: ESEStream (ESE_VIDEO_FORMAT_IVF)
{
  reset ();
}

ESEIVFStream::~ESEIVFStream ()
{
}

void
ESEIVFStream::reset ()
{
  m_headerFound = false;
  m_lastPts     = 0;
  ESEStream::reset ();
}

void
ESEIVFStream::parseOptions (const char *options)
{
  INFO ("Create a IFV stream with options %s", options);
}

void
ESEIVFStream::printHeader ()
{
  INFO ("IVF header");
  INFO ("  fourcc: %d", fourccToCodec ());
  INFO ("  width: %d", m_header.width);
  INFO ("  height: %d", m_header.height);
  INFO ("  frame count: %d", m_header.frame_count);
  INFO ("  timescale: %d/%d", m_header.timescale_num, m_header.timescale_den);
  INFO ("  length header: %d", m_header.length_header);
}

ESEVideoCodec
ESEIVFStream::fourccToCodec ()
{
  switch (m_header.fourcc) {
    case ESE_MAKE_FOURCC ('V', 'P', '8', '0'):
      return ESE_VIDEO_CODEC_VP8;
      break;
    case ESE_MAKE_FOURCC ('V', 'P', '9', '0'):
      return ESE_VIDEO_CODEC_VP9;
      break;
    case ESE_MAKE_FOURCC ('A', 'V', '0', '1'):
      return ESE_VIDEO_CODEC_AV1;
      break;
    default:
      return ESE_VIDEO_CODEC_UNKNOWN;
  }
  return ESE_VIDEO_CODEC_UNKNOWN;
}

ESEResult
ESEIVFStream::processToNextFrame ()
{
  ESEResult      res = ESE_RESULT_NEW_PACKET;
  IVFFrameHeader frame_header;

  if (m_nextPacket)
    return ESE_RESULT_NEW_PACKET;

  if (m_reader->isEOS ())
    return ESE_RESULT_EOS;

  if (!m_headerFound) {
    m_buffer = m_reader->getBuffer (sizeof (IVFHeader));
    std::memcpy (&m_header, m_buffer.data (), sizeof (IVFHeader));
    m_headerFound = true;
    m_codec       = fourccToCodec ();
    printHeader ();
  }
  m_buffer = m_reader->getBuffer (sizeof (IVFFrameHeader));
  if (m_buffer.size ()) {
    std::memcpy (&frame_header, m_buffer.data (), sizeof (IVFFrameHeader));

    m_buffer       = m_reader->getBuffer (frame_header.frame_size);
    m_currentFrame = prepareFrame (m_buffer, 0, m_buffer.size ());

    prepareNextPacket (frame_header.timestamp, frame_header.timestamp,
      m_lastPts - frame_header.timestamp);

    m_lastPts = frame_header.timestamp;
  } else {
    DBG ("The last buffer was 0, no new packet, return EOS");
    return ESE_RESULT_EOS;
  }
  if (m_reader->isEOS ())
    res = ESE_RESULT_LAST_PACKET;

  DBG ("Found a new IVF frame (%d) of size %zd", m_frameCount,
    m_currentFrame.size ());

  return res;
}
