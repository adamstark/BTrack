BTrack Max External: btrack~
============================


Build Instructions
------------------

1. Edit the file maxmspsdk.xcconfig, setting the path to the c74support folder in your version of the Max SDK:


	// ===========================================================================
	// NOTE: SET PATH TO YOUR C74SUPPORT FOLDER IN YOUR MAX SDK HERE
	
	C74SUPPORT = $(SRCROOT)/../../../SDKs/MaxSDK-6.1.4/c74support/
	
	// ===========================================================================			