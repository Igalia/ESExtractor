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

#ifndef __ESE_READER_H__
#define __ESE_READER_H__

#include <cstdint>
#include <iostream>
#include <fstream>
#include <istream>
#include <vector>

#define ESEBuffer std::vector <unsigned char>

template < typename T > static inline std::vector <T>
subVector (std::vector < T > const &v, int pos, int size)
{
  auto first = v.cbegin () + pos;
  auto last = v.cbegin () + pos + size;

  std::vector < T > vec (first, last);
  return vec;
}

class ESEReader {
public:
  ESEReader();
  ~ESEReader();
  /// @brief This method will build the next frame (NAL or AU) available.
  /// @return

  bool openFile(const char* fileName);
  /// @brief Reset the reader
  void reset (bool full = true);
  /// @brief Read data from file
  ESEBuffer getBuffer (uint32_t size);
  int32_t readSize() { return m_readSize;}
  int32_t fileSize() { return m_fileSize;}
  int32_t filePosition() { return m_filePosition;}

  bool    isEOS () {return m_bufferSize == 0 && m_readSize == m_fileSize;}

private:
  uint32_t readFile(int32_t data_size, int32_t pos = 0, bool append = false );

  std::ifstream               m_file;
  int32_t                     m_filePosition;
  int32_t                     m_fileSize;

  int32_t                     m_bufferSize;

  int32_t                     m_readSize;
  ESEBuffer                   m_buffer;
};

#endif //__ESE_READER_H__
