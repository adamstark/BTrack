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

#include <iostream>
#include <cmath>
#include "BTrack.h"
#include "samplerate.h"
using namespace std;




//=======================================================================
BTrack :: BTrack()
{	
	float rayparam = 43;
	float pi = 3.14159265;
	
	
	// initialise parameters
	tightness = 5;
	alpha = 0.9;
	tempo = 120;
	est_tempo = 120;
	p_fact = 60.*44100./512.;
	
	m0 = 10;
	beat = -1;
	
	playbeat = 0;
	
	
	
	
	// create rayleigh weighting vector
	for (int n = 0;n < 128;n++)
	{
		wv[n] = ((float) n / pow(rayparam,2)) * exp((-1*pow((float)-n,2)) / (2*pow(rayparam,2)));
	}
	
	// initialise prev_delta
	for (int i = 0;i < 41;i++)
	{
		prev_delta[i] = 1;
	}
	
	float t_mu = 41/2;
	float m_sig;
	float x;
	// create tempo transition matrix
	m_sig = 41/8;
	for (int i = 0;i < 41;i++)
	{
		for (int j = 0;j < 41;j++)
		{
			x = j+1;
			t_mu = i+1;
			t_tmat[i][j] = (1 / (m_sig * sqrt(2*pi))) * exp( (-1*pow((x-t_mu),2)) / (2*pow(m_sig,2)) );
		}
	}	
	
	// tempo is not fixed
	tempofix = 0;
}

//=======================================================================
BTrack :: ~BTrack()
{	
	
}



//=======================================================================
void BTrack :: initialise(int fsize)
{	
	framesize = fsize;
	dfbuffer_size = (512*512)/fsize;		// calculate df buffer size
	
	bperiod = round(60/((((float) fsize)/44100)*tempo));
	
	dfbuffer = new float[dfbuffer_size];	// create df_buffer
	cumscore = new float[dfbuffer_size];	// create cumscore
	
	
	// initialise df_buffer to zeros
	for (int i = 0;i < dfbuffer_size;i++)
	{
		dfbuffer[i] = 0;
		cumscore[i] = 0;
		
		
		if ((i %  ((int) round(bperiod))) == 0)
		{
			dfbuffer[i] = 1;
		}
	}
}

//=======================================================================
void BTrack :: process(float df_sample)
{	 
	m0--;
	beat--;
	playbeat = 0;
	
	// move all samples back one step
	for (int i=0;i < (dfbuffer_size-1);i++)
	{
		dfbuffer[i] = dfbuffer[i+1];
	}
	
	// add new sample at the end
	dfbuffer[dfbuffer_size-1] = df_sample;	
	
	// update cumulative score
	updatecumscore(df_sample);
	
	// if we are halfway between beats
	if (m0 == 0)
	{
		predictbeat();
	}
	
	// if we are at a beat
	if (beat == 0)
	{
		playbeat = 1;	// indicate a beat should be output
		
		// recalculate the tempo
		dfconvert();	
		calcTempo();
	}
}

//=======================================================================
void BTrack :: settempo(float tempo)
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
		prev_delta[i] = 0;
	}
	
	// set desired tempo index to 1
	prev_delta[tempo_index] = 1;
	
	
	/////////// CUMULATIVE SCORE ARTIFICAL TEMPO UPDATE //////////////////
	
	// calculate new beat period
	int new_bperiod = (int) round(60/((((float) framesize)/44100)*tempo));
	
	int bcounter = 1;
	// initialise df_buffer to zeros
	for (int i = (dfbuffer_size-1);i >= 0;i--)
	{
		if (bcounter == 1)
		{
			cumscore[i] = 150;
			dfbuffer[i] = 150;
		}
		else
		{
			cumscore[i] = 10;
			dfbuffer[i] = 10;
		}
		
		bcounter++;
		
		if (bcounter > new_bperiod)
		{
			bcounter = 1;
		}
	}
	
	/////////// INDICATE THAT THIS IS A BEAT //////////////////
	
	// beat is now
	beat = 0;
	
	// offbeat is half of new beat period away
	m0 = (int) round(((float) new_bperiod)/2);
}

//=======================================================================
void BTrack :: fixtempo(float tempo)
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
		prev_delta_fix[i] = 0;
	}
	
	// set desired tempo index to 1
	prev_delta_fix[tempo_index] = 1;
		
	// set the tempo fix flag
	tempofix = 1;	
}

//=======================================================================
void BTrack :: unfixtempo()
{	
	// set the tempo fix flag
	tempofix = 0;	
}

//=======================================================================
void BTrack :: dfconvert()
{
	float output[512];
		
	double src_ratio = 512.0/((double) dfbuffer_size);
	int BUFFER_LEN = dfbuffer_size;
	int output_len;
	SRC_DATA	src_data ;
	
	//output_len = (int) floor (((double) BUFFER_LEN) * src_ratio) ;
	output_len = 512;
	
	src_data.data_in = dfbuffer;
	src_data.input_frames = BUFFER_LEN;
	
	src_data.src_ratio = src_ratio;
	
	src_data.data_out = output;
	src_data.output_frames = output_len;
	
	src_simple (&src_data, SRC_SINC_BEST_QUALITY, 1);
			
	for (int i = 0;i < output_len;i++)
	{
		df512[i] = src_data.data_out[i];
	}
}

//=======================================================================
void BTrack :: calcTempo()
{
	// adaptive threshold on input
	adapt_thresh(df512,512);
		
	// calculate auto-correlation function of detection function
	acf_bal(df512);
	
	// calculate output of comb filterbank
	getrcfoutput();
	
	
	// adaptive threshold on rcf
	adapt_thresh(rcf,128);

	
	int t_index;
	int t_index2;
	// calculate tempo observation vector from bperiod observation vector
	for (int i = 0;i < 41;i++)
	{
		t_index = (int) round(p_fact / ((float) ((2*i)+80)));
		t_index2 = (int) round(p_fact / ((float) ((4*i)+160)));

		
		t_obs[i] = rcf[t_index-1] + rcf[t_index2-1];
	}
	
	
	float maxval;
	float maxind;
	float curval;
	
	// if tempo is fixed then always use a fixed set of tempi as the previous observation probability function
	if (tempofix == 1)
	{
		for (int k = 0;k < 41;k++)
		{
			prev_delta[k] = prev_delta_fix[k];
		}
	}
		
	for (int j=0;j < 41;j++)
	{
		maxval = -1;
		for (int i = 0;i < 41;i++)
		{
			curval = prev_delta[i]*t_tmat[i][j];
			
			if (curval > maxval)
			{
				maxval = curval;
			}
		}
		
		delta[j] = maxval*t_obs[j];
	}
	

	normalise(delta,41);
	
	maxind = -1;
	maxval = -1;
	
	for (int j=0;j < 41;j++)
	{
		if (delta[j] > maxval)
		{
			maxval = delta[j];
			maxind = j;
		}
		
		prev_delta[j] = delta[j];
	}
	
	bperiod = round((60.0*44100.0)/(((2*maxind)+80)*((float) framesize)));
	
	if (bperiod > 0)
	{
		est_tempo = 60.0/((((float) framesize) / 44100.0)*bperiod);
	}
	
	//cout << bperiod << endl;
}

//=======================================================================
void BTrack :: adapt_thresh(float x[],int N)
{
	//int N = 512; // length of df
	int i = 0;
	int k,t = 0;
	float x_thresh[N];
	
	int p_post = 7;
	int p_pre = 8;
	
	t = min(N,p_post);	// what is smaller, p_post of df size. This is to avoid accessing outside of arrays
	
	// find threshold for first 't' samples, where a full average cannot be computed yet 
	for (i = 0;i <= t;i++)
	{	
		k = min((i+p_pre),N);
		x_thresh[i] = mean_array(x,1,k);
	}
	// find threshold for bulk of samples across a moving average from [i-p_pre,i+p_post]
	for (i = t+1;i < N-p_post;i++)
	{
		x_thresh[i] = mean_array(x,i-p_pre,i+p_post);
	}
	// for last few samples calculate threshold, again, not enough samples to do as above
	for (i = N-p_post;i < N;i++)
	{
		k = max((i-p_post),1);
		x_thresh[i] = mean_array(x,k,N);
	}
	
	// subtract the threshold from the detection function and check that it is not less than 0
	for (i = 0;i < N;i++)
	{
		x[i] = x[i] - x_thresh[i];
		if (x[i] < 0)
		{
			x[i] = 0;
		}
	}
}

//=======================================================================
void BTrack :: getrcfoutput()
{
	int numelem;
	
	for (int i = 0;i < 128;i++)
	{
		rcf[i] = 0;
	}
	
	numelem = 4;
	
	for (int i = 2;i <= 127;i++) // max beat period
	{
		for (int a = 1;a <= numelem;a++) // number of comb elements
		{
			for (int b = 1-a;b <= a-1;b++) // general state using normalisation of comb elements
			{
				rcf[i-1] = rcf[i-1] + (acf[(a*i+b)-1]*wv[i-1])/(2*a-1);	// calculate value for comb filter row
			}
		}
	}
}

//=======================================================================
void BTrack :: acf_bal(float df_thresh[])
{
	int l, n = 0;
	float sum, tmp;
	
	// for l lags from 0-511
	for (l = 0;l < 512;l++)
	{
		sum = 0;	
		
		// for n samples from 0 - (512-lag)
		for (n = 0;n < (512-l);n++)
		{
			tmp = df_thresh[n] * df_thresh[n+l];	// multiply current sample n by sample (n+l)
			sum = sum + tmp;	// add to sum
		}
		
		acf[l] = sum / (512-l);		// weight by number of mults and add to acf buffer
	}
}

//=======================================================================
float BTrack :: mean_array(float array[],int start,int end)
{
	int i;
	double sum = 0;

    int length = end - start;
	
	// find sum
	for (i = start;i < end;i++)
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
void BTrack :: normalise(float array[],int N)
{
	double sum = 0;
	
	for (int i = 0;i < N;i++)
	{
		if (array[i] > 0)
		{
			sum = sum + array[i];
		}
	}
	
	if (sum > 0)
	{
		for (int i = 0;i < N;i++)
		{
			array[i] = array[i] / sum;
		}
	}
}

//=======================================================================
void BTrack :: updatecumscore(float df_sample)
{	 
	int start, end, winsize;
	float max;
	
	start = dfbuffer_size - round(2*bperiod);
	end = dfbuffer_size - round(bperiod/2);
	winsize = end-start+1;
	
	float w1[winsize];
	float v = -2*bperiod;
	float wcumscore;
	
	
	// create window
	for (int i = 0;i < winsize;i++)
	{
		w1[i] = exp((-1*pow(tightness*log(-v/bperiod),2))/2);
		v = v+1;
	}	
	
	// calculate new cumulative score value
	max = 0;
	int n = 0;
	for (int i=start;i <= end;i++)
	{
			wcumscore = cumscore[i]*w1[n];
		
			if (wcumscore > max)
			{
				max = wcumscore;
			}
		n++;
	}
	
	
	// shift cumulative score back one
	for (int i = 0;i < (dfbuffer_size-1);i++)
	{
		cumscore[i] = cumscore[i+1];
	}
	
	// add new value to cumulative score
	cumscore[dfbuffer_size-1] = ((1-alpha)*df_sample) + (alpha*max);
	
	cscoreval = cumscore[dfbuffer_size-1];
	
	//cout << cumscore[dfbuffer_size-1] << endl;
		
}

//=======================================================================
void BTrack :: predictbeat()
{	 
	int winsize = (int) bperiod;
	float fcumscore[dfbuffer_size + winsize];
	float w2[winsize];
	// copy cumscore to first part of fcumscore
	for (int i = 0;i < dfbuffer_size;i++)
	{
		fcumscore[i] = cumscore[i];
	}
	
	// create future window
	float v = 1;
	for (int i = 0;i < winsize;i++)
	{
		w2[i] = exp((-1*pow((v - (bperiod/2)),2))   /  (2*pow((bperiod/2) ,2)));
		v++;
	}
	
	// create past window
	v = -2*bperiod;
	int start = dfbuffer_size - round(2*bperiod);
	int end = dfbuffer_size - round(bperiod/2);
	int pastwinsize = end-start+1;
	float w1[pastwinsize];

	for (int i = 0;i < pastwinsize;i++)
	{
		w1[i] = exp((-1*pow(tightness*log(-v/bperiod),2))/2);
		v = v+1;
	}

	

	// calculate future cumulative score
	float max;
	int n;
	float wcumscore;
	for (int i = dfbuffer_size;i < (dfbuffer_size+winsize);i++)
	{
		start = i - round(2*bperiod);
		end = i - round(bperiod/2);
		
		max = 0;
		n = 0;
		for (int k=start;k <= end;k++)
		{
			wcumscore = fcumscore[k]*w1[n];
			
			if (wcumscore > max)
			{
				max = wcumscore;
			}
			n++;
		}
		
		fcumscore[i] = max;
	}
	
	
	// predict beat
	max = 0;
	n = 0;
	
	for (int i = dfbuffer_size;i < (dfbuffer_size+winsize);i++)
	{
		wcumscore = fcumscore[i]*w2[n];
		
		if (wcumscore > max)
		{
			max = wcumscore;
			beat = n;
		}	
		
		n++;
	}
		
	
	// set beat
	//beat = beat;
	
	// set next prediction time
	m0 = beat+round(bperiod/2);
	

}