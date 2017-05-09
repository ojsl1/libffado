/*
 * Copyright (C) 2013      by Takashi Sakamoto
 * Copyright (C) 2005-2008 by Daniel Wagner
 * Copyright (C) 2005-2008 by Pieter Palmers
 *
 * This file is part of FFADO
 * FFADO = Free Firewire (pro-)audio drivers for linux
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

#ifndef BEBOB_YAMAHA_DEVICE_H
#define BEBOB_YAMAHA_DEVICE_H

#include "debugmodule/debugmodule.h"
#include "bebob/bebob_avdevice.h"

namespace BeBoB {
namespace Yamaha {

class GoDevice : public BeBoB::Device {
public:
    GoDevice( DeviceManager& d, ffado_smartptr<ConfigRom>( configRom ));
    virtual ~GoDevice();

    virtual void showDevice();

    // override these since the go series does not support
    // the usual clock source selection mechanism
    virtual ClockSourceVector getSupportedClockSources();
    virtual bool setActiveClockSource(ClockSource);
    virtual ClockSource getActiveClockSource();

private:
    bool updateClockSources();
    ClockSource m_internal_clocksource;
    ClockSource m_spdif_clocksource;
    ClockSource *m_active_clocksource;
};

} // namespace Yamaha
} // namespace BeBoB

#endif
