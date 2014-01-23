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

#ifndef __RTONSETDF_H
#define __RTONSETDF_H

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
	OnsetDetectionFunction(int arg_hsize,int arg_fsize,int arg_df_type,int arg_win_type);
    
    /** Destructor */
	~OnsetDetectionFunction();
    
    /** Initialisation Function */
	void initialise(int arg_hsize,int arg_fsize,int arg_df_type,int arg_win_type);
	
    /** process input buffer and calculate detection function sample */
	double getDFsample(double *inputbuffer);
    
    /** set the detection function type */
	void set_df_type(int arg_df_type);
	
private:
	
    /** perform the FFT on the data in 'frame' */
	void perform_FFT();

    /** calculate energy envelope detection function sample */
	double energy_envelope();
    
    /** calculate energy difference detection function sample */
	double energy_difference();
    
    /** calculate spectral difference detection function sample */
	double spectral_difference();
    
    /** calculate spectral difference (half wave rectified) detection function sample */
	double spectral_difference_hwr();
    
    /** calculate phase deviation detection function sample */
	double phase_deviation();
    
    /** calculate complex spectral difference detection function sample */
	double complex_spectral_difference();
    
    /** calculate complex spectral difference detection function sample (half-wave rectified) */
	double complex_spectral_difference_hwr();
    
    /** calculate high frequency content detection function sample */
	double high_frequency_content();
    
    /** calculate high frequency spectral difference detection function sample */
	double high_frequency_spectral_difference();
    
    /** calculate high frequency spectral difference detection function sample (half-wave rectified) */
	double high_frequency_spectral_difference_hwr();

    /** calculate a Rectangular window */
	void set_win_rectangular();
    
    /** calculate a Hanning window */
	void set_win_hanning();
    
    /** calculate a Hamming window */
	void set_win_hamming();
    
    /** calculate a Blackman window */
	void set_win_blackman();
    
    /** calculate a Tukey window */
	void set_win_tukey();

	/** set phase values between [-pi, pi] */
	double princarg(double phaseval);
	
	
	double pi;							/**< pi, the constant */
	
	int framesize;						/**< audio framesize */
	int hopsize;						/**< audio hopsize */
	int df_type;						/**< type of detection function */
	
	fftw_plan p;						/**< create fft plan */
	fftw_complex *in;					/**< to hold complex fft values for input */
	fftw_complex *out;					/**< to hold complex fft values for output */
	
	int initialised;					/**< flag indicating whether buffers and FFT plans are initialised */

	double *frame;						/**< audio frame */
	double *window;						/**< window */
	double *wframe;						/**< windowed frame */
	
	double energy_sum_old;				/**< to hold the previous energy sum value */
	
	double *mag;						/**< magnitude spectrum */
	double *mag_old;					/**< previous magnitude spectrum */
	
	double *phase;						/**< FFT phase values */
	double *phase_old;					/**< previous phase values */
	double *phase_old_2;				/**< second order previous phase values */

};


#endif