BTrack Max External: btrack~
============================


Build Instructions
------------------

Edit the file maxmspsdk.xcconfig in this directory, setting the path to the c74support folder in your version of the Max SDK:


	C74SUPPORT = $(SRCROOT)/../../../SDKs/MaxSDK-6.1.4/c74support/
	
	
Also, to compile BTrack, you will require the following libraries:

* FFTW
* libsamplerate

Documentation
-------------

Please see the following two files for examples of how to use BTrack in Max:

* btrack~.maxhelp
* Beat Tracking Example.maxpat

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