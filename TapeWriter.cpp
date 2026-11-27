
#include "TapeWriter.h"

#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

#include "TapeWriterGUI.h"
#include "../FMR_Library/FMR_NeuralDrive/FMR_TapeWriter.h"
#include "../FMR_Library/FMR_NeuralDrive/FMR_Boost_JFET.h"
#include "../FMR_Library/FMR_Lib/FMR_ConvPot.h"

#include <algorithm>

// utiliser struct Parameters et TapeWriterDSP::SetParam pour ne pas dupliquer les infos ?
TapeWriter::TapeWriter(const InstanceInfo& info) : iplug::Plugin(info, MakeConfig(TaWr_kNumParams, kNumPresets))
{
  GetParam(TaWr_kGain)->InitDouble("Gain", 0.3, 0.1, 0.999, 0.001);
  GetParam(TaWr_kBoost)->InitDouble("Boost", 0.55, 0.1, 0.999, 0.001);
  GetParam(TaWr_kVolume)->InitInt("Volume", 2048, 0, 4095);

  GetParam(TaWr_kFilter)->InitInt("Filter", 0, 0, 2);
  GetParam(TaWr_kTone)->InitInt("Tone", 4095, 0, 4095);
  
  GetParam(TaWr_kFootSw)->InitInt("Foot Switch", 1, 0, 1);
  
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]()
  {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  mLayoutFunc = [&](IGraphics* pGraphics)
  {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    TapeWriterGUI::Build(pGraphics, 0.f, 0.f, 0, true);
	mDSP.setDisplayedLed(true);
  };
#endif
}

void TapeWriter::OnIdle()
{
#if IPLUG_EDITOR
  if (!GetUI()) return;
  
  for(int l = 0; l < TaWr_kNumLeds ; l++ )
  {
    if ( auto* led = dynamic_cast<ILEDControl*>( GetUI()->GetControlWithTag( kNumStartLED + l) ) )
	{
      float value = GetBypassed() ? 0.0f : mDSP.GetLedValue(l);
      led->SetValue(value);
      if (value != 0.0f)
      {
          led->TriggerWithDecay(mDSP.GetLedDecay(l));
      }
      led->SetHue(mDSP.GetLedColor(l));
      led->SetDirty(true);
    }
  }

#endif
}

#if IPLUG_DSP

void TapeWriter::OnReset()
{
  mDSP.Reset(GetSampleRate(), std::max(GetBlockSize(), 64));
  SetLatency(mDSP.GetLatency());
}

void TapeWriter::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const int nChans = std::min(NOutChansConnected(),ND_TAPE_WRITER_NB_CHANNELS);

  // Conversion unique : TapeWriterDSP::SetParam (partagée avec le board).
  // lecture de tous les knobs et conversion (mise à l'échelle)
  for (int k = 0; k < mDSP.NumParams(); k++)
    mDSP.SetParam(k, static_cast<float>(GetParam(k)->GetNormalized()));

  mDSP.ProcessBlock(inputs, outputs, nFrames, nChans);  
}

#endif
