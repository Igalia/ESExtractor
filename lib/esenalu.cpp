/* ESENalu
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

#include "esenalu.h"
#include "esestream.h"

const int NAL_UNIT_TYPE_MASK = 0x1F;

const ESEBuffer h264_aud_nalu = { 0x00, 0x00, 0x00, 0x01, 0x09, 0xF0 };

const ESEBuffer h265_aud_nalu = { 0x00, 0x00, 0x00, 0x01, 0x46, 0x01, 0x10 };

ESENalu::ESENalu (ESEBuffer buffer, ESENaluCodec codec)
: m_buffer (buffer)
, m_naluCodec (codec)
{
}

ESEH264Nalu::ESEH264Nalu (ESEBuffer buffer)
: ESENalu (buffer, ESE_NALU_CODEC_H264)
{
  parseNalu ();
}

void
ESEH264Nalu::parseNalu ()
{
  m_naluType = m_buffer[4] & NAL_UNIT_TYPE_MASK;

  switch (m_naluType) {
    case ESE_H264_NAL_AUD:
      m_naluCategory = ESE_NALU_CATEGORY_AUD;
      break;
    case ESE_H264_NAL_SEI:
      m_naluCategory = ESE_NALU_CATEGORY_DATA;
      break;
    case ESE_H264_NAL_SPS:
    case ESE_H264_NAL_PPS:
    case ESE_H264_NAL_SPS_EXT:
    case ESE_H264_NAL_SUBSET_SPS:
    case ESE_H264_NAL_DEPTH_SPS:
      m_naluCategory = ESE_NALU_CATEGORY_PARAMETER_SET;
      break;
    case ESE_H264_NAL_SLICE:
    case ESE_H264_NAL_SLICE_DPA:
    case ESE_H264_NAL_SLICE_DPB:
    case ESE_H264_NAL_SLICE_DPC:
    case ESE_H264_NAL_SLICE_IDR:
      m_naluCategory = ESE_NALU_CATEGORY_SLICE;
      break;
    default:
      m_naluCategory = ESE_NALU_CATEGORY_UNKNOWN;
      break;
  }
}

ESEH265Nalu::ESEH265Nalu (ESEBuffer buffer)
: ESENalu (buffer, ESE_NALU_CODEC_H265)
{
  parseNalu ();
}

void
ESEH265Nalu::parseNalu ()
{
  m_naluType = ((m_buffer[4] & 0x7E) >> 1);

  switch (m_naluType) {
    case ESE_H265_NAL_AUD:
      m_naluCategory = ESE_NALU_CATEGORY_AUD;
      break;
    case ESE_H265_NAL_SPS:
    case ESE_H265_NAL_PPS:
    case ESE_H265_NAL_VPS:
      m_naluCategory = ESE_NALU_CATEGORY_PARAMETER_SET;
      break;
    case ESE_H265_NAL_SLICE_TRAIL_N:
    case ESE_H265_NAL_SLICE_TRAIL_R:
    case ESE_H265_NAL_SLICE_TSA_N:
    case ESE_H265_NAL_SLICE_TSA_R:
    case ESE_H265_NAL_SLICE_STSA_N:
    case ESE_H265_NAL_SLICE_STSA_R:
    case ESE_H265_NAL_SLICE_RADL_N:
    case ESE_H265_NAL_SLICE_RADL_R:
    case ESE_H265_NAL_SLICE_RASL_N:
    case ESE_H265_NAL_SLICE_RASL_R:
    case ESE_H265_NAL_SLICE_BLA_W_LP:
    case ESE_H265_NAL_SLICE_BLA_W_RADL:
    case ESE_H265_NAL_SLICE_BLA_N_LP:
    case ESE_H265_NAL_SLICE_IDR_W_RADL:
    case ESE_H265_NAL_SLICE_IDR_N_LP:
    case ESE_H265_NAL_SLICE_CRA_NUT:
      m_naluCategory = ESE_NALU_CATEGORY_SLICE;
      break;
    default:
      m_naluCategory = ESE_NALU_CATEGORY_UNKNOWN;
      break;
  }
}

static ESENalu *
getNalu (ESEBuffer buffer, ESENaluCodec codec)
{
  ESENalu *nalu;
  if (!buffer.size ())
    return nullptr;
  if (codec == ESE_NALU_CODEC_H264) {
    nalu = new ESEH264Nalu (buffer);
  } else {
    nalu = new ESEH265Nalu (buffer);
  }
  return nalu;
}

ESENaluCategory
ese_nalu_get_category (ESEBuffer buffer, ESENaluCodec codec)
{
  ESENaluCategory cat  = ESE_NALU_CATEGORY_UNKNOWN;
  ESENalu        *nalu = getNalu (buffer, codec);
  if (nalu) {
    cat = nalu->naluCategory ();
    delete nalu;
  }
  return cat;
}

bool
ese_is_aud_nalu (ESEBuffer buffer, ESENaluCodec codec)
{
  ESENaluCategory cat = ese_nalu_get_category (buffer, codec);
  return cat == ESE_NALU_CATEGORY_AUD;
}

bool
ese_is_new_frame (ESEBuffer buffer, ESENaluCodec codec)
{
  ESENaluCategory cat = ese_nalu_get_category (buffer, codec);
  return (cat >= ESE_NALU_CATEGORY_SLICE);
}

const ESEBuffer &
ese_aud_nalu (ESENaluCodec codec)
{
  if (codec == ESE_NALU_CODEC_H264)
    return h264_aud_nalu;
  else // H265
    return h265_aud_nalu;
}
