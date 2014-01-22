//=======================================================================
/** @file BTrack.h
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

#ifndef __BTRACK_H
#define __BTRACK_H

class BTrack {
	
public:
    
    /** constructor */
	BTrack();
    
    /** destructor */
	~BTrack();		
	
    /** Initialise with frame size and set all frame sizes accordingly */
	void initialise(int fsize);
    
    /** Add new sample to buffer and apply beat tracking */
	void process(double df_sample);
   
    /** Set the tempo of the beat tracker */
	void settempo(double tempo);
    
    /** fix tempo to roughly around some value */
	void fixtempo(double tempo);
    
    /** do not fix the tempo anymore */
	void unfixtempo();
	
	int playbeat;
	double cscoreval;
	double est_tempo;
			
private:
    
    /** Convert detection function from N samples to 512 */
	void dfconvert();
    
    /** update the cumulative score */
	void updatecumscore(double df_sample);
	
    /** predicts the next beat */
    void predictbeat();
    
    /** Calculates the current tempo expressed as the beat period in detection function samples */
    void calcTempo();
    
    /** calculates an adaptive threshold which is used to remove low level energy from detection 
     * function and emphasise peaks 
     */
	void adapt_thresh(double *x,int N);
    
    /** calculates the mean of values in an array from index locations [start,end] */
	double mean_array(double *array,int start,int end);
    
    /** normalises a given array */
	void normalise(double *array,int N);
    
    /** calculates the balanced autocorrelation of the smoothed detection function */
	void acf_bal(double *df_thresh);
    
    /** returns the output of the comb filter */
	void getrcfoutput();
	
	// buffers
	double *dfbuffer;			/**< to hold detection function */
	double df512[512];			/**< to hold resampled detection function */
	double *cumscore;			/**<  to hold cumulative score */
	
	double acf[512];				/**<  to hold autocorrelation function */
	
	double wv[128];				/**<  to hold weighting vector */
	
	double rcf[128];				/**<  to hold comb filter output */
	double t_obs[41];			/**<  to hold tempo version of comb filter output */
	
	double delta[41];			/**<  to hold final tempo candidate array */
	double prev_delta[41];		/**<  previous delta */
	double prev_delta_fix[41];	/**<  fixed tempo version of previous delta */
	
	double t_tmat[41][41];		/**<  transition matrix */
	
	
	// parameters
	double tightness;
	double alpha;
	double bperiod;
	double tempo;
	
	
	double p_fact;
	
	
	//
	int m0;				// indicates when the next point to predict the next beat is
	int beat;
	
	int dfbuffer_size;
		
	
	int framesize;
	
	
	int tempofix;
	

};

#endif
