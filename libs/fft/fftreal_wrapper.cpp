/***************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as
** published by the Free Software Foundation, either version 2.1. This
** program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
** for more details. You should have received a copy of the GNU General
** Public License along with this program. If not, see
** <http://www.gnu.org/licenses/>.
**
***************************************************************************/
#include "fftreal_wrapper.h"


FFTRealWrapper::FFTRealWrapper()
    :   m_private(new FFTRealWrapperPrivate)
{
}

FFTRealWrapper::~FFTRealWrapper()
{
    delete m_private;
}

void FFTRealWrapper::calculateFFT(float out[], float in[])
{
    m_private->m_fft.do_fft(out, in);
}

void FFTRealWrapper::calculateInverseFFT(float in[], float out[])
{
    m_private->m_fft.do_ifft(in,out);
}

void FFTRealWrapper::rescale(float data[])
{
    m_private->m_fft.rescale(data);
}
