#define PY_SSIZE_T_CLEAN
#include <iostream>
#include <vector>
#include <Python.h>
#include <numpy/arrayobject.h>
#include "OnsetDetectionFunction.h"
#include "BTrack.h"

//=======================================================================
static PyObject* detectBeats (PyObject* dummy, PyObject* args)
{
    PyObject* inputObject = nullptr;
    
    if (! PyArg_ParseTuple (args, "O", &inputObject))
        return nullptr;
    
    PyArrayObject* inputArray = (PyArrayObject*) PyArray_FROM_OTF (inputObject, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);

    if (! inputArray)
        return nullptr;
        
    const double* audioSampleArray = static_cast<double*> (PyArray_DATA (inputArray));
    long signalLength = PyArray_Size ((PyObject*)inputArray);
    constexpr int hopSize = 512;
    constexpr int frameSize = 1024;
    constexpr int sampleRate = 44100;
    int numFrames = signalLength / hopSize;
    
    std::vector<double> buffer (hopSize); // buffer to hold one hopsize worth of audio samples
    
    BTrack b (hopSize, frameSize);

    std::vector<double> beats;
    beats.reserve (numFrames);
	
	for (int i = 0; i < numFrames; i++)
	{
        std::copy_n (audioSampleArray + (i * hopSize), hopSize, buffer.begin());
		
        b.processAudioFrame (buffer.data()); // process the current audio frame
        
        // if a beat is currently scheduled, store the time
		if (b.beatDueInCurrentFrame())
            beats.push_back (BTrack::getBeatTimeInSeconds (i, hopSize, sampleRate));
	}
        
    npy_intp dims = static_cast<npy_intp> (beats.size());
    PyObject* outputArray = PyArray_SimpleNew (1, &dims, NPY_DOUBLE);
    double* out = static_cast<double*> (PyArray_DATA ((PyArrayObject*)outputArray));
    std::copy (beats.begin(), beats.end(), out);
    
    Py_DECREF (inputArray);
    return outputArray;
}

//=======================================================================
static PyObject* calculateOnsetDetectionFunction (PyObject* dummy, PyObject* args)
{
    PyObject *inputObject = nullptr;
    
    if (! PyArg_ParseTuple (args, "O", &inputObject)) 
        return nullptr;
    
    PyArrayObject* inputArray = (PyArrayObject*) PyArray_FROM_OTF (inputObject, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);

    if (! inputArray) 
        return nullptr;
    
    const double* audioSampleArray = static_cast<double*> (PyArray_DATA (inputArray));
    long signalLength = PyArray_Size ((PyObject*)inputArray);
    constexpr int hopSize = 512;
    constexpr int frameSize = 1024;
    int onsetDetectionFunctionType = 6;

    std::vector<double> buffer (hopSize); // buffer to hold one hopsize worth of audio samples

    int numFrames = signalLength / hopSize;
    
    OnsetDetectionFunction onset (hopSize, frameSize, onsetDetectionFunctionType, 1);

    std::vector<double> odf (numFrames);
	
	for (int i = 0; i < numFrames; i++)
	{		
        std::copy_n (audioSampleArray + (i * hopSize), hopSize, buffer.begin());
		odf[i] = onset.calculateOnsetDetectionFunctionSample (buffer.data());	
	}

    npy_intp dims = static_cast<npy_intp> (numFrames);
    PyObject* outputArray = PyArray_SimpleNew (1, &dims, NPY_DOUBLE);
    double* out = static_cast<double*> (PyArray_DATA ((PyArrayObject*)outputArray));
    std::copy (odf.begin(), odf.end(), out);
     
    Py_DECREF(inputArray);      
    return outputArray;
}

//=======================================================================
static PyObject* detectBeatsFromOnsetDetectionFunction (PyObject* dummy, PyObject* args)
{
    PyObject* inputObject = nullptr;
    
    if (! PyArg_ParseTuple (args, "O", &inputObject)) 
        return nullptr;
    
    PyArrayObject* inputArray = (PyArrayObject*) PyArray_FROM_OTF (inputObject, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
    
    if (! inputArray)
        return nullptr;
        
    const double* odfArray = static_cast<double*> (PyArray_DATA(inputArray));
    long numFrames = PyArray_Size((PyObject*)inputArray);
    constexpr int hopSize = 512;
    constexpr int frameSize = 1024;
    constexpr int sampleRate = 44100;
    constexpr double epsilon = 1e-4;

    BTrack b (hopSize, frameSize);
    
    std::vector<double> beats;
    beats.reserve (numFrames);

	for (long i = 0; i < numFrames; i++)
	{	
		b.processOnsetDetectionFunctionSample (odfArray[i] + epsilon);
		
		if (b.beatDueInCurrentFrame())
            beats.push_back (BTrack::getBeatTimeInSeconds (i, hopSize, sampleRate));
	}
    
    npy_intp dims = static_cast<npy_intp> (beats.size());
    PyObject* outputArray = PyArray_SimpleNew (1, &dims, NPY_DOUBLE);
    double* out = static_cast<double*> (PyArray_DATA ((PyArrayObject*)outputArray));
    std::copy (beats.begin(), beats.end(), out);
    
    Py_DECREF(inputArray);      
    return outputArray;
}

//=======================================================================
static PyMethodDef btrack_methods[] = {
    { "calculate_onset_detection_function", calculateOnsetDetectionFunction, METH_VARARGS, "Calculate the onset detection function"},
    { "detect_beats", detectBeats, METH_VARARGS, "Detect beats from audio"},
    { "detect_beats_from_odf", detectBeatsFromOnsetDetectionFunction, METH_VARARGS, "Detect beats from an onset detection function"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

//=======================================================================
static struct PyModuleDef btrack_definition = {
    PyModuleDef_HEAD_INIT,
    "btrack_beat_tracker",
    "Python bindings for the BTrack beat tracker",
    -1,
    btrack_methods
};

//=======================================================================
PyMODINIT_FUNC PyInit_btrack_beat_tracker(void)
{
    import_array();
    PyObject* m = PyModule_Create(&btrack_definition);
    PyModule_AddStringConstant(m, "__version__", BTRACK_VERSION);
    return m;
}
