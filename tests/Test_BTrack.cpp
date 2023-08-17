#include "doctest.h"
#include <BTrack.h>

//======================================================================
//==================== CHECKING INITIALISATION =========================
//======================================================================
TEST_SUITE ("checkingInitialisation")
{
    //======================================================================
    TEST_CASE ("constructorWithNoArguments")
    {
        BTrack b;
        
        CHECK_EQ (b.getHopSize(), 512);
    }

    //======================================================================
    TEST_CASE("constructorWithHopSize")
    {
        BTrack b(1024);
        
        CHECK_EQ (b.getHopSize(), 1024);
    }

    //======================================================================
    TEST_CASE("constructorWithHopSizeAndFrameSize")
    {
        BTrack b(256,512);
        
        CHECK_EQ (b.getHopSize(), 256);
    }

}

//======================================================================
//=================== PROCESSING SIMPLE VALUES =========================
//======================================================================

TEST_SUITE("processingSimpleValues")
{
    //======================================================================
    TEST_CASE ("processZeroValuedOnsetDetectionFunctionSamples")
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
        CHECK (maxInterval < 100);
        
        // check that we have at least a beat for every 100 samples
        CHECK (numBeats > (numSamples/100));
        
    }

    //======================================================================
    TEST_CASE ("processRandomOnsetDetectionFunctionSamples")
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
        CHECK(maxInterval < 100);
        
        // check that we have at least a beat for every 100 samples
        CHECK(numBeats > (numSamples/100));
        
    }

    //======================================================================
    TEST_CASE ("processNegativeOnsetDetectionFunctionSamples")
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
        CHECK (maxInterval < 100);
        
        // check that we have at least a beat for every 100 samples
        CHECK (numBeats > (numSamples/100));
        
    }

    //======================================================================
    TEST_CASE ("processSeriesOfDeltaFunctions")
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
        CHECK (maxInterval < 100);
        
        // check that we have at least a beat for every 100 samples
        CHECK (numBeats > (numSamples/100));
        
        // check that the number of correct beats is larger than 99%
        // of the total number of beats
        CHECK (((double)correct) > (((double)numBeats)*0.99));
    }
}