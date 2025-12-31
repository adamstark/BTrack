BTrack Max External: btrack~
============================

Download the latest build
-------------------------

Download latest Max external on the [releases](https://github.com/adamstark/BTrack/releases/latest) page.

This is currently Mac-only.

IMPORTANT: Once you have downloaded the external, in terminal navigate to where the `btrack~.mxo` is and run:

	xattr -d com.apple.quarantine btrack~.mxo 

This will remove any quarantine complaints

Build Instructions
------------------

To build the Max external clone the repo:

	git clone https://github.com/adamstark/BTrack.git

Change to the `btrack~` max external directory:

	cd plugins/max-external/btrack~

Run the build script:

	chmod +x ./build-max-external.sh
	./build-max-external.sh

The resulting external will be:

	plugins/max-external/btrack~/output/btrack~.mxo

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
