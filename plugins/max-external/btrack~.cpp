//===========================================================================
/** @file btrack~.cpp
 *  @brief The btrack~ Max external
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
//===========================================================================

//===========================================================================
#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects

//===========================================================================
// BTrack includes
#include "BTrack.h"
#include "OnsetDetectionFunction.h"

//===========================================================================
// struct to represent the object's state
typedef struct _btrack {
    
    // The object itself (t_pxobject in MSP instead of t_object)
	t_pxobject		ob;
    
    // An instance of the BTrack beat tracker
    BTrack			*b;
    
    // Indicates whether the beat tracker should output beats
    bool            should_output_beats;
    
    // the time of the last bang received in milliseconds
    long            time_of_last_bang_ms;
    
    // a count in counter
    long            count_in;
    
    // the recent tempi observed during count ins
    double           count_in_tempi[3];
    
    // An outlet for beats
    void            *beat_outlet;
    
    // An outlet for tempo estimates
    void            *tempo_outlet;
    
} t_btrack;


//===========================================================================
// method prototypes
void *btrack_new(t_symbol *s, long argc, t_atom *argv);
void btrack_free(t_btrack *x);
void btrack_assist(t_btrack *x, void *b, long m, long a, char *s);
void btrack_float(t_btrack *x, double f);
//void btrack_dsp(t_btrack *x, t_signal **sp, short *count);
void btrack_dsp64(t_btrack *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
//t_int *btrack_perform(t_int *w);
void btrack_perform64(t_btrack *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

//===========================================================================
void btrack_process(t_btrack *x,double* audioFrame);

void btrack_on(t_btrack *x);
void btrack_off(t_btrack *x);

void btrack_fixtempo(t_btrack *x, double f);
void btrack_unfixtempo(t_btrack *x);

void btrack_bang(t_btrack *x);
void btrack_countin(t_btrack *x);

void outlet_beat(t_btrack *x, t_symbol *s, long argc, t_atom *argv);

// global class pointer variable
static t_class *btrack_class = NULL;




//===========================================================================
//int C74_EXPORT main(void)
void ext_main(void *r)
{
    //--------------------------------------------------------------
	t_class *c = class_new("btrack~", (method)btrack_new, (method)btrack_free, (long)sizeof(t_btrack), 0L, A_GIMME, 0);
	
    //--------------------------------------------------------------
	class_addmethod(c, (method)btrack_float,		"float",	A_FLOAT, 0);
	//class_addmethod(c, (method)btrack_dsp,		"dsp",		A_CANT, 0);		// Old 32-bit MSP dsp chain compilation for Max 5 and earlier
	class_addmethod(c, (method)btrack_dsp64,		"dsp64",	A_CANT, 0);		// New 64-bit MSP dsp chain compilation for Max 6
	class_addmethod(c, (method)btrack_assist,	"assist",	A_CANT, 0);
    
    //--------------------------------------------------------------
    class_addmethod(c, (method)btrack_on,	"on", 0);
    class_addmethod(c, (method)btrack_off,	"off", 0);

    //--------------------------------------------------------------
    class_addmethod(c, (method)btrack_fixtempo,		"fixtempo",	A_FLOAT, 0);
    class_addmethod(c, (method)btrack_unfixtempo,	"unfixtempo", 0);
    
    //--------------------------------------------------------------
    class_addmethod(c, (method)btrack_bang,		"bang", 0);
    class_addmethod(c, (method)btrack_countin,	"countin", 0);
    
    //--------------------------------------------------------------
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	btrack_class = c;

	//return 0;
}

//===========================================================================
void *btrack_new(t_symbol *s, long argc, t_atom *argv)
{
	t_btrack *x = (t_btrack *)object_alloc(btrack_class);

	if (x) {
		dsp_setup((t_pxobject *)x, 1);	// MSP inlets: arg is # of inlets and is REQUIRED! 
										// use 0 if you don't need inlets

        // create detection function and beat tracking objects
        x->b = new BTrack();
        
        // create outlets for bpm and beats
        x->tempo_outlet = floatout(x);
        x->beat_outlet = bangout(x);
        
        // initialise variables
        x->should_output_beats = true;
        x->time_of_last_bang_ms = 0;
        x->count_in = 4;
        x->count_in_tempi[0] = 120;
        x->count_in_tempi[1] = 120;
        x->count_in_tempi[2] = 120;

	}
	return (x);
}


//===========================================================================
void btrack_free(t_btrack *x) 
{
    // delete the beat tracker
    delete x->b;
    x->b = NULL;
    
    // call the dsp free function on our object
    dsp_free((t_pxobject *)x);
}


//===========================================================================
void btrack_assist(t_btrack *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { //inlet
        if (a == 0)
        {
            sprintf(s, "(signal) Audio In");
        }
    }
    else {	// outlet
        if (a == 0)
        {
            sprintf(s, "Beats Out");
        }
        if (a == 1)
        {
            sprintf(s, "Tempo (bpm)");
        }
        
    }
}


//===========================================================================
void btrack_float(t_btrack *x, double f)
{


}

//===========================================================================
// this function is called when the DAC is enabled, and "registers" a function for the signal chain in Max 5 and earlier.
// In this case we register the 32-bit, "btrack_perform" method.
//void btrack_dsp(t_btrack *x, t_signal **sp, short *count)
//{
//    // get hop size and frame size
//    int hopSize = (int) sp[0]->s_n;
//    int frameSize = hopSize*2;
//    
//    // initialise the beat tracker
//    x->b->updateHopAndFrameSize(hopSize, frameSize);
//    
//    // set up dsp
//	dsp_add(btrack_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
//}


//===========================================================================
// this is the Max 6 version of the dsp method -- it registers a function for the signal chain in Max 6,
// which operates on 64-bit audio signals.
void btrack_dsp64(t_btrack *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
     // get hop size and frame size
    int hopSize = (int) maxvectorsize;
    int frameSize = hopSize*2;
    
    // initialise the beat tracker
    x->b->updateHopAndFrameSize(hopSize, frameSize);
    		
    // set up dsp
	object_method(dsp64, gensym("dsp_add64"), x, btrack_perform64, 0, NULL);
}


////===========================================================================
//// this is the 32-bit perform method for Max 5 and earlier
//t_int *btrack_perform(t_int *w)
//{
//	t_btrack *x = (t_btrack *)(w[1]);
//	t_float *inL = (t_float *)(w[2]);
//	int n = (int)w[3];
//	
//    double audioFrame[n];
//    
//    for (int i = 0;i < n;i++)
//    {
//        audioFrame[i] = (double) inL[i];
//    }
//    
//    btrack_process(x,audioFrame);
//		
//	// you have to return the NEXT pointer in the array OR MAX WILL CRASH
//	return w + 4;
//}

//===========================================================================
// this is 64-bit perform method for Max 6
void btrack_perform64(t_btrack *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	t_double *inL = ins[0];		// we get audio for each inlet of the object from the **ins argument
	int n = sampleframes;
    
    double audioFrame[n];
    
    for (int i = 0;i < n;i++)
    {
        audioFrame[i] = (double) inL[i];
    }
    
    btrack_process(x,audioFrame);
}

//===========================================================================
void btrack_process(t_btrack *x,double* audioFrame)
{
    // process the audio frame
    x->b->processAudioFrame(audioFrame);
    
    
    // if there is a beat in this frame
    if (x->b->beatDueInCurrentFrame())
    {
        // outlet a beat
        defer_low((t_object *)x, (method)outlet_beat, NULL, 0, NULL);
    }
}

//===========================================================================
void outlet_beat(t_btrack *x, t_symbol *s, long argc, t_atom *argv)
{
    if (x->should_output_beats)
    {
        // send a bang out of the beat outlet
        outlet_bang(x->beat_outlet);
    
        // send the tempo out of the tempo outlet
        outlet_float(x->tempo_outlet, (float) x->b->getCurrentTempoEstimate());
    }
}


//===========================================================================
void btrack_on(t_btrack *x)
{
    x->should_output_beats = true;
}

//===========================================================================
void btrack_off(t_btrack *x)
{
    x->should_output_beats = false;
    x->count_in = 4;
}

//===========================================================================
void btrack_fixtempo(t_btrack *x, double f)
{
    x->b->fixTempo(f);
    object_post((t_object *) x,"Tempo fixed to %f BPM",f);
}

//===========================================================================
void btrack_unfixtempo(t_btrack *x)
{
    x->b->doNotFixTempo();
    object_post((t_object *) x,"Tempo no longer fixed");
}

//===========================================================================
void btrack_countin(t_btrack *x)
{
    x->count_in = x->count_in-1;
    
    btrack_bang(x);
    if (x->count_in == 0)
    {
        x->should_output_beats = 1;
    }
}

//===========================================================================
void btrack_bang(t_btrack *x)
{
    double bperiod;
    double tempo;
    double mean_tempo;
    
    // get current time in milliseconds
    long ms = systime_ms();
    
    // calculate beat period
    bperiod = ((double) (ms - x->time_of_last_bang_ms))/1000.0;
    
    // store time since last bang
    x->time_of_last_bang_ms = ms;
    
    // if beat period is between 0 and 1
    if ((bperiod < 1.0) && (bperiod > 0.0))
    {
        // calculate tempo from beat period
        tempo = (1/bperiod)*60;
        
        double sum = 0;
        
        // move back elements in tempo history and sum remaining elements
        for (int i = 0;i < 2;i++)
        {
            x->count_in_tempi[i] = x->count_in_tempi[i+1];
            sum = sum+x->count_in_tempi[i];
        }
        
        // set final element to be the newly calculated tempo
        x->count_in_tempi[2] = tempo;
        
        // add the new tempo to the sum
        sum = sum+x->count_in_tempi[2];
        
        // calculate the mean tempo by dividing the tempo by 3
        mean_tempo = sum/3;
        
        // set the tempo in the beat tracker
        x->b->setTempo(mean_tempo);
    }
}



