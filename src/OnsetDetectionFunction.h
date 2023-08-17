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

#ifdef USE_FFTW
#include "fftw3.h"
#endif

#ifdef USE_KISS_FFT
#include "kiss_fft.h"
#endif

#include <vector>

//=======================================================================
/** The type of onset detection function to calculate */
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
/** The type of window to use when calculating onset detection function samples */
enum WindowType
{
    RectangularWindow,
    HanningWindow,
    HammingWindow,
    BlackmanWindow,
    TukeyWindow
};

//=======================================================================
/** A class for calculating onset detection functions. */
class OnsetDetectionFunction
{
public:
    
    /** Constructor that defaults the onset detection function type to ComplexSpectralDifferenceHWR
     * and the window type to HanningWindow
     * @param hopSize_ the hop size in audio samples
     * @param frameSize_ the frame size in audio samples
     */
	OnsetDetectionFunction (int hopSize, int frameSize);
    
    
    /** Constructor 
     * @param hopSize_ the hop size in audio samples
     * @param frameSize_ the frame size in audio samples
     * @param onsetDetectionFunctionType_ the type of onset detection function to use - (see OnsetDetectionFunctionType)
     * @param windowType the type of window to use (see WindowType)
     */
	OnsetDetectionFunction (int hopSize, int frameSize, int onsetDetectionFunctionType, int windowType);
    
    /** Destructor */
	~OnsetDetectionFunction();
    
    /** Initialisation function for only updating hop size and frame size (and not window type 
     * or onset detection function type
     * @param hopSize_ the hop size in audio samples
     * @param frameSize_ the frame size in audio samples
     */
	void initialise (int hopSize, int frameSize);
    
    /** Initialisation Function 
     * @param hopSize_ the hop size in audio samples
     * @param frameSize_ the frame size in audio samples
     * @param onsetDetectionFunctionType_ the type of onset detection function to use - (see OnsetDetectionFunctionType)
     * @param windowType the type of window to use (see WindowType)
     */
	void initialise (int hopSize, int frameSize, int onsetDetectionFunctionType, int windowType);
	
    /** Process input frame and calculate detection function sample 
     * @param buffer a pointer to an array containing the audio samples to be processed
     * @returns the onset detection function sample
     */
	double calculateOnsetDetectionFunctionSample (double* buffer);
    
    /** Set the detection function type 
     * @param onsetDetectionFunctionType_ the type of onset detection function to use - (see OnsetDetectionFunctionType)
     */
	void setOnsetDetectionFunctionType (int onsetDetectionFunctionType);
	
private:
	
    /** Perform the FFT on the data in 'frame' */
	void performFFT();

    //=======================================================================
    /** Calculate energy envelope detection function sample */
	double energyEnvelope();
    
    /** Calculate energy difference detection function sample */
	double energyDifference();
    
    /** Calculate spectral difference detection function sample */
	double spectralDifference();
    
    /** Calculate spectral difference (half wave rectified) detection function sample */
	double spectralDifferenceHWR();
    
    /** Calculate phase deviation detection function sample */
	double phaseDeviation();
    
    /** Calculate complex spectral difference detection function sample */
	double complexSpectralDifference();
    
    /** Calculate complex spectral difference detection function sample (half-wave rectified) */
	double complexSpectralDifferenceHWR();
    
    /** Calculate high frequency content detection function sample */
	double highFrequencyContent();
    
    /** Calculate high frequency spectral difference detection function sample */
	double highFrequencySpectralDifference();
    
    /** Calculate high frequency spectral difference detection function sample (half-wave rectified) */
	double highFrequencySpectralDifferenceHWR();

    //=======================================================================
    /** Calculate a Rectangular window */
	void calculateRectangularWindow();
    
    /** Calculate a Hanning window */
	void calculateHanningWindow();
    
    /** Calculate a Hamming window */
	void calclulateHammingWindow();
    
    /** Calculate a Blackman window */
	void calculateBlackmanWindow();
    
    /** Calculate a Tukey window */
	void calculateTukeyWindow();

    //=======================================================================
	/** Set phase values between [-pi, pi] 
     * @param phaseVal the phase value to process
     * @returns the wrapped phase value
     */
	double princarg(double phaseVal);
	
    void initialiseFFT();
    void freeFFT();
	
	double pi;							/**< pi, the constant */
	
	int frameSize;						/**< audio framesize */
	int hopSize;						/**< audio hopsize */
	int onsetDetectionFunctionType;		/**< type of detection function */
    int windowType;                     /**< type of window used in calculations */

    //=======================================================================
#ifdef USE_FFTW
	fftw_plan p;						/**< fftw plan */
	fftw_complex* complexIn;			/**< to hold complex fft values for input */
	fftw_complex* complexOut;			/**< to hold complex fft values for output */
#endif
    
#ifdef USE_KISS_FFT
    kiss_fft_cfg cfg;                   /**< Kiss FFT configuration */
    kiss_fft_cpx* fftIn;                /**< FFT input samples, in complex form */
    kiss_fft_cpx* fftOut;               /**< FFT output samples, in complex form */
    std::vector<std::vector<double> > complexOut;
#endif
	
    //=======================================================================
	bool initialised;					/**< flag indicating whether buffers and FFT plans are initialised */

    std::vector<double> frame;          /**< audio frame */
    std::vector<double> window;         /**< window */
	
	double prevEnergySum;				/**< to hold the previous energy sum value */
	
    std::vector<double> magSpec;        /**< magnitude spectrum */
    std::vector<double> prevMagSpec;    /**< previous magnitude spectrum */
	
    std::vector<double> phase;          /**< FFT phase values */
    std::vector<double> prevPhase;      /**< previous phase values */
    std::vector<double> prevPhase2;     /**< second order previous phase values */
};


#endif
