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

#include "plugincore.h"
#include "plugindescription.h"

PluginCore::PluginCore()
{
    // --- describe the plugin; call the helper to init the static parts you setup in plugindescription.h
    initPluginDescriptors();

    // --- default I/O combinations
	// --- for FX plugins
	if (getPluginType() == kFXPlugin)
	{
		addSupportedIOCombination({ kCFMono, kCFMono });
		addSupportedIOCombination({ kCFMono, kCFStereo });
		addSupportedIOCombination({ kCFStereo, kCFStereo });
	}
	else // --- synth plugins have no input, only output
	{
		addSupportedIOCombination({ kCFNone, kCFMono });
		addSupportedIOCombination({ kCFNone, kCFStereo });
	}

	// --- for sidechaining, we support mono and stereo inputs; auxOutputs reserved for future use
	addSupportedAuxIOCombination({ kCFMono, kCFNone });
	addSupportedAuxIOCombination({ kCFStereo, kCFNone });

	// --- create the parameters
    initPluginParameters();

    // --- create the presets
    initPluginPresets();
}

bool PluginCore::reset(ResetInfo& resetInfo)
{
    // --- save for audio processing
    audioProcDescriptor.sampleRate = resetInfo.sampleRate;
    audioProcDescriptor.bitDepth = resetInfo.bitDepth;

	// -- ENVELOPE SHAPER Prepare for audio -- //
	m_Boost.prepareForPlayback(getSampleRate());			// Resets Memory usage for function Envelope Shaper to start taking samples for processing.
	m_Ceiling.prepareForPlayback(getSampleRate());			//
	m_Attack.prepareForPlayback(getSampleRate());			//
	m_Release.prepareForPlayback(getSampleRate());			//

	// -- Filters ---------------------------- //
	z1L = z2L = z1R = z2R = 0.0;					// Resets Variables used in the Filter stage. 

	// -- Delay Block 1----------------------- //
	rPtr = wPtr = 0;						// Resets the Write, Read pointer to 0, for Left.
	rPtrI = wPtrI = 0;						// Resets the Write, Read pointer to 0, for Right.
	memset(dBlock, 0, maxDelay * sizeof(float));			// Resets the memory buffer used in dBlock to 0 referencing the maxDelay. Process for Left Channel.
	memset(dBlockI, 0, maxDelay * sizeof(float));			// Resets the memory buffer used in dBlockI to 0 referencing the maxDelay. Process for Right Channel.
	delay = maxDelay * delayVar;					// Redefention of delay with variables used outside ( PluginCore::reset(ResetInfo& resetInfo) ).

	// -- Delay Block 2----------------------- //
	rPtr2L = wPtr2L = 0;						// Same As Delay Block 1.
	rPtr2R = wPtr2R = 0;						// Same As Delay Block 1.
	memset(dBlock2L, 0, maxDelay2 * sizeof(float));			// Same As Delay Block 1.
	memset(dBlock2R, 0, maxDelay2 * sizeof(float));			// Same As Delay Block 1.
	delay2 = maxDelay2 * delayVar2;					// Same As Delay Block 1.

	// -- Delay Block 3----------------------- //
	rPtr3L = wPtr3L = 0;						// Same As Delay Block 1.
	rPtr3R = wPtr3R = 0;						// Same As Delay Block 1.
	memset(dBlock3L, 0, maxDelay3 * sizeof(float));			// Same As Delay Block 1.
	memset(dBlock3R, 0, maxDelay3 * sizeof(float));			// Same As Delay Block 1.
	delay3 = maxDelay3 * delayVar3;					// Same As Delay Block 1.

	// -- Delay Block 4----------------------- //
	rPtr4L = wPtr4L = 0;						// Same As Delay Block 1.
	rPtr4R = wPtr4R = 0;						// Same As Delay Block 1.
	memset(dBlock4L, 0, maxDelay4 * sizeof(float));			// Same As Delay Block 1.
	memset(dBlock4R, 0, maxDelay4 * sizeof(float));			// Same As Delay Block 1.
	delay4 = maxDelay4 * delayVar4;					// Same As Delay Block 1.

	//Echo Block 1---------------------------- //
	erP1L = ewP1L = 0;						// Same As Delay Block 1.
	erP2L = ewP2L = 0;						// Same As Delay Block 1.
	erP3L = ewP3L = 0;						// Same As Delay Block 1.
	erP4L = ewP4L = 0;						// Same As Delay Block 1.
	erP5L = ewP5L = 0;						// Same As Delay Block 1.
	erP6L = ewP6L = 0;						// Same As Delay Block 1
	memset(eBlock1L, 0, maxEcho1 * sizeof(float));			// Same As Delay Block 1.
	echo1 = maxEcho1 * echoVar1;					// Same As Delay Block 1.

    return PluginBase::reset(resetInfo);
}

bool PluginCore::initialize(PluginInfo& pluginInfo)
{
	// -- Initialize Variables 
	double LimiterMainL{};						// Initialization of Variables used for Processing frames for Envelope Shaper.
	double EnvelopeShaperMainL{};					//

	// -- FILTER ---------
	z1L = z2L = z1R = z2R = 0.0;					// Sets Variables used in Filter to 0. 
	
	// -- DELAY BLOCK 1 --
	maxDelay = getSampleRate();					// Max delay is referenced by the sample rate of what is being played always set to 1 second if getSampleRate() is not modified.
	dBlock = new float[maxDelay];					// Creates a new float for dBlock that references the size of maxDelay ( getSampleRate(); ) as its max size. 
	dBlockI = new float[maxDelay];					// Creates a new float for dBlockI that references the size of maxDelay ( getSampleRate(); ) as its max size. 

	// -- DELAY BLOCK 2 --
	maxDelay2 = getSampleRate();					// Same As Delay Block 1.
	dBlock2L = new float[maxDelay2];				// Same As Delay Block 1.
	dBlock2R = new float[maxDelay2];				// Same As Delay Block 1.

	// -- DELAY BLOCK 3 --
	maxDelay3 = getSampleRate();					// Same As Delay Block 1.
	dBlock3L = new float[maxDelay3];				// Same As Delay Block 1.
	dBlock3R = new float[maxDelay3];				// Same As Delay Block 1.

	// -- DELAY BLOCK 4 --
	maxDelay4 = getSampleRate();					// Same As Delay Block 1.
	dBlock4L = new float[maxDelay4];				// Same As Delay Block 1.
	dBlock4R = new float[maxDelay4];				// Same As Delay Block 1.
	
	// -- ECHO BLOCK 1 --
	maxEcho1 = getSampleRate();					// Same As Delay Block 1 but process is used for Echo Block.
	eBlock1L = new float[maxEcho1];					// Same As Delay Block 1 but process is used for Echo Block.

	return true;
}

bool PluginCore::preProcessAudioBuffers(ProcessBufferInfo& processInfo)
{
    // --- sync internal variables to GUI parameters; you can also do this manually if you don't
    //     want to use the auto-variable-binding
    syncInBoundVariables();

    return true;
}

bool PluginCore::processAudioFrame(ProcessFrameInfo& processFrameInfo)
{
	// --- fire any MIDI events for this sample interval
	processFrameInfo.midiEventQueue->fireMidiEvents(processFrameInfo.currentFrame);

	// --- do per-frame smoothing
	doParameterSmoothing();

	// --- decode the channelIOConfiguration and process accordingly
	//
	// --- Synth Plugin:
	// --- Synth Plugin --- remove for FX plugins
	if (getPluginType() == kSynthPlugin)
	{
		// --- output silence: change this with your signal render code
		processFrameInfo.audioOutputFrame[0] = 0.0;
		if (processFrameInfo.channelIOConfig.outputChannelFormat == kCFStereo)
			processFrameInfo.audioOutputFrame[1] = 0.0;

		return true;	/// processed
	}

	// Master Input -----------------------------------------------//
// Master Input Processing frame [0] - Left.
	double inL = processFrameInfo.audioInputFrame[0];				
// Master Input Processing frame [1] - Right. 
	double inR = processFrameInfo.audioInputFrame[1];				

	// Volume De Zipper function call for volumelin Log db scaling //
// Calls function from ZeZipper.h (Realtime Processing found in ZeZZipper.cpp).
	double volumelinDZ = dz_volume.smooth(volumelin);				

	// Process of each delay block output doubles. ----------------//
// Outputs for Delay Blocks 1, 2, 3, 4.
	double outL, outR, out2L, out2R, out3L, out3R, out4L, out4R;	
// Sumation double's used for series processing output.
	double mainL, mainR, mainEL, mainER;							
// Master sumation doubles used for series processing before and after Envelope Shapeing.
	double masterSumL, masterSumR, L, R;							

	// Envelope Shaping (Saturation). ------------------------------//
// {} Initalizer used to provent memory utilization problems.
	double LimiterMainL{}, LimiterMainR{};							
// {} is recommended because it disallows narrowing convertions and can be applied to types without any declared constructors.
	double EnvelopeShaperMainL{}, EnvelopeShaperMainR{};			

	// Echo Doubles ------------------------------------------------//
// Value used for the seperation of the echo from the delay lines.
	double echoMain;												
// Value used initally for troubleshooing.
	double echotemp = 0;											

	// Wet & Dry Variable ------------------------------------------//
// Variable used for Wet and Dry mix before Envelope Shaper.
	double wetDry = 1;												

	// Filter Logic for Pre Defined Variables ----------------------//
// Variables used for Filter Logic.
	double inLt, inRt;												
// FB Gain.
	double m_OPFB = 0.6;											
// FF coef to keep gain at 0db.
	double m_OPFF = 1 - m_OPFB;										
	double m_OPMM = 0;

	// Vu Meter Variables 
// Float referenced in Vu Meter Logic.
	float sum, dif;								
// Same as above statement.
	double VRM, VLM;												


	// -- EnvelopeShaper On/Off -----------------------------// 
// Statement used as a logic switch that when conditions are met, the process of setting LimiterMain,
// and EnvelopeShaperMain variables to 0. cutting sound from those processes. 
	if (voldistonoff == 0) {										 
		LimiterMainL = 0.0;											
		LimiterMainR = 0.0;											
		EnvelopeShaperMainL = 0.0;									
		EnvelopeShaperMainR = 0.0;									
	}

	//-- Filter Main On/Off ------------------------------------//
// Filter logic for switching the input of the filter On and Off, same logic as envelopeshaper on off, but reversed.
	if (mainFilterOnOff == 0) {										
		inLt = b0 * inL + z1L;
		z1L = b1 * inL - a1 * inLt + z2L;       //z1
		z2L = b2 * inL - a2 * inLt;             //z2

		inRt = b0 * inR + z1R;
		z1R = b1 * inR - a1 * inRt + z2R;       //z1
		z2R = b2 * inR - a2 * inRt;			    //z2

		inL = inLt;
		inR = inRt;
	}

	//-- DELAY BLOCK 1 -----------------------------------------//

// Defines the read and write pointer of left, to start at a set variable (delay) initialized in (PluginCore::reset(ResetInfo& resetInfo)).
	rPtr = wPtr - delay;											
// Defines the read and write pointer of Right, to start at a set variable (delay) initialized in (PluginCore::reset(ResetInfo& resetInfo)).
	rPtrI = wPtrI - delay;											

// Statement defines if the read Left pointer is above 0, the read pointer is equal to max delay,
// + any variable after the fact. used for circular buffer.
	if (rPtr < 0)													
		rPtr += maxDelay;											

// Statement defines if the read Right pointer is above 0, the read pointer is equal to max delay,
// + any variable after the fact. used for circular buffer.
	if (rPtrI < 0)													
		rPtrI += maxDelay;											

// Defines that within the left dBlock memory buffer referencing the Write pointer, this will write the value of ( inL ).
	dBlock[wPtr] = inL;												
// Defines that within the left dBlock memory buffer referencing the Read pointer, this will read the buffer at this point, outputing ( outL ).
	outL = dBlock[rPtr];														

// Defines that within the right dBlock memory buffer referencing the Write pointer, this will write the value of ( inR ).
	dBlockI[wPtrI] = inR;											
// Defines that within the right dBlock memory buffer referencing the Read pointer, this will read the buffer at this point, outputing ( outR ).
	outR = dBlockI[rPtrI];											

// States that the left write pointer will move position in the buffer dBlock, every time it finishes writing each memory location.
	wPtr++;															
// States that the right write pointer will move position in the buffer dBlock, every time it finishes writing each memory location.
	wPtrI++;														

// Defines the left write pointer with logic that when the write pointer is at the same memory location as the maxDelay aka. getSampleRate(),
// the write pointer will be redefined to 0. The start of the memeory block. 
	if (wPtr >= maxDelay)											
		wPtr = 0;													

// Defines the Right write pointer with logic that when the write pointer is at the same memory location as the maxDelay aka. getSampleRate(),
// the write pointer will be redefined to 0. The start of the memeory block. 
	if (wPtrI >= maxDelay)											
		wPtrI = 0;													

// This if statement is used for an on off switch that will cut the output of the delay block when the variable delay () onoff is set to 0.
// In future work, the process of setting the write pointer to 0, would be more beneficial and less processer heavy.
	if (delay1onoff == 0) {											 
		outL = outL - outL;											
		outR = outR - outR;
	}

	//-- DELAY BLOCK 2 -----------------------------------------//
	rPtr2L = wPtr2L - delay2;
	rPtr2R = wPtr2R - delay2;
	if (rPtr2L < 0)
		rPtr2L += maxDelay2;
	if (rPtr2R < 0)
		rPtr2R += maxDelay2;
	dBlock2L[wPtr2L] = inL;
	out2L = dBlock2L[rPtr2L];
	dBlock2R[wPtr2R] = inR;
	out2R = dBlock2R[rPtr2R];
	wPtr2L++;
	wPtr2R++;
	if (wPtr2L >= maxDelay2)
		wPtr2L = 0;
	if (wPtr2R >= maxDelay2)
		wPtr2R = 0;
	if (delay2onoff == 0) {
		out2L = out2L - out2L;
		out2R = out2R - out2R;
	}

	//-- DELAY BLOCK 3 -----------------------------------------//
	rPtr3L = wPtr3L - delay3;
	rPtr3R = wPtr3R - delay3;
	if (rPtr3L < 0)
		rPtr3L += maxDelay3;
	if (rPtr3R < 0)
		rPtr3R += maxDelay3;
	dBlock3L[wPtr3L] = inL;
	out3L = dBlock3L[rPtr3L];
	dBlock3R[wPtr3R] = inR;
	out3R = dBlock3R[rPtr3R];
	wPtr3L++;
	wPtr3R++;
	if (wPtr3L >= maxDelay3)
		wPtr3L = 0;
	if (wPtr3R >= maxDelay3)
		wPtr3R = 0;
	if (delay3onoff == 0) {
		out3L = out3L - out3L;
		out3R = out3R - out3R;
	}

	//-- DELAY BLOCK 4 -----------------------------------------//
	rPtr4L = wPtr4L - delay4;
	rPtr4R = wPtr4R - delay4;
	if (rPtr4L < 0)
		rPtr4L += maxDelay4;
	if (rPtr4R < 0)
		rPtr4R += maxDelay4;
	dBlock4L[wPtr4L] = inL;
	out4L = dBlock4L[rPtr4L];

	dBlock4R[wPtr4R] = inR;
	out4R = dBlock4R[rPtr4R];

	wPtr4L++;
	wPtr4R++;
	if (wPtr4L >= maxDelay4)
		wPtr4L = 0;
	if (wPtr4R >= maxDelay4)
		wPtr4R = 0;
	if (delay4onoff == 0) {
		out4L = out4L - out4L;
		out4R = out4R - out4R;
	}

	// -- Echo Main On/Off -------------------------------------//

// This if statement is used for an on off switch that will cut the output of the echo block when the variable echo () onoff is set to 0.
	if (echoOnOff == 0) {													
		echoMain = 0.0;
	}
	else {

	// -- Echo Blocks -------------------------------------------------------------------------------------------------------------------//
	//------------------------ (1) -----------------------//

// Defines the read and write pointer of left, to start at a set variable (echo1) initialized in (PluginCore::reset(ResetInfo& resetInfo)).
		erP1L = ewP1L - echo1;

// Statement defines if the read pointer is less than 0, the read pointer is equal to maxEcho1.
// if statement is true process below.
		if (erP1L < 0) erP1L += maxEcho1;									
		{																	

// Defines that within the dBlock memory buffer referencing the Read pointer, this will read the buffer at this point, outputing (echoMain).
			echoMain = eBlock1L[erP1L];										

// Defines the write pointer within eBlock1L to the input + LP Filter feedback.
// 0.6 is defined as the echo [write] position offset. 
			// Add variable to 0.6. Float. 
			eBlock1L[ewP1L] = inL + m_OPFF * (echoFeedback * echoMain) + m_OPMM;		
																			
// outputs the same samples as above but with the LP Added on.
			m_OPMM = eBlock1L[ewP1L] * m_OPFB;								

//increment write pointer
// States that the write pointer will move position in the buffer dBlock, every time it finishes writing each memory location.
			ewP1L++;														
		}

// Defines the left write pointer with logic that when the write pointer is at the same memory location as the maxDelay aka. getSampleRate(),
// the write pointer will be redefined to 0. The start of the memeory block. 
		if (ewP1L >= maxEcho1) ewP1L = 0;									
		
// I have tested the sound of just 1 echo block playing, and it does not seem to have clarity in the LP repeats.
		//------------------------ (2) -----------------------//
		erP2L = ewP2L - echo1;
		if (erP2L < 0) erP2L += maxEcho1; {
			echoMain = eBlock1L[erP2L];
			eBlock1L[ewP2L] = inL + m_OPFF * (0.5 * echoMain) + m_OPMM;
			m_OPMM = eBlock1L[ewP2L] * m_OPFB;
			//increment write pointer
			ewP2L++;
		}
		if (ewP2L >= maxEcho1) ewP2L = 0;

		//------------------------ (3) -----------------------//
		erP3L = ewP3L - echo1;
		if (erP3L < 0) erP3L += maxEcho1; {
			echoMain = eBlock1L[erP3L];
			eBlock1L[ewP3L] = inL + m_OPFF * (0.4 * echoMain) + m_OPMM;
			m_OPMM = eBlock1L[ewP3L] * m_OPFB;
			//increment write pointer
			ewP3L++;
		}
		if (ewP3L >= maxEcho1) ewP3L = 0;

		//------------------------ (4) -----------------------//
		erP4L = ewP4L - echo1;
		if (erP4L < 0) erP4L += maxEcho1; {
			echoMain = eBlock1L[erP4L];
			eBlock1L[ewP4L] = inL + m_OPFF * (0.8 * echoMain) + m_OPMM;
			m_OPMM = eBlock1L[ewP4L] * m_OPFB;
			//increment write pointer
			ewP4L++;
		}
		if (ewP4L >= maxEcho1) ewP4L = 0;

		//------------------------ (5) -----------------------//
		erP5L = ewP5L - echo1;
		if (erP5L < 0) erP5L += maxEcho1; {
			echoMain = eBlock1L[erP5L];
			eBlock1L[ewP5L] = inL + m_OPFF * (0.7 * echoMain) + m_OPMM;
			m_OPMM = eBlock1L[ewP5L] * m_OPFB;
			//increment write pointer
			ewP5L++;
		}
		if (ewP5L >= maxEcho1) ewP5L = 0;

		//------------------------ (6) -----------------------//
		erP6L = ewP6L - echo1;
		if (erP6L < 0) erP6L += maxEcho1; {
			echoMain = eBlock1L[erP6L];
			eBlock1L[ewP6L] = inL + m_OPFF * (0.2 * echoMain) + m_OPMM;
			m_OPMM = eBlock1L[ewP6L] * m_OPFB;
			//increment write pointer
			ewP6L++;
		}
		if (ewP6L >= maxEcho1) ewP6L = 0;
		
	}
	
	// -- Limiter & Envelope Sautration --------------------------------------------------------------------------------//
	// -- PRE OUTPUT LOGIC ---------------------------------------------------------------------------------------------//

		/* Main Addition of Delay Blocks including Volume adjustments */
// Adding all delay lines together for series processing
	mainL = ((outL * delayvol) + (out2L * delayvol2) + (out3L * delayvol3) + (out4L * delayvol4))/2;  
	mainR = ((outR * delayvol) + (out2R * delayvol2) + (out3R * delayvol3) + (out4R * delayvol4))/2;

// Echo is seperate for troubleshooting early into the development, tried to get an Echo VU meter working but this was not possible at the time.
	mainEL = (echoMain * echoVol)/2;
	mainER = (echoMain * echoVol)/2;

// Creating a master sum double for processing output. 
	masterSumL = (mainL + mainEL + (inL * wetDryMix))/2; 
	masterSumR = (mainR + mainER + (inR * wetDryMix))/2;


	// -- Vu Meter Logic --------------------------------------------------------------------------------------------------------------------------//
// Adding all processes to a single double value per channel to make processing limited for VU Meters shown below.
// Further Development into Vu Meters would need a real time sample by sample measurement for a meter system to function as intended. 
	VLM = (((outL * delayvol) + (out2L * delayvol2) + (out3L * delayvol3) + (out4L * delayvol4)) + (echoMain * echoVol) + (inL * wetDryMix) + (EnvelopeShaperMainL * LimiterMainL));
	VRM = (((outR * delayvol) + (out2R * delayvol2) + (out3R * delayvol3) + (out4R * delayvol4)) + (echoMain * echoVol) + (inR * wetDryMix) + (EnvelopeShaperMainR * LimiterMainR));


// Volume locked out of code due to changes in the VU meters not working.
	// I cant seem to get the Vu Meter to work with Log scaling without it being very quiet.
	VuL = VLM;
	VuR = VRM;

// * 20 due to the same issues found in VLM,VRM scaling log issues.
	sum = (VLM + VRM);
	dif = (VLM - VRM);
	VuS = sum;
	VuD = dif;

// Values for L,R channels
	VuDelay1L = (outL * delayvol) + (outR * delayvol) * volumelinDZ;

// Due to a change from digital level meters to Analouge meters, the Right variabes were not needed.
	VuDelay1R = 0;																		
	VuDelay2L = (out2L * delayvol2) + (out2R * delayvol2) * volumelinDZ;
	VuDelay2R = 0;
	VuDelay3L = (out3L * delayvol3) + (out3R * delayvol3) * volumelinDZ;;
	VuDelay3R = 0;
	VuDelay4L = (out4L * delayvol4) + (out4R * delayvol4) * volumelinDZ;;
	VuDelay4R = 0;

// Vu Meter output.
	VuWetMix = (VLM + VRM) - (inL + inR) * wetDryMix;
	VuDryMix = (inL + inR) * wetDryMix;

	// Copyright AUDIO DEV ACHADEMY :: Author (Joe).
	// Creating an Envelope Shaper that has a Limiter function.
	// All rightes reserved ( Open Source Code ). 
	// Referenced code found in the following ( AudioDevEffect.h / EnvelopeShaper.h / Limiter.h ). 
	//
	// Processing of audio frames from classes stated above. 

// L,R master channels for series processing.
	LimiterMainL = masterSumR;
	LimiterMainR = masterSumL;															 
	EnvelopeShaperMainL = masterSumL;													
	EnvelopeShaperMainR = masterSumR;

// Calls the process function from class m_Boost found in (Limiter.h). Function will process every sample frame in sequence.
		m_Boost.processAudioSample(LimiterMainL);										
		m_Boost.processAudioSample(LimiterMainR);

		m_Ceiling.processAudioSample(LimiterMainL);										
		m_Ceiling.processAudioSample(LimiterMainR);

		m_Attack.processAudioSample(EnvelopeShaperMainL);								
		m_Attack.processAudioSample(EnvelopeShaperMainR);		

		m_Release.processAudioSample(EnvelopeShaperMainL);								 
		m_Release.processAudioSample(EnvelopeShaperMainR);

// Sumation of Processing outputting L as final stage of series blocks.
		L = (masterSumL + (EnvelopeShaperMainL * LimiterMainL)) * volumelinDZ;
		R = (masterSumR + (EnvelopeShaperMainR * LimiterMainR)) * volumelinDZ;

	// -- STEREO OUTPUT ------------------------------------------------------------------------------------------------//
	
// Processing of Master output + Envelope shaper 
// Stil needing to add zipper to output, due to process issues with cracking noises.
	processFrameInfo.audioOutputFrame[0] = L/2;
	processFrameInfo.audioOutputFrame[1] = R/2;


		return true; /// processed	}
    return false; /// NOT processed
}
bool PluginCore::preProcessAudioBlock(IMidiEventQueue* midiEventQueue)
{
	// --- pre-process the block
	processBlockInfo.clearMidiEvents();

	// --- sample accurate parameter updates
	for (uint32_t sample = processBlockInfo.blockStartIndex;
		sample < processBlockInfo.blockStartIndex + processBlockInfo.blockSize;
		sample++)
	{
		// --- the MIDI handler will load up the vector in processBlockInfo
		if (midiEventQueue)
			midiEventQueue->fireMidiEvents(sample);
	}

	// --- this will do parameter smoothing ONLY ONCE AT THE TOP OF THE BLOCK PROCESSING
	//
	// --- to perform per-sample parameter smoothing, move this line of code, AND your updating
	//     functions (see updateParameters( ) in comment below) into the for( ) loop above
	//     NOTE: smoothing only once per block usually SAVES CPU cycles
	//           smoothing once per sample period usually EATS CPU cycles, potentially unnecessarily
	doParameterSmoothing();

	// --- call your GUI update/cooking function here, now that smoothing has occurred
	//
	//     NOTE:
	//     updateParameters is the name used in Will Pirkle's books for the GUI update function
	//     you may name it what you like - this is where GUI control values are cooked
	//     for the DSP algorithm at hand
	//     NOTE: updating (cooking) only once per block usually SAVES CPU cycles
	//           updating (cooking) once per sample period usually EATS CPU cycles, potentially unnecessarily
	// updateParameters();

	return true;
}



bool PluginCore::processAudioBlock(ProcessBlockInfo& processBlockInfo)
{
	// --- FX or Synth Render
	//     call your block processing function here
	// --- Synth
	if (getPluginType() == kSynthPlugin)
		renderSynthSilence(processBlockInfo);

	// --- or FX
	else if (getPluginType() == kFXPlugin)
		renderFXPassThrough(processBlockInfo);

	return true;
}

bool PluginCore::renderSynthSilence(ProcessBlockInfo& blockInfo)
{
	// --- process all MIDI events in this block (same as SynthLab)
	uint32_t midiEvents = blockInfo.getMidiEventCount();
	for (uint32_t i = 0; i < midiEvents; i++)
	{
		// --- get the event
		midiEvent event = *blockInfo.getMidiEvent(i);

		// --- do something with it...
		// myMIDIMessageHandler(event); // <-- you write this
	}

	// --- render a block of audio; here it is silence but in your synth
	//     it will likely be dependent on the MIDI processing you just did above
	for (uint32_t sample = blockInfo.blockStartIndex, i = 0;
		 sample < blockInfo.blockStartIndex + blockInfo.blockSize;
		 sample++, i++)
	{
		// --- write outputs
		for (uint32_t channel = 0; channel < blockInfo.numAudioOutChannels; channel++)
		{
			// --- silence (or, your synthesized block of samples)
			blockInfo.outputs[channel][sample] = 0.0;
		}
	}
	return true;
}

bool PluginCore::renderFXPassThrough(ProcessBlockInfo& blockInfo)
{
	// --- block processing -- write to outputs
	for (uint32_t sample = blockInfo.blockStartIndex, i = 0;
		sample < blockInfo.blockStartIndex + blockInfo.blockSize;
		sample++, i++)
	{
		// --- handles multiple channels, but up to you for bookkeeping
		for (uint32_t channel = 0; channel < blockInfo.numAudioOutChannels; channel++)
		{
			// --- pass through code, or your processed FX version
			blockInfo.outputs[channel][sample] = blockInfo.inputs[channel][sample];
		}
	}
	return true;
}

bool PluginCore::postProcessAudioBuffers(ProcessBufferInfo& processInfo)
{
	// --- update outbound variables; currently this is meter data only, but could be extended
	//     in the future
	updateOutBoundVariables();

    return true;
}

bool PluginCore::updatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo)
{
    // --- use base class helper
    setPIParamValue(controlID, controlValue);

    // --- do any post-processing
    postUpdatePluginParameter(controlID, controlValue, paramInfo);

    return true; /// handled
}

bool PluginCore::updatePluginParameterNormalized(int32_t controlID, double normalizedValue, ParameterUpdateInfo& paramInfo)
{
	// --- use base class helper, returns actual value
	double controlValue = setPIParamValueNormalized(controlID, normalizedValue, paramInfo.applyTaper);

	// --- do any post-processing
	postUpdatePluginParameter(controlID, controlValue, paramInfo);

	return true; /// handled
}

bool PluginCore::postUpdatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo)
{

	switch (controlID)
	{

	// -- FILTER LOGIC -----------------------------------------------------------------------------------------------------//

	// This area Cooks the parameters/mathmatics of the plugin. 

	// Copyright RBJ Cook Book.
	// Filter ("LP,HP,BP1,BP2,Notch,APF,PEQ,LShelf,HShelf")
	// All rightes reserved ( Open Source Code ). 
		{
	case controlID::f0:
	case controlID::Q:
	case controlID::FilterType:
	{
		double fs = getSampleRate();
		double w0 = 2.0 * M_PI * f0 / fs;
		double alpha = sin(w0) / (2.0 * Q);
		double A = sqrt(pow(10, (dbgain / 20.0)));
		double a0 = 0.00000000;

		switch (FilterType)
		{
			//"LP,HP,BP1,BP2,Notch,APF,PEQ,LShelf,HShelf"
		case enumToInt(FilterTypeEnum::LP): //0 - LowPass
			a0 = 1.0 + alpha;
			b0 = (1.0 - cos(w0)) / 2.0;
			b1 = 1.0 - cos(w0);
			b2 = (1.0 - cos(w0)) / 2.0;
			a1 = -2.0 * cos(w0);
			a2 = 1.0 - alpha;
			break;
		case enumToInt(FilterTypeEnum::HP): //1 - HighPass
			a0 = 1.0 + alpha;
			b0 = (1.0 + cos(w0)) / 2.0;
			b1 = -(1.0 + cos(w0));
			b2 = (1.0 + cos(w0)) / 2.0;
			a1 = -2.0 * cos(w0);
			a2 = 1.0 - alpha;
			break;
		case enumToInt(FilterTypeEnum::BP1):
			//BPF:        H(s) = s / (s^2 + s/Q + 1)  (constant skirt gain, peak gain = Q)
			a0 = 1.0 + alpha;
			b0 = Q * alpha;
			b1 = 0.0;
			b2 = -Q * alpha;
			a1 = -2.0 * cos(w0);
			a2 = 1.0 - alpha;
			break;
		case enumToInt(FilterTypeEnum::BP2):
			//BPF:        H(s) = (s/Q) / (s^2 + s/Q + 1)      (constant 0 dB peak gain)
			b0 = alpha;
			b1 = 0.0;
			b2 = -alpha;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cos(w0);
			a2 = 1.0 - alpha;
			break;
		case enumToInt(FilterTypeEnum::Notch):
			//notch:      H(s) = (s^2 + 1) / (s^2 + s/Q + 1)
			b0 = 1.0;
			b1 = -2.0 * cos(w0);
			b2 = 1.0;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cos(w0);
			a2 = 1.0 - alpha;
			break;
		case enumToInt(FilterTypeEnum::APF):
			//APF:        H(s) = (s^2 - s/Q + 1) / (s^2 + s/Q + 1)
			b0 = 1.0 - alpha;
			b1 = -2.0 * cos(w0);
			b2 = 1.0 + alpha;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cos(w0);
			a2 = 1.0 - alpha;
			break;
		case enumToInt(FilterTypeEnum::PEQ):
			//peakingEQ:  H(s) = (s^2 + s*(A/Q) + 1) / (s^2 + s/(A*Q) + 1)
			b0 = 1.0 + alpha * A;
			b1 = -2.0 * cos(w0);
			b2 = 1.0 - alpha * A;
			a0 = 1.0 + alpha / A;
			a1 = -2.0 * cos(w0);
			a2 = 1.0 - alpha / A;
			break;
		case enumToInt(FilterTypeEnum::LShelf):
			//lowShelf: H(s) = A * (s^2 + (sqrt(A)/Q)*s + A)/(A*s^2 + (sqrt(A)/Q)*s + 1)
			b0 = A * ((A + 1.0) - (A - 1.0) * cos(w0) + 2.0 * sqrt(A) * alpha);
			b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cos(w0));
			b2 = A * ((A + 1.0) - (A - 1.0) * cos(w0) - 2.0 * sqrt(A) * alpha);
			a0 = (A + 1.0) + (A - 1.0) * cos(w0) + 2.0 * sqrt(A) * alpha;
			a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cos(w0));
			a2 = (A + 1.0) + (A - 1.0) * cos(w0) - 2.0 * sqrt(A) * alpha;
			break;
		case enumToInt(FilterTypeEnum::HShelf):
			//highShelf: H(s) = A * (A*s^2 + (sqrt(A)/Q)*s + 1)/(s^2 + (sqrt(A)/Q)*s + A)
			b0 = A * ((A + 1.0) + (A - 1.0) * cos(w0) + 2.0 * sqrt(A) * alpha);
			b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cos(w0));
			b2 = A * ((A + 1.0) + (A - 1.0) * cos(w0) - 2.0 * sqrt(A) * alpha);
			a0 = (A + 1.0) - (A - 1.0) * cos(w0) + 2.0 * sqrt(A) * alpha;
			a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cos(w0));
			a2 = (A + 1.0) - (A - 1.0) * cos(w0) - 2.0 * sqrt(A) * alpha;
			break;
		}
		b0 /= a0;
		b1 /= a0;
		b2 /= a0;
		a1 /= a0;
		a2 /= a0;
		return true;    /// handled
	}

	default:
		return false;   /// not handled
		}

		// -- Limiter -----------------------------------------------------------------//

		case controlID::m_Boost1:
		{
			m_Boost.setBoost(Boost);	// Referencing Limiter.h for setBoost(); function call.
			return true;			// handled : this is a break in the process so that each case is processed seperately. 
		}

		case controlID::m_Ceiling1:
		{
			m_Ceiling.setCeiling(Ceiling);
			return true;    /// handled
		}

		// -- SHAPER ------------------------------------------------------------------//

		case controlID::m_Attack1:
		{
			m_Attack.setAttack(EnvAttack);
			return true;    /// handled
		}

		case controlID::m_Release1:
		{
			m_Release.setAttack(EnvRelease);
			return true;    /// handled
		}
		
	case controlID::volume: // --------------------------------------- MASTER VOLUME -------------------------------------------//
	{
		volumelin = pow(10, volume / 20); // Converts to DB log base 20.
		return true;    /// handled
	}

	case controlID::dist: // ----------------------------------------- DISTORTION LOGIC ----------------------------------------//
	{
		volumedist = sin(dist * 30 / 4) - 1 / 2;	// experimental prameters for distortion, Not used ( early versions of the plugin ).
		return true;    /// handled
	}

	case controlID::voldistonoff: // --------------------------------- DISTORTION VOLUME ON/OFF --------------------------------//
	{
		voldistonoff;
		return true;    /// handled
	}

	case controlID::delayVar: // ------------------------------------- DELAY VARIABLE (1) --------------------------------------//
	{
		delay = maxDelay * delayVar; // defining the process of setting the max delay of this parameter to the delay variable, allowing timing of delay block to be controlled.
		return true;    /// handled
	}

	case controlID::delayVar2: // ------------------------------------ DELAY VARIABLE (2) --------------------------------------//
	{
		delay2 = maxDelay2 * delayVar2;
		return true;    /// handled
	}

	case controlID::delayVar3: // ------------------------------------ DELAY VARIABLE (3) --------------------------------------//
	{
		delay3 = maxDelay3 * delayVar3;
		return true;    /// handled
	}

	case controlID::delayVar4: // ------------------------------------ DELAY VARIABLE (4) --------------------------------------//
	{
		delay4 = maxDelay4 * delayVar4;
		return true;    /// handled
	}

	case controlID::echoVar1: // ------------------------------------- ECHO VARIABLE (2) ---------------------------------------//
	{
		echo1 = maxEcho1 * echoVar1;
		return true;    /// handled
	}

	case controlID::echoVar2:// -------------------------------------- ECHO VARIABLE (2) ---------------------------------------//
	{
		echoRate = echoVar1 * (echoVar2 / 200); // experimental prameters for distortion, Not used ( early versions of the plugin ).
		return true;
	}

	case controlID::wetDryMix: // ------------------------------------ (WET / DRY) MIX -----------------------------------------//
	{
		wetDryMix = wetDry / wetDryMix;
		return true;
	}

	case controlID::delayOneVolume: // ------------------------------- DELAY VOLUME (1) ----------------------------------------//
	{
		delayvol = pow(10, delayOneVolume / 20); // Converts to DB log base 20. RMS 
		return true;
	}

	case controlID::delayTwoVolume: // ------------------------------- DELAY VOLUME (2) ----------------------------------------//
	{
		delayvol2 = pow(10, delayTwoVolume / 20);
		return true;
	}

	case controlID::delayThreeVolume: // ----------------------------- DELAY VOLUME (3) ----------------------------------------//
	{
		delayvol3 = pow(10, delayThreeVolume / 20);
		return true;
	}

	case controlID::delayFourVolume: // ------------------------------ DELAY VOLUME (4) ----------------------------------------//
	{
		delayvol4 = pow(10, delayFourVolume / 20);
		return true;
	}

	case controlID::echoVol1: // ------------------------------------- ECHO VOLUME ---------------------------------------------//
	{
		echoVol = pow(10, echoVol1 / 20); // Converts to DB log base 20.
		return true;
	}

	case controlID::delay1onoff: // ---------------------------------- DELAY ON OFF (1) ----------------------------------------//
	{
		delay1onoff;
		return true;
	}

	case controlID::delay2onoff: // ---------------------------------- DELAY ON OFF (2) ----------------------------------------//
	{
		delay2onoff;
		return true;
	}

	case controlID::delay3onoff: // ---------------------------------- DELAY ON OFF (3) ----------------------------------------//
	{
		delay3onoff;
		return true;
	}

	case controlID::delay4onoff: // ---------------------------------- DELAY ON OFF (4) ----------------------------------------//
	{
		delay4onoff;
		return true;
	}

	case controlID::mainFilterOnOff: //------------------------------- FILTER MAIN ON/OFF --------------------------------------//
	{
		mainFilterOnOff;
		return true;
	}

	case controlID::echoOnOff: //------------------------------------- ECHO MAIN ON/OFF ----------------------------------------//
	{
		echoOnOff;
		return true;
	}

	case controlID::echoFeedback: //------------------------------------- ECHO MAIN ON/OFF ----------------------------------------//
	{
		echoFeedback;
		return true;
	}

		return false;
	}
}

bool PluginCore::guiParameterChanged(int32_t controlID, double actualValue)
{
	/*
	switch (controlID)
	{
		case controlID::<your control here>
		{

			return true; // handled
		}

		default:
			break;
	}*/

	return false; /// not handled
}

bool PluginCore::processMessage(MessageInfo& messageInfo)
{
	// --- decode message
	switch (messageInfo.message)
	{
		// --- add customization appearance here
	case PLUGINGUI_DIDOPEN:
	{
		return false;
	}

	// --- NULL pointers so that we don't accidentally use them
	case PLUGINGUI_WILLCLOSE:
	{
		return false;
	}

	// --- update view; this will only be called if the GUI is actually open
	case PLUGINGUI_TIMERPING:
	{
		return false;
	}

	// --- register the custom view, grab the ICustomView interface
	case PLUGINGUI_REGISTER_CUSTOMVIEW:
	{

		return false;
	}

	case PLUGINGUI_REGISTER_SUBCONTROLLER:
	case PLUGINGUI_QUERY_HASUSERCUSTOM:
	case PLUGINGUI_USER_CUSTOMOPEN:
	case PLUGINGUI_USER_CUSTOMCLOSE:
	case PLUGINGUI_EXTERNAL_SET_NORMVALUE:
	case PLUGINGUI_EXTERNAL_SET_ACTUALVALUE:
	{

		return false;
	}

	default:
		break;
	}

	return false; /// not handled
}

bool PluginCore::processMIDIEvent(midiEvent& event)
{
	// --- IF PROCESSING AUDIO BLOCKS: push into vector for block processing
	if (!pluginDescriptor.processFrames)
	{
		processBlockInfo.pushMidiEvent(event);
		return true;
	}

	// --- IF PROCESSING AUDIO FRAMES: decode AND service this MIDI event here
	//     for sample accurate MIDI
	// myMIDIMessageHandler(event); // <-- you write this

	return true;
}

bool PluginCore::setVectorJoystickParameters(const VectorJoystickData& vectorJoysickData)
{
	return true;
}

bool PluginCore::initPluginParameters()
{
	if (pluginParameterMap.size() > 0)
		return false;

	PluginParameter* piParam = nullptr;

	// -- LOG Volume Control -----------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::volume, "Volume", " ", controlVariableType::kDouble, -96.000000, 12.000000, 0.000000, taper::kLinearTaper);
	piParam->setBoundVariable(&volume, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// -- WET / DRY MIX ----------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::wetDryMix, "Wet Dry Mix", " ", controlVariableType::kDouble, -50.000000, -0.98, -0.98, taper::kLinearTaper);
	piParam->setBoundVariable(&wetDryMix, boundVariableType::kDouble);
	addPluginParameter(piParam);


	// -- EXPERIMENTAL FILTERS --------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::f0, "Filter: Cut-Off Frequency", " ", controlVariableType::kDouble, 50, 18000, 18000, taper::kVoltOctaveTaper);
	piParam->setBoundVariable(&f0, boundVariableType::kDouble);
	addPluginParameter(piParam);

	piParam = new PluginParameter(controlID::Q, "Filter: Q", " ", controlVariableType::kDouble, 0.100000, 10.000000, 0.707100, taper::kLinearTaper);
	piParam->setBoundVariable(&Q, boundVariableType::kDouble);
	addPluginParameter(piParam);

	/* List of Filter Types.  Default is LP (0)*/
	piParam = new PluginParameter(controlID::FilterType, "Filter Type", "LP,HP,BP1,BP2,Notch,APF,PEQ,LShelf,HShelf", "LP");
	piParam->setBoundVariable(&FilterType, boundVariableType::kInt);
	addPluginParameter(piParam);

	/* dB Gain Value for Shelving EQ and peaking EQ*/
	piParam = new PluginParameter(controlID::dbgain, "Filter: dB Gain", " ", controlVariableType::kDouble, -36.000000, 36.000000, 0.000000, taper::kLinearTaper);
	piParam->setBoundVariable(&dbgain, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// -- FILTER ON OFF ---------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::mainFilterOnOff, "Filter On Off ", "ON, OFF", " OFF");
	piParam->setBoundVariable(&mainFilterOnOff, boundVariableType::kInt);
	addPluginParameter(piParam);

	// -- DISTORTION ------------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::dist, "Distortion: Gain", "Unit", controlVariableType::kDouble, 0.000000, 3.000000, 0.000000, taper::kLinearTaper);
	piParam->setBoundVariable(&dist, boundVariableType::kDouble);
	addPluginParameter(piParam);
	piParam = new PluginParameter(controlID::voldistonoff, "Distortion:", "OFF, ON", " ON");
	piParam->setBoundVariable(&voldistonoff, boundVariableType::kInt);
	addPluginParameter(piParam);

	// -- DELAY BLOCK 1 ---------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::delayVar, "Delay Block 1", "Seconds", controlVariableType::kDouble, 0.0100000, 1.000000, 0.010000, taper::kLinearTaper);
	piParam->setBoundVariable(&delayVar, boundVariableType::kDouble);
	addPluginParameter(piParam);
	piParam = new PluginParameter(controlID::delay1onoff, "ON/OFF DELAY 1", "OFF, ON", " ON");
	piParam->setBoundVariable(&delay1onoff, boundVariableType::kInt);
	addPluginParameter(piParam);

	// -- DELAY BLOCK 2 ---------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::delayVar2, "Delay Block 2", "Seconds", controlVariableType::kDouble, 0.0100000, 1.000000, 0.010000, taper::kLinearTaper);
	piParam->setBoundVariable(&delayVar2, boundVariableType::kDouble);
	addPluginParameter(piParam);
	piParam = new PluginParameter(controlID::delay2onoff, "ON/OFF DELAY 2", "OFF, ON", " ON");
	piParam->setBoundVariable(&delay2onoff, boundVariableType::kInt);
	addPluginParameter(piParam);

	// -- DELAY BLOCK 3 ---------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::delayVar3, "Delay Block 3", "Seconds", controlVariableType::kDouble, 0.0100000, 1.000000, 0.010000, taper::kLinearTaper);
	piParam->setBoundVariable(&delayVar3, boundVariableType::kDouble);
	addPluginParameter(piParam);
	piParam = new PluginParameter(controlID::delay3onoff, "ON/OFF DELAY 3", "OFF, ON", " ON");
	piParam->setBoundVariable(&delay3onoff, boundVariableType::kInt);
	addPluginParameter(piParam);

	// -- DELAY BLOCK 4 ---------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::delayVar4, "Delay Block 4", "Seconds", controlVariableType::kDouble, 0.0100000, 1.000000, 0.010000, taper::kLinearTaper);
	piParam->setBoundVariable(&delayVar4, boundVariableType::kDouble);
	addPluginParameter(piParam);
	piParam = new PluginParameter(controlID::delay4onoff, "ON/OFF DELAY 4", "OFF, ON", " ON");
	piParam->setBoundVariable(&delay4onoff, boundVariableType::kInt);
	addPluginParameter(piParam);

	// -- ECHO BLOCK ------------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::echoVar1, "Echo Block", "Mutiples", controlVariableType::kDouble, 0.0100000, 0.99999, 0.010000, taper::kLinearTaper);
	piParam->setBoundVariable(&echoVar1, boundVariableType::kDouble);
	addPluginParameter(piParam);

	piParam = new PluginParameter(controlID::echoVol1, "Echo Volume", "dB", controlVariableType::kDouble, -96.000000, 12.000000, -96.00000, taper::kLinearTaper);
	piParam->setBoundVariable(&echoVol1, boundVariableType::kDouble);
	addPluginParameter(piParam);

	piParam = new PluginParameter(controlID::echoOnOff, "Echo ON/OFF ", "OFF, ON", " ON");
	piParam->setBoundVariable(&echoOnOff, boundVariableType::kInt);
	addPluginParameter(piParam);

	piParam = new PluginParameter(controlID::echoFeedback, "Feedback", "Unit", controlVariableType::kFloat, 0.000000, 1.000000, 0.000000, taper::kLinearTaper);
	piParam->setBoundVariable(&echoFeedback, boundVariableType::kFloat);
	addPluginParameter(piParam);


	// -- DELAY 1 VOLUME --------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::delayOneVolume, "Delay Volume", "dB", controlVariableType::kDouble, -96.000000, -5.000000, -96.000000, taper::kLinearTaper);
	piParam->setBoundVariable(&delayOneVolume, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// -- DELAY 2 VOLUME --------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::delayTwoVolume, "Delay 2 Volume", "dB", controlVariableType::kDouble, -96.000000, -5.000000, -96.000000, taper::kLinearTaper);
	piParam->setBoundVariable(&delayTwoVolume, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// -- DELAY 3 VOLUME --------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::delayThreeVolume, "Delay 3 Volume", "dB", controlVariableType::kDouble, -96.000000, -5.000000, -96.000000, taper::kLinearTaper);
	piParam->setBoundVariable(&delayThreeVolume, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// -- DELAY 4 VOLUME --------------------------------------------------------------------------------------//
	piParam = new PluginParameter(controlID::delayFourVolume, "Delay 4 Volume", "dB", controlVariableType::kDouble, -96.000000, -5.000000, -96.000000, taper::kLinearTaper);
	piParam->setBoundVariable(&delayFourVolume, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// -- LIMITER BOOST (Gain) --------------------------------------------------------------------------------//

	piParam = new PluginParameter(controlID::m_Boost1, "Limiter Boost", "", controlVariableType::kDouble, 0.000000, 1000.000000, 0.0000000, taper::kLinearTaper);
	piParam->setBoundVariable(&Boost, boundVariableType::kDouble);
	addPluginParameter(piParam);

	piParam = new PluginParameter(controlID::m_Ceiling1, "Limiter Ceiling", "dB", controlVariableType::kDouble, -96.00000, 0.000000, -50.00000, taper::kLinearTaper);
	piParam->setBoundVariable(&Ceiling, boundVariableType::kDouble);
	addPluginParameter(piParam);

	piParam = new PluginParameter(controlID::m_Attack1, "Attack", "Ms", controlVariableType::kDouble, 0.0100000, 1.000000, 0.010000, taper::kLinearTaper);
	piParam->setBoundVariable(&EnvAttack, boundVariableType::kDouble);
	addPluginParameter(piParam);

	piParam = new PluginParameter(controlID::m_Release1, "Release", "Ms", controlVariableType::kDouble, 0.0100000, 1.000000, 0.010000, taper::kLinearTaper);
	piParam->setBoundVariable(&EnvRelease, boundVariableType::kDouble);
	addPluginParameter(piParam);

	piParam = new PluginParameter(controlID::limiteronoff, "Shaper ON/OFF ", "OFF, ON", " ON");
	piParam->setBoundVariable(&limiteronoff, boundVariableType::kInt);
	addPluginParameter(piParam);

	// -- VU Meter Out ----------------------------------------------------------------------------------------//
	//
		/* Log RMS VU Meter Left */
	piParam = new PluginParameter(controlID::VuL, "VU Left", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuL, boundVariableType::kFloat);
	addPluginParameter(piParam);

		/* Log RMS VU Meter Right */
	piParam = new PluginParameter(controlID::VuR, "VU Right", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false); 
	piParam->setBoundVariable(&VuR, boundVariableType::kFloat);
	addPluginParameter(piParam);
		/* Log RMS VU Meter Sustain */
	piParam = new PluginParameter(controlID::VuS, "VU Sustain", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuS, boundVariableType::kFloat);
	addPluginParameter(piParam);
		/* Log RMS VU Meter DIF */
	piParam = new PluginParameter(controlID::VuD, "VU Dif", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuD, boundVariableType::kFloat);
	addPluginParameter(piParam);

		/* Log RMS VU Meter Delay 1 Left */
	piParam = new PluginParameter(controlID::VuDelay1L, "VU Delay 1 L", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuDelay1L, boundVariableType::kFloat);
	addPluginParameter(piParam);
		/* Log RMS VU Meter Delay 1 Right */
	piParam = new PluginParameter(controlID::VuDelay1R, "VU Delay 1 R", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuDelay1R, boundVariableType::kFloat);
	addPluginParameter(piParam);

		/* Log RMS VU Meter Delay 2 Left */
	piParam = new PluginParameter(controlID::VuDelay2L, "VU Delay 2 L", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuDelay2L, boundVariableType::kFloat);
	addPluginParameter(piParam);
		/* Log RMS VU Meter Delay 2 Right */
	piParam = new PluginParameter(controlID::VuDelay2R, "VU Delay 2 R", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuDelay2R, boundVariableType::kFloat);
	addPluginParameter(piParam);

		/* Log RMS VU Meter Delay 3 Left */
	piParam = new PluginParameter(controlID::VuDelay3L, "VU Delay 3 L", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuDelay3L, boundVariableType::kFloat);
	addPluginParameter(piParam);
		/* Log RMS VU Meter Delay 3 Right */
	piParam = new PluginParameter(controlID::VuDelay3R, "VU Delay 3 R", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuDelay3R, boundVariableType::kFloat);
	addPluginParameter(piParam);

		/* Log RMS VU Meter Delay 4 Left */
	piParam = new PluginParameter(controlID::VuDelay4L, "VU Delay 4 L", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuDelay4L, boundVariableType::kFloat);
	addPluginParameter(piParam);
		/* Log RMS VU Meter Delay 4 Right */
	piParam = new PluginParameter(controlID::VuDelay4R, "VU Delay 4 R", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuDelay4R, boundVariableType::kFloat);
	addPluginParameter(piParam);
	/* Log RMS VU Meter Wet Mix Left */
	piParam = new PluginParameter(controlID::VuWetMix, "VU Wet Mix L", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuWetMix, boundVariableType::kFloat);
	addPluginParameter(piParam);
	/* Log RMS VU Meter Dry Mix Right */
	piParam = new PluginParameter(controlID::VuDryMix, "VU Dry Mix R", 5.00, 250.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuDryMix, boundVariableType::kFloat);
	addPluginParameter(piParam);

	/* Meter Peak L */
	piParam = new PluginParameter(controlID::VuLMax, "VuLMax", 0.00, 500.00, ENVELOPE_DETECT_MODE_PEAK, meterCal::kLinearMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuLMax, boundVariableType::kFloat);
	addPluginParameter(piParam);
	/* Meter Peak R */
	piParam = new PluginParameter(controlID::VuRMax, "VuRMax", 0.00, 500.00, ENVELOPE_DETECT_MODE_PEAK, meterCal::kLinearMeter);
	piParam->setInvertedMeter(false);
	piParam->setIsProtoolsGRMeter(false);
	piParam->setBoundVariable(&VuRMax, boundVariableType::kFloat);
	addPluginParameter(piParam);

	// **--0xEDA5--**
	PluginParameter* piParamBonus = new PluginParameter(SCALE_GUI_SIZE, "Scale GUI", "tiny,small,medium,normal,large,giant", "normal");
	addPluginParameter(piParamBonus);

	// --- create the super fast access array
	initPluginParameterArray();

	return true;
}

bool PluginCore::initPluginPresets()
{
	// **--0xFF7A--**
	
	// **--0xA7FF--**

    return true;
}

bool PluginCore::initPluginDescriptors()
{
	// --- setup audio procssing style
	//
	// --- kProcessFrames and kBlockSize are set in plugindescription.h
	//
	// --- true:  process audio frames --- less efficient, but easier to understand when starting out
	//     false: process audio blocks --- most efficient, but somewhat more complex code
	pluginDescriptor.processFrames = kProcessFrames;

	// --- for block processing (if pluginDescriptor.processFrame == false),
	//     this is the block size
	processBlockInfo.blockSize = kBlockSize;

    pluginDescriptor.pluginName = PluginCore::getPluginName();
    pluginDescriptor.shortPluginName = PluginCore::getShortPluginName();
    pluginDescriptor.vendorName = PluginCore::getVendorName();
    pluginDescriptor.pluginTypeCode = PluginCore::getPluginType();

	// --- describe the plugin attributes; set according to your needs
	pluginDescriptor.hasSidechain = kWantSidechain;
	pluginDescriptor.latencyInSamples = kLatencyInSamples;
	pluginDescriptor.tailTimeInMSec = kTailTimeMsec;
	pluginDescriptor.infiniteTailVST3 = kVSTInfiniteTail;

    // --- AAX
    apiSpecificInfo.aaxManufacturerID = kManufacturerID;
    apiSpecificInfo.aaxProductID = kAAXProductID;
    apiSpecificInfo.aaxBundleID = kAAXBundleID;  /* MacOS only: this MUST match the bundle identifier in your info.plist file */
    apiSpecificInfo.aaxEffectID = "aaxDeveloper.";
    apiSpecificInfo.aaxEffectID.append(PluginCore::getPluginName());
    apiSpecificInfo.aaxPluginCategoryCode = kAAXCategory;

    // --- AU
    apiSpecificInfo.auBundleID = kAUBundleID;   /* MacOS only: this MUST match the bundle identifier in your info.plist file */
    apiSpecificInfo.auBundleName = kAUBundleName;

    // --- VST3
    apiSpecificInfo.vst3FUID = PluginCore::getVSTFUID(); // OLE string format
    apiSpecificInfo.vst3BundleID = kVST3BundleID;/* MacOS only: this MUST match the bundle identifier in your info.plist file */
	apiSpecificInfo.enableVST3SampleAccurateAutomation = kVSTSAA;
	apiSpecificInfo.vst3SampleAccurateGranularity = kVST3SAAGranularity;

    // --- AU and AAX
    apiSpecificInfo.fourCharCode = PluginCore::getFourCharCode();

    return true;
}

// --- static functions required for VST3/AU only --------------------------------------------- //
const char* PluginCore::getPluginBundleName() { return getPluginDescBundleName(); }
const char* PluginCore::getPluginName(){ return kPluginName; }
const char* PluginCore::getShortPluginName(){ return kShortPluginName; }
const char* PluginCore::getVendorName(){ return kVendorName; }
const char* PluginCore::getVendorURL(){ return kVendorURL; }
const char* PluginCore::getVendorEmail(){ return kVendorEmail; }
const char* PluginCore::getAUCocoaViewFactoryName(){ return AU_COCOA_VIEWFACTORY_STRING; }
pluginType PluginCore::getPluginType(){ return kPluginType; }
const char* PluginCore::getVSTFUID(){ return kVSTFUID; }
int32_t PluginCore::getFourCharCode(){ return kFourCharCode; }
