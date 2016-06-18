//=======================================================================
/** @file BTrack.cpp
 *  @brief BTrack - a real-time beat tracker
 *  @author Adam Stark
 *  @copyright Copyright (C) 2008-2014  Queen Mary University of London
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//=======================================================================

#include <cmath>
#include <algorithm>
#include "BTrack.h"
#include "samplerate.h"
#include <iostream>

//=======================================================================
BTrack::BTrack()
 :  odf (512, 1024, ComplexSpectralDifferenceHWR, HanningWindow)
{
    initialise (512, 1024);
}

//=======================================================================
BTrack::BTrack (int hopSize_)
 :  odf(hopSize_, 2*hopSize_, ComplexSpectralDifferenceHWR, HanningWindow)
{	
    initialise (hopSize_, 2*hopSize_);
}

//=======================================================================
BTrack::BTrack (int hopSize_, int frameSize_)
 : odf (hopSize_, frameSize_, ComplexSpectralDifferenceHWR, HanningWindow)
{
    initialise (hopSize_, frameSize_);
}

//=======================================================================
BTrack::~BTrack()
{
#ifdef USE_FFTW
    // destroy fft plan
    fftw_destroy_plan (acfForwardFFT);
    fftw_destroy_plan (acfBackwardFFT);
    fftw_free (complexIn);
    fftw_free (complexOut);
#endif
    
#ifdef USE_KISS_FFT
    free (cfgForwards);
    free (cfgBackwards);
    delete [] fftIn;
    delete [] fftOut;
#endif
}

//=======================================================================
double BTrack::getBeatTimeInSeconds (long frameNumber, int hopSize, int fs)
{
    double hop = (double) hopSize;
    double samplingFrequency = (double) fs;
    double frameNum = (double) frameNumber;
    
    return ((hop / samplingFrequency) * frameNum);
}

//=======================================================================
double BTrack::getBeatTimeInSeconds (int frameNumber, int hopSize, int fs)
{
    long frameNum = (long) frameNumber;
    
    return getBeatTimeInSeconds (frameNum, hopSize, fs);
}



//=======================================================================
void BTrack::initialise (int hopSize_, int frameSize_)
{
    double rayparam = 43;
	double pi = 3.14159265;
	
	
	// initialise parameters
	tightness = 5;
	alpha = 0.9;
	tempo = 120;
	estimatedTempo = 120.0;
	tempoToLagFactor = 60.*44100./512.;
	
	m0 = 10;
	beatCounter = -1;
	
	beatDueInFrame = false;
	

	// create rayleigh weighting vector
	for (int n = 0; n < 128; n++)
	{
		weightingVector[n] = ((double) n / pow(rayparam,2)) * exp((-1*pow((double)-n,2)) / (2*pow(rayparam,2)));
	}
	
	// initialise prev_delta
	for (int i = 0; i < 41; i++)
	{
		prevDelta[i] = 1;
	}
	
	double t_mu = 41/2;
	double m_sig;
	double x;
	// create tempo transition matrix
	m_sig = 41/8;
	for (int i = 0;i < 41;i++)
	{
		for (int j = 0;j < 41;j++)
		{
			x = j+1;
			t_mu = i+1;
			tempoTransitionMatrix[i][j] = (1 / (m_sig * sqrt(2*pi))) * exp( (-1*pow((x-t_mu),2)) / (2*pow(m_sig,2)) );
		}
	}
	
	// tempo is not fixed
	tempoFixed = false;
    
    // initialise latest cumulative score value
    // in case it is requested before any processing takes place
    latestCumulativeScoreValue = 0;
    
    // initialise algorithm given the hopsize
    setHopSize(hopSize_);
    
    
    // Set up FFT for calculating the auto-correlation function
    FFTLengthForACFCalculation = 1024;
    
#ifdef USE_FFTW
    complexIn = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * FFTLengthForACFCalculation);		// complex array to hold fft data
    complexOut = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * FFTLengthForACFCalculation);	// complex array to hold fft data
    
    acfForwardFFT = fftw_plan_dft_1d (FFTLengthForACFCalculation, complexIn, complexOut, FFTW_FORWARD, FFTW_ESTIMATE);	// FFT plan initialisation
    acfBackwardFFT = fftw_plan_dft_1d (FFTLengthForACFCalculation, complexOut, complexIn, FFTW_BACKWARD, FFTW_ESTIMATE);	// FFT plan initialisation
#endif
    
#ifdef USE_KISS_FFT
    fftIn = new kiss_fft_cpx[FFTLengthForACFCalculation];
    fftOut = new kiss_fft_cpx[FFTLengthForACFCalculation];
    cfgForwards = kiss_fft_alloc (FFTLengthForACFCalculation, 0, 0, 0);
    cfgBackwards = kiss_fft_alloc (FFTLengthForACFCalculation, 1, 0, 0);
#endif
}

//=======================================================================
void BTrack::setHopSize (int hopSize_)
{	
	hopSize = hopSize_;
	onsetDFBufferSize = (512*512)/hopSize;		// calculate df buffer size
	
	beatPeriod = round(60/((((double) hopSize)/44100)*tempo));

    // set size of onset detection function buffer
    onsetDF.resize (onsetDFBufferSize);
    
    // set size of cumulative score buffer
    cumulativeScore.resize (onsetDFBufferSize);
	
	// initialise df_buffer to zeros
	for (int i = 0; i < onsetDFBufferSize; i++)
	{
		onsetDF[i] = 0;
		cumulativeScore[i] = 0;
		
		if ((i %  ((int) round(beatPeriod))) == 0)
		{
			onsetDF[i] = 1;
		}
	}
}

//=======================================================================
void BTrack::updateHopAndFrameSize (int hopSize_, int frameSize_)
{
    // update the onset detection function object
    odf.initialise (hopSize_, frameSize_);
    
    // update the hop size being used by the beat tracker
    setHopSize (hopSize_);
}

//=======================================================================
bool BTrack::beatDueInCurrentFrame()
{
    return beatDueInFrame;
}

//=======================================================================
double BTrack::getCurrentTempoEstimate()
{
    return estimatedTempo;
}

//=======================================================================
int BTrack::getHopSize()
{
    return hopSize;
}

//=======================================================================
double BTrack::getLatestCumulativeScoreValue()
{
    return latestCumulativeScoreValue;
}

//=======================================================================
void BTrack::processAudioFrame (double* frame)
{
    // calculate the onset detection function sample for the frame
    double sample = odf.calculateOnsetDetectionFunctionSample (frame);
    
    // process the new onset detection function sample in the beat tracking algorithm
    processOnsetDetectionFunctionSample (sample);
}

//=======================================================================
void BTrack::processOnsetDetectionFunctionSample (double newSample)
{
    // we need to ensure that the onset
    // detection function sample is positive
    newSample = fabs (newSample);
    
    // add a tiny constant to the sample to stop it from ever going
    // to zero. this is to avoid problems further down the line
    newSample = newSample + 0.0001;
    
	m0--;
	beatCounter--;
	beatDueInFrame = false;
		
	// add new sample at the end
    onsetDF.addSampleToEnd (newSample);
	
	// update cumulative score
	updateCumulativeScore (newSample);
	
	// if we are halfway between beats
	if (m0 == 0)
	{
		predictBeat();
	}
	
	// if we are at a beat
	if (beatCounter == 0)
	{
		beatDueInFrame = true;	// indicate a beat should be output
		
		// recalculate the tempo
		resampleOnsetDetectionFunction();
		calculateTempo();
	}
}

//=======================================================================
void BTrack::setTempo (double tempo)
{	 
	
	/////////// TEMPO INDICATION RESET //////////////////
	
	// firstly make sure tempo is between 80 and 160 bpm..
	while (tempo > 160)
	{
		tempo = tempo/2;
	}
	
	while (tempo < 80)
	{
		tempo = tempo * 2;
	}
		
	// convert tempo from bpm value to integer index of tempo probability 
	int tempo_index = (int) round((tempo - 80)/2);
	
	// now set previous tempo observations to zero
	for (int i=0;i < 41;i++)
	{
		prevDelta[i] = 0;
	}
	
	// set desired tempo index to 1
	prevDelta[tempo_index] = 1;
	
	
	/////////// CUMULATIVE SCORE ARTIFICAL TEMPO UPDATE //////////////////
	
	// calculate new beat period
	int new_bperiod = (int) round(60/((((double) hopSize)/44100)*tempo));
	
	int bcounter = 1;
	// initialise df_buffer to zeros
	for (int i = (onsetDFBufferSize-1);i >= 0;i--)
	{
		if (bcounter == 1)
		{
			cumulativeScore[i] = 150;
			onsetDF[i] = 150;
		}
		else
		{
			cumulativeScore[i] = 10;
			onsetDF[i] = 10;
		}
		
		bcounter++;
		
		if (bcounter > new_bperiod)
		{
			bcounter = 1;
		}
	}
	
	/////////// INDICATE THAT THIS IS A BEAT //////////////////
	
	// beat is now
	beatCounter = 0;
	
	// offbeat is half of new beat period away
	m0 = (int) round(((double) new_bperiod)/2);
}

//=======================================================================
void BTrack::fixTempo (double tempo)
{	
	// firstly make sure tempo is between 80 and 160 bpm..
	while (tempo > 160)
	{
		tempo = tempo/2;
	}
	
	while (tempo < 80)
	{
		tempo = tempo * 2;
	}
	
	// convert tempo from bpm value to integer index of tempo probability 
	int tempo_index = (int) round((tempo - 80)/2);
	
	// now set previous fixed previous tempo observation values to zero
	for (int i=0;i < 41;i++)
	{
		prevDeltaFixed[i] = 0;
	}
	
	// set desired tempo index to 1
	prevDeltaFixed[tempo_index] = 1;
		
	// set the tempo fix flag
	tempoFixed = true;
}

//=======================================================================
void BTrack::doNotFixTempo()
{	
	// set the tempo fix flag
	tempoFixed = false;
}

//=======================================================================
void BTrack::resampleOnsetDetectionFunction()
{
	float output[512];
    
    float input[onsetDFBufferSize];
    
    for (int i = 0;i < onsetDFBufferSize;i++)
    {
        input[i] = (float) onsetDF[i];
    }
        
    double src_ratio = 512.0/((double) onsetDFBufferSize);
    int BUFFER_LEN = onsetDFBufferSize;
    int output_len;
    SRC_DATA	src_data ;
    
    //output_len = (int) floor (((double) BUFFER_LEN) * src_ratio) ;
    output_len = 512;
    
    src_data.data_in = input;
    src_data.input_frames = BUFFER_LEN;
    
    src_data.src_ratio = src_ratio;
    
    src_data.data_out = output;
    src_data.output_frames = output_len;
    
    src_simple (&src_data, SRC_SINC_BEST_QUALITY, 1);
            
    for (int i = 0;i < output_len;i++)
    {
        resampledOnsetDF[i] = (double) src_data.data_out[i];
    }
}

//=======================================================================
void BTrack::calculateTempo()
{
	// adaptive threshold on input
	adaptiveThreshold (resampledOnsetDF,512);
		
	// calculate auto-correlation function of detection function
	calculateBalancedACF (resampledOnsetDF);
	
	// calculate output of comb filterbank
	calculateOutputOfCombFilterBank();
	
	// adaptive threshold on rcf
	adaptiveThreshold (combFilterBankOutput,128);

	
	int t_index;
	int t_index2;
	// calculate tempo observation vector from beat period observation vector
	for (int i = 0;i < 41;i++)
	{
		t_index = (int) round (tempoToLagFactor / ((double) ((2*i)+80)));
		t_index2 = (int) round (tempoToLagFactor / ((double) ((4*i)+160)));

		
		tempoObservationVector[i] = combFilterBankOutput[t_index-1] + combFilterBankOutput[t_index2-1];
	}
	
	
	double maxval;
	double maxind;
	double curval;
	
	// if tempo is fixed then always use a fixed set of tempi as the previous observation probability function
	if (tempoFixed)
	{
		for (int k = 0;k < 41;k++)
		{
			prevDelta[k] = prevDeltaFixed[k];
		}
	}
		
	for (int j=0;j < 41;j++)
	{
		maxval = -1;
		for (int i = 0;i < 41;i++)
		{
			curval = prevDelta[i] * tempoTransitionMatrix[i][j];
			
			if (curval > maxval)
			{
				maxval = curval;
			}
		}
		
		delta[j] = maxval * tempoObservationVector[j];
	}
	

	normaliseArray(delta,41);
	
	maxind = -1;
	maxval = -1;
	
	for (int j=0;j < 41;j++)
	{
		if (delta[j] > maxval)
		{
			maxval = delta[j];
			maxind = j;
		}
		
		prevDelta[j] = delta[j];
	}
	
	beatPeriod = round ((60.0*44100.0)/(((2*maxind)+80)*((double) hopSize)));
	
	if (beatPeriod > 0)
	{
		estimatedTempo = 60.0/((((double) hopSize) / 44100.0) * beatPeriod);
	}
}

//=======================================================================
void BTrack::adaptiveThreshold (double* x, int N)
{
	int i = 0;
	int k,t = 0;
	double x_thresh[N];
	
	int p_post = 7;
	int p_pre = 8;
	
	t = std::min(N,p_post);	// what is smaller, p_post of df size. This is to avoid accessing outside of arrays
	
	// find threshold for first 't' samples, where a full average cannot be computed yet 
	for (i = 0;i <= t;i++)
	{	
		k = std::min ((i+p_pre),N);
		x_thresh[i] = calculateMeanOfArray (x,1,k);
	}
	// find threshold for bulk of samples across a moving average from [i-p_pre,i+p_post]
	for (i = t+1;i < N-p_post;i++)
	{
		x_thresh[i] = calculateMeanOfArray (x,i-p_pre,i+p_post);
	}
	// for last few samples calculate threshold, again, not enough samples to do as above
	for (i = N-p_post;i < N;i++)
	{
		k = std::max ((i-p_post),1);
		x_thresh[i] = calculateMeanOfArray (x,k,N);
	}
	
	// subtract the threshold from the detection function and check that it is not less than 0
	for (i = 0; i < N; i++)
	{
		x[i] = x[i] - x_thresh[i];
		if (x[i] < 0)
		{
			x[i] = 0;
		}
	}
}

//=======================================================================
void BTrack::calculateOutputOfCombFilterBank()
{
	int numelem;
	
	for (int i = 0;i < 128;i++)
	{
		combFilterBankOutput[i] = 0;
	}
	
	numelem = 4;
	
	for (int i = 2; i <= 127; i++) // max beat period
	{
		for (int a = 1; a <= numelem; a++) // number of comb elements
		{
			for (int b = 1-a; b <= a-1; b++) // general state using normalisation of comb elements
			{
				combFilterBankOutput[i-1] = combFilterBankOutput[i-1] + (acf[(a*i+b)-1]*weightingVector[i-1])/(2*a-1);	// calculate value for comb filter row
			}
		}
	}
}

//=======================================================================
void BTrack::calculateBalancedACF (double* onsetDetectionFunction)
{
    int onsetDetectionFunctionLength = 512;
    
#ifdef USE_FFTW
    // copy into complex array and zero pad
    for (int i = 0;i < FFTLengthForACFCalculation;i++)
    {
        if (i < onsetDetectionFunctionLength)
        {
            complexIn[i][0] = onsetDetectionFunction[i];
            complexIn[i][1] = 0.0;
        }
        else
        {
            complexIn[i][0] = 0.0;
            complexIn[i][1] = 0.0;
        }
    }
    
    // perform the fft
    fftw_execute (acfForwardFFT);
    
    // multiply by complex conjugate
    for (int i = 0;i < FFTLengthForACFCalculation;i++)
    {
        complexOut[i][0] = complexOut[i][0]*complexOut[i][0] + complexOut[i][1]*complexOut[i][1];
        complexOut[i][1] = 0.0;
    }
    
    // perform the ifft
    fftw_execute (acfBackwardFFT);
    
#endif
    
#ifdef USE_KISS_FFT
    // copy into complex array and zero pad
    for (int i = 0;i < FFTLengthForACFCalculation;i++)
    {
        if (i < onsetDetectionFunctionLength)
        {
            fftIn[i].r = onsetDetectionFunction[i];
            fftIn[i].i = 0.0;
        }
        else
        {
            fftIn[i].r = 0.0;
            fftIn[i].i = 0.0;
        }
    }
    
    // execute kiss fft
    kiss_fft (cfgForwards, fftIn, fftOut);
    
    // multiply by complex conjugate
    for (int i = 0;i < FFTLengthForACFCalculation;i++)
    {
        fftOut[i].r = fftOut[i].r * fftOut[i].r + fftOut[i].i * fftOut[i].i;
        fftOut[i].i = 0.0;
    }
    
    // perform the ifft
    kiss_fft (cfgBackwards, fftOut, fftIn);
    
#endif
    
    double lag = 512;
    
    for (int i = 0; i < 512; i++)
    {
#ifdef USE_FFTW
        // calculate absolute value of result
        double absValue = sqrt (complexIn[i][0]*complexIn[i][0] + complexIn[i][1]*complexIn[i][1]);
#endif
        
#ifdef USE_KISS_FFT
        // calculate absolute value of result
        double absValue = sqrt (fftIn[i].r * fftIn[i].r + fftIn[i].i * fftIn[i].i);
#endif
        // divide by inverse lad to deal with scale bias towards small lags
        acf[i] = absValue / lag;
        
        // this division by 1024 is technically unnecessary but it ensures the algorithm produces
        // exactly the same ACF output as the old time domain implementation. The time difference is
        // minimal so I decided to keep it
        acf[i] = acf[i] / 1024.;
        
        lag = lag - 1.;
    }
}

//=======================================================================
double BTrack::calculateMeanOfArray (double* array, int startIndex, int endIndex)
{
	int i;
	double sum = 0;

    int length = endIndex - startIndex;
	
	// find sum
	for (i = startIndex; i < endIndex; i++)
	{
		sum = sum + array[i];
	}
	
    if (length > 0)
    {
        return sum / length;	// average and return
    }
    else
    {
        return 0;
    }
}

//=======================================================================
void BTrack::normaliseArray (double* array, int N)
{
	double sum = 0;
	
	for (int i = 0; i < N; i++)
	{
		if (array[i] > 0)
		{
			sum = sum + array[i];
		}
	}
	
	if (sum > 0)
	{
		for (int i = 0; i < N; i++)
		{
			array[i] = array[i] / sum;
		}
	}
}

//=======================================================================
void BTrack::updateCumulativeScore (double odfSample)
{	 
	int start, end, winsize;
	double max;
	
	start = onsetDFBufferSize - round (2 * beatPeriod);
	end = onsetDFBufferSize - round (beatPeriod / 2);
	winsize = end-start+1;
	
	double w1[winsize];
	double v = -2*beatPeriod;
	double wcumscore;
	
	// create window
	for (int i = 0; i < winsize; i++)
	{
		w1[i] = exp((-1 * pow (tightness * log (-v / beatPeriod), 2)) / 2);
		v = v+1;
	}	
	
	// calculate new cumulative score value
	max = 0;
	int n = 0;
	for (int i=start; i <= end; i++)
	{
			wcumscore = cumulativeScore[i]*w1[n];
		
			if (wcumscore > max)
			{
				max = wcumscore;
			}
		n++;
	}
	
    latestCumulativeScoreValue = ((1 - alpha) * odfSample) + (alpha * max);
    
    cumulativeScore.addSampleToEnd (latestCumulativeScoreValue);
}

//=======================================================================
void BTrack::predictBeat()
{	 
	int windowSize = (int) beatPeriod;
	double futureCumulativeScore[onsetDFBufferSize + windowSize];
	double w2[windowSize];
    
	// copy cumscore to first part of fcumscore
	for (int i = 0;i < onsetDFBufferSize;i++)
	{
		futureCumulativeScore[i] = cumulativeScore[i];
	}
	
	// create future window
	double v = 1;
	for (int i = 0; i < windowSize; i++)
	{
		w2[i] = exp((-1*pow((v - (beatPeriod/2)),2))   /  (2*pow((beatPeriod/2) ,2)));
		v++;
	}
	
	// create past window
	v = -2*beatPeriod;
	int start = onsetDFBufferSize - round(2*beatPeriod);
	int end = onsetDFBufferSize - round(beatPeriod/2);
	int pastwinsize = end-start+1;
	double w1[pastwinsize];

	for (int i = 0;i < pastwinsize;i++)
	{
		w1[i] = exp((-1*pow(tightness*log(-v/beatPeriod),2))/2);
		v = v+1;
	}

	// calculate future cumulative score
	double max;
	int n;
	double wcumscore;
	for (int i = onsetDFBufferSize; i < (onsetDFBufferSize + windowSize); i++)
	{
		start = i - round (2*beatPeriod);
		end = i - round (beatPeriod/2);
		
		max = 0;
		n = 0;
		for (int k=start;k <= end;k++)
		{
			wcumscore = futureCumulativeScore[k]*w1[n];
			
			if (wcumscore > max)
			{
				max = wcumscore;
			}
			n++;
		}
		
		futureCumulativeScore[i] = max;
	}
	
	// predict beat
	max = 0;
	n = 0;
	
	for (int i = onsetDFBufferSize; i < (onsetDFBufferSize + windowSize); i++)
	{
		wcumscore = futureCumulativeScore[i]*w2[n];
		
		if (wcumscore > max)
		{
			max = wcumscore;
			beatCounter = n;
		}	
		
		n++;
	}
		
	// set next prediction time
	m0 = beatCounter + round (beatPeriod / 2);
}