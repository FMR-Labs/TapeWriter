
#include "TapeWriterDSP.h"

#include "../FMR_Library/FMR_Lib/FMR_Neural.h"
#include "../FMR_Library/FMR_NeuralDrive/FMR_TapeWriter.h"
#include "../FMR_Library/FMR_NeuralDrive/FMR_Boost_JFET.h"
#include "../FMR_Library/FMR_Lib/FMR_Resampler.h"
#include "../FMR_Library/FMR_Lib/FMR_ConvPot.h"

#include <algorithm>
#include <cmath>

using iplug::sample;

namespace
{
  // Ordre = EParams du VST3 autonome (hors footswitch). Noms stables : ne jamais les renommer.
  constexpr const char* kParamNames[] = { "gain", "boost", "volume", "filter", "tone", "footsw"};

  // Valeurs initiales de TapeWriter.cpp, normalisées 0..1.
  // valeurs redondantes avec Parameters dans TapeWriterDPS.h ! Est-ce que l'on peut se passer de l'un ou de l'autres ?
  constexpr float kParamDefaults[] = { 0.3f, 0.5f, 0.5f, 0, 1.0f, 1 };

  inline float Clamp01(float val) { return std::clamp(val, 0.0f, 1.0f); }
  inline int Steps(float val, int n) { return static_cast<int>(std::lround(Clamp01(val) * n)); }
}

TapeWriterDSP::TapeWriterDSP()
{
  mDrive = std::make_unique<ND_TapeWriter>();
  mBoost = std::make_unique<ND_Boost_JFET>();
  mRate = std::make_unique<FMR_RateAdapter<sample>>();
}

TapeWriterDSP::~TapeWriterDSP() = default;

void TapeWriterDSP::Reset(double sampleRate, int blockSize)
{
  // Le moteur tourne à 48 kHz ; le resampler adapte la fréquence du host.
  mRate->Init(sampleRate, std::max(blockSize, 64));
  ResetEngine();
}

void TapeWriterDSP::ResetEngine()
{
  InitFilter(&filterGain, 10.0f, LP_1st_order, 1.0f);
  InitFilter(&filterBoost, 10.0f, LP_1st_order, 1.0f);
  InitFilter(&filterVolume, 10.0f, LP_1st_order, 1.0f);
  InitFilter(&filterTone, 10.0f, LP_1st_order, 1.0f);

  currentGain = 0.0f;
  currentBoost = 0.0f;
  currentVolume = 0.0f;
  currentTone = 0.0f;
}

int TapeWriterDSP::GetLatency() const
{
  return mRate->Latency();
}

void TapeWriterDSP::SetLed(int led, float color, float value, int decay)
{
    pedalLed[led].color = color;
    pedalLed[led].value = value;
    pedalLed[led].decay = decay;
}

float TapeWriterDSP::GetLedColor(int led)
{
    return pedalLed[led].color;
}

float TapeWriterDSP::GetLedValue(int led)
{
    return pedalLed[led].value;
}

int TapeWriterDSP::GetLedDecay(int led)
{
    return pedalLed[led].decay;
}

const char* TapeWriterDSP::ParamName(int k) const
{
  return (k >= 0 && k < TaWr_kNumParams) ? kParamNames[k] : "";
}

float TapeWriterDSP::ParamDefault(int k) const
{
  return (k >= 0 && k < TaWr_kNumParams) ? kParamDefaults[k] : 0.5f;
}

void TapeWriterDSP::SetParam(int k, float val)
{
  switch (k)
  {
    case TaWr_kGain:	mParams.gain = 0.1f + Clamp01(val) * (0.999f - 0.1f); break;
    case TaWr_kBoost:	mParams.boost = 0.1f + Clamp01(val) * (0.999f - 0.1f); break;
    case TaWr_kVolume:	mParams.volume = PotCurve_0_23[static_cast<unsigned short int>(Steps(val, 4095))]; break;
    case TaWr_kFilter:	mParams.filter = Steps(val, 2); break;
    case TaWr_kTone:	mParams.tone = PotCurve_0_23[static_cast<unsigned short int>(Steps(val, 4095))]; break;
    case TaWr_kFootSw:	mParams.footSw = Steps(val, 1); break;
    default: break;
  }
}

void TapeWriterDSP::ProcessBlock( sample** inputs, sample** outputs, int nFrames, int nChans )
{
  if (mRate->Active())
  {
    // Host -> 48 kHz -> moteur -> host, par tranches <= MaxBlock().
    for (int done = 0; done < nFrames; )
    {
      const int n = std::min(nFrames - done, mRate->MaxBlock());

      sample* in[ND_TAPE_WRITER_NB_CHANNELS];
      sample* out[ND_TAPE_WRITER_NB_CHANNELS];

      for (int c = 0; c < nChans; c++)
      {
        in[c] = inputs[c] + done;
        out[c] = outputs[c] + done;
      }

      const int n48 = mRate->ToEngine(in, n, nChans);
      ProcessEngine(mRate->Planar(), mRate->Planar(), n48, nChans, mParams);
      mRate->FromEngine(n48, out, n, nChans);

      done += n;
    }
  }
  else
  {
    ProcessEngine(inputs, outputs, nFrames, nChans, mParams);
  }
}

void TapeWriterDSP::levelColorLed(int led, float level, float fullScale, float greenAt, float orangeAt)
{
  
  const float normalized = std::clamp(level / fullScale, 0.0f, 1.0f);
  
  if (normalized < 0.005f)	pedalLed[led].value = 0.0f;	// éteinte
  else					    pedalLed[led].value = 1.0f;	// allumée

  if (normalized < greenAt)			pedalLed[led].color = LedVerte;
  else if (normalized < orangeAt)	pedalLed[led].color = LedOrange;
  else								pedalLed[led].color = LedRouge;

  if( displayedLed )	pedalLed[led].decay = LedDecay;
  else  				pedalLed[led].decay = 0;
}

void TapeWriterDSP::ProcessEngine(sample** inputs, sample** outputs, int nFrames, int nChans, const Parameters& params)
{
  float inputPeak = 0.0f;
  float outputPeak = 0.0f;

  mDrive->Request(params.filter);

  for (int s = 0; s < nFrames; s++)
  {
    currentGain = Filter1st(&filterGain, params.gain);
	currentBoost = Filter1st(&filterBoost, params.boost);
    currentVolume = Filter1st(&filterVolume, params.volume);
    currentTone = Filter1st(&filterTone, params.tone);

    mDrive->Tick();
    mDrive->SetEQ(currentTone);

    for (int c = 0; c < nChans; c++)
    {
      if( params.footSw == 1 )
	  {
        SetLed(TaWr_kCtrlLED, LedRouge, 1.0f, 0);
	    const float input = inputs[c][s];
        inputPeak = std::max(inputPeak, std::fabs(input));
	    levelColorLed(TaWr_kCtrlInputLED, inputPeak, 4.5f, moins9dB, moins3dB);

        const float out1stStage = mDrive->Process(c, std::clamp(input, -4.5f, 4.5f), currentGain);
        const float output = currentVolume * mBoost->Process(c, out1stStage, currentBoost);
	    
        outputPeak = std::max(outputPeak, std::fabs(output));
	    levelColorLed(TaWr_kCtrlOutputLED, outputPeak, 2.0f, moins9dB, moins3dB);

	  	outputs[c][s] = output;
	  }
      else
      {
        outputs[c][s] = inputs[c][s];
        SetLed(TaWr_kCtrlLED, LedRouge, 0.0f, 0);
        SetLed(TaWr_kCtrlInputLED, LedVerte, 0.0f, 0);
        SetLed(TaWr_kCtrlOutputLED, LedVerte, 0.0f, 0);
      }
    }
  }
}
