#include <iostream>
#include <Python.h>
#include "../../src/OnsetDetectionFunction.h"
#include "../../src/BTrack.h"
#include <numpy/arrayobject.h>

//=======================================================================
static PyObject * btrack_trackBeats(PyObject *dummy, PyObject *args)
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
static PyObject * btrack_calculateOnsetDF(PyObject *dummy, PyObject *args)
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
    { "calculateOnsetDF",btrack_calculateOnsetDF,METH_VARARGS,"Calculate the onset detection function"},
    { "trackBeats",btrack_trackBeats,METH_VARARGS,"Track beats from audio"},
    { "trackBeatsFromOnsetDF",btrack_trackBeatsFromOnsetDF,METH_VARARGS,"Track beats from an onset detection function"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

//=======================================================================
static struct PyModuleDef btrack =
{
    PyModuleDef_HEAD_INIT,
    "btrack",              /* name of module */
    "",                    /* module documentation, may be NULL */
    -1,                    /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    btrack_methods
};

PyMODINIT_FUNC PyInit_btrack(void)
{
    import_array();
    return PyModule_Create(&btrack);
}

//=======================================================================
int main(int argc, char *argv[])
{
    /* Convert argv[0] from char to wchar_t and pass to the Python interpreter */
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    Py_SetProgramName(program);

    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();
    
    /* Add a static module */
    PyInit_btrack();
}