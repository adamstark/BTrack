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
#include <iostream>
#include "OnsetDetectionFunction.h"
using namespace std;

//-------------------------------------------------------------------------------
// Constructor
OnsetDetectionFunction :: OnsetDetectionFunction()
{	
	// indicate that we have not initialised yet
	initialised = 0;		
	
	// set pi
	pi = 3.14159265358979;	
	
	// initialise with hopsize = 512, framesize = 1024, complex spectral difference DF and hanning window
	initialise(512,1024,6,1);	
}


//-------------------------------------------------------------------------------
// Constructor (with arguments)
OnsetDetectionFunction :: OnsetDetectionFunction(int arg_hsize,int arg_fsize,int arg_df_type,int arg_win_type)
{	
	// indicate that we have not initialised yet
	initialised = 0;		
	
	// set pi
	pi = 3.14159265358979;	
	
	// initialise with arguments to constructor
	initialise(arg_hsize,arg_fsize,arg_df_type,arg_win_type);
}


//--------------------------------------------------------------------------------------
// Destructor
OnsetDetectionFunction :: ~OnsetDetectionFunction()
{
	// destroy fft plan
    fftw_destroy_plan(p);
	fftw_free(in); 
	fftw_free(out);
	
	// deallocate memory
	delete [] frame;
	frame = NULL;	
	delete [] window;
	window = NULL;									
	delete [] wframe;
	wframe = NULL;											
	delete [] mag;
	mag = NULL;
	delete [] mag_old;
	mag_old = NULL;
	delete [] phase;
	phase = NULL;
	delete [] phase_old;
	phase_old = NULL;	
	delete [] phase_old_2;
	phase_old_2 = NULL;
}

//-------------------------------------------------------------------------------
// Initialisation
void OnsetDetectionFunction :: initialise(int arg_hsize,int arg_fsize,int arg_df_type,int arg_win_type)
{
	if (initialised == 1) // if we have already initialised some buffers and an FFT plan
	{
		//////////////////////////////////
		// TIDY UP FIRST - If initialise is called after the class has been initialised
		// then we want to free up memory and cancel existing FFT plans
	
		// destroy fft plan
		fftw_destroy_plan(p);
		fftw_free(in); 
		fftw_free(out);
	
	
		// deallocate memory
		delete [] frame;
		frame = NULL;	
		delete [] window;
		window = NULL;									
		delete [] wframe;
		wframe = NULL;											
		delete [] mag;
		mag = NULL;
		delete [] mag_old;
		mag_old = NULL;
		delete [] phase;
		phase = NULL;
		delete [] phase_old;
		phase_old = NULL;	
		delete [] phase_old_2;
		phase_old_2 = NULL;
	
		////// END TIDY UP ///////////////
		//////////////////////////////////
	}
	
	hopsize = arg_hsize; // set hopsize
	framesize = arg_fsize; // set framesize
	
	df_type = arg_df_type; // set detection function type
		
	// initialise buffers
	frame = new double[framesize];											
	window = new double[framesize];	
	wframe = new double[framesize];		
	
	mag = new double[framesize];											
	mag_old = new double[framesize];
	
	phase = new double[framesize];
	phase_old = new double[framesize];
	phase_old_2 = new double[framesize];
	
	
	// set the window to the specified type
	switch (arg_win_type){
		case 0:
			set_win_rectangular();		// Rectangular window
			break;	
		case 1:	
			set_win_hanning();			// Hanning Window
			break;
		case 2:
			set_win_hamming();			// Hamming Window
			break;
		case 3:
			set_win_blackman();			// Blackman Window
			break;
		case 4:
			set_win_tukey();			// Tukey Window
			break;
		default:
			set_win_hanning();			// DEFAULT: Hanning Window
	}
	
	
	
	
	// initialise previous magnitude spectrum to zero
	for (int i = 0;i < framesize;i++)
	{
		mag_old[i] = 0.0;
		phase_old[i] = 0.0;
		phase_old_2[i] = 0.0;
		frame[i] = 0.0;
	}
	
	energy_sum_old = 0.0;	// initialise previous energy sum value to zero
	
	/*  Init fft */
	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * framesize);		// complex array to hold fft data
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * framesize);	// complex array to hold fft data
	p = fftw_plan_dft_1d(framesize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);	// FFT plan initialisation
	
	initialised = 1;
}

//--------------------------------------------------------------------------------------
// set the detection function type to that specified by the argument
void OnsetDetectionFunction :: set_df_type(int arg_df_type)
{
	df_type = arg_df_type; // set detection function type
}


//--------------------------------------------------------------------------------------
// calculates a single detection function sample from a single audio frame.
double OnsetDetectionFunction :: getDFsample(double inputbuffer[])
{	
	double df_sample;
		
	// shift audio samples back in frame by hop size
	for (int i = 0; i < (framesize-hopsize);i++)
	{
		frame[i] = frame[i+hopsize];
	}
	
	// add new samples to frame from input buffer
	int j = 0;
	for (int i = (framesize-hopsize);i < framesize;i++)
	{
		frame[i] = inputbuffer[j];
		j++;
	}
		
	switch (df_type){
		case 0:
			df_sample = energy_envelope();	// calculate energy envelope detection function sample
			break;	
		case 1:
			df_sample = energy_difference();	// calculate half-wave rectified energy difference detection function sample
			break;
		case 2:
			df_sample = spectral_difference();	// calculate spectral difference detection function sample
			break;
		case 3:
			df_sample = spectral_difference_hwr(); // calculate spectral difference detection function sample (half wave rectified)
			break;
		case 4:
			df_sample = phase_deviation();		// calculate phase deviation detection function sample (half wave rectified)
			break;
		case 5:
			df_sample = complex_spectral_difference(); // calcualte complex spectral difference detection function sample
			break;
		case 6:
			df_sample = complex_spectral_difference_hwr();  // calcualte complex spectral difference detection function sample (half-wave rectified)
			break;
		case 7:
			df_sample = high_frequency_content(); // calculate high frequency content detection function sample
			break;
		case 8:
			df_sample = high_frequency_spectral_difference(); // calculate high frequency spectral difference detection function sample
			break;
		case 9:
			df_sample = high_frequency_spectral_difference_hwr(); // calculate high frequency spectral difference detection function (half-wave rectified)
			break;
		default:
			df_sample = 1.0;
	}
		
	return df_sample;
}


//--------------------------------------------------------------------------------------
// performs the fft, storing the complex result in 'out'
void OnsetDetectionFunction :: perform_FFT()
{
	int fsize2 = (framesize/2);
	
	// window frame and copy to complex array, swapping the first and second half of the signal
	for (int i = 0;i < fsize2;i++)
	{
		in[i][0] = frame[i+fsize2] * window[i+fsize2];
		in[i][1] = 0.0;
		in[i+fsize2][0] = frame[i] * window[i];
		in[i+fsize2][1] = 0.0;
	}
	
	// perform the fft
	fftw_execute(p);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Methods for Detection Functions /////////////////////////////////

//--------------------------------------------------------------------------------------
// calculates an energy envelope detection function sample
double OnsetDetectionFunction :: energy_envelope()
{
	double sum;
	
	sum = 0;	// initialise sum
	
	// sum the squares of the samples
	for (int i = 0;i < framesize;i++)
	{
		sum = sum + (frame[i]*frame[i]);
	}
	
	return sum;		// return sum
}

//--------------------------------------------------------------------------------------
// calculates a half-wave rectified energy difference detection function sample
double OnsetDetectionFunction :: energy_difference()
{
	double sum;
	double sample;
	
	sum = 0;	// initialise sum
	
	// sum the squares of the samples
	for (int i = 0;i < framesize;i++)
	{
		sum = sum + (frame[i]*frame[i]);
	}
	
	sample = sum - energy_sum_old;	// sample is first order difference in energy
	
	energy_sum_old = sum;	// store energy value for next calculation
	
	if (sample > 0)
	{
		return sample;		// return difference
	}
	else
	{
		return 0;
	}
}

//--------------------------------------------------------------------------------------
// calculates a spectral difference detection function sample
double OnsetDetectionFunction :: spectral_difference()
{
	double diff;
	double sum;
	
	// perform the FFT
	perform_FFT();
	
	// compute first (N/2)+1 mag values
	for (int i = 0;i < (framesize/2)+1;i++)
	{
		mag[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
	}
	// mag spec symmetric above (N/2)+1 so copy previous values
	for (int i = (framesize/2)+1;i < framesize;i++)
	{
		mag[i] = mag[framesize-i];		
	}
	
	sum = 0;	// initialise sum to zero

	for (int i = 0;i < framesize;i++)
	{
		// calculate difference
		diff = mag[i] - mag_old[i];
		
		// ensure all difference values are positive
		if (diff < 0)
		{
			diff = diff*-1;
		}
		
		// add difference to sum
		sum = sum+diff;
		
		// store magnitude spectrum bin for next detection function sample calculation
		mag_old[i] = mag[i];
	}
	
	return sum;		
}

//--------------------------------------------------------------------------------------
// calculates a spectral difference detection function sample
double OnsetDetectionFunction :: spectral_difference_hwr()
{
	double diff;
	double sum;
	
	// perform the FFT
	perform_FFT();
	
	// compute first (N/2)+1 mag values
	for (int i = 0;i < (framesize/2)+1;i++)
	{
		mag[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
	}
	// mag spec symmetric above (N/2)+1 so copy previous values
	for (int i = (framesize/2)+1;i < framesize;i++)
	{
		mag[i] = mag[framesize-i];		
	}
	
	sum = 0;	// initialise sum to zero
	
	for (int i = 0;i < framesize;i++)
	{
		// calculate difference
		diff = mag[i] - mag_old[i];
		
		// only add up positive differences
		if (diff > 0)
		{
			// add difference to sum
			sum = sum+diff;
		}
		
		
		
		// store magnitude spectrum bin for next detection function sample calculation
		mag_old[i] = mag[i];
	}
	
	return sum;		
}


//--------------------------------------------------------------------------------------
// calculates a phase deviation detection function sample
double OnsetDetectionFunction :: phase_deviation()
{
	double dev,pdev;
	double sum;
	
	// perform the FFT
	perform_FFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < framesize;i++)
	{
		// calculate phase value
		phase[i] = atan2(out[i][1],out[i][0]);
		
		// calculate magnitude value
		mag[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
		
		
		// if bin is not just a low energy bin then examine phase deviation
		if (mag[i] > 0.1)
		{
			dev = phase[i] - (2*phase_old[i]) + phase_old_2[i];	// phase deviation
			pdev = princarg(dev);	// wrap into [-pi,pi] range
		
			// make all values positive
			if (pdev < 0)	
			{
				pdev = pdev*-1;
			}
						
			// add to sum
			sum = sum + pdev;
		}
				
		// store values for next calculation
		phase_old_2[i] = phase_old[i];
		phase_old[i] = phase[i];
	}
	
	return sum;		
}

//--------------------------------------------------------------------------------------
// calculates a complex spectral difference detection function sample
double OnsetDetectionFunction :: complex_spectral_difference()
{
	double dev,pdev;
	double sum;
	double mag_diff,phase_diff;
	double value;
	
	// perform the FFT
	perform_FFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < framesize;i++)
	{
		// calculate phase value
		phase[i] = atan2(out[i][1],out[i][0]);
		
		// calculate magnitude value
		mag[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
		
		
		// phase deviation
		dev = phase[i] - (2*phase_old[i]) + phase_old_2[i];	
		
		// wrap into [-pi,pi] range
		pdev = princarg(dev);	
		
		
		// calculate magnitude difference (real part of Euclidean distance between complex frames)
		mag_diff = mag[i] - mag_old[i];
		
		// calculate phase difference (imaginary part of Euclidean distance between complex frames)
		phase_diff = -mag[i]*sin(pdev);
		

		
		// square real and imaginary parts, sum and take square root
		value = sqrt(pow(mag_diff,2) + pow(phase_diff,2));
	
			
		// add to sum
		sum = sum + value;
		
		
		// store values for next calculation
		phase_old_2[i] = phase_old[i];
		phase_old[i] = phase[i];
		mag_old[i] = mag[i];
	}
	
	return sum;		
}

//--------------------------------------------------------------------------------------
// calculates a complex spectral difference detection function sample (half-wave rectified)
double OnsetDetectionFunction :: complex_spectral_difference_hwr()
{
	double dev,pdev;
	double sum;
	double mag_diff,phase_diff;
	double value;
	
	// perform the FFT
	perform_FFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < framesize;i++)
	{
		// calculate phase value
		phase[i] = atan2(out[i][1],out[i][0]);
		
		// calculate magnitude value
		mag[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
		
		
		// phase deviation
		dev = phase[i] - (2*phase_old[i]) + phase_old_2[i];	
		
		// wrap into [-pi,pi] range
		pdev = princarg(dev);	
		
		
		// calculate magnitude difference (real part of Euclidean distance between complex frames)
		mag_diff = mag[i] - mag_old[i];
		
		// if we have a positive change in magnitude, then include in sum, otherwise ignore (half-wave rectification)
		if (mag_diff > 0)
		{
			// calculate phase difference (imaginary part of Euclidean distance between complex frames)
			phase_diff = -mag[i]*sin(pdev);

			// square real and imaginary parts, sum and take square root
			value = sqrt(pow(mag_diff,2) + pow(phase_diff,2));
		
			// add to sum
			sum = sum + value;
		}
		
		// store values for next calculation
		phase_old_2[i] = phase_old[i];
		phase_old[i] = phase[i];
		mag_old[i] = mag[i];
	}
	
	return sum;		
}


//--------------------------------------------------------------------------------------
// calculates a high frequency content detection function sample
double OnsetDetectionFunction :: high_frequency_content()
{
	double sum;
	double mag_diff;
	
	// perform the FFT
	perform_FFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < framesize;i++)
	{		
		// calculate magnitude value
		mag[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
		
		
		sum = sum + (mag[i]*((double) (i+1)));
		
		// store values for next calculation
		mag_old[i] = mag[i];
	}
	
	return sum;		
}

//--------------------------------------------------------------------------------------
// calculates a high frequency spectral difference detection function sample
double OnsetDetectionFunction :: high_frequency_spectral_difference()
{
	double sum;
	double mag_diff;
	
	// perform the FFT
	perform_FFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < framesize;i++)
	{		
		// calculate magnitude value
		mag[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
		
		// calculate difference
		mag_diff = mag[i] - mag_old[i];
		
		if (mag_diff < 0)
		{
			mag_diff = -mag_diff;
		}
		
		sum = sum + (mag_diff*((double) (i+1)));
		
		// store values for next calculation
		mag_old[i] = mag[i];
	}
	
	return sum;		
}

//--------------------------------------------------------------------------------------
// calculates a high frequency spectral difference detection function sample (half-wave rectified)
double OnsetDetectionFunction :: high_frequency_spectral_difference_hwr()
{
	double sum;
	double mag_diff;
	
	// perform the FFT
	perform_FFT();
	
	sum = 0; // initialise sum to zero
	
	// compute phase values from fft output and sum deviations
	for (int i = 0;i < framesize;i++)
	{		
		// calculate magnitude value
		mag[i] = sqrt(pow(out[i][0],2) + pow(out[i][1],2));
		
		// calculate difference
		mag_diff = mag[i] - mag_old[i];
		
		if (mag_diff > 0)
		{
			sum = sum + (mag_diff*((double) (i+1)));
		}

		// store values for next calculation
		mag_old[i] = mag[i];
	}
	
	return sum;		
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Methods to Calculate Windows ////////////////////////////////////

//--------------------------------------------------------------------------------------
// HANNING: set the window in the buffer 'window' to a Hanning window
void OnsetDetectionFunction :: set_win_hanning()
{
	double N;		// variable to store framesize minus 1
	
	N = (double) (framesize-1);	// framesize minus 1
	
	// Hanning window calculation
	for (int n = 0;n < framesize;n++)
	{
		window[n] = 0.5*(1-cos(2*pi*(n/N)));
	}
}

//--------------------------------------------------------------------------------------
// HAMMING: set the window in the buffer 'window' to a Hanning window
void OnsetDetectionFunction :: set_win_hamming()
{
	double N;		// variable to store framesize minus 1
	double n_val;	// double version of index 'n'
	
	N = (double) (framesize-1);	// framesize minus 1
	n_val = 0;
	
	// Hamming window calculation
	for (int n = 0;n < framesize;n++)
	{
		window[n] = 0.54 - (0.46*cos(2*pi*(n_val/N)));
		n_val = n_val+1;
	}
}

//--------------------------------------------------------------------------------------
// BLACKMAN: set the window in the buffer 'window' to a Blackman window
void OnsetDetectionFunction :: set_win_blackman()
{
	double N;		// variable to store framesize minus 1
	double n_val;	// double version of index 'n'
	
	N = (double) (framesize-1);	// framesize minus 1
	n_val = 0;
	
	// Blackman window calculation
	for (int n = 0;n < framesize;n++)
	{
		window[n] = 0.42 - (0.5*cos(2*pi*(n_val/N))) + (0.08*cos(4*pi*(n_val/N)));
		n_val = n_val+1;
	}
}

//--------------------------------------------------------------------------------------
// TUKEY: set the window in the buffer 'window' to a Tukey window
void OnsetDetectionFunction :: set_win_tukey()
{
	double N;		// variable to store framesize minus 1
	double n_val;	// double version of index 'n'
	double alpha;	// alpha [default value = 0.5];
	
	int lim1,lim2;
	
	alpha = 0.5;
	
	N = (double) (framesize-1);	// framesize minus 1
		
	// Tukey window calculation
	
	n_val = (double) (-1*((framesize/2)))+1;

	for (int n = 0;n < framesize;n++)	// left taper
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

//--------------------------------------------------------------------------------------
// RECTANGULAR: set the window in the buffer 'window' to a Rectangular window
void OnsetDetectionFunction :: set_win_rectangular()
{
	// Rectangular window calculation
	for (int n = 0;n < framesize;n++)
	{
		window[n] = 1.0;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Other Handy Methods //////////////////////////////////////////

//--------------------------------------------------------------------------------------
// set phase values to the range [-pi,pi]
double OnsetDetectionFunction :: princarg(double phaseval)
{	
	// if phase value is less than or equal to -pi then add 2*pi
	while (phaseval <= (-pi)) 
	{
		phaseval = phaseval + (2*pi);
	}
	
	// if phase value is larger than pi, then subtract 2*pi
	while (phaseval > pi)
	{
		phaseval = phaseval - (2*pi);
	}
			
	return phaseval;
}













