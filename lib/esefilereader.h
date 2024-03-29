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

#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <string>
#include <vector>

#include "esereader.h"
#include "eseutils.h"

class ESEFileReader : public ESEReader {
  public:
  ESEFileReader (const char *fileName);

  virtual bool prepare ();

  virtual ESEBuffer getBuffer (size_t size);
  virtual size_t    streamSize () { return m_fileSize; }
  virtual bool      isEOS () { return m_bufferSize == 0 && m_readSize == streamSize (); }

  private:
  size_t readFile (size_t data_size, int32_t pos = 0, bool append = false);

  std::ifstream m_file;
  std::string   m_fileName;
  size_t        m_fileSize;
};
