#include <iostream>
#include <Python.h>
#include "../../src/OnsetDetectionFunction.h"
#include "../../src/BTrack.h"
#include <numpy/arrayobject.h>

//=======================================================================
static PyObject * btrack_onsetdf(PyObject *dummy, PyObject *args) 
{
    PyObject *arg1=NULL;
    PyObject *arr1=NULL;
    
    if (!PyArg_ParseTuple(args, "O", &arg1)) 
    {
        return NULL;
    }
    
    arr1 = PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_IN_ARRAY); 
    if (arr1 == NULL) 
    {
        return NULL;
    }


    
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
    
    
    ////////// END PROCESS ///////////////////
    
    
    
    ////////// CREATE ARRAY AND RETURN IT ///////////////////
    int nd=1;
    npy_intp m= numframes;
    //double fArray[5] = {0,1,2,3,4};
    
    PyObject* c=PyArray_SimpleNew(nd, &m, NPY_DOUBLE);
    
    void *arr_data = PyArray_DATA((PyArrayObject*)c);
        
    memcpy(arr_data, df, PyArray_ITEMSIZE((PyArrayObject*) c) * m); 
    
     
    Py_DECREF(arr1);  
    Py_INCREF(Py_None); 
    //return Py_None;
    
    return (PyObject *)c;
}

//=======================================================================
static PyObject * btrack_btrack(PyObject *dummy, PyObject *args) 
{
    PyObject *arg1=NULL;
    PyObject *arr1=NULL;
    
    if (!PyArg_ParseTuple(args, "O", &arg1)) 
    {
        return NULL;
    }
    
    arr1 = PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_IN_ARRAY); 
    if (arr1 == NULL) 
    {
        return NULL;
    }
    
    
    
    ////////// GET INPUT DATA ///////////////////
    
    // get data as array
    double* data = (double*) PyArray_DATA(arr1);
    
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
static PyObject * btrack_btrack_df(PyObject *dummy, PyObject *args) 
{
    PyObject *arg1=NULL;
    PyObject *arr1=NULL;
    
    if (!PyArg_ParseTuple(args, "O", &arg1)) 
    {
        return NULL;
    }
    
    arr1 = PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_IN_ARRAY); 
    if (arr1 == NULL) 
    {
        return NULL;
    }
    
    
    
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
    { "onsetdf",btrack_onsetdf,METH_VARARGS,"onset detection function"},
    { "btrack",btrack_btrack,METH_VARARGS,"beat tracker"},
    { "btrack_df",btrack_btrack_df,METH_VARARGS,"beat tracker with detection function input"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

//=======================================================================
PyMODINIT_FUNC initbtrack(void)
{
    (void)Py_InitModule("btrack", btrack_methods);
    import_array();
}

//=======================================================================
int main(int argc, char *argv[])
{
    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(argv[0]);
    
    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();
    
    /* Add a static module */
    initbtrack();
}