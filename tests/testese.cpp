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

#include <algorithm>
#include <cassert>
#include <cstring>

#include "esefilereader.h"
#include "eselogger.h"
#include "eseutils.h"
#include "esextractor.h"

static void
dump_packet (ESExtractor *esextractor, ESEPacket *pkt)
{
  const char *packet_type_name = es_extractor_video_codec_name (esextractor);
  INFO ("Got a %s packet of size %d pts=%lld", packet_type_name, pkt->data_size, pkt->pts);
  MEM_DUMP (pkt->data, pkt->data_size, "Buffer=");
}

/// @brief Return a new esextactor
/// @param fileName the file path to extract frames from
/// @param options Options used by the esextractor
/// @param debug_level set the debug level of the esextractor
/// @return a new esextractor object or nullptr if the path is not a valid media file.
ESExtractor *
create_es_extractor (const char *fileName, const char *options, uint8_t debug_level)
{
  es_extractor_set_log_level (debug_level);
  INFO ("--- Extracting packets from %s with options %s", fileName, options);
  ESExtractor *esextractor = es_extractor_new (fileName, options);
  return esextractor;
}

/// @brief  Returns the number of frame found
/// @param extractor a valid extractor
/// @return the number of frames
int
parse (ESExtractor *esextractor)
{
  ESEResult  res;
  ESEPacket *pkt;
  int        packet_count;

  while ((res = es_extractor_read_packet (esextractor,
            &pkt))
    < ESE_RESULT_EOS) {

    dump_packet (esextractor, pkt);
    es_extractor_clear_packet (pkt);
  }

  packet_count = es_extractor_packet_count (esextractor);
  INFO ("Got %d packet(s)", packet_count);
  return packet_count;
}

int
parse_file (const char *fileName, const char *options, uint8_t debug_level)
{
  ESExtractor *esextractor = create_es_extractor (fileName, options, debug_level);
  if (!esextractor) {
    ERR ("Unable to discover a compatible stream. Exit");
    return -1;
  }
  int packet_count = parse (esextractor);
  es_extractor_teardown (esextractor);
  return packet_count;
}

class DataProvider {
  public:
  DataProvider (const char *uri)
  : m_streamSize (0)
  {
    m_file = std::ifstream (uri, std::ios::binary | std::ios::ate);
    reset ();
  }
  ~DataProvider () { }

  void reset ()
  {
    m_streamPosition = 0;
    m_readSize       = 0;
    m_buffer         = ESEBuffer ();
  }

  uint32_t readFile (int32_t data_size, int32_t pos, bool append)
  {
    ESEBuffer buffer;
    size_t    read_size;
    if (!m_streamSize)
      m_streamSize = m_file.tellg ();
    if (!m_streamSize) {
      ERR ("The file is empty. Exit.");
      return 0;
    }

    DBG ("Read %d at pos %d append %d from file size %d", data_size, pos, append,
      m_streamSize);
    m_file.clear ();
    m_file.seekg (pos, m_file.beg);
    m_streamPosition = pos;

    buffer.resize (data_size);
    m_file.read ((char *)buffer.data (), data_size);
    read_size = m_file.gcount ();
    buffer.resize (read_size);
    m_readSize += read_size;
    m_streamPosition += read_size;
    if (append) {
      m_buffer.insert (m_buffer.end (), buffer.begin (), buffer.end ());
      DBG ("ReadFile: Append %d to a buffer of new size %zd read %zd", data_size,
        m_buffer.size (), read_size);
    } else {
      m_buffer = buffer;
      DBG ("ReadFile: Read buffer %d of size read %zd", data_size, read_size);
    }
    return read_size;
  }

  ESEBuffer getBuffer (uint32_t size)
  {
    uint32_t  real_size = size;
    ESEBuffer buffer;

    while (m_buffer.size () < size) {
      if (readFile (BUFFER_MAX_PROBE_LENGTH, m_streamPosition,
            true)
        < BUFFER_MAX_PROBE_LENGTH)
        break;
    }
    if (m_buffer.size () < size)
      real_size = m_buffer.size ();

    buffer = subVector (m_buffer, 0, real_size);
    m_buffer.erase (m_buffer.begin (), m_buffer.begin () + real_size);
    return buffer;
  }
  uint32_t getData (uint8_t *buffer, uint32_t size, int32_t offset)
  {
    if (!offset)
      reset ();
    m_buffer = getBuffer (size);
    std::memcpy (buffer, m_buffer.data (), m_buffer.size ());
    return m_buffer.size ();
  }

  std::ifstream m_file;
  int32_t       m_streamPosition;
  int32_t       m_streamSize;
  int32_t       m_readSize;
  ESEBuffer     m_buffer;
};

static int
ReadBuffer (void *opaque, unsigned char *pBuf, int size, int32_t offset)
{
  int read_size = ((DataProvider *)opaque)->getData (pBuf, size, offset);
  DBG ("ReadBuf read_size=%d size=%d", read_size, size);
  return read_size;
}

int
parse_data (const char *fileName, const char *options, uint8_t debug_level)
{
  es_extractor_set_log_level (debug_level);
  INFO ("Extracting packets from %s with options %s", fileName, options);
  std::unique_ptr<DataProvider> pDataProvider = make_unique<DataProvider> (fileName);

  ESExtractor *esextractor = es_extractor_new_with_read_func (&ReadBuffer, pDataProvider.get (), options);
  if (!esextractor) {
    ERR ("Unable to discover a compatible stream. Exit");
    return -1;
  }
  int packet_count = parse (esextractor);
  es_extractor_teardown (esextractor);

  return packet_count;
}

