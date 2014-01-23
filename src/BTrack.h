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

#include "OnsetDetectionFunction.h"

class BTrack {
	
public:
    
    /** constructor assuming hop size of 512 and frame size of 1024 */
    BTrack();
    
    /** constructor assuming frame size will be double hopSize 
     * @param hopSize the step size in audio samples by which we will receive audio frames
     */
    BTrack(int hopSize_);
    
    /** constructor taking both hopSize and frameSize
     * @param hopSize the step size in audio samples by which we will receive audio frames
     * @param frameSize the audio frame size in audio samples
     */
    BTrack(int hopSize_,int frameSize_);
    
    /** Process a single audio frame */
    void processAudioFrame(double *frame);
    
    /** Add new onset detection function sample to buffer and apply beat tracking */
    void processOnsetDetectionFunctionSample(double sample);
   
    /** @returns the current hop size being used by the beat tracker */
    int getHopSize();
    
    /** Set the tempo of the beat tracker */
    void setTempo(double tempo);
    
    /** fix tempo to roughly around some value */
    void fixTempo(double tempo);
    
    /** do not fix the tempo anymore */
    void doNotFixTempo();
    
    static double getBeatTimeInSeconds(long frameNumber,int hopSize,int fs);
    
    static double getBeatTimeInSeconds(int frameNumber,int hopSize,int fs);
    
    /** @returns true if a beat should occur in the current audio frame */
    bool beatDueInCurrentFrame();
    
    double cscoreval;
    double est_tempo;
			
private:
    
    void initialise(int hopSize_,int frameSize_);
    
    /** Initialise with hop size and set all frame sizes accordingly */
    void setHopSize(int hopSize_);
    
    /** Convert detection function from N samples to 512 */
    void resampleOnsetDetectionFunction();
    
    /** update the cumulative score */
    void updateCumulativeScore(double df_sample);
	
    /** predicts the next beat */
    void predictBeat();
    
    /** Calculates the current tempo expressed as the beat period in detection function samples */
    void calculateTempo();
    
    /** calculates an adaptive threshold which is used to remove low level energy from detection 
     * function and emphasise peaks 
     */
    void adaptiveThreshold(double *x,int N);
    
    /** calculates the mean of values in an array from index locations [start,end] */
    double calculateMeanOfArray(double *array,int start,int end);
    
    /** normalises a given array */
    void normaliseArray(double *array,int N);
    
    /** calculates the balanced autocorrelation of the smoothed detection function */
    void calculateBalancedACF(double *df_thresh);
    
    /** calculates the output of the comb filter bank */
    void calculateOutputOfCombFilterBank();
	
	// buffers
    double *dfbuffer;			/**< to hold detection function */
    double df512[512];			/**< to hold resampled detection function */
    double *cumscore;			/**<  to hold cumulative score */
	
    double acf[512];			/**<  to hold autocorrelation function */
	
    double wv[128];				/**<  to hold weighting vector */
	
    double rcf[128];			/**<  to hold comb filter output */
    double t_obs[41];			/**<  to hold tempo version of comb filter output */
	
    double delta[41];			/**<  to hold final tempo candidate array */
    double prev_delta[41];		/**<  previous delta */
    double prev_delta_fix[41];	/**<  fixed tempo version of previous delta */
	
    double t_tmat[41][41];		/**<  transition matrix */
	
    OnsetDetectionFunction odf;
	
    // parameters
    double tightness;
    double alpha;
    double beatPeriod;
    double tempo;
	
	
    double p_fact;
	
	
    //
    int m0;				// indicates when the next point to predict the next beat is
    int beat;
	
    int dfbuffer_size;
		
	
    int hopSize;
	
	
    int tempofix;
	
    
    bool beatDueInFrame;

};

#endif
