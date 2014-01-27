BTrack - A Real-Time Beat Tracker
=================================

** Version 0.9 **

*by Adam Stark, Matthew Davies and Mark Plumbley.*


About BTrack
------------

BTrack is a causal beat tracking algorithm intended for real-time use. It is implemented in C++ with a wrapper for Python.

Full details of the working of the algorithm can be found in:

* Musicians and Machines: Bridging the Semantic Gap in Live Performance, Chapter 3, A. M. Stark, PhD Thesis, Queen Mary, University of London, 2011.

* Real-Time Beat-Synchronous Analysis of Musical Audio, A. M. Stark, M. E. P. Davies and M. D. Plumbley. In Proceedings of the 12th International Conference on Digital Audio Effects (DAFx-09), Como, Italy, September 1-4, 2009.


Versions
--------

==== 0.9 ====

* This is the original version of the BTrack algorithm



License
-------

BTrack is made available under the GNU General Public License, version 3. Please see the included LICENSE.txt for more details.

Usage - C++
-----------

**STEP 1**

Include the BTrack header file as follows:

	#include "BTrack.h"
	
**STEP 2**

Instantiate the algorithm by one of the following:

	// to use the default 512 hop size and 1024 frame size
	BTrack b; 

or:	

	// to specify a hop size (e.g. 512) and have a frame size of 2 x the hop size
	BTrack b(512); 
	
or:

	// to specify both the hop size and frame size
	BTrack b(512,1024);
	
**STEP 3.1 - Audio Input**

In the processing loop, fill a double precision array with one frame of audio samples (as determined in step 2): 

	double *frame; 
	
	// !
	// do something here to fill the frame with audio samples
	// !

and then call:

	b.processAudioFrame(frame);
	
and to check for beats, simply call:

	if (b.beatDueInCurrentFrame())
	{
		// do something on the beat
	}

**STEP 3.2 - Onset Detection Function Input**	

The algorithm can process onset detection function samples. Given a double precision onset detection function sample called 'newSamples', at each step, call:

	b.processOnsetDetectionFunctionSample(newSample);
	
and then check for beats with:

	if (b.beatDueInCurrentFrame())
	{
		// do something on the beat
	}
