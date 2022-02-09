//
//  DeZipper.hpp
//  MyFirstPlug_VST
//
//  Created by Bruce Wiggins on 21/10/2020.
//

#ifndef DeZip_hpp
#define DeZip_hpp

#include <stdio.h>

class DeZipper
{
public:
    DeZipper();
    double smooth(double sample);
private:
    float m_DZMM;   //single sample delay memory
    float m_DZFB;   //Feed Back Coef
    float m_DZFF;   //Feed Forward Coef
};

#endif /* DeZip_hpp */

