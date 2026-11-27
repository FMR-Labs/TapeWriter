#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "../FMR_Library/FMR_Lib/FMR_DSP_float.h"
#include "../FMR_Library/FMR_Pedal/IPedal.h"

#include "../FMR_Library/FMR_Pedal/PedalConst.h"

#include <atomic>
#include <memory>
#include <array>

enum TaWr_EParams
{
	TaWr_kGain = 0,
	TaWr_kBoost,
	TaWr_kVolume,
	TaWr_kFilter,
	TaWr_kTone,
	TaWr_kFootSw,
	TaWr_kNumParams
};

enum TaWr_EControlTags
{
	TaWr_kCtrlInputLED = 0,
	TaWr_kCtrlOutputLED,
	TaWr_kCtrlLED,
	TaWr_kNumLeds
};

template <typename T> class FMR_RateAdapter;
class ND_TapeWriter;
class ND_Boost_JFET;

class TapeWriterDSP : public IPedal
{
public:
  struct Parameters
  {
    float gain = 0.3f;
    float boost = 0.5f;
    float volume = 0.5f;
    int filter = 0;
    float tone = 1.0f;
    int footSw = 1;
  };

  TapeWriterDSP();
  ~TapeWriterDSP();

  TapeWriterDSP(const TapeWriterDSP&) = delete;
  TapeWriterDSP& operator=(const TapeWriterDSP&) = delete;

  void Reset(double sampleRate, int blockSize);

  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames, int nChans);

  int GetLatency() const;

  // IPedal (utilisé par le board : moteur seul, 48 kHz, en place)
  const char* Id() const override { return "TapeWriter"; }
  
  int NumParams() const override { return TaWr_kNumParams; }
  int NumLeds() const override { return TaWr_kNumLeds; }

  const char* ParamName(int k) const override;
  float ParamDefault(int k) const override;
  void SetParam(int k, float norm) override;
  void ResetEngine() override;
  void Process(iplug::sample** io, int nFrames, int nChans) override
  {
    ProcessEngine(io, io, nFrames, nChans, mParams);
  }

  void SetLed(int led, float color, float value, int decay);
  float GetLedColor(int led);
  float GetLedValue(int led);
  int GetLedDecay(int led);
  
  void setDisplayedLed(bool value) {displayedLed = value;}

private:
  void ProcessEngine(iplug::sample** inputs, iplug::sample** outputs, int nFrames, int nChans, const Parameters& params);
  void levelColorLed(int led, float level, float fullScale, float greenAt, float orangeAt);

  Parameters mParams;
  
  std::unique_ptr<ND_TapeWriter> mDrive;
  std::unique_ptr<ND_Boost_JFET> mBoost;
  std::unique_ptr<FMR_RateAdapter<iplug::sample>> mRate;
  
  bool displayedLed = false;

  FilterFloat_t filterGain{};
  FilterFloat_t filterBoost{};
  FilterFloat_t filterVolume{};
  FilterFloat_t filterTone{};

  float currentGain = 0.0f;
  float currentBoost = 0.0f;
  float currentVolume = 0.0f;
  float currentTone = 0.0f;

  struct myLed
  {
      float color = 0.0f;
      float value = 0.0f;
      int decay = 1;
  };
  std::array<myLed, TaWr_kNumLeds> pedalLed;
};
