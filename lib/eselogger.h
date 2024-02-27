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
#include <cstdio>
#include <stdarg.h>
#include <string.h>

enum {
  ES_LOG_LEVEL_NONE = 0,
  ES_LOG_LEVEL_ERROR,
  ES_LOG_LEVEL_INFO,
  ES_LOG_LEVEL_DEBUG,
  ES_LOG_LEVEL_MEMDUMP,
  ES_LOG_LEVEL_MAX
};

class Logger {
  public:
  Logger ()
  {
    m_level = ES_LOG_LEVEL_ERROR;
  }
  static Logger &instance ()
  {
    static Logger instance;
    return instance;
  }
  int  level () { return m_level; }
  void setLogLevel (uint8_t level)
  {
    m_level = level;
    if (m_level > ES_LOG_LEVEL_MAX)
      m_level = ES_LOG_LEVEL_DEBUG;
  }
  void createLog (const char *format, ...)
  {
    va_list argptr;
    va_start (argptr, format);
    vfprintf (stdout, format, argptr);
    va_end (argptr);
  }

  void createLogData (const uint8_t *buffer, size_t length, const char *format, ...)
  {
    va_list argptr;
    va_start (argptr, format);
    vfprintf (stdout, format, argptr);
    va_end (argptr);
    for (size_t i = 0; i < length; i++) {
      fprintf (stdout, "0x%.2X ", buffer[i]);
    }
    fprintf (stdout, "\n");
  }

  private:
  uint8_t m_level;
};

#define __FILENAME__ (strrchr (__FILE__, '/') ? strrchr (__FILE__, '/') + 1 : __FILE__)

#define LOGGER(LEVEL, PREFIX, FMT, SUFFIX, ...) \
  if (LEVEL <= Logger::instance ().level ())    \
  Logger::instance ().createLog (PREFIX "%s:%d:%s:\t" FMT SUFFIX, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOGGER_DATA(LEVEL, DATA, LENGTH, PREFIX, FMT, SUFFIX, ...) \
  if (LEVEL <= Logger::instance ().level ())                       \
  Logger::instance ().createLogData (DATA, LENGTH, PREFIX "%s:%d:%s:\t" FMT SUFFIX, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ERR(FMT, ...) LOGGER (ES_LOG_LEVEL_ERROR, "ESE_ERROR\t", FMT, "\n", ##__VA_ARGS__)
#define INFO(FMT, ...) LOGGER (ES_LOG_LEVEL_INFO, "ESE_INFO\t", FMT, "\n", ##__VA_ARGS__)

#define DBG(FMT, ...) LOGGER (ES_LOG_LEVEL_DEBUG, "ESE_DEBUG\t", FMT, "\n", ##__VA_ARGS__)
#define MEM_DUMP(DATA, LENGTH, FMT, ...) LOGGER_DATA (ES_LOG_LEVEL_MEMDUMP, DATA, LENGTH, "ESE_MEMDUMP\t", FMT, "", ##__VA_ARGS__)
