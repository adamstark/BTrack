#ifndef BTRACK_TESTS
#define BTRACK_TESTS

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <iostream>
#include "../../../src/BTrack.h"

//======================================================================
//==================== CHECKING INITIALISATION =========================
//======================================================================
BOOST_AUTO_TEST_SUITE(checkingInitialisation)

//======================================================================
BOOST_AUTO_TEST_CASE(constructorWithNoArguments)
{
    BTrack b;
    
    BOOST_CHECK_EQUAL(b.getHopSize(), 512);
}

//======================================================================
BOOST_AUTO_TEST_CASE(constructorWithHopSize)
{
    BTrack b(1024);
    
    BOOST_CHECK_EQUAL(b.getHopSize(), 1024);
}

//======================================================================
BOOST_AUTO_TEST_CASE(constructorWithHopSizeAndFrameSize)
{
    BTrack b(256,512);
    
    BOOST_CHECK_EQUAL(b.getHopSize(), 256);
}

BOOST_AUTO_TEST_SUITE_END()
//======================================================================
//======================================================================


//======================================================================
//=================== PROCESSING SIMPLE VALUES =========================
//======================================================================
BOOST_AUTO_TEST_SUITE(processingSimpleValues)

//======================================================================
BOOST_AUTO_TEST_CASE(processZeroValuedOnsetDetectionFunctionSamples)
{
    BTrack b(512);
    
    long numSamples = 20000;
    
    std::vector<double> odfSamples;
    
    int maxInterval = 0;
    int currentInterval = 0;
    int numBeats = 0;
    
    for (int i = 0;i < numSamples;i++)
    {
        b.processOnsetDetectionFunctionSample(0.0);
        
        currentInterval++;
        
        if (b.beatDueInCurrentFrame())
        {
            numBeats++;
            
            if (currentInterval > maxInterval)
            {
                maxInterval = currentInterval;
            }
            
            currentInterval = 0;
        }
    }
    
    // check that the maximum interval between beats does not
    // exceed 100 onset detection function samples (~ 1.3 seconds)
    BOOST_CHECK(maxInterval < 100);
    
    // check that we have at least a beat for every 100 samples
    BOOST_CHECK(numBeats > (numSamples/100));
    
}

//======================================================================
BOOST_AUTO_TEST_CASE(processRandomOnsetDetectionFunctionSamples)
{
    BTrack b(512);
    
    long numSamples = 20000;
    
    std::vector<double> odfSamples;
    
    int maxInterval = 0;
    int currentInterval = 0;
    int numBeats = 0;
    
    for (int i = 0;i < numSamples;i++)
    {
        odfSamples.push_back(random() % 1000);
    }
    
    for (int i = 0;i < numSamples;i++)
    {
        b.processOnsetDetectionFunctionSample(odfSamples[i]);
        
        currentInterval++;
        
        if (b.beatDueInCurrentFrame())
        {
            numBeats++;
            
            if (currentInterval > maxInterval)
            {
                maxInterval = currentInterval;
            }
            
            currentInterval = 0;
        }
    }
    
    // check that the maximum interval between beats does not
    // exceed 100 onset detection function samples (~ 1.3 seconds)
    BOOST_CHECK(maxInterval < 100);
    
    // check that we have at least a beat for every 100 samples
    BOOST_CHECK(numBeats > (numSamples/100));
    
}

//======================================================================
BOOST_AUTO_TEST_CASE(processNegativeOnsetDetectionFunctionSamples)
{
    BTrack b(512);
    
    long numSamples = 20000;
    
    std::vector<double> odfSamples;
    
    int maxInterval = 0;
    int currentInterval = 0;
    int numBeats = 0;
    
    for (int i = 0;i < numSamples;i++)
    {
        odfSamples.push_back(-1.0*(random() % 1000));
    }
    
    for (int i = 0;i < numSamples;i++)
    {
        b.processOnsetDetectionFunctionSample(odfSamples[i]);
        
        currentInterval++;
        
        if (b.beatDueInCurrentFrame())
        {
            numBeats++;
            
            if (currentInterval > maxInterval)
            {
                maxInterval = currentInterval;
            }
            
            currentInterval = 0;
        }
    }
    
    // check that the maximum interval between beats does not
    // exceed 100 onset detection function samples (~ 1.3 seconds)
    BOOST_CHECK(maxInterval < 100);
    
    // check that we have at least a beat for every 100 samples
    BOOST_CHECK(numBeats > (numSamples/100));
    
}

//======================================================================
BOOST_AUTO_TEST_CASE(processSeriesOfDeltaFunctions)
{
    BTrack b(512);
    
    long numSamples = 20000;
    int beatPeriod = 43;
    
    std::vector<double> odfSamples;
    
    int maxInterval = 0;
    int currentInterval = 0;
    int numBeats = 0;
    int correct = 0;
    
    for (int i = 0;i < numSamples;i++)
    {
        if (i % beatPeriod == 0)
        {
            odfSamples.push_back(1000);
        }
        else
        {
            odfSamples.push_back(0.0);
        }
    }
    
    for (int i = 0;i < numSamples;i++)
    {
        b.processOnsetDetectionFunctionSample(odfSamples[i]);
        
        currentInterval++;
        
        if (b.beatDueInCurrentFrame())
        {
            numBeats++;
            
            if (currentInterval > maxInterval)
            {
                maxInterval = currentInterval;
            }
            
            if (currentInterval == beatPeriod)
            {
                correct++;
            }
            
            currentInterval = 0;
        }
    }
    
    // check that the maximum interval between beats does not
    // exceed 100 onset detection function samples (~ 1.3 seconds)
    BOOST_CHECK(maxInterval < 100);
    
    // check that we have at least a beat for every 100 samples
    BOOST_CHECK(numBeats > (numSamples/100));
    
    // check that the number of correct beats is larger than 99%
    // of the total number of beats
    BOOST_CHECK(((double)correct) > (((double)numBeats)*0.99));
}


BOOST_AUTO_TEST_SUITE_END()
//======================================================================
//======================================================================





#endif
