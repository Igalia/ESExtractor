/* ESExtractor
 * Copyright (C) 2023 Igalia, S.L.
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

ESEFileReader::ESEFileReader (const char *fileName)
{
  if (fileName)
    m_fileName = fileName;
  reset ();
}

bool
ESEFileReader::prepare ()
{
  if (m_file.is_open ())
    return true;

  if (m_fileName.empty ())
    return false;

  m_file = std::ifstream (m_fileName, std::ios::binary | std::ios::ate);
  DBG ("The file %s is now %s", m_fileName.c_str (), m_file.is_open () ? "open" : "closed");
  m_fileSize = m_file.tellg ();
  if (!m_file.is_open () || m_fileSize <= 0) {
    ERR ("Unable to open the file %s", m_fileName.c_str ());
    return false;
  }

  return true;
}

uint32_t
ESEFileReader::readFile (int32_t data_size, int32_t pos, bool append)
{
  ESEBuffer buffer;
  size_t    read_size;

  if (!m_fileSize)
    m_fileSize = m_file.tellg ();
  if (m_fileSize <= 0) {
    ERR ("The file is empty. Exit.");
    return 0;
  }

  DBG ("Read %d at pos %d append %d from file size %d", data_size, pos, append,
    m_fileSize);
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
  m_bufferSize = m_buffer.size ();
  return read_size;
}

ESEBuffer
ESEFileReader::getBuffer (uint32_t size)
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
  m_bufferSize = m_buffer.size ();
  return buffer;
}
