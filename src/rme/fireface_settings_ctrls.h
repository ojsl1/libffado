/*
 * Copyright (C) 2009 by Jonathan Woithe
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

#include "debugmodule/debugmodule.h"

#include "libcontrol/BasicElements.h"
#include "libcontrol/MatrixMixer.h"

namespace Rme {

/* Control types for an RmeSettingsCtrl object */
#define RME_CTRL_NONE                  0x0000
#define RME_CTRL_PHANTOM_SW            0x0001
#define RME_CTRL_SPDIF_INPUT_MODE      0x0002
#define RME_CTRL_SPDIF_OUTPUT_OPTICAL  0x0003
#define RME_CTRL_SPDIF_OUTPUT_PRO      0x0004
#define RME_CTRL_SPDIF_OUTPUT_EMPHASIS 0x0005
#define RME_CTRL_SPDIF_OUTPUT_NONAUDIO 0x0006
#define RME_CTRL_CLOCK_MODE            0x0007
#define RME_CTRL_SYNC_REF              0x0008
#define RME_CTRL_DEV_OPTIONS           0x0009
#define RME_CTRL_LIMIT_BANDWIDTH       0x000a
#define RME_CTRL_INPUT_LEVEL           0x000b
#define RME_CTRL_OUTPUT_LEVEL          0x000c
#define RME_CTRL_INSTRUMENT_OPTIONS    0x000d
#define RME_CTRL_WCLK_SINGLE_SPEED     0x000e
#define RME_CTRL_PHONES_LEVEL          0x000f
#define RME_CTRL_INPUT_SOURCE          0x0010
#define RME_CTRL_FF400_PAD_SW          0x0013
#define RME_CTRL_FF400_INSTR_SW        0x0014

#define RME_CTRL_FLASH                 0x0050
#define RME_CTRL_MIXER_PRESET          0x0051

#define RME_CTRL_INFO_MODEL            0x0100
#define RME_CTRL_INFO_TCO_PRESENT      0x0200
#define RME_CTRL_INFO_SYSCLOCK_MODE    0x0300
#define RME_CTRL_INFO_SYSCLOCK_FREQ    0x0301
#define RME_CTRL_INFO_AUTOSYNC_FREQ    0x0310
#define RME_CTRL_INFO_AUTOSYNC_SRC     0x0311
#define RME_CTRL_INFO_SYNC_STATUS      0x0312
#define RME_CTRL_INFO_SPDIF_FREQ       0x0313

#define RME_CTRL_TCO_FIRST             0x0400
#define RME_CTRL_TCO_LTC_IN            0x0400
#define RME_CTRL_TCO_INPUT_LTC_VALID   0x0401
#define RME_CTRL_TCO_INPUT_LTC_FPS     0x0402
#define RME_CTRL_TCO_INPUT_LTC_DROPFRAME 0x0403
#define RME_CTRL_TCO_INPUT_VIDEO_TYPE  0x0404
#define RME_CTRL_TCO_INPUT_WORD_CLK    0x0405
#define RME_CTRL_TCO_INPUT_LOCK        0x0406
#define RME_CTRL_TCO_SYNC_SRC          0x0407
#define RME_CTRL_TCO_VIDEO_IN_TERM     0x0408
#define RME_CTRL_TCO_FREQ              0x0409
#define RME_CTRL_TCO_FRAME_RATE        0x040a
#define RME_CTRL_TCO_SAMPLE_RATE       0x040b
#define RME_CTRL_TCO_SAMPLE_RATE_OFS   0x040c
#define RME_CTRL_TCO_LAST              0x040c

/* Control types for an RmeSettingsMatrixCtrl object */
#define RME_MATRIXCTRL_NONE            0x0000
#define RME_MATRIXCTRL_GAINS           0x0001
#define RME_MATRIXCTRL_INPUT_FADER     0x0002
#define RME_MATRIXCTRL_PLAYBACK_FADER  0x0003
#define RME_MATRIXCTRL_OUTPUT_FADER    0x0004
#define RME_MATRIXCTRL_INPUT_MUTE      0x0005
#define RME_MATRIXCTRL_PLAYBACK_MUTE   0x0006
#define RME_MATRIXCTRL_OUTPUT_MUTE     0x0007
#define RME_MATRIXCTRL_INPUT_INVERT    0x0008
#define RME_MATRIXCTRL_PLAYBACK_INVERT 0x0009

/* Commands sent via RME_CTRL_FLASH values */
#define RME_CTRL_FLASH_SETTINGS_LOAD   0x0000
#define RME_CTRL_FLASH_SETTINGS_SAVE   0x0001
#define RME_CTRL_FLASH_MIXER_LOAD      0x0002
#define RME_CTRL_FLASH_MIXER_SAVE      0x0003


class Device;

class RmeSettingsCtrl
    : public Control::Discrete
{
public:
    RmeSettingsCtrl(Device &parent, unsigned int type, unsigned int info);
    RmeSettingsCtrl(Device &parent, unsigned int type, unsigned int info,
        std::string name, std::string label, std::string descr);
    virtual bool setValue(int v);
    virtual int getValue();

    virtual bool setValue(int idx, int v) { return setValue(v); };
    virtual int getValue(int idx) { return getValue(); };
    virtual int getMinimum() {return 0; };
    virtual int getMaximum() {return 0; };

protected:
    Device &m_parent;
    unsigned int m_type;
    unsigned int m_value, m_info;

};

class RmeSettingsMatrixCtrl
    : public Control::MatrixMixer
{
public:
    RmeSettingsMatrixCtrl(Device &parent, unsigned int type);
    RmeSettingsMatrixCtrl(Device &parent, unsigned int type,
        std::string name);

    virtual void show();

    bool hasNames() const { return true; }
    bool canConnect() const { return false; }

    virtual std::string getRowName(const int row);
    virtual std::string getColName(const int col);
    virtual int canWrite( const int, const int ) { return true; }
    virtual int getRowCount();
    virtual int getColCount();

    virtual double setValue(const int row, const int col, const double val);
    virtual double getValue(const int row, const int col);

    // functions to access the entire coefficient map at once
    virtual bool getCoefficientMap(int &) {return false;};
    virtual bool storeCoefficientMap(int &) {return false;};

protected:
    Device &m_parent;
    unsigned int m_type;
};

}
