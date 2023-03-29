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

#include "esextractor.h"
#include "eselogger.h"

#include <fstream>
#include <algorithm>


static void
dumpPacket (ESExtractor * esextractor, ESEPacket* pkt)
{
  const char *frame_type_name = es_extractor_video_codec_name (esextractor);
  DBG ("Got a %s frame of size %d pts=%lld", frame_type_name, pkt->data_size, pkt->pts);
  MEM_DUMP (pkt->data, pkt->data_size, "Buffer=");
}

int
parseFile (const char *fileName, const char* options, uint8_t debug_level)
{
  ESEResult res;
  ESEPacket *pkt;
  ESExtractor *esextractor;
  int frame_count;

  es_extractor_set_log_level (debug_level);
  esextractor = es_extractor_new (fileName, options);

  if (!esextractor) {
    ERR ("Unable to discover a compatible stream. Exit");
    return -1;
  }
  INFO ("Extracting frames from %s with options %s", fileName, options);
  while ((res =
          es_extractor_read_frame (esextractor,
              &pkt)) < ESE_RESULT_EOS) {

    dumpPacket (esextractor, pkt);
    es_extractor_clear_packet (pkt);
  }

  frame_count = es_extractor_frame_count (esextractor);
  INFO ("Got %d frame(s)", frame_count);
  es_extractor_teardown (esextractor);

  return frame_count;
}


