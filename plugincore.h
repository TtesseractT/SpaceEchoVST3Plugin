// -----------------------------------------------------------------------------
//				Created by	: Sabian Hibbs
//				institute	: University Of Derby 
//				
//				09/02/2022 - Fair Use Copyright
//				**Not for Commercial Use** 
// 
//				Thanks to 
//				Will Perkle (ASPik) 
//				Dr Bruce Wiggins 
// -----------------------------------------------------------------------------

#ifndef __pluginCore_h__
#define __pluginCore_h__

#include "pluginbase.h"
#include "C:\Users\foxyb\Desktop\Bruce CW2\ALL_SDK\myprojects\MyFirstPlug\win_build\project_source\cmake\vst_cmake\ZeZipper.h"
#include "C:\Users\foxyb\Desktop\Bruce CW2\ALL_SDK\myprojects\MyFirstPlug\win_build\project_source\cmake\vst_cmake\AudioDevEffect.h"
#include "C:\Users\foxyb\Desktop\Bruce CW2\ALL_SDK\myprojects\MyFirstPlug\win_build\project_source\cmake\vst_cmake\EnvelopeShaper.h"
#include "C:\Users\foxyb\Desktop\Bruce CW2\ALL_SDK\myprojects\MyFirstPlug\win_build\project_source\cmake\vst_cmake\Limiter.h"

#include <cmath>

enum controlID {
	//-- Volume
	volume,				// 0 

	//-- Wet Dry Mix
	wetDryMix,			// 1 

	//-- Distortion 
	dist,				// 2 

	//-- Distortion On/Off
	voldistonoff,		// 3 

	//-- Delay Variables
	delayVar,			// 4 
	delayVar2,			// 5 
	delayVar3,			// 6 
	delayVar4,			// 7 

	//-- Delay Volume
	delayOneVolume,		// 8 
	delayTwoVolume,		// 9 
	delayThreeVolume,	// 10
	delayFourVolume,	// 11

	//-- Delay On/Off
	delay1onoff,		// 12
	delay2onoff,		// 13
	delay3onoff,		// 14
	delay4onoff,		// 15

	//-- Delay Filter Switches
	delay1filteronoff,	// 16
	delay2filteronoff,	// 17
	delay3filteronoff,	// 18
	delay4filteronoff,	// 19

	//-- Echo Variables, Echo Volume, Echo Rate (Non defined yet)
	echoVar1,			// 20
	echoVar2,			// 21
	echoVol1,			// 22
	echoRate,			// 23

	//-- Filter Type Parameters
	f0,					// 24
	Q,					// 25
	FilterType,			// 26
	dbgain,				// 27

	//-- Filter On / Off
	mainFilterOnOff,	// 28

	//-- Echo On / Of	
	echoOnOff,			// 29

	// -- Meter Out 
	VuL,				// 30
	VuR,				// 31
	VuS,				// 32
	VuD,				// 33

	// Delay Vu Out 
	VuDelay1L,			// 34
	VuDelay1R,			// 35
	VuDelay2L,			// 36
	VuDelay2R,			// 37
	VuDelay3L,			// 38
	VuDelay3R,			// 39
	VuDelay4L,			// 40
	VuDelay4R,			// 41
	
	//Echo Vu Out
	VuEcho1,			// 42

	//Wet Dry Vu Out
	VuWetMix,			// 43
	VuDryMix,			// 44

//Limiter
	m_Boost1,			//45
	m_Ceiling1,			//46

//Envelope Shaper
	m_Envelope1,		//47
	m_AttackInMilliseconds1,	//48
	m_ReleaseInMilliseconds1,	//49
	m_Attack1,			//50
	m_Release1,			//51

	//MasterVuOut
	VuLMax,				//52
	VuRMax,				//53

	// Limiter Saturation
	limiteronoff,		//54

	// Echo Feedback
	echoFeedback		//55


};

class PluginCore : public PluginBase
{
public:
    PluginCore();

	/** Destructor: empty in default version */
    virtual ~PluginCore(){
		delete[] dBlockI; // Deletes the values in memory when closed (Stops Leaking) 
		delete[] dBlock;
		delete[] dBlock2L;
		delete[] dBlock2R;
		delete[] dBlock3L;
		delete[] dBlock3R;
		delete[] dBlock4L;
		delete[] dBlock4R;
		delete[] eBlock1L;

	}

	/** this is the creation function for all plugin parameters */
	bool initPluginParameters();

	/** called when plugin is loaded, a new audio file is playing or sample rate changes */
	virtual bool reset(ResetInfo& resetInfo);

	/** one-time post creation init function; pluginInfo contains path to this plugin */
	virtual bool initialize(PluginInfo& _pluginInfo);

	/** preProcess: sync GUI parameters here; override if you don't want to use automatic variable-binding */
	virtual bool preProcessAudioBuffers(ProcessBufferInfo& processInfo);

	/** process frames of data (DEFAULT MODE) */
	virtual bool processAudioFrame(ProcessFrameInfo& processFrameInfo);

	/** Pre-process the block with: MIDI events for the block and parametet smoothing */
	virtual bool preProcessAudioBlock(IMidiEventQueue* midiEventQueue = nullptr);

	/** process sub-blocks of data (OPTIONAL MODE) */
	virtual bool processAudioBlock(ProcessBlockInfo& processBlockInfo);

	// virtual bool processAudioBuffers(ProcessBufferInfo& processBufferInfo);

	/** preProcess: do any post-buffer processing required; default operation is to send metering data to GUI  */
	virtual bool postProcessAudioBuffers(ProcessBufferInfo& processInfo);

	/** called by host plugin at top of buffer proccess; this alters parameters prior to variable binding operation  */
	virtual bool updatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo);

	/** called by host plugin at top of buffer proccess; this alters parameters prior to variable binding operation  */
	virtual bool updatePluginParameterNormalized(int32_t controlID, double normalizedValue, ParameterUpdateInfo& paramInfo);

	/** this can be called: 1) after bound variable has been updated or 2) after smoothing occurs  */
	virtual bool postUpdatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo);

	/** this is ony called when the user makes a GUI control change */
	virtual bool guiParameterChanged(int32_t controlID, double actualValue);

	/** processMessage: messaging system; currently used for custom/special GUI operations */
	virtual bool processMessage(MessageInfo& messageInfo);

	/** processMIDIEvent: MIDI event processing */
	virtual bool processMIDIEvent(midiEvent& event);

	/** specialized joystick servicing (currently not used) */
	virtual bool setVectorJoystickParameters(const VectorJoystickData& vectorJoysickData);

	/** create the presets */
	bool initPluginPresets();

	/** FX EXAMPLE: process audio by passing through */
	bool renderFXPassThrough(ProcessBlockInfo& blockInfo);

	/** SYNTH EXAMPLE: render a block of silence */
	bool renderSynthSilence(ProcessBlockInfo& blockInfo);

	//-- Meter Out ----------------------------------------------------------------- //
	float VuL = 0.0;
	float VuR = 0.0;
	int VuLMax = 0;
	int VuRMax = 0;
	float VuS = 0.0;
	float VuD = 0.0;
	float VuDelay1L = 0.0;
	float VuDelay1R = 0.0;
	float VuDelay2L = 0.0;
	float VuDelay2R = 0.0;
	float VuDelay3L = 0.0;
	float VuDelay3R = 0.0;
	float VuDelay4L = 0.0;
	float VuDelay4R = 0.0;
	float VuEcho1= 0.0;
	float VuWetMix = 0.0;
	float VuDryMix = 0.0;

	// -- Master Volume ------------------------------------------------------------ //
	double volumelin = 1.00000000;

	//-- Distortion ---------------------------------------------------------------- //
	double volumedist = 0.00000000;
	double voldistonoff = 0;

	//-- Filter On / Off ----------------------------------------------------------- //
	double mainFilterOnOff = 0;

	//-- Envelope Saturation / Limiter ON / OFF ------------------------------------ //
	double limiteronoff = 0;

	//-- Wet / Dry Mix ------------------------------------------------------------- //
	double wetDry = 1.00000000;

	//-- DELAY --------------------------------------------------------------------- //
	double delayvol = 0.00000000;
	double delayvol2 = 0.00000000;
	double delayvol3 = 0.00000000;
	double delayvol4 = 0.00000000;
	double delay1onoff = 0;
	double delay2onoff = 0;
	double delay3onoff = 0;
	double delay4onoff = 0;
	double delay1filteronoff = 1;
	double delay2filteronoff = 1;
	double delay3filteronoff = 1;
	double delay4filteronoff = 1;

	//-- Echo ---------------------------------------------------------------------- //
	double echoRate = 0.0000000;
	double echoVol = 0.0000000;
	double echoInten = 0.0000000;
	int echoOnOff = 0;
	float echoFeedback = 0;

	// --- END USER VARIABLES AND FUNCTIONS ---------------------------------------- //

protected:

private:
	//  **--0x07FD--**
	//-- Pre-req ------------------------------------------------------------------- //
	double M_PI = 3.1415925;
	double f0 = 20000.000000;
	double Q = 0.707100;
	int FilterType = 0;
	enum class FilterTypeEnum { LP, HP, BP1, BP2, Notch, APF, PEQ, LShelf, HShelf };
	double dbgain = 0.000000;
	double b0, b1, b2, a1, a2, z1L, z2L, z1R, z2R;
	double realbal, theta, costh, sinth;

	// -- Limiter ------------------------------------------------------------------ //
	Limiter m_Boost;
	double Boost = 0.f;
	Limiter m_Ceiling;
	double Ceiling = 0.f;

	// -- Envelope Shaper ---------------------------------------------------------- //
	EnvelopeShaper m_Attack;
	double EnvAttack = 1000.f;
	EnvelopeShaper m_Release;
	double EnvRelease = 1000.f;

	//-- Master Volume ------------------------------------------------------------- //
	double volume = 0.00000000;
	DeZipper dz_volume;

	//-- Delay [1,2,3,4] Volume ---------------------------------------------------- //
	double delayOneVolume = 10.00000000;
	double delayTwoVolume = 10.00000000;
	double delayThreeVolume = 10.00000000;
	double delayFourVolume = 10.00000000;

	//-- Distortion ---------------------------------------------------------------- //
	double dist = 0.00000000;

	//-- Echo Volume --------------------------------------------------------------- //
	double echoVol1 = 10.000000000;

	//-- Wet / Dry Mix ------------------------------------------------------------- //
	double wetDryMix = 1.00000000;

	//------------------ DelayBlock 1 -----------------------------------------------//
	int delay;
	int maxDelay;
	float* dBlock;
	float* dBlockI;
	int rPtr, wPtr;
	int rPtrI, wPtrI;
	double delayVar = 0.00000000;

	//------------------ DelayBlock 2 -----------------------------------------------//
	int delay2;
	int maxDelay2;
	float* dBlock2L;
	float* dBlock2R;
	int rPtr2L, wPtr2L;
	int rPtr2R, wPtr2R;
	double delayVar2 = 0.00000000;

	//------------------ DelayBlock 3 -----------------------------------------------//
	int delay3;
	int maxDelay3;
	float* dBlock3L;
	float* dBlock3R;
	int rPtr3L, wPtr3L;
	int rPtr3R, wPtr3R;
	double delayVar3 = 0.00000000;

	//------------------ DelayBlock 4 -----------------------------------------------//
	int delay4;
	int maxDelay4;
	float* dBlock4L;
	float* dBlock4R;
	int rPtr4L, wPtr4L;
	int rPtr4R, wPtr4R;
	double delayVar4 = 0.00000000;
	
	//------------------ Echo Block 1=6 (Mono to Stereo) ----------------------------//
	int echo1;
	int maxEcho1;
	float* eBlock1L;
	int erP1L, ewP1L;
	int	erP2L, ewP2L;
	int erP3L, ewP3L;
	int erP4L, ewP4L;
	int erP5L, ewP5L;
	int erP6L, ewP6L;
	double echoVar1 = 0.00000000;
	double echovar2 = 0.10000000;

public:
    /** static description: bundle folder name

	\return bundle folder name as a const char*
	*/
    static const char* getPluginBundleName();

    /** static description: name

	\return name as a const char*
	*/
    static const char* getPluginName();

	/** static description: short name

	\return short name as a const char*
	*/
	static const char* getShortPluginName();

	/** static description: vendor name

	\return vendor name as a const char*
	*/
	static const char* getVendorName();

	/** static description: URL

	\return URL as a const char*
	*/
	static const char* getVendorURL();

	/** static description: email

	\return email address as a const char*
	*/
	static const char* getVendorEmail();

	/** static description: Cocoa View Factory Name

	\return Cocoa View Factory Name as a const char*
	*/
	static const char* getAUCocoaViewFactoryName();

	/** static description: plugin type

	\return type (FX or Synth)
	*/
	static pluginType getPluginType();

	/** static description: VST3 GUID

	\return VST3 GUID as a const char*
	*/
	static const char* getVSTFUID();

	/** static description: 4-char code

	\return 4-char code as int
	*/
	static int32_t getFourCharCode();

	/** initalizer */
	bool initPluginDescriptors();

    /** Status Window Messages for hosts that can show it */
    void sendHostTextMessage(std::string messageString)
    {
        HostMessageInfo hostMessageInfo;
        hostMessageInfo.hostMessage = sendRAFXStatusWndText;
        hostMessageInfo.rafxStatusWndText.assign(messageString);
        if(pluginHostConnector)
            pluginHostConnector->sendHostMessage(hostMessageInfo);
    }

};

#endif /* defined(__pluginCore_h__) */


