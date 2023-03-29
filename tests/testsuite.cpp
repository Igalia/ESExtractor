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
#include <assert.h>
#include "config.h"

int
main (int argc, char *argv[])
{
  int log_level = 0;

  // AVC tests
  assert(parseFile (ESE_SAMPLES_FOLDER "/Sample_10.avc", nullptr, log_level) == 10);
  assert(parseFile (ESE_SAMPLES_FOLDER "/Sample_10.avc", "alignment:AU", log_level) == 10);
  assert(parseFile (ESE_SAMPLES_FOLDER "/Sample_10.avc", "alignment:NAL", log_level) == 22);

  //HEVC tests
  assert(parseFile (ESE_SAMPLES_FOLDER "/Sample_10.hevc", nullptr, log_level) == 10);
  assert(parseFile (ESE_SAMPLES_FOLDER "/Sample_10.hevc", "alignment:AU", log_level) == 10);
  assert(parseFile (ESE_SAMPLES_FOLDER "/Sample_10.hevc", "alignment:NAL", log_level) == 23);

  // IVF tests
  assert(parseFile (ESE_SAMPLES_FOLDER "/clip-a.ivf", nullptr, log_level) == 30);

  return 0;
}
