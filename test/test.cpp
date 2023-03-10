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
#include <algorithm>

class CmdLineParser{
    public:
        CmdLineParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }

        const std::string& getOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        bool optionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};

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

void
dumpFrame (ESExtractor * esextractor, uint8_t * data, int data_size)
{
  const char *frame_type_name =
      frameTypeName (es_extractor_video_codec (esextractor));
  INFO ("Got a %s frame of size %d", frame_type_name, data_size);
  printBufferHex (data, data_size);
}

int
parseFile (const char *fileName, ESExtractorPacketAlignment alignment, bool debug)
{
  ESExtractorResult res;
  ESEPacket *pkt;
  ESExtractor *esextractor = es_extractor_new (fileName, alignment);

  if (!esextractor) {
    ERR ("Unable to discover a compatible stream. Exit");
    return -1;
  }
  INFO ("Extracting frames from %s with alignment %s", fileName, alignment == ES_EXTRACTOR_PACKET_ALIGNMENT_NAL ? "NAL":"AU");
  while ((res =
          es_extractor_read_frame (esextractor,
              &pkt)) < ES_EXTRACTOR_RESULT_EOS) {
    if (debug)
      dumpFrame (esextractor, pkt->data, pkt->data_size);
    es_extractor_clear_packet (pkt);
  }
  es_extractor_clear_packet (pkt);
  INFO ("Got %d frame(s)", es_extractor_frame_count (esextractor));
  es_extractor_teardown (esextractor);

  return 0;
}

int
main (int argc, char *argv[])
{
  int res = 0;
  bool debug = false;

  std::ofstream myfile;

  CmdLineParser cmdLine(argc, argv);
  if(cmdLine.optionExists("-d")){
    debug = true;
  }

  const std::string &fileName = cmdLine.getOption("-f");
  if (fileName.empty()){
    std::cerr << "Error: No input file specified. Usage: " << std::endl;
    return -1;
  }

  res = parseFile (fileName.c_str(), ES_EXTRACTOR_PACKET_ALIGNMENT_NAL, debug);
  res = parseFile (fileName.c_str(), ES_EXTRACTOR_PACKET_ALIGNMENT_AU, debug);

  return res;
}
