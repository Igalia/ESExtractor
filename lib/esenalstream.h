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

#include <vector>

#include "esestream.h"

typedef enum ESEPacketAlignment {
  ESE_PACKET_ALIGNMENT_NAL = 0,
  ESE_PACKET_ALIGNMENT_AU,
} ESEPacketAlignment;

typedef enum _ESENALFrameState {
  ESE_NAL_FRAME_STATE_NONE = 0,
  ESE_NAL_FRAME_STATE_START,
  ESE_NAL_FRAME_STATE_END,
} ESENALFrameState;

class ESENALStream : public ESEStream {

  public:
  ESENALStream ();
  ~ESENALStream ();

  virtual void reset ();

  ESEResult processToNextFrame ();
  /// @brief Returns the NAL count.
  /// @return
  int nalCount () { return m_nalCount; }
  /// @brief Set frame format, either NAL or a complete access unit.
  /// @param alignment

  void parseOptions (const char *options);

  protected:
  ESEBuffer getStartCode () { return { 0x00, 0x00, 0x00, 0x01 }; }

  private:
  ESEResult   readStream ();
  int32_t     parseStream (int32_t start_position);
  const char *alignmentName ();

  ESENALFrameState   m_frameState;
  int32_t            m_frameStartPos;
  uint32_t           m_nalCount;
  bool               m_mpegDetected;
  ESEBuffer          m_nextNAL;
  bool               m_audNalDetected;
  ESEPacketAlignment m_alignment;
  ESEBuffer          m_nextFrame;
};
