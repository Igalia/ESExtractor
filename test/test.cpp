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

const char* frameTypeName(ESExtractorVideoCodec codec_id) {
  switch (codec_id) {
    case ES_EXTRACTOR_VIDEO_CODEC_H264:
      return "h264";
    case ES_EXTRACTOR_VIDEO_CODEC_H265:
      return "h265";
    default:
      return "unknown";
  };
}

int
main (int argc, char *argv[])
{
  ESExtractorResult res;
  ESExtractor *esextractor;
  uint8_t *data = nullptr;
  int data_size = 0;
  const char *fileName;
  const char* frame_type_name;

  if (argc < 2) {
    std::cerr << "Error: No input file specified" << std::endl;
    return -1;
  }

  fileName = argv[1];

  esextractor = es_extractor_new (fileName);
  if (!esextractor) {
    ERR("Unable to discover a compatible stream. Exit");
    return -1;
  }

  frame_type_name = frameTypeName(es_extractor_video_codec (esextractor));
  while ((res = es_extractor_read_frame (esextractor, &data,
              &data_size)) < ES_EXTRACTOR_RESULT_EOS) {
    INFO ("Got a %s frame of size %d", frame_type_name, data_size);
  }
  INFO ("Got %d %s frame(s)", es_extractor_frame_count (esextractor), frame_type_name);
  es_extractor_teardown (esextractor);

  return 0;
}
