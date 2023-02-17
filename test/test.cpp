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
#include <fstream>

const char *
frameTypeName (ESExtractorVideoCodec codec_id)
{
  switch (codec_id) {
    case ES_EXTRACTOR_VIDEO_CODEC_H264:
      return "h264";
    case ES_EXTRACTOR_VIDEO_CODEC_H265:
      return "h265";
    default:
      return "unknown";
  };
}

void dumpFrame(ESExtractor *esextractor, uint8_t *buffer,  int data_size) {
    const char * frame_type_name = frameTypeName (es_extractor_video_codec (esextractor));
    INFO ("Got a %s frame of size %d", frame_type_name, data_size);
    for (int i = 0; i < data_size; i++) {
        DUMP ("0x%.2X ", buffer[i]);
    }
    DUMP ("\n");
}

int
main (int argc, char *argv[])
{
  ESExtractorResult res;
  ESExtractor *esextractor;
  ESPacket *pkt;
  const char *fileName;
  std::ofstream myfile;

  if (argc < 2) {
    std::cerr << "Error: No input file specified" << std::endl;
    return -1;
  }

  fileName = argv[1];
  if (argc > 2) {
    myfile.open (argv[2], std::ofstream::binary);
  }

  esextractor = es_extractor_new (fileName);
  if (!esextractor) {
    ERR ("Unable to discover a compatible stream. Exit");
    return -1;
  }

  while ((res = es_extractor_read_frame (esextractor, &pkt)) < ES_EXTRACTOR_RESULT_EOS) {
    dumpFrame(esextractor, pkt->data, pkt->data_size);
    if (argc > 2) {
      myfile.write ((const char *) pkt->data, pkt->data_size);
    }
    es_extractor_clear_packet (pkt);
  }
  es_extractor_clear_packet (pkt);
  INFO ("Got %d frame(s)", es_extractor_frame_count (esextractor));
  es_extractor_teardown (esextractor);
  if (argc > 2)
    myfile.close ();
  return 0;
}
