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

#include "esedatareader.h"

ESEDataReader::ESEDataReader (ese_read_buffer_func read_func, void *pointer)
{
  m_readFunc    = read_func;
  m_dataPointer = pointer;
  m_eos         = false;

  reset ();
}

bool
ESEDataReader::prepare ()
{
  m_eos = false;
  return true;
}

uint32_t
ESEDataReader::readData (int32_t size, int32_t position, bool append)
{
  ESEBuffer buffer;
  int32_t   read_size;

  buffer.resize (size);
  m_streamPosition = position;

  // Ask the app to provide data with size from a position in the stream. Can return less than expected.
  read_size = m_readFunc (m_dataPointer, buffer.data (), size, m_streamPosition);
  //
  buffer.resize (read_size);
  if (read_size == 0) {
    m_eos = true;
    return read_size;
  }

  m_readSize += read_size;
  m_streamPosition += read_size;

  if (append) {
    m_buffer.insert (m_buffer.end (), buffer.begin (), buffer.end ());
    INFO ("ReadFile: Append to a buffer of new size %zd requested size %d read %zd", size,
      m_buffer.size (), read_size);
  } else {
    m_buffer = buffer;
    DBG ("ReadFile: Read buffer %d of size read %zd", size, read_size);
  }
  m_bufferSize = m_buffer.size ();
  return read_size;
}

ESEBuffer
ESEDataReader::getBuffer (uint32_t size)
{
  uint32_t  real_size = size;
  ESEBuffer buffer;

  while (m_buffer.size () < size) {
    if (readData (bufferReadLength (), m_streamPosition,
          true)
      < bufferReadLength ())
      break;
  }
  if (m_buffer.size () < size)
    real_size = m_buffer.size ();

  buffer = subVector (m_buffer, 0, real_size);
  m_buffer.erase (m_buffer.begin (), m_buffer.begin () + real_size);
  m_bufferSize = m_buffer.size ();
  return buffer;
}
