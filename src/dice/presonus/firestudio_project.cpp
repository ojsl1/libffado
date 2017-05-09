/*
 * Copyright (C) 2009 by Pieter Palmers
 * Copyright (C) 2012 by Philippe Carriere
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

#include "firestudio_project.h"

namespace Dice {
namespace Presonus {

//
// Firestudio Project has
//  - 8 mic/line inputs
//  - 2 SPDIF inputs
//  - 10 ieee1394 inputs
//  - 18 mixer inputs
//
//  - 8 analogic line outputs
//  - 2 SPDIF outputs
//  - 10 ieee1394 outputs
//  - 16 mixer outputs
//
void FirestudioProject::FirestudioProjectEAP::setupSources_low() {
    addSource("SPDIF/In",  2,  2, eRS_AES, 1);
    addSource("Mic/Inst/In", 0,  2, eRS_InS0, 1);
    addSource("Mic/Lin/In", 2,  6, eRS_InS0, 3);
    addSource("Mixer/Out",  0, 16, eRS_Mixer, 1);
    addSource("1394/In",   0, 10, eRS_ARX0, 1);
    addSource("Mute",   0,  1, eRS_Muted);
}

void FirestudioProject::FirestudioProjectEAP::setupDestinations_low() {
    addDestination("SPDIF/Out",  2,  2, eRD_AES, 1);
    addDestination("Line/Out", 0,  8, eRD_InS0, 1);
    addDestination("Mixer/In",  0, 16, eRD_Mixer0, 1);
    addDestination("Mixer/In",  0,  2, eRD_Mixer1, 17);
    addDestination("1394/Out",   0, 10, eRD_ATX0, 1);
// Is a Mute destination useful ?
//    addDestination("Mute",   0,  1, eRD_Muted);
}

//
// Independent of samplerate
void FirestudioProject::FirestudioProjectEAP::setupSources_mid() {
    setupSources_low();
}

void FirestudioProject::FirestudioProjectEAP::setupDestinations_mid() {
    setupDestinations_low();
}

//
// 192 kHz is not supported
//
void FirestudioProject::FirestudioProjectEAP::setupSources_high() {
    printMessage("High (192 kHz) sample rate not supported by Firestudio Tube\n");
}

void FirestudioProject::FirestudioProjectEAP::setupDestinations_high() {
    printMessage("High (192 kHz) sample rate not supported by Firestudio Tube\n");
}

/**
 * The default configuration for the Firestudio Project.
 * 82 destinations; each "group" every 32 registers
 **FIXME What follows is extracted from a listing of an existing router configuration.
 *       However, the origin of such a router configuration was unknown.
 */
void
FirestudioProject::FirestudioProjectEAP::setupDefaultRouterConfig_low() {
    unsigned int i;
    // the 1394 stream receivers
    for (i=0; i<8; i++) {
        addRoute(eRS_InS0, i, eRD_ATX0, i);
    }
    for (i=0; i<2; i++) {
        addRoute(eRS_AES, i+2, eRD_ATX0, i+8);
    }
    // Then 22 muted destinations
    for (i=0; i<22; i++) {
        addRoute(eRS_Muted, 0, eRD_Muted, 0);
    }
    
    // the Mixer inputs
    for (i=0; i<8; i++) {
        addRoute(eRS_InS0, i, eRD_Mixer0, i);
    }
    for (i=0; i<2; i++) {
        addRoute(eRS_AES, i+2, eRD_Mixer0, i+8);
    }
    for (i=0; i<6; i++) {
        addRoute(eRS_ARX0, i, eRD_Mixer0, i+10);
    }
    for (i=0; i<2; i++) {
        addRoute(eRS_ARX0, i+6, eRD_Mixer1, i);
    }
    // Then 14 muted destinations
    for (i=0; i<14; i++) {
        addRoute(eRS_Muted, 0, eRD_Muted, 0);
    }

    // The audio ports
    // Ensure that audio port are not muted
    for (i=0; i<8; i++) {
        addRoute(eRS_ARX0, i, eRD_InS0, i);
    }
    // The SPDIF ports
    for (i=0; i<2; i++) {
        addRoute(eRS_ARX0, i+8, eRD_AES, i+2);
    }
    // Then 8 muted destinations
    for (i=0; i<8; i++) {
        addRoute(eRS_Muted, 0, eRD_Muted, 0);
    }  
}

/**
 *  Identical to mid-rate
 */
void
FirestudioProject::FirestudioProjectEAP::setupDefaultRouterConfig_mid() {
    setupDefaultRouterConfig_low();
}

/**
 *  High rate not supported
 */
void
FirestudioProject::FirestudioProjectEAP::setupDefaultRouterConfig_high() {
    printMessage("High (192 kHz) sample rate not supported by Firestudio Project\n");
}


/**
  Device
*/
FirestudioProject::FirestudioProject( DeviceManager& d,
                                    ffado_smartptr<ConfigRom>( configRom ))
    : Dice::Device( d , configRom)
{
    debugOutput( DEBUG_LEVEL_VERBOSE, "Created Dice::Presonus::FirestudioProject (NodeID %d)\n",
                 getConfigRom().getNodeId() );
}

FirestudioProject::~FirestudioProject()
{
    getEAP()->storeFlashConfig();
}

bool FirestudioProject::discover() {
    if (Dice::Device::discover()) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "Discovering Dice::Presonus::FirestudioProject\n");
        return true;
    }
    return false;
}

void
FirestudioProject::showDevice()
{
    debugOutput(DEBUG_LEVEL_VERBOSE, "This is a Dice::Presonus::FirestudioProject\n");
    Dice::Device::showDevice();
}

Dice::EAP* FirestudioProject::createEAP() {
    return new FirestudioProjectEAP(*this);
}

}
}
// vim: et
