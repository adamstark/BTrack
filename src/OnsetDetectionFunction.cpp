//=======================================================================
/** @file OnsetDetectionFunction.cpp
 *  @brief A class for calculating onset detection functions
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

#include <math.h>
#include "OnsetDetectionFunction.h"

//=======================================================================
OnsetDetectionFunction::OnsetDetectionFunction (int hopSize_,int frameSize_)
 :  onsetDetectionFunctionType (ComplexSpectralDifferenceHWR), windowType (HanningWindow)
{
    // indicate that we have not initialised yet
	initialised = false;
	
	// set pi
	pi = 3.14159265358979;
	
	// initialise with arguments to constructor
	initialise (hopSize_, frameSize_, ComplexSpectralDifferenceHWR, HanningWindow);
}

//=======================================================================
OnsetDetectionFunction::OnsetDetectionFunction(int hopSize_,int frameSize_,int onsetDetectionFunctionType_,int windowType_)
 :  onsetDetectionFunctionType (ComplexSpectralDifferenceHWR), windowType (HanningWindow)
{	
	// indicate that we have not initialised yet
	initialised = false;
	
	// set pi
	pi = 3.14159265358979;	
	
	// initialise with arguments to constructor
	initialise (hopSize_, frameSize_, onsetDetectionFunctionType_, windowType_);
}


//=======================================================================
OnsetDetectionFunction::~OnsetDetectionFunction()
{
    if (initialised)
    {
        freeFFT();
    }
}

//=======================================================================
void OnsetDetectionFunction::initialise (int hopSize_, int frameSize_)
{
    // use the already initialised onset detection function and window type and
    // pass the new frame and hop size to the main initialisation function
    initialise (hopSize_, frameSize_, onsetDetectionFunctionType, windowType);
}

//=======================================================================
void OnsetDetectionFunction::initialise(int hopSize_,int frameSize_,int onsetDetectionFunctionType_,int windowType_)
{
	hopSize = hopSize_; // set hopsize
	frameSize = frameSize_; // set framesize
	
	onsetDetectionFunctionType = onsetDetectionFunctionType_; // set detection function type
    windowType = windowType_; // set window type
		
	// initialise buffers
    frame.resize (frameSize);
    window.resize (frameSize);
    magSpec.resize (frameSize);
    prevMagSpec.resize (frameSize);
    phase.resize (frameSize);
    prevPhase.resize (frameSize);
    prevPhase2.resize (frameSize);
	
	
	// set the window to the specified type
	switch (windowType)
    {
		case RectangularWindow:
			calculateRectangularWindow();		// Rectangular window
			break;	
		case HanningWindow:
			calculateHanningWindow();			// Hanning Window
			break;
		case HammingWindow:
			calclulateHammingWindow();			// Hamming Window
			break;
		case BlackmanWindow:
			calculateBlackmanWindow();			// Blackman Window
			break;
		case TukeyWindow:
			calculateTukeyWindow();             // Tukey Window
			break;
		default:
			calculateHanningWindow();			// DEFAULT: Hanning Window
	}
	
	// initialise previous magnitude spectrum to zero
	for (int i = 0; i < frameSize; i++)
	{
		prevMagSpec[i] = 0.0;
		prevPhase[i] = 0.0;
		prevPhase2[i] = 0.0;
		frame[i] = 0.0;
	}
	
	prevEnergySum = 0.0;	// initialise previous energy sum value to zero
	
    initialiseFFT();
}

//=======================================================================
void OnsetDetectionFunction::initialiseFFT()
{
    if (initialised) // if we have already initialised FFT plan
    {
        freeFFT();
    }
    
#ifdef USE_FFTW
    complexIn = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * frameSize);		// complex array to hold fft data
    complexOut = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * frameSize);	// complex array to hold fft data
    p = fftw_plan_dft_1d (frameSize, complexIn, complexOut, FFTW_FORWARD, FFTW_ESTIMATE);	// FFT plan initialisation
#endif
    
#ifdef USE_KISS_FFT
    complexOut.resize (frameSize);
    
    for (int i = 0; i < frameSize;i++)
    {
        complexOut[i].resize(2);
    }
    
    fftIn = new kiss_fft_cpx[frameSize];
    fftOut = new kiss_fft_cpx[frameSize];
    cfg = kiss_fft_alloc (frameSize, 0, 0, 0);
#endif

    initialised = true;
}

//=======================================================================
void OnsetDetectionFunction::freeFFT()
{
#ifdef USE_FFTW
    fftw_destroy_plan (p);
    fftw_free (complexIn);
    fftw_free (complexOut);
#endif
    
#ifdef USE_KISS_FFT
    free (cfg);
    delete [] fftIn;
    delete [] fftOut;
#endif
}

//=======================================================================
void OnsetDetectionFunction::setOnsetDetectionFunctionType (int onsetDetectionFunctionType_)
{
	onsetDetectionFunctionType = onsetDetectionFunctionType_; // set detection function type
}

//=======================================================================
double OnsetDetectionFunction::calculateOnsetDetectionFunctionSample (double* buffer)
{	
	double odfSample;
		
	// shift audio samples back in frame by hop size
	for (int i = 0; i < (frameSize-hopSize);i++)
	{
		frame[i] = frame[i+hopSize];
	}
	
	// add new samples to frame from input buffer
	int j = 0;
	for (int i = (frameSize-hopSize);i < frameSize;i++)
	{
		frame[i] = buffer[j];
		j++;
	}
		
	switch (onsetDetectionFunctionType)
    {
		case EnergyEnvelope:
        {
            // calculate energy envelope detection function sample
			odfSample = energyEnvelope();
			break;
        }
		case EnergyDifference:
        {
            // calculate half-wave rectified energy difference detection function sample
			odfSample = energyDifference();
			break;
        }
		case SpectralDifference:
        {
            // calculate spectral difference detection function sample
			odfSample = spectralDifference();
			break;
        }
		case SpectralDifferenceHWR:
        {
            // calculate spectral difference detection function sample (half wave rectified)
			odfSample = spectralDifferenceHWR();
			break;
        }
		case PhaseDeviation:
        {
            // calculate phase deviation detection function sample (half wave rectified)
			odfSample = phaseDeviation();
			break;
        }
		case ComplexSpectralDifference:
        {
            // calcualte complex spectral difference detection function sample
			odfSample = complexSpectralDifference();
			break;
        }
		case ComplexSpectralDifferenceHWR:
        {
            // calcualte complex spectral difference detection function sample (half-wave rectified)
			odfSample = complexSpectralDifferenceHWR();
			break;
        }
		case HighFrequencyContent:
        {
            // calculate high frequency content detection function sample
			odfSample = highFrequencyContent();
			break;
        }
		case HighFrequencySpectralDifference:
        {
            // calculate high frequency spectral difference detection function sample
			odfSample = highFrequencySpectralDifference();
			break;
        }
		case HighFrequencySpectralDifferenceHWR:
        {
            // calculate high frequency spectral difference detection function (half-wave rectified)
			odfSample = highFrequencySpectralDifferenceHWR();
			break;
        }
		default:
        {
			odfSample = 1.0;
        }
	}
		
	return odfSample;
}


//=======================================================================
void OnsetDetectionFunction::performFFT()
{
    int fsize2 = (frameSize/2);
    
#ifdef USE_FFTW
	// window frame and copy to complex array, swapping the first and second half of the signal
	for (int i = 0;i < fsize2;i++)
	{
		complexIn[i][0] = frame[i + fsize2] * window[i + fsize2];
		complexIn[i][1] = 0.0;
		complexIn[i+fsize2][0] = frame[i] * window[i];
		complexIn[i+fsize2][1] = 0.0;
	}
	
	// perform the fft
	fftw_execute (p);
#endif
    
#ifdef USE_KISS_FFT
    for (int i = 0; i < fsize2; i++)
    {
        fftIn[i].r = frame[i + fsize2] * window[i + fsize2];
        fftIn[i].i = 0.0;
        fftIn[i + fsize2].r = frame[i] * window[i];
        fftIn[i + fsize2].i = 0.0;
    }
    
    // execute kiss fft
    kiss_fft (cfg, fftIn, fftOut);
    
    // store real and imaginary parts of FFT
    for (int i = 0; i < frameSize; i++)
    {
        complexOut[i][0] = fftOut[i].r;
        complexOut[i][1] = fftOut[i].i;
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Methods for Detection Functions /////////////////////////////////

//=======================================================================
double OnsetDetectionFunction::energyEnvelope()
{
	double sum;
	
	sum = 0;	// initialise sum
	
	// sum the squares of the samples
	for (int i = 0;i < frameSize;i++)
	{
		sum = sum + (frame[i] * frame[i]);
	}
	
	return sum;		// return sum
}

//=======================================================================
double OnsetDetectionFunction::energyDifference()
{
	double sum;
	double sample;
	
	sum = 0;	// initialise sum
	
	// sum the squares of the samples
	for (int i = 0; i < frameSize; i++)
	{
		sum = sum + (frame[i] * frame[i]);
	}
	
	sample = sum - prevEnergySum;	// sample is first order difference in energy
	
	prevEnergySum = sum;	// store energy value for next calculation
	
	if (sample > 0)
	{
		return sample;		// return difference
	}
	else
	{
		return 0;
	}
}

//=======================================================================
double OnsetDetectionFunction::spectralDifference()
{
	double diff;
	double sum;
	
	// perform the FFT
	performFFT();
	
	// compute first (N/2)+1 mag values
	for (int i = 0;i < (frameSize/2)+1;i++)
	{
		magSpec[i] = sqrt (pow (complexOut[i][0], 2) + pow (complexOut[i][1], 2));
	}
	// mag spec symmetric above (N/2)+1 so copy previous values
	for (int i = (frameSize/2)+1; i < frameSize; i++)
	{
		magSpec[i] = magSpec[frameSize-i];
	}
	
	sum = 0;	// initialise sum to zero

	for (int i = 0; i < frameSize; i++)
	{
		// calculate difference
		diff = magSpec[i] - prevMagSpec[i];
		
		// ensure all difference values are positive
		if (diff < 0)
		{
			diff = diff*-1;
		}
		
		// add difference to sum
		sum = sum + diff;
		
		// store magnitude spectrum bin for next detection function sample calculation
		prevMagSpec[i] = magSpec[i];
	}
	
	return sum;		
}

//=======================================================================
double OnsetDetectionFunction::spectralDifferenceHWR()
{
	double diff;
	double sum;
	
	// perform the FFT
	performFFT();
	
	// compute first (N/2)+1 mag values
	for (int i = 0;i < (frameSize/2) + 1; i++)
	{
		magSpec[i] = sqrt (pow (complexOut[i][0],2) + pow (complexOut[i][1],2));
	}
	// mag spec symmetric above (N/2)+1 so copy previous values
	for (int i = (frameSize/2)+1;i < frameSize;i++)
	{
		magSpec[i] = magSpec[frameSize-i];
	}
	
	sum = 0;	// initialise sum to zero
	
	for (int i = 0;i < frameSize;i++)
	{
		// calculate difference
		diff = magSpec[i] - prevMagSpec[i];
		
		// only add up positive differences
		if (diff > 0)
		{
			// add difference to sum
			sum = sum+diff;
		}
		
		// store magnitude spectrum bin for next detection function sample calculation
		prevMagSpec[i] = magSpec[i];
	}
	
	return sum;		
}


//=======================================================================
double OnsetDetectionFunction::phaseDeviation()
{
	double dev,pdev;
	double sum;
	
	// perform the FFT
	performFFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < frameSize;i++)
	{
		// calculate phase value
		phase[i] = atan2 (complexOut[i][1], complexOut[i][0]);
		
		// calculate magnitude value
		magSpec[i] = sqrt (pow (complexOut[i][0],2) + pow (complexOut[i][1],2));
		
		
		// if bin is not just a low energy bin then examine phase deviation
		if (magSpec[i] > 0.1)
		{
			dev = phase[i] - (2*prevPhase[i]) + prevPhase2[i];	// phase deviation
			pdev = princarg (dev);	// wrap into [-pi,pi] range
		
			// make all values positive
			if (pdev < 0)	
			{
				pdev = pdev*-1;
			}
						
			// add to sum
			sum = sum + pdev;
		}
				
		// store values for next calculation
		prevPhase2[i] = prevPhase[i];
		prevPhase[i] = phase[i];
	}
	
	return sum;		
}

//=======================================================================
double OnsetDetectionFunction::complexSpectralDifference()
{
	double phaseDeviation;
	double sum;
	double csd;
	
	// perform the FFT
	performFFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < frameSize;i++)
	{
		// calculate phase value
		phase[i] = atan2 (complexOut[i][1], complexOut[i][0]);
		
		// calculate magnitude value
		magSpec[i] = sqrt (pow (complexOut[i][0],2) + pow(complexOut[i][1],2));
		
		// phase deviation
		phaseDeviation = phase[i] - (2 * prevPhase[i]) + prevPhase2[i];
		
        // calculate complex spectral difference for the current spectral bin
		csd = sqrt (pow (magSpec[i], 2) + pow (prevMagSpec[i], 2) - 2 * magSpec[i] * prevMagSpec[i] * cos (phaseDeviation));
			
		// add to sum
		sum = sum + csd;
		
		// store values for next calculation
		prevPhase2[i] = prevPhase[i];
		prevPhase[i] = phase[i];
		prevMagSpec[i] = magSpec[i];
	}
	
	return sum;		
}

//=======================================================================
double OnsetDetectionFunction::complexSpectralDifferenceHWR()
{
	double phaseDeviation;
	double sum;
	double magnitudeDifference;
	double csd;
	
	// perform the FFT
	performFFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < frameSize;i++)
	{
		// calculate phase value
		phase[i] = atan2 (complexOut[i][1], complexOut[i][0]);
		
		// calculate magnitude value
		magSpec[i] = sqrt (pow (complexOut[i][0],2) + pow(complexOut[i][1],2));
		
        // phase deviation
        phaseDeviation = phase[i] - (2 * prevPhase[i]) + prevPhase2[i];
        
        // calculate magnitude difference (real part of Euclidean distance between complex frames)
        magnitudeDifference = magSpec[i] - prevMagSpec[i];
        
        // if we have a positive change in magnitude, then include in sum, otherwise ignore (half-wave rectification)
        if (magnitudeDifference > 0)
        {
            // calculate complex spectral difference for the current spectral bin
            csd = sqrt (pow (magSpec[i], 2) + pow (prevMagSpec[i], 2) - 2 * magSpec[i] * prevMagSpec[i] * cos (phaseDeviation));
        
            // add to sum
            sum = sum + csd;
        }
        
		// store values for next calculation
		prevPhase2[i] = prevPhase[i];
		prevPhase[i] = phase[i];
		prevMagSpec[i] = magSpec[i];
	}
	
	return sum;		
}


//=======================================================================
double OnsetDetectionFunction::highFrequencyContent()
{
	double sum;
	
	// perform the FFT
	performFFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0; i < frameSize; i++)
	{		
		// calculate magnitude value
		magSpec[i] = sqrt (pow (complexOut[i][0],2) + pow (complexOut[i][1],2));
		
		
		sum = sum + (magSpec[i] * ((double) (i+1)));
		
		// store values for next calculation
		prevMagSpec[i] = magSpec[i];
	}
	
	return sum;		
}

//=======================================================================
double OnsetDetectionFunction::highFrequencySpectralDifference()
{
	double sum;
	double mag_diff;
	
	// perform the FFT
	performFFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < frameSize;i++)
	{		
		// calculate magnitude value
		magSpec[i] = sqrt (pow (complexOut[i][0],2) + pow (complexOut[i][1],2));
		
		// calculate difference
		mag_diff = magSpec[i] - prevMagSpec[i];
		
		if (mag_diff < 0)
		{
			mag_diff = -mag_diff;
		}
		
		sum = sum + (mag_diff * ((double) (i+1)));
		
		// store values for next calculation
		prevMagSpec[i] = magSpec[i];
	}
	
	return sum;		
}

//=======================================================================
double OnsetDetectionFunction::highFrequencySpectralDifferenceHWR()
{
	double sum;
	double mag_diff;
	
	// perform the FFT
	performFFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < frameSize;i++)
	{		
		// calculate magnitude value
		magSpec[i] = sqrt (pow (complexOut[i][0],2) + pow (complexOut[i][1],2));
		
		// calculate difference
		mag_diff = magSpec[i] - prevMagSpec[i];
		
		if (mag_diff > 0)
		{
			sum = sum + (mag_diff * ((double) (i+1)));
		}

		// store values for next calculation
		prevMagSpec[i] = magSpec[i];
	}
	
	return sum;		
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Methods to Calculate Windows ////////////////////////////////////

//=======================================================================
void OnsetDetectionFunction::calculateHanningWindow()
{
	double N;		// variable to store framesize minus 1
	
	N = (double) (frameSize-1);	// framesize minus 1
	
	// Hanning window calculation
	for (int n = 0; n < frameSize; n++)
	{
		window[n] = 0.5 * (1 - cos (2 * pi * (n / N)));
	}
}

//=======================================================================
void OnsetDetectionFunction::calclulateHammingWindow()
{
	double N;		// variable to store framesize minus 1
	double n_val;	// double version of index 'n'
	
	N = (double) (frameSize-1);	// framesize minus 1
	n_val = 0;
	
	// Hamming window calculation
	for (int n = 0;n < frameSize;n++)
	{
		window[n] = 0.54 - (0.46 * cos (2 * pi * (n_val/N)));
		n_val = n_val+1;
	}
}

//=======================================================================
void OnsetDetectionFunction::calculateBlackmanWindow()
{
	double N;		// variable to store framesize minus 1
	double n_val;	// double version of index 'n'
	
	N = (double) (frameSize-1);	// framesize minus 1
	n_val = 0;
	
	// Blackman window calculation
	for (int n = 0;n < frameSize;n++)
	{
		window[n] = 0.42 - (0.5*cos(2*pi*(n_val/N))) + (0.08*cos(4*pi*(n_val/N)));
		n_val = n_val+1;
	}
}

//=======================================================================
void OnsetDetectionFunction::calculateTukeyWindow()
{
	double N;		// variable to store framesize minus 1
	double n_val;	// double version of index 'n'
	double alpha;	// alpha [default value = 0.5];
	
	alpha = 0.5;
	
	N = (double) (frameSize-1);	// framesize minus 1
		
	// Tukey window calculation
	
	n_val = (double) (-1*((frameSize/2)))+1;

	for (int n = 0;n < frameSize;n++)	// left taper
	{
		if ((n_val >= 0) && (n_val <= (alpha*(N/2))))
		{
			window[n] = 1.0;
		}
		else if ((n_val <= 0) && (n_val >= (-1*alpha*(N/2))))
		{
			window[n] = 1.0;
		}
		else
		{
			window[n] = 0.5*(1+cos(pi*(((2*n_val)/(alpha*N))-1)));
		}

		n_val = n_val+1;			 
	}

}

//=======================================================================
void OnsetDetectionFunction::calculateRectangularWindow()
{
	// Rectangular window calculation
	for (int n = 0;n < frameSize;n++)
	{
		window[n] = 1.0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Other Handy Methods //////////////////////////////////////////

//=======================================================================
double OnsetDetectionFunction::princarg(double phaseVal)
{	
	// if phase value is less than or equal to -pi then add 2*pi
	while (phaseVal <= (-pi))
	{
		phaseVal = phaseVal + (2 * pi);
	}
	
	// if phase value is larger than pi, then subtract 2*pi
	while (phaseVal > pi)
	{
		phaseVal = phaseVal - (2 * pi);
	}
			
	return phaseVal;
}
