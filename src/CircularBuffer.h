//=======================================================================
/** @file CircularBuffer.h
 *  @brief A circular buffer
 *  @author Adam Stark
 *  @copyright Copyright (C) 2008-2014  Queen Mary University of London
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//=======================================================================

#ifndef CircularBuffer_h
#define CircularBuffer_h

#include <vector>

//=======================================================================
/** A circular buffer that allows you to add new samples to the end
 * whilst removing them from the beginning. This is implemented in an
 * efficient way which doesn't involve any memory allocation as samples
 * are added to the end of the buffer
 */
class CircularBuffer
{
public:
    
    /** Constructor */
    CircularBuffer()
     :  writeIndex (0)
    {
        
    }
    
    /** Access the ith element in the buffer */
    double &operator[] (int i)
    {
        int index = (i + writeIndex) % buffer.size();
        return buffer[index];
    }
    
    /** Add a new sample to the end of the buffer */
    void addSampleToEnd (double v)
    {
        buffer[writeIndex] = v;
        writeIndex = (writeIndex + 1) % buffer.size();
    }
    
    /** Resize the buffer */
    void resize (int size)
    {
        buffer.resize (size);
        std::fill (buffer.begin(), buffer.end(), 0.0);
        writeIndex = 0;
    }
    
    /** Returns the size of the buffer */
    int size()
    {
        return static_cast<int> (buffer.size());
    }
    
private:
    
    std::vector<double> buffer;
    int writeIndex;
};

#endif /* CircularBuffer_hpp */
