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
  reset ();
}

ESEDataReader::~ESEDataReader ()
{
}

bool
ESEDataReader::prepare ()
{
  return true;
}

ESEBuffer
ESEDataReader::getBuffer (uint32_t size)
{
  ESEBuffer buffer;

  if (!m_readFunc)
    return ESEBuffer ();

  buffer.resize (size);
  uint32_t read_size = m_readFunc (m_dataPointer, buffer.data (), size, m_streamPosition);
  if (read_size < size)
    buffer.resize (read_size);

  return buffer;
}
