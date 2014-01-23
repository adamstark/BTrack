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
    
    //=======================================================================
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
    
    //=======================================================================
    /** Process a single audio frame */
    void processAudioFrame(double *frame);
    
    /** Add new onset detection function sample to buffer and apply beat tracking */
    void processOnsetDetectionFunctionSample(double sample);
   
    //=======================================================================
    /** @returns the current hop size being used by the beat tracker */
    int getHopSize();
    
    /** @returns true if a beat should occur in the current audio frame */
    bool beatDueInCurrentFrame();

    /** @returns the current tempo estimate being used by the beat tracker */
    double getCurrentTempoEstimate();
    
    /** @returns the most recent value of the cumulative score function */
    double getLatestCumulativeScoreValue();
    
    //=======================================================================
    /** Set the tempo of the beat tracker */
    void setTempo(double tempo);
    
    /** fix tempo to roughly around some value */
    void fixTempo(double tempo);
    
    /** do not fix the tempo anymore */
    void doNotFixTempo();
    
    //=======================================================================
    static double getBeatTimeInSeconds(long frameNumber,int hopSize,int fs);
    
    static double getBeatTimeInSeconds(int frameNumber,int hopSize,int fs);
    
		
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
	
    //=======================================================================

    OnsetDetectionFunction odf;
    
    //=======================================================================
	// buffers
    double *onsetDF;                        /**< to hold detection function */
    double resampledOnsetDF[512];           /**< to hold resampled detection function */
    double *cumulativeScore;                /**<  to hold cumulative score */
	
    double acf[512];                        /**<  to hold autocorrelation function */
	
    double weightingVector[128];            /**<  to hold weighting vector */
	
    double combFilterBankOutput[128];       /**<  to hold comb filter output */
    double tempoObservationVector[41];      /**<  to hold tempo version of comb filter output */
	
    double delta[41];                       /**<  to hold final tempo candidate array */
    double prevDelta[41];                   /**<  previous delta */
    double prevDeltaFixed[41];              /**<  fixed tempo version of previous delta */
	
    double tempoTransitionMatrix[41][41];   /**<  tempo transition matrix */
	
    
	
    // parameters
    double tightness;
    double alpha;
    double beatPeriod;
    double tempo;
	
    double estimatedTempo;                  /**< the current tempo estimation being used by the algorithm */
    
    double latestCumulativeScoreValue;      /**< holds the latest value of the cumulative score function */
    
    double p_fact;
	
    int m0;                                 // indicates when the next point to predict the next beat is
    
    int beatCounter;                        /**< keeps track of when the next beat is - will be zero when the beat is due, and is set elsewhere in the algorithm to be positive once a beat prediction is made */
	
    int hopSize;                            /**< the hop size being used by the algorithm */
    
    int onsetDFBufferSize;                  /**< the onset detection function buffer size */
	
    bool tempoFixed;                        /**< indicates whether the tempo should be fixed or not */
    
    bool beatDueInFrame;                    /**< indicates whether a beat is due in the current frame */

};

#endif
