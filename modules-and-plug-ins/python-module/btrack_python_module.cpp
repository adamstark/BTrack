#include <iostream>
#include <Python.h>
#include "../../src/OnsetDetectionFunction.h"
#include "../../src/BTrack.h"
#include <numpy/arrayobject.h>

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
    //int k = (int) theSize;
    
    // get data type 
    //char type = PyArray_DESCR(arr1)->type;
    
    ////////// BEGIN PROCESS ///////////////////
    int hsize = 512;
    int fsize = 1024;
    int df_type = 6;
    int numframes;
    double buffer[hsize];	// buffer to hold one hopsize worth of audio samples

    
    // get number of audio frames, given the hop size and signal length
	numframes = (int) floor(((double) signal_length) / ((double) hsize));
    
    OnsetDetectionFunction onset(hsize,fsize,df_type,1);

    double df[numframes];
    

    
    ///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (int i=0;i < numframes;i++)
	{		
		// add new samples to frame
		for (int n = 0;n < hsize;n++)
		{
			buffer[n] = data[(i*hsize)+n];
		}
		
		df[i] = onset.getDFsample(buffer);
		
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
    
    //return Py_BuildValue("c", type);
    //return Py_BuildValue("d", sum);
    //return Py_BuildValue("i", k);
/*    
fail:
    Py_XDECREF(arr1); 
    Py_XDECREF(arr2); 
    PyArray_XDECREF_ERR(oarr); 
    return NULL;*/
}


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
    //int k = (int) theSize;
    
    // get data type 
    //char type = PyArray_DESCR(arr1)->type;
    
    ////////// BEGIN PROCESS ///////////////////
    int hsize = 512;
    int fsize = 1024;
    int df_type = 6;
    int numframes;
    double buffer[hsize];	// buffer to hold one hopsize worth of audio samples
    
    
    // get number of audio frames, given the hop size and signal length
	numframes = (int) floor(((double) signal_length) / ((double) hsize));
    
    OnsetDetectionFunction onset(hsize,fsize,df_type,1);
    BTrack b;
    
    b.initialise((int) hsize);	// initialise beat tracker
	
	// set parameters
    //b.setparams(0.9,5);
    
    double df[numframes];
    double beats[5000];
    int beatnum = 0;
    float df_val;
    
    ///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (int i=0;i < numframes;i++)
	{		
		// add new samples to frame
		for (int n = 0;n < hsize;n++)
		{
			buffer[n] = data[(i*hsize)+n];
		}
		
		df[i] = onset.getDFsample(buffer);
        
        df_val = (float) (df[i] + 0.0001);
                
		b.process(df_val);				// process df sample in beat tracker
		
		if (b.playbeat == 1)
		{
			beats[beatnum] = (((double) hsize) / 44100) * ((double) i);
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
    
    //return Py_BuildValue("c", type);
    //return Py_BuildValue("d", sum);
    //return Py_BuildValue("i", k);
    /*    
     fail:
     Py_XDECREF(arr1); 
     Py_XDECREF(arr2); 
     PyArray_XDECREF_ERR(oarr); 
     return NULL;*/
}

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
    //int k = (int) theSize;
    
    // get data type 
    //char type = PyArray_DESCR(arr1)->type;
    
    ////////// BEGIN PROCESS ///////////////////
    int hsize = 512;

    BTrack b;
    
    b.initialise((int) hsize);	// initialise beat tracker
	
	// set parameters
    //b.setparams(0.9,5);
    
    double beats[5000];
    int beatnum = 0;
    float df_val;
    
    ///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (long i=0;i < numframes;i++)
	{		
        df_val = (float) (data[i] + 0.0001);
        
		b.process(df_val);				// process df sample in beat tracker
		
		if (b.playbeat == 1)
		{
			beats[beatnum] = (((double) hsize) / 44100) * ((double) i);
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
    
    //return Py_BuildValue("c", type);
    //return Py_BuildValue("d", sum);
    //return Py_BuildValue("i", k);
    /*    
     fail:
     Py_XDECREF(arr1); 
     Py_XDECREF(arr2); 
     PyArray_XDECREF_ERR(oarr); 
     return NULL;*/
}



static PyMethodDef btrack_methods[] = {
    { "onsetdf",btrack_onsetdf,METH_VARARGS,"onset detection function"},
    { "btrack",btrack_btrack,METH_VARARGS,"beat tracker"},
    { "btrack_df",btrack_btrack_df,METH_VARARGS,"beat tracker with detection function input"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

PyMODINIT_FUNC initbtrack(void)
{
    (void)Py_InitModule("btrack", btrack_methods);
    import_array();
}

int main(int argc, char *argv[])
{
    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(argv[0]);
    
    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();
    
    /* Add a static module */
    initbtrack();
}