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

#pragma once

#include "eselogger.h"
#include "eseutils.h"
#include "esextractor.h"

#define DEFAULT_BUFFER_READ_LENGTH 1024

class ESEReader {
  public:
  ESEReader ();
  virtual ~ESEReader ();
  /// @brief Reset the reader
  virtual void reset ();

  virtual bool      prepare ()              = 0;
  virtual ESEBuffer getBuffer (size_t size) = 0;

  size_t         readSize () { return m_readSize; }
  virtual size_t streamSize () = 0;
  int32_t        streamPosition () { return m_streamPosition; }
  size_t         bufferReadLength () { return m_bufferReadLength; }
  void           setBufferReadLength (size_t bufferReadLength) { m_bufferReadLength = bufferReadLength; }

  virtual bool isEOS () = 0;

  protected:
  int32_t   m_streamPosition;
  size_t    m_bufferSize;
  size_t    m_readSize;
  size_t    m_bufferReadLength;
  ESEBuffer m_buffer;
};
