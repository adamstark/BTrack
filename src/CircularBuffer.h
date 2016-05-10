//
//  CircularBuffer.hpp
//  BTrack Tests
//
//  Created by Adam Stark on 04/02/2016.
//  Copyright © 2016 Adam Stark. All rights reserved.
//

#ifndef CircularBuffer_h
#define CircularBuffer_h

#include <vector>

class CircularBuffer
{
public:
    CircularBuffer() : writeIndex (0)
    {
        
    }
    
    double &operator[](int i)
    {
        int index = (i + writeIndex) % buffer.size();
        return buffer[index];
    }
        
    void addSampleToEnd (double v)
    {
        buffer[writeIndex] = v;
        writeIndex = (writeIndex + 1) % buffer.size();
    }
    
    void resize(int size)
    {
        buffer.resize (size);
        writeIndex = 0;
    }
    
private:
    
    std::vector<double> buffer;
    int writeIndex;
};

#endif /* CircularBuffer_hpp */
