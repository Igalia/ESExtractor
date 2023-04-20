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

#include "esenalstream.h"
#include "eselogger.h"
#include "esenalu.h"
#include "esereader.h"

#define MPEG_HEADER_SIZE 3
#define MAX_SEARCH_SIZE 5

ESENALStream::ESENALStream ()
: ESEStream (ESE_VIDEO_FORMAT_NAL)
{
  reset ();
}

ESENALStream::~ESENALStream ()
{
}

void
ESENALStream::reset ()
{
  m_frameState = ESE_NAL_FRAME_STATE_NONE;
  m_frameStartPos = 0;
  m_nalCount = false;
  m_mpegDetected = false;
  m_audNalDetected = false;
  m_alignment = ESE_PACKET_ALIGNMENT_NAL;
  m_nextNAL = ESEBuffer ();
  m_nextFrame = ESEBuffer ();
  ESEStream::reset ();
}

const char *
ESENALStream::alignmentName ()
{

  if (m_alignment == ESE_PACKET_ALIGNMENT_AU)
    return "AU";
  else if (m_alignment == ESE_PACKET_ALIGNMENT_NAL)
    return "NAL";
  else
    return "unknown";
}

void
ESENALStream::parseOptions (const char *options)
{
  ESEStream::parseOptions (options);
  if (m_options["alignment"] == "NAL")
    m_alignment = ESE_PACKET_ALIGNMENT_NAL;
  else if (m_options["alignment"] == "AU")
    m_alignment = ESE_PACKET_ALIGNMENT_AU;
  INFO ("Create a NAL stream with alignment %s", alignmentName ());
}

int32_t
ESENALStream::parseStream (int32_t start_position)
{
  int32_t pos = start_position;
  int32_t buffer_size = m_buffer.size ();

  while (pos < buffer_size) {
    if (!m_mpegDetected) {
      pos = probeH26x ();
      if (pos > 0) {
        /* start code might have 2 or 3 0-bytes */
        m_frameStartPos = pos;
        m_frameState = ESE_NAL_FRAME_STATE_START;
        m_mpegDetected = true;
      } else {
        ERR ("Unable to find any start code in buffer size %d. Exit.",
          buffer_size);
        return -1;
      }
    } else if (m_mpegDetected && m_codec != ESE_VIDEO_CODEC_UNKNOWN) {
      pos = scanMPEGHeader (m_buffer, pos);
      if (pos >= 0) {
        DBG ("Found a NAL delimiter, stop pos %d ", pos);
        if (m_frameState == ESE_NAL_FRAME_STATE_NONE) {
          m_frameState = ESE_NAL_FRAME_STATE_START;
          pos += 3;
          m_frameStartPos = pos;
        } else {
          m_frameState = ESE_NAL_FRAME_STATE_END;
          if (m_buffer[pos - 1] == 0x00)
            pos -= 1;
        }
        return pos;
      } else {
        DBG ("Unable to find a new MPEG code, end of file");
        pos = buffer_size;
      }
    }
  }

  return pos;
}

ESEResult
ESENALStream::readStream ()
{
  int32_t pos;

  if (m_eos) {
    m_nextFrame = ESEBuffer ();
    return ESE_RESULT_EOS;
  }

  if (m_bufferPosition >= (uint32_t)m_buffer.size ())
    m_buffer = m_reader.getBuffer (BUFFER_MAX_PROBE_LENGTH);

  while (m_bufferPosition <= (uint32_t)m_buffer.size () || !m_reader.isEOS ()) {
    pos = parseStream (m_bufferPosition);
    if (pos == (int32_t)-1) {
      return ESE_RESULT_NO_PACKET;
    } else {
      if (m_frameState == ESE_NAL_FRAME_STATE_END) {
        m_nextFrame = prepareFrame (m_buffer, m_frameStartPos, pos);
        m_nalCount++;
        DBG ("Found a new frame (%d) of size %zd at pos %d", m_nalCount,
          m_nextFrame.size (), m_reader.filePosition () + m_frameStartPos);
        m_frameStartPos = pos;
        m_bufferPosition = pos;
        m_frameState = ESE_NAL_FRAME_STATE_NONE;
        return ESE_RESULT_NEW_PACKET;
      } else {
        m_bufferPosition = pos;
        if (m_bufferPosition >= (uint32_t)m_buffer.size ()) {
          if (m_reader.isEOS ()) {
            m_nextFrame = prepareFrame (m_buffer, m_frameStartPos, m_buffer.size ());
            m_nalCount++;
            DBG ("Found a last frame (%d) of size %zd at pos %d",
              m_nalCount, m_nextFrame.size (),
              m_reader.filePosition () + m_frameStartPos);
            m_eos = true;
            return ESE_RESULT_LAST_PACKET;
          } else {
            ESEBuffer buffer = m_reader.getBuffer (BUFFER_MAX_PROBE_LENGTH);
            m_buffer.insert (m_buffer.end (), buffer.begin (), buffer.end ());
          }
        }
      }
    }
  }
  return ESE_RESULT_NO_PACKET;
}

ESEResult
ESENALStream::processToNextFrame ()
{
  ESEResult res;

  if (m_nextPacket)
    return ESE_RESULT_NEW_PACKET;

  if (m_alignment == ESE_PACKET_ALIGNMENT_NAL) {
    res = readStream ();
    if (res <= ESE_RESULT_LAST_PACKET) {
      m_currentFrame = m_nextFrame;
      prepareNextPacket ();
    }
  } else {
    m_currentFrame = {};
    while ((res = readStream ()) <= ESE_RESULT_EOS) {
      if (!ese_is_aud_nalu (m_nextFrame, (ESENaluCodec)m_codec)) {
        m_currentFrame.insert (m_currentFrame.end (), m_nextFrame.begin (),
          m_nextFrame.end ());
      }
      if (res == ESE_RESULT_EOS
        || ese_is_new_frame (m_nextFrame, (ESENaluCodec)m_codec)) {
        if (m_currentFrame.size () > 0) {
          const ESEBuffer &audNalu = ese_aud_nalu ((ESENaluCodec)m_codec);
          m_currentFrame.insert (m_currentFrame.begin (), audNalu.begin (),
            audNalu.end ());
          prepareNextPacket ();
        }
        break;
      }
    }
  }
  return res;
}
