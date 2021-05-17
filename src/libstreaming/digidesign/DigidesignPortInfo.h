/*
 * Copyright (C) 2005-2008, 2011 by Jonathan Woithe
 * Copyright (C) 2005-2008 by Pieter Palmers
 *
 * This file is part of FFADO
 * FFADO = Free FireWire (pro-)audio drivers for Linux
 *
 * FFADO is based upon FreeBoB.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __FFADO_DIGIDESIGNPORTINFO__
#define __FFADO_DIGIDESIGNPORTINFO__

#include "debugmodule/debugmodule.h"
#include <string>

namespace Streaming {
/*!
\brief Class containing the stream information for a Digidesign channel

 Contains the information that enables the decoding routine to find
 this port's data in the ISO events

*/
class DigidesignPortInfo {

public:
    /**
     * Initialize Digidesign portinfo
     * should not be called directly, is inherited by digidesign ports
     *
     * the position parameter is an example
     * the name parameter is mandatory
     *
     * @param position Start position of port's data in iso event
     * @param format Format of data in iso event
     * @param size Size in bits of port's data in iso event
     * @return
     */
    DigidesignPortInfo( int position, int size)
      : m_position(position), m_size(size)
    {};
    virtual ~DigidesignPortInfo() {};


    int getPosition()     {return m_position;};
    int getSize()         {return m_size;};

protected:

    int m_position;
    int m_size;

};

} // end of namespace Streaming

#endif /* __FFADO_DIGIDESIGNPORTINFO__ */
