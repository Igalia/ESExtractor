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


#include "testese.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

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
        int optionExists(const std::string &option) const{
            return std::count(this->tokens.begin(), this->tokens.end(), option);
        }
    private:
        std::vector <std::string> tokens;
};

void usage(int argc, char *argv[]) {
  std::cout << "Usage: " << argv[0] << " -f input_file" << std::endl;
  std::cout << std::endl;
  std::cout << "Options: "<< std::endl;
  std::cout << "-h:\t show this help message and exit"<< std::endl;
  std::cout << "-o:\t add option to the parser cmd line"<< std::endl;
  std::cout << "-d:\t increment the debug level which each occurences (ie -d -d = level info(2))"<< std::endl;
}

int
main (int argc, char *argv[])
{
  uint8_t debug = ES_LOG_LEVEL_ERROR;

  std::ofstream myfile;

  CmdLineParser cmdLine(argc, argv);
  if(cmdLine.optionExists("-h")) {
    usage(argc, argv);
    return 0;
  }

  for (int i = 0; i < cmdLine.optionExists("-d"); i++){
    debug ++;
  }

  const std::string &option = cmdLine.getOption("-o");

  const std::string &fileName = cmdLine.getOption("-f");
  if (fileName.empty()){
    std::cerr << "Error: No input file specified." << std::endl;
    usage(argc, argv);
    return 1;
  }

  return (int)(parseFile (fileName.c_str(), option.c_str(), debug) < 0);
}
