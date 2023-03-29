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

#ifndef __ESE_IVFSTREAM_H__
#define __ESE_IVFSTREAM_H__

#include "esestream.h"

struct IVFHeader {
    uint32_t signature; // FourCC
    uint16_t version;
    uint16_t length_header;
    uint32_t fourcc;
    uint16_t width;
    uint16_t height;
    uint32_t timescale_den;
    uint32_t timescale_num;
    uint32_t frame_count;
    uint32_t unused;
};


class ESEIVFStream : public ESEStream {
public:  
  ESEIVFStream ();
  ~ESEIVFStream ();

protected:
  ESEBuffer getStartCode() {return {};}
  ESEResult processToNextFrame();

private:
  ESEVideoCodec fourccToCodec();
  void parseOptions(const char* options);

  IVFHeader m_header;
  bool m_header_found;
  uint64_t m_last_pts;
};

#endif //__ESE_IVFSTREAM_H__