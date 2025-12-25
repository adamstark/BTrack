#define PY_SSIZE_T_CLEAN
#include <iostream>
#include <Python.h>
#include <numpy/arrayobject.h>
#include "OnsetDetectionFunction.h"
#include "BTrack.h"

//=======================================================================
static PyObject * btrack_trackBeats(PyObject *dummy, PyObject *args)
{
    PyObject *arg1=NULL;
    
    if (! PyArg_ParseTuple(args, "O", &arg1))
        return NULL;
    
    PyArrayObject* arr1 = (PyArrayObject*) PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);

    if (! arr1)
        return NULL;
    
    ////////// GET INPUT DATA ///////////////////
    
    // get data as array
    double* data = static_cast<double*>(PyArray_DATA(arr1));
    
    // get array size
    long signal_length = PyArray_Size((PyObject*)arr1);
    
    
    ////////// BEGIN PROCESS ///////////////////
    int hopSize = 512;
    int frameSize = 1024;
    
    int numframes;
    double buffer[hopSize];	// buffer to hold one hopsize worth of audio samples
    
    
    // get number of audio frames, given the hop size and signal length
	numframes = (int) floor(((double) signal_length) / ((double) hopSize));
    
    
    BTrack b(hopSize,frameSize);
    
    
    double beats[5000];
    int beatnum = 0;
    
    ///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (int i=0;i < numframes;i++)
	{
		// add new samples to frame
		for (int n = 0;n < hopSize;n++)
		{
			buffer[n] = data[(i*hopSize)+n];
		}
		
        // process the current audio frame
        b.processAudioFrame(buffer);
        
        // if a beat is currently scheduled
		if (b.beatDueInCurrentFrame())
		{
			beats[beatnum] = BTrack::getBeatTimeInSeconds(i,hopSize,44100);
            beatnum = beatnum + 1;
		}
		
	}
	
	///////// End Processing Loop /////////////
	///////////////////////////////////////////
    
    
    ////////// END PROCESS ///////////////////
    
    double beats_out[beatnum];          // create output array
    
    // copy beats into output array
    for (int i = 0;i < beatnum;i++)
    {
        beats_out[i] = beats[i];
    }
    
    
    
    ////////// CREATE ARRAY AND RETURN IT ///////////////////
    int nd=1;
    npy_intp m= beatnum;
    //double fArray[5] = {0,1,2,3,4};
    
    PyObject* c=PyArray_SimpleNew(nd, &m, NPY_DOUBLE);
    
    void *arr_data = PyArray_DATA((PyArrayObject*)c);
    
    memcpy(arr_data, beats_out, PyArray_ITEMSIZE((PyArrayObject*) c) * m);
    
    
    Py_DECREF(arr1);
    Py_INCREF(Py_None);
    //return Py_None;
    
    return (PyObject *)c;
}


//=======================================================================
static PyObject * btrack_calculateOnsetDF(PyObject *dummy, PyObject *args)
{
    PyObject *arg1=NULL;
    
    if (! PyArg_ParseTuple(args, "O", &arg1)) 
        return NULL;
    
    PyArrayObject* arr1 = (PyArrayObject*) PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
    if (! arr1) 
        return NULL;
    
    ////////// GET INPUT DATA ///////////////////
    
    // get data as array
    double* data = (double*) PyArray_DATA(arr1);
    
    // get array size
    long signal_length = PyArray_Size((PyObject*)arr1);
    
    ////////// BEGIN PROCESS ///////////////////
    int hopSize = 512;
    int frameSize = 1024;
    int df_type = 6;
    int numframes;
    double buffer[hopSize];	// buffer to hold one hopsize worth of audio samples

    
    // get number of audio frames, given the hop size and signal length
	numframes = (int) floor(((double) signal_length) / ((double) hopSize));
    
    OnsetDetectionFunction onset(hopSize,frameSize,df_type,1);

    double df[numframes];
    

    
    ///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (int i=0;i < numframes;i++)
	{		
		// add new samples to frame
		for (int n = 0;n < hopSize;n++)
		{
			buffer[n] = data[(i*hopSize)+n];
		}
		
		df[i] = onset.calculateOnsetDetectionFunctionSample(buffer);
		
	}
	
	///////// End Processing Loop /////////////
	///////////////////////////////////////////
    
    

    
    ////////// CREATE ARRAY AND RETURN IT ///////////////////
    int nd=1;
    npy_intp m= numframes;

    
    PyObject* c=PyArray_SimpleNew(nd, &m, NPY_DOUBLE);
    
    void *arr_data = PyArray_DATA((PyArrayObject*)c);
        
    memcpy(arr_data, df, PyArray_ITEMSIZE((PyArrayObject*) c) * m); 
    
     
    Py_DECREF(arr1);  
    Py_INCREF(Py_None); 
    //return Py_None;
    
    return (PyObject *)c;
}


//=======================================================================
static PyObject * btrack_trackBeatsFromOnsetDF(PyObject *dummy, PyObject *args)
{
    PyObject *arg1=NULL;
    
    if (! PyArg_ParseTuple(args, "O", &arg1)) 
        return NULL;
    
    PyArrayObject* arr1 = (PyArrayObject*) PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
    
    if (! arr1)
        return NULL;
    
    ////////// GET INPUT DATA ///////////////////
    
    // get data as array
    double* data = (double*) PyArray_DATA(arr1);
    
    // get array size
    long numframes = PyArray_Size((PyObject*)arr1);

    ////////// BEGIN PROCESS ///////////////////
    int hopSize = 512;
    int frameSize = 2*hopSize;

    BTrack b(hopSize,frameSize);
    
    double beats[5000];
    int beatnum = 0;
    double df_val;
    
    ///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (long i=0;i < numframes;i++)
	{		
        df_val = data[i] + 0.0001;
        
		b.processOnsetDetectionFunctionSample(df_val);				// process df sample in beat tracker
		
		if (b.beatDueInCurrentFrame())
		{
            beats[beatnum] = BTrack::getBeatTimeInSeconds(i,hopSize,44100);
			beatnum = beatnum + 1;	
		}
		
	}
	
	///////// End Processing Loop /////////////
	///////////////////////////////////////////
    
    
    ////////// END PROCESS ///////////////////
    
    double beats_out[beatnum];          // create output array
    
    
    // copy beats into output array
    for (int i = 0;i < beatnum;i++)     
    {
        beats_out[i] = beats[i];
    }
    
    
    ////////// CREATE ARRAY AND RETURN IT ///////////////////
    int nd=1;
    npy_intp m= beatnum;
    //double fArray[5] = {0,1,2,3,4};
    
    PyObject* c=PyArray_SimpleNew(nd, &m, NPY_DOUBLE);
    
    void *arr_data = PyArray_DATA((PyArrayObject*)c);
    
    memcpy(arr_data, beats_out, PyArray_ITEMSIZE((PyArrayObject*) c) * m); 
    
    
    Py_DECREF(arr1);  
    Py_INCREF(Py_None); 
    //return Py_None;
    
    return (PyObject *)c;
}

//=======================================================================
static PyMethodDef btrack_methods[] = {
    { "calculate_onset_detection_function", btrack_calculateOnsetDF, METH_VARARGS, "Calculate the onset detection function"},
    { "detect_beats", btrack_trackBeats, METH_VARARGS, "Detect beats from audio"},
    { "detect_beats_from_odf", btrack_trackBeatsFromOnsetDF, METH_VARARGS, "Detect beats from an onset detection function"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

//=======================================================================
static struct PyModuleDef btrack_definition = {
    PyModuleDef_HEAD_INIT,
    "btrack",
    "Python bindings for the BTrack beat tracker",
    -1,
    btrack_methods
};

//=======================================================================
PyMODINIT_FUNC PyInit_btrack(void)
{
    import_array();
    PyObject* m = PyModule_Create(&btrack_definition);

    PyModule_AddStringConstant(m, "__version__", BTRACK_VERSION);
    return m;
}
