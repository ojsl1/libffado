/*
 * Copyright (C) 2005-2008 by Pieter Palmers
 * Copyright (C) 2008-2009 by Jonathan Woithe
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
#include <math.h>

#include "rme/rme_avdevice.h"
#include "rme/fireface_settings_ctrls.h"

namespace Rme {

RmeSettingsCtrl::RmeSettingsCtrl(Device &parent, unsigned int type, 
    unsigned int info)
: Control::Discrete(&parent)
, m_parent(parent)
, m_type(type)
, m_value(0)
, m_info(info)
{
}

RmeSettingsCtrl::RmeSettingsCtrl(Device &parent, unsigned int type,
    unsigned int info,
    std::string name, std::string label, std::string descr)
: Control::Discrete(&parent)
, m_parent(parent)
, m_type(type)
, m_value(0)
, m_info(info)
{
    setName(name);
    setLabel(label);
    setDescription(descr);
}

bool
RmeSettingsCtrl::setValue(int v) {

signed int i;
signed int err = 0;

    if (m_type>=RME_CTRL_TCO_FIRST && m_type<=RME_CTRL_TCO_LAST) {
    }

    switch (m_type) {
        case RME_CTRL_NONE:
            debugOutput(DEBUG_LEVEL_ERROR, "control has no type set\n");
            err = 1;
            break;
        case RME_CTRL_PHANTOM_SW:
            // Lowest 16 bits are phantom status bits (max 16 channels). 
            // High 16 bits are "write enable" bits for the corresponding
            // channel represented in the low 16 bits.  This way changes can
            // be made to one channel without needing to first read the
            // existing value.
            //
            // At present there are at most 4 phantom-capable channels.
            // Flag attempts to write to the bits corresponding to channels
            // beyond this.
            if (v & 0xfff00000) {
                debugOutput(DEBUG_LEVEL_WARNING, "Ignored out-of-range phantom set request: mask=0x%04x, value=0x%04x\n",
                  (v >> 16) & 0xfff0, v && 0xfff0);
            }

            for (i=0; i<4; i++) {
                if (v & (0x00010000 << i)) {
                    unsigned int on = (v & (0x00000001 << i)) != 0;
                    err = m_parent.setPhantom(i, on);
                    if (!err) {
                        if (on) {
                            m_value |= (0x01 << i);
                        } else {
                            m_value &= ~(0x01 << i);
                        }
                    }
                }
            }
            break;
        case RME_CTRL_INPUT_LEVEL:
            switch (v) {
              case 0: i = FF_SWPARAM_ILEVEL_LOGAIN; break;
              case 1: i = FF_SWPARAM_ILEVEL_m10dBV; break;
              default: i = FF_SWPARAM_ILEVEL_4dBU;
            }
            if (m_parent.setInputLevel(i) == 0) {
                m_value = v;
            }
            break;
        case RME_CTRL_OUTPUT_LEVEL:
            switch (v) {
              case 0: i = FF_SWPARAM_OLEVEL_m10dBV; break;
              case 1: i = FF_SWPARAM_OLEVEL_4dBU; break;
              default: i = FF_SWPARAM_OLEVEL_HIGAIN; break;
            }
            if (m_parent.setOutputLevel(i) == 0) {
                m_value = v;
            }
            break;
        case RME_CTRL_FF400_PAD_SW:
            // Object's "m_info" field is the channel
            if (m_parent.setInputPadOpt(m_info, v) == 0) {
                m_value = (v != 0);
            }
            break;
        case RME_CTRL_FF400_INSTR_SW:
            // Object's "m_info" field is the channel
            if (m_parent.setInputInstrOpt(m_info, v) == 0) {
                m_value = (v != 0);
            }
            break;
        case RME_CTRL_INPUT_SOURCE: {
            // m_info is the channel number
            signed int src = 0;
            if (v==0 || v==2) 
                src |= FF_SWPARAM_FF800_INPUT_OPT_FRONT;
            if (v==1 || v==2)
                src |= FF_SWPARAM_FF800_INPUT_OPT_REAR;
            if (m_parent.setInputSource(m_info, src) == 0)
                m_value = src;
            break;
        }
        case RME_CTRL_INSTRUMENT_OPTIONS:
            // m_info is the channel number, v is expected to be a bitmap
            // comprised of the FF800_INSTR_OPT_* values.
            if (m_parent.setInputInstrOpt(m_info, v) == 0)
                m_value = v;
            break;
        case RME_CTRL_SPDIF_INPUT_MODE:
            if (m_parent.setSpdifInputMode(v==0?FF_SWPARAM_SPDIF_INPUT_COAX:FF_SWPARAM_SPDIF_INPUT_OPTICAL)) {
                m_value = v;
            }
            break;
        case RME_CTRL_SPDIF_OUTPUT_OPTICAL:
            if (m_parent.setSpdifOutputIsOptical(v!=0) == 0) {
               m_value = (v != 0);
            }
            break;
        case RME_CTRL_SPDIF_OUTPUT_PRO:
            if (m_parent.setSpdifOutputProOn(v!=0) == 0) {
               m_value = (v != 0);
            }
            break;
        case RME_CTRL_SPDIF_OUTPUT_EMPHASIS:
            if (m_parent.setSpdifOutputEmphasisOn(v!=0) == 0) {
               m_value = (v != 0);
            }
            break;
        case RME_CTRL_SPDIF_OUTPUT_NONAUDIO:
            if (m_parent.setSpdifOutputNonAudioOn(v!=0) == 0) {
               m_value = (v != 0);
            }
            break;
        case RME_CTRL_PHONES_LEVEL:
            switch (v) {
              case 0: i = FF_SWPARAM_PHONESLEVEL_HIGAIN; break;
              case 1: i = FF_SWPARAM_PHONESLEVEL_4dBU; break;
              default: i = FF_SWPARAM_PHONESLEVEL_m10dBV;
            }
            if (m_parent.setPhonesLevel(i) == 0) {
                m_value = v;
            }
            break;

        case RME_CTRL_CLOCK_MODE:
            if (m_parent.setClockMode(v==1?FF_SWPARAM_CLOCK_MODE_AUTOSYNC:FF_SWPARAM_CLOCK_MODE_MASTER) == 0) {
                m_value = v;
            }
            break;
        case RME_CTRL_SYNC_REF: {
            signed int val;
            switch (v) {
              case 0: val = FF_SWPARAM_SYNCREF_WORDCLOCK; break;
              case 1: val = FF_SWPARAM_SYNCREF_ADAT1; break;
              case 2: val = FF_SWPARAM_SYNCREF_ADAT2; break;
              case 3: val = FF_SWPARAM_SYNCREF_SPDIF; break;
              case 4: val = FF_SWPARAM_SYNCREC_TCO; break;
              default:
                val = FF_SWPARAM_SYNCREF_WORDCLOCK;
            }
            if (m_parent.setSyncRef(val) == 0) {
              m_value = v;
            }
            break;
        }

        case RME_CTRL_LIMIT_BANDWIDTH: {
            signed int val;
            switch (v) {
              case 0: val = FF_DEV_FLASH_BWLIMIT_SEND_ALL_CHANNELS; break;
              case 1: val = FF_DEV_FLASH_BWLIMIT_NO_ADAT2; break;
              case 2: val = FF_DEV_FLASH_BWLIMIT_ANALOG_SPDIF_ONLY; break;
              case 3: val = FF_DEV_FLASH_BWLIMIT_ANALOG_ONLY; break;
              default:
                val = FF_DEV_FLASH_BWLIMIT_SEND_ALL_CHANNELS;
            }
            if (m_parent.setBandwidthLimit(val) == 0) {
              m_value = v;
            }
            break;
        }

        case RME_CTRL_FLASH:
            switch (v) {
                case RME_CTRL_FLASH_SETTINGS_LOAD:
                    m_parent.read_device_flash_settings(NULL);
                    break;
                case RME_CTRL_FLASH_SETTINGS_SAVE:
                    m_parent.write_device_flash_settings(NULL);
                    break;
                case RME_CTRL_FLASH_MIXER_LOAD:
                    m_parent.read_device_mixer_settings(NULL);
                    break;
                case RME_CTRL_FLASH_MIXER_SAVE:
                    m_parent.write_device_mixer_settings(NULL);
                    break;
                default:
                    debugOutput(DEBUG_LEVEL_ERROR, "unknown command value %d for flash control type 0x%04x\n", v, m_type);
                    err = 1;
            }
            break;
        case RME_CTRL_MIXER_PRESET:
            debugOutput(DEBUG_LEVEL_VERBOSE, "mixer presets not implemented yet\n");
            break;

        // All RME_CTRL_INFO_* controls are read-only.  Warn on attempts to
        // set these.
        case RME_CTRL_INFO_MODEL:
        case RME_CTRL_INFO_TCO_PRESENT:
        case RME_CTRL_INFO_SYSCLOCK_MODE:
        case RME_CTRL_INFO_SYSCLOCK_FREQ:
        case RME_CTRL_INFO_AUTOSYNC_FREQ:
        case RME_CTRL_INFO_AUTOSYNC_SRC:
        case RME_CTRL_INFO_SYNC_STATUS:
        case RME_CTRL_INFO_SPDIF_FREQ:
            debugOutput(DEBUG_LEVEL_ERROR, "Attempt to set readonly info control 0x%08x\n", m_type);
            err = 1;
            break;

        case RME_CTRL_TCO_LTC_IN:
        case RME_CTRL_TCO_INPUT_LTC_VALID:
        case RME_CTRL_TCO_INPUT_LTC_FPS:
        case RME_CTRL_TCO_INPUT_LTC_DROPFRAME:
        case RME_CTRL_TCO_INPUT_VIDEO_TYPE:
        case RME_CTRL_TCO_INPUT_WORD_CLK:
        case RME_CTRL_TCO_INPUT_LOCK:
        case RME_CTRL_TCO_FREQ:
            debugOutput(DEBUG_LEVEL_ERROR, "Attempt to set readonly TCO control 0x%08x\n", m_type);
            err = 1;
            break;
        case RME_CTRL_TCO_SYNC_SRC:
            switch (v) {
                case 0: i = FF_TCOPARAM_INPUT_LTC; break;
                case 1: i = FF_TCOPARAM_INPUT_VIDEO; break;
                case 2: i = FF_TCOPARAM_INPUT_VIDEO; break;
                default: i = FF_TCOPARAM_INPUT_VIDEO;
            }
            return m_parent.setTcoSyncSrc(i);
            break;
        case RME_CTRL_TCO_FRAME_RATE:
            switch (v) {
                case 0: i = FF_TCOPARAM_FRAMERATE_24fps; break;
                case 1: i = FF_TCOPARAM_FRAMERATE_25fps; break;
                case 2: i = FF_TCOPARAM_FRAMERATE_29_97fps; break;
                case 3: i = FF_TCOPARAM_FRAMERATE_29_97dfps; break;
                case 4: i = FF_TCOPARAM_FRAMERATE_30fps; break;
                case 5: i = FF_TCOPARAM_FRAMERATE_30dfps; break;
                default: i = FF_TCOPARAM_FRAMERATE_25fps; break;
            }
            return m_parent.setTcoFrameRate(i);
            break;
        case RME_CTRL_TCO_SAMPLE_RATE:
            switch (v) {
                case 0: i = FF_TCOPARAM_SRATE_44_1; break;
                case 1: i = FF_TCOPARAM_SRATE_48; break;
                default: i = FF_TCOPARAM_SRATE_48; break;
            }
            return m_parent.setTcoSampleRate(i);
            break;
        case RME_CTRL_TCO_SAMPLE_RATE_OFS:
            switch (v) {
                case 0: i = FF_TCOPARAM_PULL_NONE; break;
                case 1: i = FF_TCOPARAM_PULL_UP_01; break;
                case 2: i = FF_TCOPARAM_PULL_DOWN_01; break;
                case 3: i = FF_TCOPARAM_PULL_UP_40; break;
                case 4: i = FF_TCOPARAM_PULL_DOWN_40; break;
                default: i = FF_TCOPARAM_PULL_NONE;
            }
            return m_parent.setTcoPull(i);
            break;
        case RME_CTRL_TCO_VIDEO_IN_TERM:
            return m_parent.setTcoTermination(v == 1);
            break;
        case RME_CTRL_TCO_WORD_CLK_CONV:
            switch (v) {
                case 0: i = FF_TCOPARAM_WORD_CLOCK_CONV_1_1; break;
                case 1: i = FF_TCOPARAM_WORD_CLOCK_CONV_44_48; break;
                case 2: i = FF_TCOPARAM_WORD_CLOCK_CONV_48_44; break;
                default: i =  FF_TCOPARAM_WORD_CLOCK_CONV_1_1;
            }
            return m_parent.setTcoWordClkConv(i);
            break;

        default:
            debugOutput(DEBUG_LEVEL_ERROR, "Unknown control type 0x%08x\n", m_type);
            err = 1;
    }

    return err==0?true:false;
}

int
RmeSettingsCtrl::getValue() {

signed int i;
signed int val = 0;
FF_state_t ff_state;

    switch (m_type) {
        case RME_CTRL_NONE:
            debugOutput(DEBUG_LEVEL_ERROR, "control has no type set\n");
            break;

        case RME_CTRL_PHANTOM_SW:
            for (i=0; i<3; i++)
                val |= (m_parent.getPhantom(i) << i);
            return val;
            break;
        case RME_CTRL_INPUT_LEVEL:
            switch (m_parent.getInputLevel()) {
                case FF_SWPARAM_ILEVEL_LOGAIN: return 0;
                case FF_SWPARAM_ILEVEL_m10dBV: return 1;
                default: return 2;
            }
            break;
        case RME_CTRL_OUTPUT_LEVEL:
            switch (m_parent.getOutputLevel()) {
                case FF_SWPARAM_OLEVEL_m10dBV: return 0;
                case FF_SWPARAM_OLEVEL_4dBU: return 1;
                default: return 2;
            }
            break;
        case RME_CTRL_FF400_PAD_SW:
            return m_parent.getInputPadOpt(m_info);
            break;
        case RME_CTRL_FF400_INSTR_SW:
            return m_parent.getInputInstrOpt(m_info);
            break;
        case RME_CTRL_INPUT_SOURCE: {
            signed int src;
            src = m_parent.getInputSource(m_info);
            if (src == FF_SWPARAM_FF800_INPUT_OPT_FRONT)
                return 0;
            if (src == FF_SWPARAM_FF800_INPUT_OPT_REAR)
                return 1;
            return 2;
            break;
        }
        case RME_CTRL_INSTRUMENT_OPTIONS: 
            return m_parent.getInputInstrOpt(m_info);
            break;
        case RME_CTRL_SPDIF_INPUT_MODE:
            i = m_parent.getSpdifInputMode();
            return i==FF_SWPARAM_SPDIF_INPUT_COAX?0:1;
            break;
        case RME_CTRL_SPDIF_OUTPUT_OPTICAL:
            return m_parent.getSpdifOutputIsOptical();
            break;
        case RME_CTRL_SPDIF_OUTPUT_PRO:
            return m_parent.getSpdifOutputProOn();
            break;
        case RME_CTRL_SPDIF_OUTPUT_EMPHASIS:
            return m_parent.getSpdifOutputEmphasisOn();
            break;
        case RME_CTRL_SPDIF_OUTPUT_NONAUDIO:
            return m_parent.getSpdifOutputNonAudioOn();
            break;
        case RME_CTRL_PHONES_LEVEL:
            return m_parent.getPhonesLevel();
            break;
        case RME_CTRL_CLOCK_MODE:
            return m_parent.getClockMode()==FF_SWPARAM_CLOCK_MODE_AUTOSYNC?1:0;
            break;
        case RME_CTRL_SYNC_REF: {
            signed int val = m_parent.getSyncRef();
            switch (val) {
              case FF_SWPARAM_SYNCREF_WORDCLOCK: return 0;
              case FF_SWPARAM_SYNCREF_ADAT1: return 1;
              case FF_SWPARAM_SYNCREF_ADAT2: return 2;
              case FF_SWPARAM_SYNCREF_SPDIF: return 3;
              case FF_SWPARAM_SYNCREC_TCO: return 4;
              default:
                return 0;
            }
            break;
        }
        case RME_CTRL_LIMIT_BANDWIDTH: {
            signed int val = m_parent.getBandwidthLimit();
            switch (val) {
              case FF_DEV_FLASH_BWLIMIT_SEND_ALL_CHANNELS: return 0;
              case FF_DEV_FLASH_BWLIMIT_NO_ADAT2: return 1;
              case FF_DEV_FLASH_BWLIMIT_ANALOG_SPDIF_ONLY: return 2;
              case FF_DEV_FLASH_BWLIMIT_ANALOG_ONLY: return 3;
              default:
                return 0;
            }
            break;
        }
        case RME_CTRL_INFO_MODEL:
            return m_parent.getRmeModel();
            break;

        case RME_CTRL_FLASH:
        case RME_CTRL_MIXER_PRESET:
            debugOutput(DEBUG_LEVEL_ERROR, "read requested for write-only control type 0x%04x\n", m_type);
            return 0;
            break;

        case RME_CTRL_INFO_TCO_PRESENT:
            return m_parent.getTcoPresent();

        case RME_CTRL_INFO_SYSCLOCK_MODE:
            if (m_parent.get_hardware_state(&ff_state) == 0)
                return ff_state.clock_mode;
            else
                debugOutput(DEBUG_LEVEL_ERROR, "failed to read device state\n");
            break;
        case RME_CTRL_INFO_SYSCLOCK_FREQ:
            return m_parent.getSamplingFrequency();
        case RME_CTRL_INFO_AUTOSYNC_FREQ:
            if (m_parent.get_hardware_state(&ff_state) == 0)
                return ff_state.autosync_freq;
            else
                debugOutput(DEBUG_LEVEL_ERROR, "failed to read device state\n");
            break;
        case RME_CTRL_INFO_AUTOSYNC_SRC:
            if (m_parent.get_hardware_state(&ff_state) == 0)
                return ff_state.autosync_source;
            else
                debugOutput(DEBUG_LEVEL_ERROR, "failed to read device state\n");
            break;
        case RME_CTRL_INFO_SYNC_STATUS:
            if (m_parent.get_hardware_state(&ff_state) == 0)
                return (ff_state.adat1_sync_status) |
                       (ff_state.adat2_sync_status << 2) |
                       (ff_state.spdif_sync_status << 4) |
                       (ff_state.wclk_sync_status << 6) |
                       (ff_state.tco_sync_status << 8);
            else
                debugOutput(DEBUG_LEVEL_ERROR, "failed to read device state\n");
            break;
        case RME_CTRL_INFO_SPDIF_FREQ:
            if (m_parent.get_hardware_state(&ff_state) == 0)
                return ff_state.spdif_freq;
            else
                debugOutput(DEBUG_LEVEL_ERROR, "failed to read device state\n");
            break;

        case RME_CTRL_TCO_LTC_IN:
            return m_parent.getTcoLtc();
        case RME_CTRL_TCO_INPUT_LTC_VALID:
            return m_parent.getTcoLtcValid();
            break;
        case RME_CTRL_TCO_INPUT_LTC_FPS:
            switch (m_parent.getTcoLtcFrameRate()) {
                case FF_TCOSTATE_FRAMERATE_24fps: return 0;
                case FF_TCOSTATE_FRAMERATE_25fps: return 1;
                case FF_TCOSTATE_FRAMERATE_29_97fps: return 2;
                case FF_TCOSTATE_FRAMERATE_30fps: return 3;
                default: return 1;
            }
            break;
        case RME_CTRL_TCO_INPUT_LTC_DROPFRAME:
            return m_parent.getTcoLtcDropFrame();
        case RME_CTRL_TCO_INPUT_VIDEO_TYPE:
            switch (m_parent.getTcoVideoType()) {
                case FF_TCOSTATE_VIDEO_NONE: return 0;
                case FF_TCOSTATE_VIDEO_PAL: return 1;
                case FF_TCOSTATE_VIDEO_NTSC: return 2;
                default: return 1;
            }
            break;
        case RME_CTRL_TCO_INPUT_WORD_CLK:
            switch (m_parent.getTcoWordClk()) {
                case FF_TCOSTATE_WORDCLOCK_1x: return 0;
                case FF_TCOSTATE_WORDCLOCK_2x: return 1;
                case FF_TCOSTATE_WORDCLOCK_4x: return 2;
                default: return 0;
            }
            break;
        case RME_CTRL_TCO_INPUT_LOCK:
            return m_parent.getTcoLock();
            break;
        case RME_CTRL_TCO_FREQ:
            return m_parent.getTcoFrequency();
            break;
        case RME_CTRL_TCO_SYNC_SRC:
            switch (m_parent.getTcoSyncSrc()) {
                case FF_TCOPARAM_INPUT_LTC: return 0;
                case FF_TCOPARAM_INPUT_VIDEO: return 1;
                case FF_TCOPARAM_INPUT_WCK: return 2;
                default: return 1;
            }
            break;
        case RME_CTRL_TCO_FRAME_RATE:
            switch (m_parent.getTcoFrameRate()) {
                case FF_TCOPARAM_FRAMERATE_24fps: return 0;
                case FF_TCOPARAM_FRAMERATE_25fps: return 1;
                case FF_TCOPARAM_FRAMERATE_29_97fps: return 2;
                case FF_TCOPARAM_FRAMERATE_29_97dfps: return 3;
                case FF_TCOPARAM_FRAMERATE_30fps: return 4;
                case FF_TCOPARAM_FRAMERATE_30dfps: return 5;
                default: return 1;
            }
            break;
        case RME_CTRL_TCO_SAMPLE_RATE:
            switch (m_parent.getTcoSampleRate()) {
                case FF_TCOPARAM_SRATE_44_1: return 0;
                case FF_TCOPARAM_SRATE_48: return 1;
                default: return 1;
            }
            break;
        case RME_CTRL_TCO_SAMPLE_RATE_OFS:
            switch (m_parent.getTcoPull()) {
                case FF_TCOPARAM_PULL_NONE: return 0;
                case FF_TCOPARAM_PULL_UP_01: return 1;
                case FF_TCOPARAM_PULL_DOWN_01: return 2;
                case FF_TCOPARAM_PULL_UP_40: return 3;
                case FF_TCOPARAM_PULL_DOWN_40: return 4;
                default: return 0;
            }
            break;
        case RME_CTRL_TCO_VIDEO_IN_TERM:
            return m_parent.getTcoTermination();
            break;
        case RME_CTRL_TCO_WORD_CLK_CONV:
            switch (m_parent.getTcoWordClkConv()) {
                case FF_TCOPARAM_WORD_CLOCK_CONV_1_1: return 0;
                case FF_TCOPARAM_WORD_CLOCK_CONV_44_48: return 1;
                case FF_TCOPARAM_WORD_CLOCK_CONV_48_44: return 2;
                default: return 0;
            }
            break;

        default:
            debugOutput(DEBUG_LEVEL_ERROR, "Unknown control type 0x%08x\n", m_type);
    }

    return 0;
}


static std::string getOutputName(const signed int model, const int idx)
{
    char buf[64];
    switch(model) {
        case RME_MODEL_FIREFACE400:
            if (idx >= 10)
                snprintf(buf, sizeof(buf), "ADAT out %d", idx-9);
            else
            if (idx >= 8)
                snprintf(buf, sizeof(buf), "SPDIF out %d", idx-7);
            else
            if (idx >= 6)
                snprintf(buf, sizeof(buf), "Mon out %d", idx+1);
            else
                snprintf(buf, sizeof(buf), "Line out %d", idx+1);
            break;
        case RME_MODEL_FIREFACE800:
            if (idx >= 20)
                snprintf(buf, sizeof(buf), "ADAT-2 out %d", idx-19);
            else
            if (idx >= 12)
                snprintf(buf, sizeof(buf), "ADAT-1 out %d", idx-11);
            else
            if (idx >= 10)
                snprintf(buf, sizeof(buf), "SPDIF out %d", idx-9);
            else
            if (idx >= 8)
                snprintf(buf, sizeof(buf), "Mon, ch %d", idx+1);
            else
                snprintf(buf, sizeof(buf), "Line out %d", idx+1);
            break;
        default:
            snprintf(buf, sizeof(buf), "out %d", idx);
    }
    return buf;
}

static std::string getInputName(const signed int model, const int idx)
{
    char buf[64];
    switch (model) {
        case RME_MODEL_FIREFACE400:
            if (idx >= 10)
                snprintf(buf, sizeof(buf), "ADAT in %d", idx-9);
            else
            if (idx >= 8)
                snprintf(buf, sizeof(buf), "SPDIF in %d", idx-7);
            else
            if (idx >= 4)
                snprintf(buf, sizeof(buf), "Line in %d", idx+1);
            else
            if (idx >= 2)
                snprintf(buf, sizeof(buf), "Inst/line %d", idx+1);
            else
                snprintf(buf, sizeof(buf), "Mic/line %d", idx+1);
            break;
        case RME_MODEL_FIREFACE800:
            if (idx >= 20)
                snprintf(buf, sizeof(buf), "ADAT-2 in %d", idx-19);
            else
            if (idx >= 12)
                snprintf(buf, sizeof(buf), "ADAT-1 in %d", idx-11);
            else
            if (idx >= 10)
                snprintf(buf, sizeof(buf), "SPDIF in %d", idx-9);
            else
            if (idx >= 6)
                snprintf(buf, sizeof(buf), "Mic/line %d", idx+1);
            else
            if (idx >= 1)
                snprintf(buf, sizeof(buf), "Line %d", idx+1);
            else
                snprintf(buf, sizeof(buf), "Instr/line %d", idx+1);
            break;
        default:
            snprintf(buf, sizeof(buf), "in %d", idx);
    }
    return buf;
}

RmeSettingsMatrixCtrl::RmeSettingsMatrixCtrl(Device &parent, unsigned int type)
: Control::MatrixMixer(&parent)
, m_parent(parent)
, m_type(type)
{
}

RmeSettingsMatrixCtrl::RmeSettingsMatrixCtrl(Device &parent, unsigned int type,
    std::string name)
: Control::MatrixMixer(&parent)
, m_parent(parent)
, m_type(type)
{
    setName(name);
}

void RmeSettingsMatrixCtrl::show()
{
    debugOutput(DEBUG_LEVEL_NORMAL, "RME matrix mixer type %d\n", m_type);
}

std::string RmeSettingsMatrixCtrl::getRowName(const int row)
{
    if (m_type == RME_MATRIXCTRL_OUTPUT_FADER)
        return "";
    return getOutputName(m_parent.getRmeModel(), row);
}

std::string RmeSettingsMatrixCtrl::getColName(const int col)
{
    if (m_type == RME_MATRIXCTRL_PLAYBACK_FADER)
        return "";
    if (m_type == RME_MATRIXCTRL_OUTPUT_FADER)
        return getOutputName(m_parent.getRmeModel(), col);

    return getInputName(m_parent.getRmeModel(), col);
}

int RmeSettingsMatrixCtrl::getRowCount() 
{
    switch (m_type) {
        case RME_MATRIXCTRL_GAINS:
            if (m_parent.getRmeModel() == RME_MODEL_FIREFACE400)
                return 1;
            break;
        case RME_MATRIXCTRL_INPUT_FADER:
        case RME_MATRIXCTRL_PLAYBACK_FADER:
            if (m_parent.getRmeModel() == RME_MODEL_FIREFACE400)
                return RME_FF400_MAX_CHANNELS;
            else
                return RME_FF800_MAX_CHANNELS;
            break;
        case RME_MATRIXCTRL_OUTPUT_FADER:
            return 1;
            break;
    }

    return 0;
}

int RmeSettingsMatrixCtrl::getColCount() 
{
    switch (m_type) {
        case RME_MATRIXCTRL_GAINS:
            if (m_parent.getRmeModel() == RME_MODEL_FIREFACE400)
                return 22;
            break;
        case RME_MATRIXCTRL_INPUT_FADER:
        case RME_MATRIXCTRL_PLAYBACK_FADER:
        case RME_MATRIXCTRL_OUTPUT_FADER:
            if (m_parent.getRmeModel() == RME_MODEL_FIREFACE400)
                return RME_FF400_MAX_CHANNELS;
            else
                return RME_FF800_MAX_CHANNELS;
            break;
    }

    return 0;
}

double RmeSettingsMatrixCtrl::setValue(const int row, const int col, const double val) 
{
    signed int ret = true;
    signed int i;

    switch (m_type) {
        case RME_MATRIXCTRL_GAINS:
            i = val;
            if (i >= 0)
                ret = m_parent.setAmpGain(col, val);
            else
                ret = -1;
            break;

        // For values originating from the input, playback or output faders,
        // the MatrixMixer widget uses a value of 0x004000 for 0 dB gain. 
        // The RME hardware (via setMixerGain()) uses 0x008000 as the 0 dB
        // reference point.  Correct for this mismatch when calling
        // setMixerGain().
        case RME_MATRIXCTRL_INPUT_FADER:
          return m_parent.setMixerGain(RME_FF_MM_INPUT, col, row, val*2);
          break;
        case RME_MATRIXCTRL_PLAYBACK_FADER:
          return m_parent.setMixerGain(RME_FF_MM_PLAYBACK, col, row, val*2);
          break;
        case RME_MATRIXCTRL_OUTPUT_FADER:
          return m_parent.setMixerGain(RME_FF_MM_OUTPUT, col, row, val*2);
          break;

        case RME_MATRIXCTRL_INPUT_MUTE:
          return m_parent.setMixerFlags(RME_FF_MM_INPUT, col, row, FF_SWPARAM_MF_MUTED, val!=0);
          break;
        case RME_MATRIXCTRL_PLAYBACK_MUTE:
          return m_parent.setMixerFlags(RME_FF_MM_PLAYBACK, col, row, FF_SWPARAM_MF_MUTED, val!=0);
          break;
        case RME_MATRIXCTRL_OUTPUT_MUTE:
          return m_parent.setMixerFlags(RME_FF_MM_OUTPUT, col, row, FF_SWPARAM_MF_MUTED, val!=0);
          break;
        case RME_MATRIXCTRL_INPUT_INVERT:
          return m_parent.setMixerFlags(RME_FF_MM_INPUT, col, row, FF_SWPARAM_MF_INVERTED, val!=0);
          break;
        case RME_MATRIXCTRL_PLAYBACK_INVERT:
          return m_parent.setMixerFlags(RME_FF_MM_PLAYBACK, col, row, FF_SWPARAM_MF_INVERTED, val!=0);
          break;

    }

    return ret;
}

double RmeSettingsMatrixCtrl::getValue(const int row, const int col) 
{
    double val = 0.0;
    switch (m_type) {
        case RME_MATRIXCTRL_GAINS:
            val = m_parent.getAmpGain(col);
            break;

        // The MatrixMixer widget (as used for the input, playback and
        // output faders) uses a value of 0x004000 for 0 dB gain, but the
        // gain value from the RME hardware (received getMixerGain()) uses
        // 0x008000 as the 0 dB reference point.  Correct for this mismatch
        // as the value is obtained.
        case RME_MATRIXCTRL_INPUT_FADER:
            val = m_parent.getMixerGain(RME_FF_MM_INPUT, col, row) / 2;
            break;
        case RME_MATRIXCTRL_PLAYBACK_FADER:
            val = m_parent.getMixerGain(RME_FF_MM_PLAYBACK, col, row) / 2;
            break;
        case RME_MATRIXCTRL_OUTPUT_FADER:
            val = m_parent.getMixerGain(RME_FF_MM_OUTPUT, col, row) / 2;
            break;

        case RME_MATRIXCTRL_INPUT_MUTE:
          return m_parent.getMixerFlags(RME_FF_MM_INPUT, col, row, FF_SWPARAM_MF_MUTED) != 0;
          break;
        case RME_MATRIXCTRL_PLAYBACK_MUTE:
          return m_parent.getMixerFlags(RME_FF_MM_PLAYBACK, col, row, FF_SWPARAM_MF_MUTED) != 0;
          break;
        case RME_MATRIXCTRL_OUTPUT_MUTE:
          return m_parent.getMixerFlags(RME_FF_MM_OUTPUT, col, row, FF_SWPARAM_MF_MUTED) != 0;
          break;
        case RME_MATRIXCTRL_INPUT_INVERT:
          return m_parent.getMixerFlags(RME_FF_MM_INPUT, col, row, FF_SWPARAM_MF_INVERTED) != 0;
          break;
        case RME_MATRIXCTRL_PLAYBACK_INVERT:
          return m_parent.getMixerFlags(RME_FF_MM_PLAYBACK, col, row, FF_SWPARAM_MF_INVERTED) != 0;
          break;
    }

    return val;
}
      

}
