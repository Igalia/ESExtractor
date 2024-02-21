/* ESExtractor
 * Copyright (C) 2023 Igalia, S.L.
 *     Author: Charlie Turner <cturner@igalia.com>
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

#include "eseannexbstream.h"
#include "eselogger.h"
#include "esereader.h"

#include <cassert>
#include <limits>

ESEAnnexBStream::ESEAnnexBStream ()
: ESEStream (ESE_VIDEO_FORMAT_ANNEX_B)
{
  reset ();
}

ESEAnnexBStream::~ESEAnnexBStream ()
{
}

void
ESEAnnexBStream::reset ()
{
  m_frameCount                   = 0;
  m_inTemporalUnit               = false;
  m_remainingBytesInTemporalUnit = 0;
  ESEStream::reset ();
}

static uint32_t
getUleb128 (const uint8_t *in, uint32_t *num_bytes)
{
  uint64_t val        = 0;
  uint32_t i          = 0, more;
  uint32_t bytes_read = 0;
  do {
    const int v = in[bytes_read];
    more        = v & 0x80;
    val |= ((uint64_t)(v & 0x7F)) << i;
    bytes_read += 1;
    i += 7;
  } while (more && i < 56);

  if (val > std::numeric_limits<uint32_t>::max () || more)
    return 0;

  if (num_bytes)
    *num_bytes = bytes_read;

  return (uint32_t)val;
}

ESEResult
ESEAnnexBStream::processToNextFrame ()
{
  ESEResult res = ESE_RESULT_NEW_PACKET;
  m_codec       = ESE_VIDEO_CODEC_AV1;

  if (m_bufferPosition >= (uint32_t)m_buffer.size ())
    // HACK: The API is racey, work around that for CTS
    // requirements. No bitstream will be bigger that 5MiB.
    m_buffer = m_reader->getBuffer (5 * 1024 * 1024);

  if (m_nextPacket)
    return res;

  if (m_eos)
    return ESE_RESULT_EOS;

  const uint8_t *ptr = m_buffer.data () + m_bufferPosition;
  DBG ("ptr offset %ld %ld ", m_bufferPosition, m_buffer.size ());

  if (!m_inTemporalUnit) {
    uint32_t tuUlebSize = 0;
    uint32_t tuSize     = getUleb128 (ptr, &tuUlebSize);
    ptr += tuUlebSize;
    m_remainingBytesInTemporalUnit = tuSize;
    m_inTemporalUnit               = true;
  }

  // ptr is at the start of a frame
  uint32_t frameUlebSize = 0;
  uint32_t frameSize     = getUleb128 (ptr, &frameUlebSize);

  ptr += frameUlebSize;
  uint32_t frameStartOffset = static_cast<uint32_t> (ptr - m_buffer.data ());
  uint32_t frameEndOffset   = frameStartOffset + frameSize;
  assert (frameEndOffset <= m_buffer.size ());

  m_currentFrame = prepareFrame (m_buffer, frameStartOffset, frameEndOffset);

  prepareNextPacket ();

  m_remainingBytesInTemporalUnit -= (frameSize + frameUlebSize);
  if (m_remainingBytesInTemporalUnit == 0) {
    m_inTemporalUnit = false;
  }

  m_bufferPosition = frameEndOffset;

  if (frameEndOffset == m_buffer.size ()) {
    m_eos = true;
    return ESE_RESULT_LAST_PACKET;
  }

  DBG ("Found a new Annex B frame (%d) of size %zd at offset %ld", m_frameCount,
    m_currentFrame.size (), frameStartOffset);

  return ESE_RESULT_NEW_PACKET;
}

void
ESEAnnexBStream::parseOptions (const char *options)
{
  (void)options;
  INFO ("Create a AnnexB stream");
}
