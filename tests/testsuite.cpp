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

#include <cassert>
#include <string>

#include "config.h"

#include "testese.h"

void
check_nal_file (const char *uri, int log_level, ESEVideoCodec codec, std::string codec_name, int num_packets_nal, int num_packets_au, int buffer_size)
{
  ESExtractor *extractor;

  extractor = create_es_extractor (uri, nullptr, log_level);
  es_extractor_set_buffer_read_length (extractor, buffer_size);
  assert (extractor);
  assert (es_extractor_video_format (extractor) == ESE_VIDEO_FORMAT_NAL);
  assert (es_extractor_video_codec (extractor) == codec);
  assert (std::string (es_extractor_video_codec_name (extractor)) == codec_name);
  assert (parse (extractor) == num_packets_nal);
  es_extractor_set_options (extractor, "alignment:AU");
  assert (parse (extractor) == num_packets_au);
  es_extractor_set_options (extractor, "alignment:NAL");
  assert (parse (extractor) == num_packets_nal);
  es_extractor_teardown (extractor);
}

void
check_ivf_file (const char *uri, int log_level, ESEVideoCodec codec, std::string codec_name, int num_packets, int buffer_size)
{
  ESExtractor *extractor;

  extractor = create_es_extractor (uri, nullptr, log_level);
  es_extractor_set_buffer_read_length (extractor, buffer_size);
  assert (extractor);
  assert (es_extractor_video_format (extractor) == ESE_VIDEO_FORMAT_IVF);
  assert (es_extractor_video_codec (extractor) == codec);
  assert (std::string (es_extractor_video_codec_name (extractor)) == codec_name);
  assert (parse (extractor) == num_packets);
  es_extractor_teardown (extractor);
}

void
check_annex_b_file (const char *uri, int log_level, ESEVideoCodec codec, std::string codec_name, int num_packets, int buffer_size)
{
  ESExtractor *extractor;

  extractor = create_es_extractor (uri, "format:annex-b", log_level);
  es_extractor_set_buffer_read_length (extractor, buffer_size);
  assert (extractor);
  assert (es_extractor_video_format (extractor) == ESE_VIDEO_FORMAT_ANNEX_B);
  assert (es_extractor_video_codec (extractor) == codec);
  assert (std::string (es_extractor_video_codec_name (extractor)) == codec_name);
  assert (parse (extractor) == num_packets);
  es_extractor_teardown (extractor);
}

int
main ()
{
  int log_level = ES_LOG_LEVEL_INFO;

  // NAL tests
  check_nal_file (ESE_SAMPLES_FOLDER "/Sample_10.avc", log_level, ESE_VIDEO_CODEC_H264, "h264", 22, 10, 57);
  check_nal_file (ESE_SAMPLES_FOLDER "/Sample_10.avc", log_level, ESE_VIDEO_CODEC_H264, "h264", 22, 10, 1024);
  check_nal_file (ESE_SAMPLES_FOLDER "/Sample_10.avc", log_level, ESE_VIDEO_CODEC_H264, "h264", 22, 10, 1024 * 1024);
  check_nal_file (ESE_SAMPLES_FOLDER "/Sample_10.hevc", log_level, ESE_VIDEO_CODEC_H265, "h265", 23, 10, 28);
  check_nal_file (ESE_SAMPLES_FOLDER "/Sample_10.hevc", log_level, ESE_VIDEO_CODEC_H265, "h265", 23, 10, 1024);
  check_nal_file (ESE_SAMPLES_FOLDER "/Sample_10.hevc", log_level, ESE_VIDEO_CODEC_H265, "h265", 23, 10, 1024);
  // IVF tests
  check_ivf_file (ESE_SAMPLES_FOLDER "/clip-a.ivf", log_level, ESE_VIDEO_CODEC_AV1, "av1", 30, 107);
  check_ivf_file (ESE_SAMPLES_FOLDER "/clip-a.ivf", log_level, ESE_VIDEO_CODEC_AV1, "av1", 30, 1024);
  check_ivf_file (ESE_SAMPLES_FOLDER "/clip-a.ivf", log_level, ESE_VIDEO_CODEC_AV1, "av1", 30, 1024 * 1024);
  // Parse with data provider
  assert (parse_data (ESE_SAMPLES_FOLDER "/Sample_10.avc", nullptr, log_level, 1024) == 22);
  assert (parse_data (ESE_SAMPLES_FOLDER "/Sample_10.hevc", nullptr, log_level, 17) == 23);
  assert (parse_data (ESE_SAMPLES_FOLDER "/clip-a.ivf", nullptr, log_level, 1024 * 1024) == 30);

  // Annex B tests
  check_annex_b_file (ESE_SAMPLES_FOLDER "/clip.obu", log_level, ESE_VIDEO_CODEC_AV1, "av1", 20, 1024 * 1024);

  // Corner case tests
  assert (parse_file (nullptr, nullptr, log_level) == -1);
  assert (parse_file ("/this/path/does/not/exists", nullptr, log_level) == -1);

  assert (es_extractor_read_packet (nullptr, nullptr) == ESE_RESULT_ERROR);
  assert (es_extractor_packet_count (nullptr) == -1);
  assert (es_extractor_video_format (nullptr) == ESE_VIDEO_FORMAT_UNKNOWN);
  assert (es_extractor_video_codec (nullptr) == ESE_VIDEO_CODEC_UNKNOWN);
  assert (es_extractor_video_codec_name (nullptr) == nullptr);

  return 0;
}
