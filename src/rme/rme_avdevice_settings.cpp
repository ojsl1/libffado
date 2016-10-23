/*
 * Copyright (C) 2005-2009 by Jonathan Woithe
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
 */

#include "rme/rme_avdevice.h"
#include "rme/fireface_def.h"

#include "debugmodule/debugmodule.h"

namespace Rme {

signed int
Device::getPhantom(unsigned int channel) {

    if (channel > 3) {
        debugOutput(DEBUG_LEVEL_WARNING, "Channel %d phantom power not supported\n", channel);
        return -1;
    }

    return settings->mic_phantom[channel] != 0;
}

signed int
Device::setPhantom(unsigned int channel, unsigned int status) {

    if (channel > 3) {
        debugOutput(DEBUG_LEVEL_WARNING, "Channel %d phantom power not supported\n", channel);
        return -1;
    }

    settings->mic_phantom[channel] = (status != 0);
    set_hardware_params();

    return 0;
}

signed int
Device::getInputLevel(void) {
    return settings->input_level;
}

signed int
Device::setInputLevel(unsigned int level) {

    if (level<FF_SWPARAM_ILEVEL_LOGAIN || level>FF_SWPARAM_ILEVEL_4dBU) {
        debugOutput(DEBUG_LEVEL_WARNING, "Invalid input level ID %d\n", level);
        return -1;
    }
    settings->input_level = level;
    set_hardware_params();

    return 0;
}

signed int
Device::getOutputLevel(void) {
    return settings->output_level;
}

signed int
Device::setOutputLevel(unsigned int level) {

    if (level<FF_SWPARAM_OLEVEL_m10dBV || level>FF_SWPARAM_OLEVEL_HIGAIN) {
        debugOutput(DEBUG_LEVEL_WARNING, "Invalid output level ID %d\n", level);
        return -1;
    }
    settings->output_level = level;
    set_hardware_params();

    return 0;
}

signed int
Device::getPhonesLevel(void) {
    return settings->phones_level;
}

signed int
Device::setPhonesLevel(unsigned int level) {

    if (level<FF_SWPARAM_PHONESLEVEL_HIGAIN || level>FF_SWPARAM_PHONESLEVEL_m10dBV) {
        debugOutput(DEBUG_LEVEL_WARNING, "Invalid phones level ID %d\n", level);
        return -1;
    }
    settings->phones_level = level;
    set_hardware_params();

    return 0;
}

signed int 
Device::getInputPadOpt(unsigned int channel) {
    switch (m_rme_model) {
        case RME_MODEL_FIREFACE800:
            debugOutput(DEBUG_LEVEL_WARNING, "channel input pad option not supported by FF800 hardware\n");
            return -1;
        case RME_MODEL_FIREFACE400:
            if (channel<3 || channel>4) {
                debugOutput(DEBUG_LEVEL_WARNING, "channel %d input pad option not supported on FF400\n", channel);
                return -1;
            }
            return settings->ff400_input_pad[channel-3] != 0;
        default:
            debugOutput(DEBUG_LEVEL_WARNING, "unimplemented for model %d\n", m_rme_model);
    }
    return 0;
}

signed int 
Device::setInputPadOpt(unsigned int channel, unsigned int status) {
    switch (m_rme_model) {
        case RME_MODEL_FIREFACE800:
            debugOutput(DEBUG_LEVEL_WARNING, "channel input pad option not supported by FF800 hardware\n");
            return -1;
        case RME_MODEL_FIREFACE400:
            if (channel<3 || channel>4) {
                debugOutput(DEBUG_LEVEL_WARNING, "channel %d input pad option not supported on FF400\n", channel);
                return -1;
            }
            settings->ff400_input_pad[channel-3] = (status != 0);
            break;
        default:
            debugOutput(DEBUG_LEVEL_WARNING, "unimplemented for model %d\n", m_rme_model);
            return -1;
    }
    set_hardware_params();
    return 0;
}

signed int 
Device::getInputInstrOpt(unsigned int channel) {
    switch (m_rme_model) {
        case RME_MODEL_FIREFACE400:
            if (channel<3 || channel>4) {
                debugOutput(DEBUG_LEVEL_WARNING, "Channel %d input instrument option not supported for model FF400\n", channel);
                return -1;
            }
            return settings->ff400_instr_input[channel-3] != 0;
            break;
        case RME_MODEL_FIREFACE800:
            if (channel != 1) {
                debugOutput(DEBUG_LEVEL_WARNING, "Channel %d input instrument options not supported for FF800\n", channel);
                return -1;
            }
            return (settings->filter?FF800_INSTR_OPT_FILTER:0) |
                   (settings->fuzz?FF800_INSTR_OPT_FUZZ:0) |
                   (settings->limiter?FF800_INSTR_OPT_LIMITER:0);
            break;
        default:
            debugOutput(DEBUG_LEVEL_WARNING, "unimplemented for model %d\n", m_rme_model);
            return -1;
    }
    return -1;
}

signed int 
Device::setInputInstrOpt(unsigned int channel, unsigned int status) {
    switch (m_rme_model) {
        case RME_MODEL_FIREFACE400:
            if (channel<3 || channel>4) {
                debugOutput(DEBUG_LEVEL_WARNING, "channel %d input instrument option not supported for FF400\n", channel);
                return -1;
            }
            settings->ff400_instr_input[channel-3] = (status != 0);
            break;
        case RME_MODEL_FIREFACE800:
            if (channel != 1) {
                debugOutput(DEBUG_LEVEL_WARNING, "channel %d input instrument options not supported for FF800\n", channel);
                return -1;
            }
            settings->filter = (status & FF800_INSTR_OPT_FILTER)!=0;
            settings->fuzz = (status & FF800_INSTR_OPT_FUZZ)!=0;
            settings->limiter = (status & FF800_INSTR_OPT_LIMITER)!=0;
            break;
        default:
            debugOutput(DEBUG_LEVEL_WARNING, "unimplemented for model %d\n", m_rme_model);
            return -1;
    }
    set_hardware_params();
    return 0;
}

signed int
Device::getInputSource(unsigned int channel) {
    if (m_rme_model != RME_MODEL_FIREFACE800) {
        debugOutput(DEBUG_LEVEL_WARNING, "selected channel sources are settable only on FF800\n");
        return -1;
    }

    if (channel!=1 && channel!=7 && channel!=8) {
        debugOutput(DEBUG_LEVEL_WARNING, "channel %d source is fixed on FF800\n", channel);
        return -1;
    }
    if (channel == 1)
        return settings->input_opt[0];
    else
        return settings->input_opt[channel-6];
}

signed int
Device::setInputSource(unsigned int channel, unsigned int src) {
    /* "opt" should be composed only of the FF_SWPARAM_FF800_INPUT_OPT_* 
     * defines.
     */
    signed int index;
    if (m_rme_model != RME_MODEL_FIREFACE800) {
        debugOutput(DEBUG_LEVEL_WARNING, "selected channel sources are settable only on FF800\n");
        return -1;
    }

    if (channel!=1 && channel!=7 && channel!=8) {
        debugOutput(DEBUG_LEVEL_WARNING, "channel %d source is fixed on FF800\n", channel);
        return -1;
    }
    if (channel == 1)
        index = 0;
    else
        index = channel-6;
    settings->input_opt[index] = src;
    set_hardware_params();
    return 0;
}

signed int 
Device::getSpdifInputMode(void) {
    return settings->spdif_input_mode;
}

signed int 
Device::setSpdifInputMode(signed int mode) {
    settings->spdif_input_mode = mode;
    set_hardware_params();
    return 0;
}

signed int
Device::getSpdifOutputIsOptical(void) {
    return settings->spdif_output_mode == FF_SWPARAM_SPDIF_OUTPUT_OPTICAL;
}
signed int
Device::setSpdifOutputIsOptical(signed int enable) {
  settings->spdif_output_mode = enable==1?FF_SWPARAM_SPDIF_OUTPUT_OPTICAL:FF_SWPARAM_SPDIF_OUTPUT_COAX;
  set_hardware_params();
  return 0;
}
signed int
Device::getSpdifOutputEmphasisOn(void) {
    return settings->spdif_output_emphasis == FF_SWPARAM_SPDIF_OUTPUT_EMPHASIS_ON;
}
signed int
Device::setSpdifOutputEmphasisOn(signed int enable) {
  settings->spdif_output_emphasis = enable==1?FF_SWPARAM_SPDIF_OUTPUT_EMPHASIS_ON:0;
  set_hardware_params();
  return 0;
}
signed int
Device::getSpdifOutputNonAudioOn(void) {
    return settings->spdif_output_nonaudio == FF_SWPARAM_SPDIF_OUTPUT_NONAUDIO_ON;
}
signed int
Device::setSpdifOutputNonAudioOn(signed int enable) {
  settings->spdif_output_nonaudio = enable==1?FF_SWPARAM_SPDIF_OUTPUT_NONAUDIO_ON:0;
  set_hardware_params();
  return 0;
}
signed int
Device::getSpdifOutputProOn(void) {
    return settings->spdif_output_pro == FF_SWPARAM_SPDIF_OUTPUT_PRO_ON;
}
signed int
Device::setSpdifOutputProOn(signed int enable) {
  settings->spdif_output_pro = enable==1?FF_SWPARAM_SPDIF_OUTPUT_PRO_ON:0;
  set_hardware_params();
  return 0;
}

signed int
Device::getAmpGain(unsigned int index) {
    if (m_rme_model != RME_MODEL_FIREFACE400) {
        debugOutput(DEBUG_LEVEL_WARNING, "Amp gains only supported on FF400\n");
        return -1;
    }
    if (index > 21) {
        debugOutput(DEBUG_LEVEL_WARNING, "Amp gain index %d invalid\n", index);
         return -1;
    }
    return settings->amp_gains[index];
}

signed int
Device::setAmpGain(unsigned int index, signed int val) {

    if (m_rme_model != RME_MODEL_FIREFACE400) {
        debugOutput(DEBUG_LEVEL_WARNING, "Amp gains only supported on FF400\n");
        return -1;
    }
    if (index > 21) {
        debugOutput(DEBUG_LEVEL_WARNING, "Amp gain index %d invalid\n", index);
         return -1;
    }
    settings->amp_gains[index] = val & 0xff;
    return set_hardware_ampgain(index, val);
}

signed int
Device::getMixerGainIndex(unsigned int src_channel, unsigned int dest_channel) {
    return dest_channel*RME_FF800_MAX_CHANNELS + src_channel;
}

signed int
Device::getMixerGain(unsigned int ctype,
    unsigned int src_channel, unsigned int dest_channel) {

    signed int idx = getMixerGainIndex(src_channel, dest_channel);
    switch (ctype) {
        case RME_FF_MM_INPUT:
            return settings->input_faders[idx];
            break;
        case RME_FF_MM_PLAYBACK:
            return settings->playback_faders[idx];
            break;
        case RME_FF_MM_OUTPUT:
            return settings->output_faders[src_channel];
            break;
    }
    return 0;
}

signed int
Device::setMixerGain(unsigned int ctype, 
    unsigned int src_channel, unsigned int dest_channel, signed int val) {

    unsigned char *mixerflags = NULL;
    signed int idx = getMixerGainIndex(src_channel, dest_channel);

    switch (ctype) {
        case RME_FF_MM_INPUT:
            settings->input_faders[idx] = val;
            mixerflags = settings->input_mixerflags;
            break;
        case RME_FF_MM_PLAYBACK:
            settings->playback_faders[idx] = val;
            mixerflags = settings->playback_mixerflags;
            break;
        case RME_FF_MM_OUTPUT:
            settings->output_faders[src_channel] = val;
            mixerflags = settings->output_mixerflags;
            break;
    }

    // If the matrix channel is muted, override the fader value and 
    // set it to zero.  Note that this is different to the hardware
    // mute control dealt with by set_hardware_channel_mute(); the
    // latter deals with a muting separate from the mixer.
    if (mixerflags!=NULL && (mixerflags[idx] & FF_SWPARAM_MF_MUTED)!=0) {
        val = 0;
    }

    // Phase inversion is effected by sending a negative volume to the
    // hardware.  However, when transitioning from 0 (-inf dB) to -1 (-90
    // dB), the hardware seems to first send the volume up to a much higher
    // level before it drops down to the set point after about a tenth of a
    // second (this also seems to be the case when switching between
    // inversion modes).  To work around this for the moment (at least until
    // it's understood, silently map a value of 0 to -1 when phase inversion
    // is active.
    if (mixerflags!=NULL && (mixerflags[idx] & FF_SWPARAM_MF_INVERTED)!=0) {
        if (val == 0)
            val = 1;
        val = -val;
    }

    return set_hardware_mixergain(ctype, src_channel, dest_channel, val);
}

signed int
Device::getMixerFlags(unsigned int ctype,
    unsigned int src_channel, unsigned int dest_channel, unsigned int flagmask) {

    unsigned char *mixerflags = NULL;
    signed int idx = getMixerGainIndex(src_channel, dest_channel);
    if (ctype == RME_FF_MM_OUTPUT) {
        mixerflags = settings->output_mixerflags;
        idx = src_channel;
    } else
    if (ctype == RME_FF_MM_INPUT)
        mixerflags = settings->input_mixerflags;
    else
        mixerflags = settings->playback_mixerflags;

    return mixerflags[idx] & flagmask;
}

signed int
Device::setMixerFlags(unsigned int ctype,
    unsigned int src_channel, unsigned int dest_channel, 
    unsigned int flagmask, signed int val) {

    unsigned char *mixerflags = NULL;
    signed int idx = getMixerGainIndex(src_channel, dest_channel);
    if (ctype == RME_FF_MM_OUTPUT) {
        mixerflags = settings->output_mixerflags;
        idx = src_channel;
    } else 
    if (ctype == RME_FF_MM_INPUT)
        mixerflags = settings->input_mixerflags;
    else
        mixerflags = settings->playback_mixerflags;

// FIXME: When switching inversion modes, the hardware seems to a channel to
// full volume for about 1/10 sec.  Attempt to avoid this by temporarily
// muting the channel.  This doesn't seem to work though.
//    if (flagmask & FF_SWPARAM_MF_INVERTED) 
//        set_hardware_mixergain(ctype, src_channel, dest_channel, 0);

    if (val == 0)
        mixerflags[idx] &= ~flagmask;
    else
        mixerflags[idx] |= flagmask;

    if (flagmask & (FF_SWPARAM_MF_MUTED|FF_SWPARAM_MF_INVERTED)) {
        // Mixer channel muting/inversion is handled via the gain control
        return setMixerGain(ctype, src_channel, dest_channel, 
            getMixerGain(ctype, src_channel, dest_channel));
    }
    return 0;
}

signed int
Device::getClockMode(void) {
  return settings->clock_mode;
}
signed int
Device::setClockMode(signed int mode) {
  if (mode!=FF_SWPARAM_CLOCK_MODE_MASTER && mode!=FF_SWPARAM_CLOCK_MODE_AUTOSYNC)
    return -1;
  settings->clock_mode = mode;
  set_hardware_params();
  return 0;
}

signed int
Device::getSyncRef(void) {
  return settings->sync_ref;
}
signed int
Device::setSyncRef(signed int ref) {
  settings->sync_ref = ref;
  set_hardware_params();
  return 0;
}

signed int
Device::getBandwidthLimit(void) {
  return settings->limit_bandwidth;
}
signed int
Device::setBandwidthLimit(signed int limit) {
  if (limit < FF_DEV_FLASH_BWLIMIT_SEND_ALL_CHANNELS ||
          limit > FF_DEV_FLASH_BWLIMIT_ANALOG_ONLY)
      return -1;

  // FF400 doesn't have a second ADAT channel, so this setting isn't
  // relevant for that interface.
  if (m_rme_model==RME_MODEL_FIREFACE400 &&
          limit==FF_DEV_FLASH_BWLIMIT_NO_ADAT2)
      return -1;

  settings->limit_bandwidth = limit;
  set_hardware_params();
  return 0;
}

signed int 
Device::getTcoLtc(void) {
    FF_TCO_state_t ff_tco_state;
    if (read_tco_state(&ff_tco_state) != 0) {
        debugOutput(DEBUG_LEVEL_ERROR, "failed to read TCO state\n");
        return 0;
    }
    return (ff_tco_state.hours<<24) | (ff_tco_state.minutes<<16) |
           (ff_tco_state.seconds<<8) | (ff_tco_state.frames);
}
signed int
Device::getTcoLtcValid(void) {
    FF_TCO_state_t ff_tco_state;
    if (read_tco_state(&ff_tco_state) != 0) {
        debugOutput(DEBUG_LEVEL_ERROR, "failed to read TCO state\n");
        return 0;
    }
    return ff_tco_state.ltc_valid;
}
signed int
Device::getTcoLock(void) {
    FF_TCO_state_t ff_tco_state;
    if (read_tco_state(&ff_tco_state) != 0) {
        debugOutput(DEBUG_LEVEL_ERROR, "failed to read TCO state\n");
        return 0;
    }
    return ff_tco_state.locked;
}
signed int
Device::getTcoLtcFrameRate(void) {
    FF_TCO_state_t ff_tco_state;
    if (read_tco_state(&ff_tco_state) != 0) {
        debugOutput(DEBUG_LEVEL_ERROR, "failed to read TCO state\n");
        return 0;
    }
    return ff_tco_state.frame_rate;
}
signed int
Device::getTcoLtcDropFrame(void) {
    FF_TCO_state_t ff_tco_state;
    if (read_tco_state(&ff_tco_state) != 0) {
        debugOutput(DEBUG_LEVEL_ERROR, "failed to read TCO state\n");
        return 0;
    }
    return ff_tco_state.drop_frame;
}
signed int
Device::getTcoVideoType(void) {
    FF_TCO_state_t ff_tco_state;
    if (read_tco_state(&ff_tco_state) != 0) {
        debugOutput(DEBUG_LEVEL_ERROR, "failed to read TCO state\n");
        return 0;
    }
    return ff_tco_state.video_input;
}
signed int
Device::getTcoWordClk(void) {
    FF_TCO_state_t ff_tco_state;
    if (read_tco_state(&ff_tco_state) != 0) {
        debugOutput(DEBUG_LEVEL_ERROR, "failed to read TCO state\n");
        return 0;
    }
    return ff_tco_state.word_clock_state;
}
float
Device::getTcoFrequency(void) {
    FF_TCO_state_t ff_tco_state;
    if (read_tco_state(&ff_tco_state) != 0) {
        debugOutput(DEBUG_LEVEL_ERROR, "failed to read TCO state\n");
        return 0;
    }
    return ff_tco_state.sample_rate;
}
signed int
Device::getTcoWordClkState(void) {
    FF_TCO_state_t ff_tco_state;
    if (read_tco_state(&ff_tco_state) != 0) {
        debugOutput(DEBUG_LEVEL_ERROR, "failed to read TCO state\n");
        return 0;
    }
    return ff_tco_state.word_clock_state;
}
signed int
Device::getTcoSyncSrc(void) {
    return tco_settings->input;
}
signed int
Device::setTcoSyncSrc(signed int src) {
    tco_settings->input = src;
    return write_tco_settings(tco_settings);
}
signed int
Device::getTcoFrameRate(void) {
    return tco_settings->frame_rate;
}
signed int
Device::setTcoFrameRate(signed int rate_id) {
    tco_settings->frame_rate = rate_id;
    return write_tco_settings(tco_settings);
}
signed int
Device::getTcoSampleRate(void) {
    return tco_settings->sample_rate;
}
signed int
Device::setTcoSampleRate(signed int rate_param_id) {
    tco_settings->sample_rate = rate_param_id;
    return write_tco_settings(tco_settings);
}
signed int
Device::getTcoPull(void) {
    return tco_settings->pull;
}
signed int
Device::setTcoPull(signed int pull) {
    tco_settings->pull = pull;
    return write_tco_settings(tco_settings);
}
signed int
Device::getTcoTermination(void) {
    return tco_settings->termination == FF_TCOPARAM_TERMINATION_ON;
}
signed int
Device::setTcoTermination(signed int enable) {
    tco_settings->termination = enable ? FF_TCOPARAM_TERMINATION_ON : 0;
    return write_tco_settings(tco_settings);
}

}
