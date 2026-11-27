
#include "TapeWriterGUI.h"
#include "IControls.h"
#include "../FMR_Library/FMR_Pedal/PedalFactory.h"

void TapeWriterGUI::Build(iplug::igraphics::IGraphics* pGraphics, float x, float y, int slot, bool displayLed)
{
	using namespace iplug::igraphics;

	const IBitmap knobBitmap = pGraphics->LoadBitmap(KNOB_FN, 101);
	const IBitmap switchBitmap = pGraphics->LoadBitmap(SWITCH_FN, 3);
	const IBitmap footswBitmap = pGraphics->LoadBitmap(FOOTSW_FN, 2);
	const IBitmap boitierBitmap = pGraphics->LoadBitmap(BOITIER_FN, 1);

	const IBitmap fondBitmap = pGraphics->LoadBitmap(TAPEWRITER_FN, 1);

	auto at = [&](const IRECT& r) { return r.GetTranslated(x, y); };

	//  x /  y     
	// 42 / 81		126 / 81	210 / 81
	// 42 / 200		126 / 200	210 / 200
	//				126 / 337
	//				126 / 415

	// 49 / 129		133 / 129	217 / 129
	// 49 / 248		133 / 248	217 / 248
	//				133 / 385
	//				133 / 463

	const IRECT boitier = at(IRECT(0, 0, 266, 535));
	const IRECT ledCtrl = at(IRECT(84, 337, 182, 435));
	const IRECT faceAvant = at(IRECT(7, 48, 259, 528));

	const IRECT gainKnob = at(IRECT(0, 80, 98, 178));
	const IRECT boostKnob = at(IRECT(84, 80, 182, 178));
	const IRECT volumeKnob = at(IRECT(165, 80, 263, 178));
	const IRECT volumeLed =  at(IRECT(145, 60, 283, 198));

	const IRECT filterSwitch = at(IRECT(0, 199, 98, 297));
	const IRECT toneKnob = at(IRECT(84, 199, 182, 297));

	const IRECT footSwitch = at(IRECT(84, 414, 182, 512));
	const IRECT inputLed = at(IRECT(64, 394, 202, 532));
	
	pGraphics->AttachControl(new IBitmapControl(boitier, boitierBitmap));
	pGraphics->AttachControl(new ILEDControl(ledCtrl, LedRouge), slot + TaWr_kCtrlLED + kNumStartLED);
	pGraphics->AttachControl(new IBitmapControl(faceAvant, fondBitmap));

	pGraphics->AttachControl(new IBKnobControl(gainKnob, knobBitmap, slot + TaWr_kGain));
	pGraphics->AttachControl(new IBKnobControl(boostKnob, knobBitmap, slot + TaWr_kBoost));
//	if( displayLed )	pGraphics->AttachControl(new ILEDControl(volumeLed, LedVerte), slot + TaWr_kCtrlOutputLED + kNumStartLED);
	pGraphics->AttachControl(new IBKnobControl(volumeKnob, knobBitmap, slot + TaWr_kVolume));

	pGraphics->AttachControl(new IBKnobControl(filterSwitch, switchBitmap, slot + TaWr_kFilter));
	pGraphics->AttachControl(new IBKnobControl(toneKnob, knobBitmap, slot + TaWr_kTone));

	if( displayLed )	pGraphics->AttachControl(new ILEDControl(inputLed, LedVerte), slot + TaWr_kCtrlInputLED + kNumStartLED);
	pGraphics->AttachControl(new IBSwitchControl(footSwitch, footswBitmap, slot + TaWr_kFootSw));
}

PEDAL_FACTORY(TapeWriter, TapeWriterDSP, TapeWriterGUI)