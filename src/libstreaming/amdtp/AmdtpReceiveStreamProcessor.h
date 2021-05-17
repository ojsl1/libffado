/*
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

#ifndef __FFADO_AMDTPRECEIVESTREAMPROCESSOR__
#define __FFADO_AMDTPRECEIVESTREAMPROCESSOR__

/**
 * This class implements IEC61883-6 / AM824 / AMDTP based streaming
 */

#include "AmdtpStreamProcessor-common.h"

namespace Streaming {

class Port;
class AmdtpAudioPort;
class AmdtpMidiPort;
/*!
\brief The Base Class for an AMDTP receive stream processor

 This class implements a ReceiveStreamProcessor that demultiplexes
 AMDTP streams into Ports.

*/
class AmdtpReceiveStreamProcessor
    : public StreamProcessor
{

public:
    /**
     * Create a AMDTP receive StreamProcessor
     * @param port 1394 port
     * @param dimension number of substreams in the ISO stream
     *                  (midi-muxed is only one stream)
     */
    AmdtpReceiveStreamProcessor(FFADODevice &parent, int dimension);
    virtual ~AmdtpReceiveStreamProcessor() {};

    virtual enum eChildReturnValue processPacketHeader(unsigned char *data, unsigned int length,
                                                       unsigned char tag, unsigned char sy,
                                                       uint32_t pkt_ctr);
    virtual enum eChildReturnValue processPacketData(unsigned char *data, unsigned int length);

    virtual bool prepareChild();

public:
    virtual unsigned int getEventSize() 
                    {return 4;};
    virtual unsigned int getMaxPacketSize() 
                    {return 4 * (2 + getSytInterval() * m_dimension);};
    virtual unsigned int getEventsPerFrame() 
                    { return m_dimension; };
    virtual unsigned int getNominalFramesPerPacket() 
                    {return getSytInterval();};


protected:
    bool processReadBlock(char *data, unsigned int nevents, unsigned int offset);

protected:
    void decodeAudioPortsFloat(quadlet_t *data, unsigned int offset, unsigned int nevents);
    void decodeAudioPortsInt24(quadlet_t *data, unsigned int offset, unsigned int nevents);
    void decodeMidiPorts(quadlet_t *data, unsigned int offset, unsigned int nevents);

    unsigned int getSytInterval();

    int m_dimension;
    unsigned int m_syt_interval;

private: // local port caching for performance
    struct _MBLA_port_cache {
        AmdtpAudioPort*     port;
        void*               buffer;
        bool                enabled;
#ifdef DEBUG
        unsigned int        buffer_size;
#endif
    };
    std::vector<struct _MBLA_port_cache> m_audio_ports;
    unsigned int m_nb_audio_ports;

    struct _MIDI_port_cache {
        AmdtpMidiPort*      port;
        void*               buffer;
        bool                enabled;
        unsigned int        position;
        unsigned int        location;
#ifdef DEBUG
        unsigned int        buffer_size;
#endif
    };
    std::vector<struct _MIDI_port_cache> m_midi_ports;
    unsigned int m_nb_midi_ports;

    /* A small MIDI buffer to cover for the case where we need to span a
     * period - that is, if more than one MIDI byte is sent per packet. 
     * Since the long-term average data rate must be close to the MIDI spec
     * (as it's coming from a physical MIDI port_ this buffer doesn't have
     * to be particularly large.  The size is a power of 2 for optimisation
     * reasons.
     *
     * FIXME: it is yet to be determined whether this is required for RME
     * devices.
     *
     * FIXME: copied from RmeReceiveStreamProcessor.h. Needs refactoring
     */
#define RX_MIDIBUFFER_SIZE_EXP 6
#define RX_MIDIBUFFER_SIZE     (1<<RX_MIDIBUFFER_SIZE_EXP)

    unsigned int midibuffer[RX_MIDIBUFFER_SIZE];
    unsigned mb_head, mb_tail;

    bool initPortCache();
    void updatePortCache();
};


} // end of namespace Streaming

#endif /* __FFADO_AMDTPRECEIVESTREAMPROCESSOR__ */

