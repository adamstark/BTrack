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
#include "CircularBuffer.h"
#include <vector>

//=======================================================================
/** The main beat tracking class and the interface to the BTrack
 * beat tracking algorithm. The algorithm can process either
 * audio frames or onset detection function samples and also
 * contains some static functions for calculating beat times in seconds
 */
class BTrack {
	
public:
    
    //=======================================================================
    /** Constructor assuming hop size of 512 and frame size of 1024 */
    BTrack();
    
    /** Constructor assuming frame size will be double the hopSize
     * @param hopSize the hop size in audio samples
     */
    BTrack (int hopSize_);
    
    /** Constructor taking both hopSize and frameSize
     * @param hopSize the hop size in audio samples
     * @param frameSize the frame size in audio samples
     */
    BTrack (int hopSize_, int frameSize_);
    
    /** Destructor */
    ~BTrack();
    
    //=======================================================================
    /** Updates the hop and frame size used by the beat tracker 
     * @param hopSize the hop size in audio samples
     * @param frameSize the frame size in audio samples
     */
    void updateHopAndFrameSize (int hopSize_, int frameSize_);
    
    //=======================================================================
    /** Process a single audio frame 
     * @param frame a pointer to an array containing an audio frame. The number of samples should 
     * match the frame size that the algorithm was initialised with.
     */
    void processAudioFrame (double* frame);
    
    /** Add new onset detection function sample to buffer and apply beat tracking 
     * @param sample an onset detection function sample
     */
    void processOnsetDetectionFunctionSample (double sample);
   
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
    /** Set the tempo of the beat tracker 
     * @param tempo the tempo in beats per minute (bpm)
     */
    void setTempo (double tempo);
    
    /** Fix tempo to roughly around some value, so that the algorithm will only try to track
     * tempi around the given tempo
     * @param tempo the tempo in beats per minute (bpm)
     */
    void fixTempo (double tempo);
    
    /** Tell the algorithm to not fix the tempo anymore */
    void doNotFixTempo();
    
    //=======================================================================
    /** Calculates a beat time in seconds, given the frame number, hop size and sampling frequency.
     * This version uses a long to represent the frame number
     * @param frameNumber the index of the current frame 
     * @param hopSize the hop size in audio samples
     * @param fs the sampling frequency in Hz
     * @returns a beat time in seconds
     */
    static double getBeatTimeInSeconds (long frameNumber, int hopSize, int fs);
    
    /** Calculates a beat time in seconds, given the frame number, hop size and sampling frequency.
     * This version uses an int to represent the frame number
     * @param frameNumber the index of the current frame
     * @param hopSize the hop size in audio samples
     * @param fs the sampling frequency in Hz
     * @returns a beat time in seconds
     */
    static double getBeatTimeInSeconds (int frameNumber, int hopSize, int fs);
    
		
private:
    
    /** Initialises the algorithm, setting internal parameters and creating weighting vectors 
     * @param hopSize_ the hop size in audio samples
     * @param frameSize_ the frame size in audio samples
     */
    void initialise (int hopSize_, int frameSize_);
    
    /** Initialise with hop size and set all array sizes accordingly
     * @param hopSize_ the hop size in audio samples
     */
    void setHopSize (int hopSize_);
    
    /** Resamples the onset detection function from an arbitrary number of samples to 512 */
    void resampleOnsetDetectionFunction();
    
    /** Updates the cumulative score function with a new onset detection function sample 
     * @param odfSample an onset detection function sample
     */
    void updateCumulativeScore (double odfSample);
	
    /** Predicts the next beat, based upon the internal program state */
    void predictBeat();
    
    /** Calculates the current tempo expressed as the beat period in detection function samples */
    void calculateTempo();
    
    /** Calculates an adaptive threshold which is used to remove low level energy from detection
     * function and emphasise peaks 
     * @param x a pointer to an array containing onset detection function samples
     * @param N the length of the array, x
     */
    void adaptiveThreshold (double* x, int N);
    
    /** Calculates the mean of values in an array between index locations [startIndex,endIndex]
     * @param array a pointer to an array that contains the values we wish to find the mean from
     * @param startIndex the start index from which we would like to calculate the mean
     * @param endIndex the final index to which we would like to calculate the mean
     * @returns the mean of the sub-section of the array
     */
    double calculateMeanOfArray (double* array, int startIndex, int endIndex);
    
    /** Normalises a given array
     * @param array a pointer to the array we wish to normalise
     * @param N the length of the array
     */
    void normaliseArray (double* array, int N);
    
    /** Calculates the balanced autocorrelation of the smoothed onset detection function
     * @param onsetDetectionFunction a pointer to an array containing the onset detection function
     */
    void calculateBalancedACF (double* onsetDetectionFunction);
    
    /** Calculates the output of the comb filter bank */
    void calculateOutputOfCombFilterBank();
	
    //=======================================================================

    /** An OnsetDetectionFunction instance for calculating onset detection functions */
    OnsetDetectionFunction odf;
    
    //=======================================================================
	// buffers
    
    CircularBuffer onsetDF;                 /**< to hold onset detection function */
    CircularBuffer cumulativeScore;         /**< to hold cumulative score */
    
    double resampledOnsetDF[512];           /**< to hold resampled detection function */
    double acf[512];                        /**<  to hold autocorrelation function */
    double weightingVector[128];            /**<  to hold weighting vector */
    double combFilterBankOutput[128];       /**<  to hold comb filter output */
    double tempoObservationVector[41];      /**<  to hold tempo version of comb filter output */
    double delta[41];                       /**<  to hold final tempo candidate array */
    double prevDelta[41];                   /**<  previous delta */
    double prevDeltaFixed[41];              /**<  fixed tempo version of previous delta */
    double tempoTransitionMatrix[41][41];   /**<  tempo transition matrix */
    
	//=======================================================================
    // parameters
    
    double tightness;                       /**< the tightness of the weighting used to calculate cumulative score */
    double alpha;                           /**< the mix between the current detection function sample and the cumulative score's "momentum" */
    double beatPeriod;                      /**< the beat period, in detection function samples */
    double tempo;                           /**< the tempo in beats per minute */
    double estimatedTempo;                  /**< the current tempo estimation being used by the algorithm */
    double latestCumulativeScoreValue;      /**< holds the latest value of the cumulative score function */
    double tempoToLagFactor;                /**< factor for converting between lag and tempo */
    int m0;                                 /**< indicates when the next point to predict the next beat is */
    int beatCounter;                        /**< keeps track of when the next beat is - will be zero when the beat is due, and is set elsewhere in the algorithm to be positive once a beat prediction is made */
    int hopSize;                            /**< the hop size being used by the algorithm */
    int onsetDFBufferSize;                  /**< the onset detection function buffer size */
    bool tempoFixed;                        /**< indicates whether the tempo should be fixed or not */
    bool beatDueInFrame;                    /**< indicates whether a beat is due in the current frame */
    int FFTLengthForACFCalculation;         /**< the FFT length for the auto-correlation function calculation */
    
#ifdef USE_FFTW
    fftw_plan acfForwardFFT;                /**< forward fftw plan for calculating auto-correlation function */
    fftw_plan acfBackwardFFT;               /**< inverse fftw plan for calculating auto-correlation function */
    fftw_complex* complexIn;                /**< to hold complex fft values for input */
    fftw_complex* complexOut;               /**< to hold complex fft values for output */
#endif
    
#ifdef USE_KISS_FFT
    kiss_fft_cfg cfgForwards;               /**< Kiss FFT configuration */
    kiss_fft_cfg cfgBackwards;              /**< Kiss FFT configuration */
    kiss_fft_cpx* fftIn;                    /**< FFT input samples, in complex form */
    kiss_fft_cpx* fftOut;                   /**< FFT output samples, in complex form */
#endif

};

#endif
