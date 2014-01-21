//=======================================================================
/** @file BTrack.h
 *  @brief Header file for the BTrack beat tracker
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

#ifndef __BTRACK_H
#define __BTRACK_H

//#include "fftw3.h"

class BTrack {
	
public:
	BTrack();				// constructor
	~BTrack();				// destructor	
	
	void initialise(int fsize);
	void process(float df_sample);
	void plotdfbuffer();
	void updatecumscore(float df_sample);
	void predictbeat();
	void dfconvert();
	void calcTempo();
	void adapt_thresh(float x[],int N);
	float mean_array(float array[],int start,int end);
	void normalise(float array[],int N);
	void acf_bal(float df_thresh[]);
	void getrcfoutput();
	void settempo(float tempo);
	void fixtempo(float tempo);
	void unfixtempo();
	
	int playbeat;
	float cscoreval;
	float est_tempo;
			
private:
	
	// buffers
	float *dfbuffer;			// to hold detection function
	float df512[512];			// to hold resampled detection function 
	float *cumscore;			// to hold cumulative score
	
	float acf[512];				// to hold autocorrelation function
	
	float wv[128];				// to hold weighting vector
	
	float rcf[128];				// to hold comb filter output
	float t_obs[41];			// to hold tempo version of comb filter output
	
	float delta[41];			// to hold final tempo candidate array
	float prev_delta[41];		// previous delta
	float prev_delta_fix[41];	// fixed tempo version of previous delta
	
	float t_tmat[41][41];		// transition matrix
	
	
	// parameters
	float tightness;
	float alpha;
	float bperiod;
	float tempo;
	
	
	float p_fact;
	
	
	//
	int m0;				// indicates when the next point to predict the next beat is
	int beat;
	
	int dfbuffer_size;
		
	
	int framesize;
	
	
	int tempofix;
	

};

#endif
