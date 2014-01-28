
// This is a skeleton file for use in creating your own plugin
// libraries.  Replace BTrackVamp and myPlugin throughout with the name
// of your first plugin class, and fill in the gaps as appropriate.


#include "BTrackVamp.h"


BTrackVamp::BTrackVamp(float inputSampleRate) :
    Plugin(inputSampleRate)
    // Also be sure to set your plugin parameters (presumably stored
    // in member variables) to their default values here -- the host
    // will not do that for you
{
}

BTrackVamp::~BTrackVamp()
{
}

string
BTrackVamp::getIdentifier() const
{
    return "btrack-vamp";
}

string
BTrackVamp::getName() const
{
    return "BTrack";
}

string
BTrackVamp::getDescription() const
{
    // Return something helpful here!
    return "A Real-Time Beat Tracker";
}

string
BTrackVamp::getMaker() const
{
    // Your name here
    return "Adam Stark, Matthew Davies and Mark Plumbley";
}

int
BTrackVamp::getPluginVersion() const
{
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string
BTrackVamp::getCopyright() const
{
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "";
}

BTrackVamp::InputDomain
BTrackVamp::getInputDomain() const
{
    return TimeDomain;
}

size_t
BTrackVamp::getPreferredBlockSize() const
{
    return 1024; // 0 means "I can handle any block size"
}

size_t 
BTrackVamp::getPreferredStepSize() const
{
    return 512; // 0 means "anything sensible"; in practice this
              // means the same as the block size for TimeDomain
              // plugins, or half of it for FrequencyDomain plugins
}

size_t
BTrackVamp::getMinChannelCount() const
{
    return 1;
}

size_t
BTrackVamp::getMaxChannelCount() const
{
    return 1;
}

BTrackVamp::ParameterList
BTrackVamp::getParameterDescriptors() const
{
    ParameterList list;

    // If the plugin has no adjustable parameters, return an empty
    // list here (and there's no need to provide implementations of
    // getParameter and setParameter in that case either).

    // Note that it is your responsibility to make sure the parameters
    // start off having their default values (e.g. in the constructor
    // above).  The host needs to know the default value so it can do
    // things like provide a "reset to default" function, but it will
    // not explicitly set your parameters to their defaults for you if
    // they have not changed in the mean time.

//    ParameterDescriptor d;
//    d.identifier = "parameter";
//    d.name = "Some Parameter";
//    d.description = "";
//    d.unit = "";
//    d.minValue = 0;
//    d.maxValue = 10;
//    d.defaultValue = 5;
//    d.isQuantized = false;
//    list.push_back(d);

    return list;
}

float
BTrackVamp::getParameter(string identifier) const
{
    if (identifier == "parameter") {
        return 5; // return the ACTUAL current value of your parameter here!
    }
    return 0;
}

void
BTrackVamp::setParameter(string identifier, float value) 
{
    if (identifier == "parameter") {
        // set the actual value of your parameter
    }
}

BTrackVamp::ProgramList
BTrackVamp::getPrograms() const
{
    ProgramList list;

    // If you have no programs, return an empty list (or simply don't
    // implement this function or getCurrentProgram/selectProgram)

    return list;
}

string
BTrackVamp::getCurrentProgram() const
{
    return ""; // no programs
}

void
BTrackVamp::selectProgram(string name)
{
}

BTrackVamp::OutputList
BTrackVamp::getOutputDescriptors() const
{
    OutputList list;

    // See OutputDescriptor documentation for the possibilities here.
    // Every plugin must have at least one output.
    
    OutputDescriptor d;
    d.identifier = "beats";
    d.name = "Beats";
    d.description = "Beat locations";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 0;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.sampleRate = m_inputSampleRate;
    list.push_back(d);

    return list;
}

bool
BTrackVamp::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    
    m_stepSize = stepSize;
    m_blockSize = blockSize;
    
    b.updateHopAndFrameSize(m_stepSize,m_blockSize);
    

    return true;
}

void
BTrackVamp::reset()
{
    // Clear buffers, reset stored values, etc
}

BTrackVamp::FeatureSet
BTrackVamp::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    // create an array to hold our audio frame
    double frame[m_blockSize];
    
    // copy samples into our frame
    for (int i = 0;i < m_blockSize;i++)
    {
        frame[i] = (double) inputBuffers[0][i];
    }
    
    // process the frame in the beat tracker
    b.processAudioFrame(frame);
    
    // create a FeatureSet
    FeatureSet featureSet;
    
    // if there is a beat in this frame
    if (b.beatDueInCurrentFrame())
    {
        // add a beat to the FeatureSet
        Feature beat;
        beat.hasTimestamp = true;
        beat.timestamp = timestamp - Vamp::RealTime::frame2RealTime(m_stepSize, int(m_inputSampleRate + 0.5));
        featureSet[0].push_back(beat);
    }
    
    // return the feature set
    return featureSet;
}

BTrackVamp::FeatureSet
BTrackVamp::getRemainingFeatures()
{
    return FeatureSet();
}

