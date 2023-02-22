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

\

#include <vector>

typedef enum ESENaluCategory {
  ESE_NALU_CATEGORY_UNKNOWN,
  ESE_NALU_CATEGORY_AUD,
  ESE_NALU_CATEGORY_PARAMETER_SET,
  ESE_NALU_CATEGORY_SLICE,
  ESE_NALU_CATEGORY_DATA,
} ESENaluCategory;

typedef enum ESENaluCodec {
  ESE_NALU_CODEC_UNKNOWN = 0,
  ESE_NALU_CODEC_H264,
  ESE_NALU_CODEC_H265,
} ESENaluCodec;

typedef enum
{
  ESE_H264_NAL_UNKNOWN      = 0,
  ESE_H264_NAL_SLICE        = 1,
  ESE_H264_NAL_SLICE_DPA    = 2,
  ESE_H264_NAL_SLICE_DPB    = 3,
  ESE_H264_NAL_SLICE_DPC    = 4,
  ESE_H264_NAL_SLICE_IDR    = 5,
  ESE_H264_NAL_SEI          = 6,
  ESE_H264_NAL_SPS          = 7,
  ESE_H264_NAL_PPS          = 8,
  ESE_H264_NAL_AUD          = 9,
  ESE_H264_NAL_SEQ_END      = 10,
  ESE_H264_NAL_STREAM_END   = 11,
  ESE_H264_NAL_FILLER_DATA  = 12,
  ESE_H264_NAL_SPS_EXT      = 13,
  ESE_H264_NAL_PREFIX_UNIT  = 14,
  ESE_H264_NAL_SUBSET_SPS   = 15,
  ESE_H264_NAL_DEPTH_SPS    = 16,
  ESE_H264_NAL_SLICE_AUX    = 19,
  ESE_H264_NAL_SLICE_EXT    = 20,
  ESE_H264_NAL_SLICE_DEPTH  = 21
} ESEH264NalUnitType;

typedef enum
{
  ESE_H265_NAL_SLICE_TRAIL_N    = 0,
  ESE_H265_NAL_SLICE_TRAIL_R    = 1,
  ESE_H265_NAL_SLICE_TSA_N      = 2,
  ESE_H265_NAL_SLICE_TSA_R      = 3,
  ESE_H265_NAL_SLICE_STSA_N     = 4,
  ESE_H265_NAL_SLICE_STSA_R     = 5,
  ESE_H265_NAL_SLICE_RADL_N     = 6,
  ESE_H265_NAL_SLICE_RADL_R     = 7,
  ESE_H265_NAL_SLICE_RASL_N     = 8,
  ESE_H265_NAL_SLICE_RASL_R     = 9,
  ESE_H265_NAL_SLICE_BLA_W_LP   = 16,
  ESE_H265_NAL_SLICE_BLA_W_RADL = 17,
  ESE_H265_NAL_SLICE_BLA_N_LP   = 18,
  ESE_H265_NAL_SLICE_IDR_W_RADL = 19,
  ESE_H265_NAL_SLICE_IDR_N_LP   = 20,
  ESE_H265_NAL_SLICE_CRA_NUT    = 21,
  ESE_H265_NAL_VPS              = 32,
  ESE_H265_NAL_SPS              = 33,
  ESE_H265_NAL_PPS              = 34,
  ESE_H265_NAL_AUD              = 35,
  ESE_H265_NAL_EOS              = 36,
  ESE_H265_NAL_EOB              = 37,
  ESE_H265_NAL_FD               = 38,
  ESE_H265_NAL_PREFIX_SEI       = 39,
  ESE_H265_NAL_SUFFIX_SEI       = 40
} ESEH265NalUnitType;

class ESENalu{
public:
  ESENalu(std::vector<unsigned char> buffer, ESENaluCodec codec);
  virtual ~ESENalu() {}

  int naluType() {return m_naluType;};
  ESENaluCodec naluCodec() {return  m_naluCodec;}
  ESENaluCategory naluCategory() {return m_naluCategory;};

protected:
  virtual void parseNalu() = 0;
  int                         m_naluType;
  std::vector<unsigned char>  m_buffer;
  ESENaluCodec               m_naluCodec;
  ESENaluCategory            m_naluCategory;

};

class ESEH264Nalu : public ESENalu{
public:
  ESEH264Nalu(std::vector<unsigned char> buffer);
  virtual ~ESEH264Nalu(){}

protected:

  virtual void parseNalu() override;

};

class ESEH265Nalu : public ESENalu {
public:
  ESEH265Nalu(std::vector<unsigned char> buffer);
  virtual ~ESEH265Nalu(){}


protected:

  virtual void parseNalu() override;

};

#ifdef __cplusplus
extern "C" {
#endif

bool ese_is_aud_nalu (std::vector<unsigned char> buffer, ESENaluCodec codec);
bool ese_is_new_frame (std::vector<unsigned char> buffer, ESENaluCodec codec);
ESENaluCategory ese_nalu_get_category (std::vector < unsigned char >buffer, ESENaluCodec codec);
const std::vector<unsigned char>& ese_aud_nalu (ESENaluCodec codec);
#ifdef __cplusplus
}
#endif