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
#include "../../src/BTrack.h"
#include "../../src/OnsetDetectionFunction.h"

//===========================================================================
// struct to represent the object's state
typedef struct _btrack {
    
    // The object itself (t_pxobject in MSP instead of t_object)
	t_pxobject		ob;
    
    // An instance of the BTrack beat tracker
    BTrack			*b;
    
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
void btrack_dsp(t_btrack *x, t_signal **sp, short *count);
void btrack_dsp64(t_btrack *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
t_int *btrack_perform(t_int *w);
void btrack_perform64(t_btrack *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

//===========================================================================
void btrack_process(t_btrack *x,double* audioFrame);
void outlet_beat(t_btrack *x, t_symbol *s, long argc, t_atom *argv);

// global class pointer variable
static t_class *btrack_class = NULL;




//===========================================================================
int C74_EXPORT main(void)
{	
	// object initialization, note the use of dsp_free for the freemethod, which is required
	// unless you need to free allocated memory, in which case you should call dsp_free from
	// your custom free function.

	t_class *c = class_new("btrack~", (method)btrack_new, (method)dsp_free, (long)sizeof(t_btrack), 0L, A_GIMME, 0);
	
	class_addmethod(c, (method)btrack_float,		"float",	A_FLOAT, 0);
	class_addmethod(c, (method)btrack_dsp,		"dsp",		A_CANT, 0);		// Old 32-bit MSP dsp chain compilation for Max 5 and earlier
	class_addmethod(c, (method)btrack_dsp64,		"dsp64",	A_CANT, 0);		// New 64-bit MSP dsp chain compilation for Max 6
	class_addmethod(c, (method)btrack_assist,	"assist",	A_CANT, 0);
	
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	btrack_class = c;

	return 0;
}


//===========================================================================
void *btrack_new(t_symbol *s, long argc, t_atom *argv)
{
	t_btrack *x = (t_btrack *)object_alloc(btrack_class);

	if (x) {
		dsp_setup((t_pxobject *)x, 1);	// MSP inlets: arg is # of inlets and is REQUIRED! 
										// use 0 if you don't need inlets

        object_post((t_object *) x,"v1.0     designed by Adam Stark and Matthew Davies at Queen Mary University of London");
        
        // create detection function and beat tracking objects
        x->b = new BTrack();
        
        x->tempo_outlet = floatout(x);
        x->beat_outlet = bangout(x);
        
        /*
        
        
        x->mode = 0;
        x->lastbang = 0;
        
        x->dobeats = 1;
        x->countin = 4;
        
        x->counttempi[0] = 120;
        x->counttempi[1] = 120;
        x->counttempi[2] = 120;
         */
	}
	return (x);
}


//===========================================================================
// NOT CALLED!, we use dsp_free for a generic free function
void btrack_free(t_btrack *x) 
{
	;
}


//===========================================================================
void btrack_assist(t_btrack *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { //inlet
		sprintf(s, "I am inlet %ld", a);
	} 
	else {	// outlet
		sprintf(s, "I am outlet %ld", a); 			
	}
}


//===========================================================================
void btrack_float(t_btrack *x, double f)
{


}

//===========================================================================
// this function is called when the DAC is enabled, and "registers" a function for the signal chain in Max 5 and earlier.
// In this case we register the 32-bit, "btrack_perform" method.
void btrack_dsp(t_btrack *x, t_signal **sp, short *count)
{
    int hopSize = (int) sp[0]->s_n;
    int frameSize = hopSize*2;
    
    x->b->updateHopAndFrameSize(hopSize, frameSize);
    
	dsp_add(btrack_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}


//===========================================================================
// this is the Max 6 version of the dsp method -- it registers a function for the signal chain in Max 6,
// which operates on 64-bit audio signals.
void btrack_dsp64(t_btrack *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    int hopSize = (int) maxvectorsize;
    int frameSize = hopSize*2;
    
    x->b->updateHopAndFrameSize(hopSize, frameSize);
		
	object_method(dsp64, gensym("dsp_add64"), x, btrack_perform64, 0, NULL);
}


//===========================================================================
// this is the 32-bit perform method for Max 5 and earlier
t_int *btrack_perform(t_int *w)
{
	t_btrack *x = (t_btrack *)(w[1]);
	t_float *inL = (t_float *)(w[2]);
	int n = (int)w[3];
	
    double audioFrame[n];
    
    for (int i = 0;i < n;i++)
    {
        audioFrame[i] = (double) inL[i];
    }
    
    btrack_process(x,audioFrame);
		
	// you have to return the NEXT pointer in the array OR MAX WILL CRASH
	return w + 4;
}

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
    // send a bang out of the beat outlet
    outlet_bang(x->beat_outlet);
    
    // send the tempo out of the tempo outlet
    outlet_float(x->tempo_outlet, (float) x->b->getCurrentTempoEstimate());
}











