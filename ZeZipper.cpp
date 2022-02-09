//
//  DeZipper.cpp
//  MyFirstPlug_VST
//
//  Created by Bruce Wiggins on 21/10/2020.
//

#include "ZeZipper.h"
DeZipper::DeZipper()
{
    m_DZMM = 0.0;
    m_DZFB = 0.999;     //FB Coef
    m_DZFF = 1 - m_DZFB;  //FF Coef for 0dB passband gain
}
//-----------------------------------------------------
double DeZipper::smooth(double sample)
{
    double temp = m_DZFF * sample + m_DZFB * m_DZMM;
    m_DZMM = temp;

    return temp;
}
