//=======================================================================
/** @file OnsetDetectionFunction.h
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

#ifndef __ONSETDETECTIONFUNCTION_H
#define __ONSETDETECTIONFUNCTION_H

#include "fftw3.h"

//=======================================================================
enum OnsetDetectionFunctionType
{
    EnergyEnvelope,
    EnergyDifference,
    SpectralDifference,
    SpectralDifferenceHWR,
    PhaseDeviation,
    ComplexSpectralDifference,
    ComplexSpectralDifferenceHWR,
    HighFrequencyContent,
    HighFrequencySpectralDifference,
    HighFrequencySpectralDifferenceHWR
};

//=======================================================================
enum WindowType
{
    RectangularWindow,
    HanningWindow,
    HammingWindow,
    BlackmanWindow,
    TukeyWindow
};

class OnsetDetectionFunction
{
public:
    
    /** Constructor */
	OnsetDetectionFunction(int hopSize_,int frameSize_,int onsetDetectionFunctionType_,int windowType);
    
    /** Destructor */
	~OnsetDetectionFunction();
    
    /** Initialisation Function */
	void initialise(int hopSize_,int frameSize_,int onsetDetectionFunctionType_,int windowType);
	
    /** process input frame and calculate detection function sample */
	double calculateOnsetDetectionFunctionSample(double *buffer);
    
    /** set the detection function type */
	void setOnsetDetectionFunctionType(int onsetDetectionFunctionType_);
	
private:
	
    /** perform the FFT on the data in 'frame' */
	void performFFT();

    //=======================================================================
    /** calculate energy envelope detection function sample */
	double energyEnvelope();
    
    /** calculate energy difference detection function sample */
	double energyDifference();
    
    /** calculate spectral difference detection function sample */
	double spectralDifference();
    
    /** calculate spectral difference (half wave rectified) detection function sample */
	double spectralDifferenceHWR();
    
    /** calculate phase deviation detection function sample */
	double phaseDeviation();
    
    /** calculate complex spectral difference detection function sample */
	double complexSpectralDifference();
    
    /** calculate complex spectral difference detection function sample (half-wave rectified) */
	double complexSpectralDifferenceHWR();
    
    /** calculate high frequency content detection function sample */
	double highFrequencyContent();
    
    /** calculate high frequency spectral difference detection function sample */
	double highFrequencySpectralDifference();
    
    /** calculate high frequency spectral difference detection function sample (half-wave rectified) */
	double highFrequencySpectralDifferenceHWR();

    //=======================================================================
    /** calculate a Rectangular window */
	void calculateRectangularWindow();
    
    /** calculate a Hanning window */
	void calculateHanningWindow();
    
    /** calculate a Hamming window */
	void calclulateHammingWindow();
    
    /** calculate a Blackman window */
	void calculateBlackmanWindow();
    
    /** calculate a Tukey window */
	void calculateTukeyWindow();

    //=======================================================================
	/** set phase values between [-pi, pi] */
	double princarg(double phaseVal);
	
	
	double pi;							/**< pi, the constant */
	
	int frameSize;						/**< audio framesize */
	int hopSize;						/**< audio hopsize */
	int onsetDetectionFunctionType;		/**< type of detection function */
	
	fftw_plan p;						/**< fftw plan */
	fftw_complex *complexIn;			/**< to hold complex fft values for input */
	fftw_complex *complexOut;			/**< to hold complex fft values for output */
	
	int initialised;					/**< flag indicating whether buffers and FFT plans are initialised */

	double *frame;						/**< audio frame */
	double *window;						/**< window */
	
	double prevEnergySum;				/**< to hold the previous energy sum value */
	
	double *magSpec;					/**< magnitude spectrum */
	double *prevMagSpec;                /**< previous magnitude spectrum */
	
	double *phase;						/**< FFT phase values */
	double *prevPhase;					/**< previous phase values */
	double *prevPhase2;                 /**< second order previous phase values */

};


#endif