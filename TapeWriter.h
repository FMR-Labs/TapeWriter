#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "TapeWriterDSP.h"

using namespace iplug;
using namespace igraphics;

const int kNumPresets = 1;

class TapeWriter final : public Plugin
{
public:
  TapeWriter(const InstanceInfo& info);
  ~TapeWriter() override = default;

  void OnIdle() override;

#if IPLUG_DSP
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;

private:
  TapeWriterDSP mDSP;
#endif
};
