BTrack - A Real-Time Beat Tracker
=================================

*by Adam Stark, Matthew Davies and Mark Plumbley.*

![Version](https://img.shields.io/badge/version-1.0.5-green.svg?style=flat-square) 
![Licence](https://img.shields.io/badge/licence-GPLv3-blue.svg?style=flat-square) 
![Language](https://img.shields.io/badge/language-C++-yellow.svg?style=flat-square) 

About BTrack
------------

BTrack is a causal beat tracking algorithm intended for real-time use. It is implemented in C++ with wrappers for Python and the Vamp plug-in framework.

Full details of the working of the algorithm can be found in:

* Musicians and Machines: Bridging the Semantic Gap in Live Performance, Chapter 3, A. M. Stark, PhD Thesis, Queen Mary, University of London, 2011.

* Real-Time Beat-Synchronous Analysis of Musical Audio, A. M. Stark, M. E. P. Davies and M. D. Plumbley. In Proceedings of the 12th International Conference on Digital Audio Effects (DAFx-09), Como, Italy, September 1-4, 2009.

BTrack is made available under the GNU General Public License, version 3. Please see the included LICENSE.txt for more details.

Versions
--------

==== 1.0.5 ==== (18th August 2023)

* Code improvements and modernisation
* Move to CMake 
* Move to doctest for unit tests

==== 1.0.4 ==== (18th June 2016)

* Added option of using Kiss FFT
* Implementation changes to improve efficiency

==== 1.0.3 ==== (10th January 2016)

* Fixed implementation error in complex spectral difference (thanks to @zbanks for pointing it out)
* Small change to python module build script

==== 1.0.2 ==== (25th November 2014)

* Added updated Max external project and associated files
* Fixed a couple of bugs

==== 1.0.1 ==== (21st November 2014)

* Moved to git & updated README
* No implementation changes

==== 1.0.0 ==== (8th July 2014)

* Many updates to stability and improvements to implementation

==== 0.9.0 ==== (circa 2008/2009)

* This is the original version of the BTrack algorithm



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

The algorithm can process onset detection function samples. Given a double precision onset detection function sample called 'newSample', at each step, call:

	b.processOnsetDetectionFunctionSample(newSample);
	
and then check for beats with:

	if (b.beatDueInCurrentFrame())
	{
		// do something on the beat
	}

Requirements
------------

To compile BTrack, you will require the following libraries:

* libsamplerate

And either:

* FFTW (add the flag -DUSE_FFTW)

or:

* Kiss FFT (included with project, use the flag -DUSE_KISS_FFT)

Please ensure that if you are using these libraries that you have secured any required licences for your project. The licence on this library does not cover any third party licences.

License
-------

Copyright (c) 2014 Queen Mary University of London

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.