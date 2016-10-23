#
# Copyright (C) 2009, 2011 by Jonathan Woithe
#
# This file is part of FFADO
# FFADO = Free Firewire (pro-)audio drivers for linux
#
# FFADO is based upon FreeBoB.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

from PyQt4 import QtGui

from PyQt4.QtCore import SIGNAL, SLOT, QObject, Qt, QTimer
from PyQt4.QtGui import QWidget, QApplication
from ffado.config import *

from ffado.widgets.matrixmixer import MatrixMixer

import logging
log = logging.getLogger('rme')

# Model defines.  These must agree with what is used in rme_avdevice.h.
RME_MODEL_NONE      = 0x0000
RME_MODEL_FF800     = 0x0001
RME_MODEL_FF400     = 0x0002

class Rme(QWidget):
    def __init__(self,parent = None):
        QWidget.__init__(self,parent)
        uicLoad("ffado/mixer/rme", self)

        self.init()

    def init(self):

        self.PhantomSwitches={
            self.phantom_0: ['/Control/Phantom', 0],
            self.phantom_1: ['/Control/Phantom', 1],
            self.phantom_2: ['/Control/Phantom', 2],
            self.phantom_3: ['/Control/Phantom', 3],
        }

        self.Switches={
            self.ff400_chan3_opt_instr: ['/Control/Chan3_opt_instr'],
            self.ff400_chan3_opt_pad:   ['/Control/Chan3_opt_pad'],
            self.ff400_chan4_opt_instr: ['/Control/Chan4_opt_instr'],
            self.ff400_chan4_opt_pad:   ['/Control/Chan4_opt_pad'],

            self.spdif_output_optical:  ['/Control/SPDIF_output_optical', 0],
            self.spdif_output_emphasis: ['/Control/SPDIF_output_emphasis', 0],
            self.spdif_output_pro:      ['/Control/SPDIF_output_pro', 0],
            self.spdif_output_nonaudio: ['/Control/SPDIF_output_nonaudio', 0],
        }

        self.Radiobuttons={
            self.level_in_lo_gain: ['/Control/Input_level', 0],
            self.level_in_p4dBu:   ['/Control/Input_level', 2],
            self.level_in_m10dBV:  ['/Control/Input_level', 1],

            self.level_out_hi_gain: ['/Control/Output_level', 2],
            self.level_out_p4dBu:   ['/Control/Output_level', 1],
            self.level_out_m10dBV:  ['/Control/Output_level', 0],

            self.spdif_input_coax:    ['/Control/SPDIF_input_mode', 0],
            self.spdif_input_optical: ['/Control/SPDIF_input_mode', 1],

            self.phones_hi_gain: ['/Control/Phones_level', 0],
            self.phones_p4dBu:   ['/Control/Phones_level', 1],
            self.phones_m10dBV:  ['/Control/Phones_level', 2],

            self.clock_mode_autosync: ['/Control/Clock_mode', 1],
            self.clock_mode_master: ['/Control/Clock_mode', 0],

            self.sync_ref_wordclk: ['/Control/Sync_ref', 0],
            self.sync_ref_adat1: ['/Control/Sync_ref', 1],
            self.sync_ref_adat2: ['/Control/Sync_ref', 2],
            self.sync_ref_spdif: ['/Control/Sync_ref', 3],
            self.sync_ref_tco: ['/Control/Sync_ref', 4],
            
            # TCO controls
            self.sync_source_ltc: ['/Control/Tco_sync_src', 0],
            self.sync_source_video: ['/Control/Tco_sync_src', 1],
            self.frame_rate_24fps: ['/Control/Tco_frame_rate', 0],
            self.frame_rate_25fps: ['/Control/Tco_frame_rate', 1],
            self.frame_rate_29_97fps: ['/Control/Tco_frame_rate', 2],
            self.frame_rate_29_97dfps: ['/Control/Tco_frame_rate', 3],
            self.frame_rate_30fps: ['/Control/Tco_frame_rate', 4],
            self.frame_rate_30dfps: ['/Control/Tco_frame_rate', 5],
            self.sample_rate_44k1: ['/Control/Tco_sample_rate', 0],
            self.sample_rate_48k: ['/Control/Tco_sample_rate', 1],
            self.sample_rate_0pc: ['/Control/Tco_sample_rate_ofs', 0],
            self.sample_rate_p01pc: ['/Control/Tco_sample_rate_ofs', 1],
            self.sample_rate_n01pc: ['/Control/Tco_sample_rate_ofs', 2],
            self.sample_rate_p40pc: ['/Control/Tco_sample_rate_ofs', 3],
            self.sample_rate_n40pc: ['/Control/Tco_sample_rate_ofs', 4],
        }

        self.Checkboxes={
            self.ch1_instr_fuzz: ['/Control/Chan1_instr_opts', 0x04],
            self.ch1_instr_limiter: ['/Control/Chan1_instr_opts', 0x08],
            self.ch1_instr_filter: ['/Control/Chan1_instr_opts', 0x02],

            # TCO controls
            self.video_input_termination_on: ['/Control/Tco_video_in_term', 0x01],
        }

        self.Gains={
            self.gain_mic1: ['/Control/Gains', 0],
            self.gain_mic2: ['/Control/Gains', 1],
            self.gain_input3: ['/Control/Gains', 2],
            self.gain_input4: ['/Control/Gains', 3],
        }

        self.Combos={
            self.ff800_ch1_src: ['/Control/Chan1_source'],
            self.ff800_ch7_src: ['/Control/Chan7_source'],
            self.ff800_ch8_src: ['/Control/Chan8_source'],
        }

        self.CommandButtons={
            self.control_load: ['/Control/Flash_control', 0],
            self.control_save: ['/Control/Flash_control', 1],
            self.mixer_load:   ['/Control/Flash_control', 2],
            self.mixer_save:   ['/Control/Flash_control', 3],
            self.mixer_preset_ffado_default: ['/Control/Mixer_preset', 0],
        }

        # Other mixer variables
        self.is_streaming = 0
        self.sample_rate = 0
        self.model = 0
        self.tco_present = 0

    # Public slot: update phantom power hardware switches
    def updatePhantomSwitch(self, a0):
        sender = self.sender()
        # Value is the phantom switch value, with a corresponding enable
        # bit in the high 16 bit word
        val = (a0 << self.PhantomSwitches[sender][1]) | (0x00010000 << self.PhantomSwitches[sender][1])
        log.debug("phantom switch %d set to %d" % (self.PhantomSwitches[sender][1], a0))
        self.hw.setDiscrete(self.PhantomSwitches[sender][0], val)

    # Public slot: update generic switches
    def updateSwitch(self, a0):
        sender = self.sender()
        log.debug("switch %s set to %d" % (self.Switches[sender][0], a0))
        self.hw.setDiscrete(self.Switches[sender][0], a0)

    # Public slot: update generic radiobuttons
    def updateRadiobutton(self, a0):
        sender = self.sender()
        if (a0 != 0):
            # Only change the control state on a button being "checked"
            log.debug("radiobutton group %s set to %d" % (self.Radiobuttons[sender][0], self.Radiobuttons[sender][1]))
            self.hw.setDiscrete(self.Radiobuttons[sender][0], self.Radiobuttons[sender][1])

    def updateCheckboxes(self, a0):
        sender = self.sender()
        val = self.hw.getDiscrete(self.Checkboxes[sender][0]);
        if (a0 != 0):
            val = val | self.Checkboxes[sender][1]
        else:
            val = val & ~self.Checkboxes[sender][1]
        log.debug("checkbox group %s set to %d" % (self.Checkboxes[sender][0], val));
        self.hw.setDiscrete(self.Checkboxes[sender][0], val)

    # Public slot: update gains
    def updateGain(self, a0):
        sender = self.sender()
        log.debug("gain %s[%d] set to %d" % (self.Gains[sender][0], self.Gains[sender][1], a0))
        self.hw.setMatrixMixerValue(self.Gains[sender][0], 0, self.Gains[sender][1], a0)

    def updateBandwidthLimit(self, a0):
        # Account for the "No ADAT-2" item which will not be present on
        # a FF400.
        if (self.model==RME_MODEL_FF400 and a0>0):
            a0 = a0 + 1
        # log.debug("limit update: %d" % (a0));
        self.hw.setDiscrete('/Control/Bandwidth_limit', a0);

    # Public slot: send command
    def sendCommand(self, a0):
        sender = self.sender()
        log.debug("command %d sent to %s" % (self.CommandButtons[sender][1], self.CommandButtons[sender][0]))
        self.hw.setDiscrete(self.CommandButtons[sender][0], self.CommandButtons[sender][1])

        # If mixer values have been reloaded, refresh the mixer GUI.  This
        # will also commit the new values to the hardware via the "changed"
        # signal handlers of the mixer elements.
        if (self.CommandButtons[sender][1] == 2):
            self.inputmatrix.refreshValues()
            self.outputmatrix.refreshValues()
            self.playbackmatrix.refreshValues()

        # If settings have been reloaded from flash, refresh the GUI.  The
        # settings will be made active in the hardware via the "changed"
        # signal handlers of the respective GUI control widgets.
        if (self.CommandButtons[sender][1] == 0):
            self.getValuesFromFF()

    def updateCombo(self, a0):
        sender = self.sender()
        log.debug("combo %s set to %d" % (self.Combos[sender][0], a0))
        self.hw.setDiscrete(self.Combos[sender][0], a0)

        # Enable the limiter control only when the front source is active
        if (sender == self.ff800_ch1_src):
            self.ch1_instr_limiter.setEnabled(a0==0)

    def updateStreamingState(self):
        ss = self.streamingstatus.selected()
        ss_txt = self.streamingstatus.getEnumLabel(ss)
        if ss_txt != 'Idle':
            self.is_streaming = True
        else:
            self.is_streaming = False
        if (self.last_streaming_state != self.is_streaming):
            self.bandwidth_limit.setEnabled(not(self.is_streaming));
            self.control_load.setEnabled(not(self.is_streaming));
        self.last_streaming_state = self.is_streaming

    def status_update(self):
        # log.debug("timer event")
        self.updateStreamingState()
        clk_mode = ['Master', 'Slave']
        src_str = ['None', 'ADAT 1', 'ADAT 2', 'SPDIF', 'Wordclock', 'TCO']
        sync_stat = ['No lock', 'Locked', 'Synced']
        sysclock_mode = self.hw.getDiscrete('/Control/sysclock_mode')
        sysclock_freq = self.hw.getDiscrete('/Control/sysclock_freq')
        autosync_freq = self.hw.getDiscrete('/Control/autosync_freq')
        autosync_src = self.hw.getDiscrete('/Control/autosync_src')
        sync_status = self.hw.getDiscrete('/Control/sync_status')
        spdif_freq = self.hw.getDiscrete('/Control/spdif_freq')
        self.sysclock_freq.setText("%d Hz" % (sysclock_freq))
        self.sysclock_mode.setText(clk_mode[sysclock_mode])
        self.autosync_freq.setText("%d Hz" % (autosync_freq))
        self.autosync_src.setText(src_str[autosync_src])
        self.sync_check_adat1_status.setText(sync_stat[sync_status & 0x03])
        self.sync_check_adat2_status.setText(sync_stat[(sync_status >> 2) & 0x03])
        self.sync_check_spdif_status.setText(sync_stat[(sync_status >> 4) & 0x03])
        self.sync_check_wclk_status.setText(sync_stat[(sync_status >> 6) & 0x03])
        self.sync_check_tco_status.setText(sync_stat[(sync_status >> 8) & 0x03])
        self.spdif_freq.setText("%d Hz" % (spdif_freq))
        
        if (self.tco_present):
            ltc_valid_str = ['Not detected', 'Valid']
            ltc_framerate_str = ['24 fps', '25 fps', '29.97 fps', '30 fps']
            ltc_frametype_str = ['Normal (full frame)', 'Dropframe']
            video_type_str = ['No video', 'PAL', 'NTSC']
            word_clock_str = ['None', 'Single Speed', 'Double Speed', 'Quad Speed']
            ltc = self.hw.getDiscrete('/Control/Tco_ltc_in')
            ltc_valid = self.hw.getDiscrete('/Control/Tco_input_ltc_valid')
            ltc_fps = self.hw.getDiscrete('/Control/Tco_input_ltc_fps')
            ltc_dropframe = self.hw.getDiscrete('/Control/Tco_input_ltc_dropframe')
            videotype = self.hw.getDiscrete('/Control/Tco_input_video_type')
            wordclk = self.hw.getDiscrete('/Control/Tco_input_word_clk')
            input_lock = self.hw.getDiscrete('/Control/Tco_input_lock')
            tco_freq = self.hw.getDiscrete('/Control/Tco_freq')
            self.ltc_in_hours.setText("%02d" % (ltc >> 24))
            self.ltc_in_minutes.setText("%02d" % ((ltc >> 16) & 0xff))
            self.ltc_in_seconds.setText("%02d" % ((ltc >> 8) & 0xff))
            self.ltc_in_frames.setText("%02d" % (ltc & 0xff))
            self.state_ltc_valid_label.setText(ltc_valid_str[ltc_valid])
            self.state_ltc_framerate.setText(ltc_framerate_str[ltc_fps])
            self.state_ltc_frame_type.setText(ltc_frametype_str[ltc_dropframe])
            self.state_video_type.setText(video_type_str[videotype])
            self.state_word_clk.setText(word_clock_str[wordclk])
            self.tco_frequency_label.setText("%d Hz" % (tco_freq))

    # Hide and disable a control
    def disable_hide(self,widget):
        for w in widget.children():
            if isinstance(w, QWidget):
                w.hide()
                w.setEnabled(False)
        widget.hide()
        widget.setEnabled(False)

    def setupSignals(self):

        # Connect signal handlers for all command buttons
        for ctrl, info in self.CommandButtons.iteritems():
            if (not(ctrl.isEnabled())):
                continue
            QObject.connect(ctrl, SIGNAL('clicked(bool)'), self.sendCommand)

        for ctrl, info in self.Combos.iteritems():
            if (not(ctrl.isEnabled())):
                continue;
            QObject.connect(ctrl, SIGNAL('currentIndexChanged(int)'), self.updateCombo)

        QObject.connect(self.bandwidth_limit, SIGNAL('currentIndexChanged(int)'), self.updateBandwidthLimit)

        # Get current hardware values and connect GUI element signals to 
        # their respective slots
        for ctrl, info in self.PhantomSwitches.iteritems():
            if (not(ctrl.isEnabled())):
                continue
            QObject.connect(ctrl, SIGNAL('toggled(bool)'), self.updatePhantomSwitch)

        for ctrl, info in self.Switches.iteritems():
            if (not(ctrl.isEnabled())):
                continue
            QObject.connect(ctrl, SIGNAL('toggled(bool)'), self.updateSwitch)

        for ctrl, info in self.Radiobuttons.iteritems():
            if (not(ctrl.isEnabled())):
                continue;
            QObject.connect(ctrl, SIGNAL('toggled(bool)'), self.updateRadiobutton)

        for ctrl, info in self.Checkboxes.iteritems():
            if (not(ctrl.isEnabled())):
                continue;
            QObject.connect(ctrl, SIGNAL('toggled(bool)'), self.updateCheckboxes)

        for ctrl, info in self.Gains.iteritems():
            if (not(ctrl.isEnabled())):
                continue
            QObject.connect(ctrl, SIGNAL('valueChanged(int)'), self.updateGain)

    # Obtain control values from the Fireface and make the GUI reflect these
    def getValuesFromFF(self):
        for ctrl, info in self.Combos.iteritems():
            if (not(ctrl.isEnabled())):
                continue;
            val = self.hw.getDiscrete(info[0])
            log.debug("combo %s is %d" % (info[0], val));
            ctrl.setCurrentIndex(val);

        # Set the bandwidth limit control to reflect the current device
        # setting, allowing for the additional "No ADAT-2" item which is
        # present on the FF800.
        val = self.hw.getDiscrete('/Control/Bandwidth_limit')
        if (self.model==RME_MODEL_FF400 and val>1):
            val = val - 1
        self.bandwidth_limit.setCurrentIndex(val);

        # Get current hardware values
        for ctrl, info in self.PhantomSwitches.iteritems():
            if (not(ctrl.isEnabled())):
                continue
            val = (self.hw.getDiscrete(info[0]) >> info[1]) & 0x01
            log.debug("phantom switch %d is %d" % (info[1], val))
            if val:
                ctrl.setChecked(True)
            else:
                ctrl.setChecked(False)

        for ctrl, info in self.Switches.iteritems():
            if (not(ctrl.isEnabled())):
                continue
            val = self.hw.getDiscrete(info[0])
            log.debug("switch %s is %d" % (info[0], val))
            if val:
                ctrl.setChecked(True)
            else:
                ctrl.setChecked(False)

        for ctrl, info in self.Radiobuttons.iteritems():
            if (not(ctrl.isEnabled())):
                continue;
            # This is a touch wasteful since it means we retrieve the control
            # value once per radio button rather than once per radio button
            # group.  In time we might introduce radiobutton groupings in the
            # self.* datastructures to avoid this, but for the moment this is
            # easy and it works.
            val = self.hw.getDiscrete(info[0])
            if (val == info[1]):
                val = 1
            else:
                val = 0
            ctrl.setChecked(val)
            log.debug("Radiobutton %s[%d] is %d" % (info[0], info[1], val))

        # Make sure the Limiter control can receive a value
        if (self.ff800_ch1_src.isEnabled()):
            self.ch1_instr_limiter.setEnabled(1)

        for ctrl, info in self.Checkboxes.iteritems():
            if (not(ctrl.isEnabled())):
                continue;
            # This is a touch wasteful since it means we retrieve the control
            # value once per checkbox button rather than once per checkbox
            # group.  In time we might introduce checkbox groupings in the
            # self.* datastructures to avoid this, but for the moment this is
            # easy and it works.
            val = self.hw.getDiscrete(info[0])
            if (val & info[1]):
                val = 1
            else:
                val = 0
            ctrl.setChecked(val)
            log.debug("Checkbox %s[%d] is %d" % (info[0], info[1], val))

        # The limiter (a checkbox) can only be controlled if the front
        # source is selected for channel 1.
        ch1_src = self.ff800_ch1_src.currentIndex()
        if (self.ff800_ch1_src.isEnabled()):
            self.ch1_instr_limiter.setEnabled(ch1_src==0)

        for ctrl, info in self.Gains.iteritems():
            if (not(ctrl.isEnabled())):
                continue
            val = self.hw.getMatrixMixerValue(info[0], 0, info[1])
            log.debug("gain %s[%d] is %d" % (info[0], info[1], val))
            ctrl.setValue(val);



    def initValues(self):

        # print self.hw.servername
        # print self.hw.basepath
        self.inputmatrix = MatrixMixer(self.hw.servername, self.hw.basepath+"/Mixer/InputFaders", self, "Columns_are_inputs", 0x8000, self.hw.basepath+"/Mixer/InputMutes", self.hw.basepath+"/Mixer/InputInverts", True)
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.inputmatrix)
        self.mixer.setLayout(layout)

        self.playbackmatrix = MatrixMixer(self.hw.servername, self.hw.basepath+"/Mixer/PlaybackFaders", self, "Columns_are_inputs", 0x8000, self.hw.basepath+"/Mixer/PlaybackMutes", self.hw.basepath+"/Mixer/PlaybackInverts", True)
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.playbackmatrix)
        self.playbackmixer.setLayout(layout)

        self.outputmatrix = MatrixMixer(self.hw.servername, self.hw.basepath+"/Mixer/OutputFaders", self, "Columns_are_inputs", 0x8000, self.hw.basepath+"/Mixer/OutputMutes", None, True)
        layout = QtGui.QVBoxLayout()

        # This is a bit of a hack, but it works to ensure this single-row
        # matrix mixer doesn't fill the entire screen but also doesn't end
        # up with a pointless scrollbar.  The matrix mixer's minimum height
        # is 0 according to minimumHeight(), which is probably the
        # fundamental issue here; however, I've already wasted too much time
        # trying to get this to work so if the hack is effective we'll run
        # with it.
        self.outputmatrix.setMinimumHeight(150)
        layout.addWidget(self.outputmatrix, 0, Qt.AlignTop)
        self.outputmixer.setLayout(layout)

        self.is_streaming = False
        self.last_streaming_state = False

        # Disable the "load settings" button if streaming is active.  Its
        # enable state will be kept up to date by updateStreamingState().
        self.control_load.setEnabled(not(self.is_streaming))

        # Also disable other controls which are not yet implemented.
        self.mixer_preset_ffado_default.setEnabled(False)

        # Retrieve other device settings as needed and customise the UI
        # based on these options.
        self.model = self.hw.getDiscrete('/Control/Model')
        log.debug("device model identifier: %d" % (self.model))
        self.tco_present = self.hw.getDiscrete('/Control/TCO_present')
        log.debug("device has TCO: %d" % (self.tco_present))
        #self.sample_rate = self.hw.getDiscrete('/Mixer/Info/SampleRate')
        #log.debug("device sample rate: %d" % (self.sample_rate))

        # Assume the TCO options tab is the second from the left (index 1)
        if (not(self.tco_present)):
            self.disable_hide(self.tco_options);
            self.tabWidget.setTabEnabled(1, False)
            self.tabWidget.removeTab(1)

        # The Fireface-400 only has 2 phantom-capable channels
        if (self.model == RME_MODEL_FF400):
            self.disable_hide(self.phantom_2)
            self.disable_hide(self.phantom_3)
        else:
            self.phantom_0.setText("Mic 7")
            self.phantom_1.setText("Mic 8")
            self.phantom_2.setText("Mic 9")
            self.phantom_3.setText("Mic 10")

        # Instrument options, input jack selection controls and an ADAT2
        # input are applicable only to the FF800
        if (self.model != RME_MODEL_FF800):
            self.instrument_options_group.setEnabled(False)
            self.input_plug_select_group.setEnabled(False)
            self.sync_ref_adat2.setEnabled(False)
            self.sync_check_adat2_label.setEnabled(False)
            self.sync_check_adat2_status.setEnabled(False)

        if (not(self.tco_present)):
            self.sync_check_tco_label.setEnabled(False)
            self.sync_check_tco_status.setEnabled(False)
            self.sync_ref_tco.setEnabled(False)

        # Only the FF400 has specific channel 3/4 options, input gain
        # controls and switchable phones level
        if (self.model != RME_MODEL_FF400):
            # Hide the upper-level frame (and everything in it) to ensure it 
            # requests no vertical space when its contents aren't needed.
            self.disable_hide(self.igains_chan34_opts_frame)
            self.phones_level_group.setEnabled(False)

        # Add the "No ADAT-2" item to the bandwidth limit control if the
        # device is not a FF400.
        if (self.model != RME_MODEL_FF400):
            self.bandwidth_limit.insertItem(1, "No ADAT-2")

        self.getValuesFromFF()
        self.setupSignals()

        # Ensure the limiter checkbox has a signal handler associated with
        # it.  If getValuesFromFF() disabled it because the front input was
        # not selected, setupSignals() would not have configured a handler.
        if (not(self.ch1_instr_limiter.isEnabled())):
            QObject.connect(self.ch1_instr_limiter, SIGNAL('toggled(bool)'), self.updateCheckboxes)

        self.updateStreamingState()
        #log.debug("device streaming flag: %d" % (self.is_streaming))

        self.update_timer = QTimer(self)
        QObject.connect(self.update_timer, SIGNAL('timeout()'), self.status_update)
        self.update_timer.start(1000)

    def saveSettings(self, indent):
        saveString = []
        idt = indent + "  "
        saveString.append('%s<inputmatrix>\n' % indent)
        saveString.extend(self.inputmatrix.saveSettings(idt))
        # Do not forget to mention the adopted rule for matrix columns mixer
        #  This might be useful for future import function
        saveString.append("%s  <col_rule>\n" % indent)
        saveString.append("%s    Columns_are_inputs\n" % indent)
        saveString.append("%s  </col_rule>\n" % indent)
        saveString.append('%s</inputmatrix>\n' % indent)

        saveString.append('%s<playbackmatrix>\n' % indent)
        saveString.extend(self.inputmatrix.saveSettings(idt))
        # Do not forget to mention the adopted rule for matrix columns mixer
        #  This might be useful for future import function
        saveString.append("%s  <col_rule>\n" % indent)
        saveString.append("%s    Columns_are_inputs\n" % indent)
        saveString.append("%s  </col_rule>\n" % indent)
        saveString.append('%s</playbackmatrix>\n' % indent)

        saveString.append('%s<outputmatrix>\n' % indent)
        saveString.extend(self.inputmatrix.saveSettings(idt))
        # Do not forget to mention the adopted rule for matrix columns mixer
        #  This might be useful for future import function
        saveString.append("%s  <col_rule>\n" % indent)
        saveString.append("%s    Columns_are_inputs\n" % indent)
        saveString.append("%s  </col_rule>\n" % indent)
        saveString.append('%s</outputmatrix>\n' % indent)
        return saveString

    def readSettings(self, readString):
        try:
            idxb = readString.index('<inputmatrix>')
            idxe = readString.index('</inputmatrix>')
        except Exception:
            log.debug("No Input matrix settings found")
            idxb = -1
            idxe = -1
        if idxb >= 0:
            if idxe > idxb + 1:
                stringMixer = []
                for s in readString[idxb+1:idxe]:
                    stringMixer.append(s)
                # When trying to import from a different device, the rule for column interpretation is
                # not necessarily the same
                try:
                    idx = stringMixer.index('<col_rule>')
                except Exception:
                    log.debug('Do not know how to handle column versus input/output')
                    idx = -1
                transpose_coeff = False
                if idx >=0:
                    if stringMixer[idx+1].find("Columns_are_inputs") == -1:
                        log.debug('Transposing the matrix coefficient; you are importing, are not you ?')
                        transpose_coeff = True
                if self.inputmatrix.readSettings(stringMixer, transpose_coeff):
                    log.debug("Input matrix settings modified")
                del stringMixer

        try:
            idxb = readString.index('<playbackmatrix>')
            idxe = readString.index('</playbackmatrix>')
        except Exception:
            log.debug("No Plaback matrix settings found")
            idxb = -1
            idxe = -1
        if idxb >= 0:
            if idxe > idxb + 1:
                stringMixer = []
                for s in readString[idxb+1:idxe]:
                    stringMixer.append(s)
                # When trying to import from a different device, the rule for column interpretation is
                # not necessarily the same
                try:
                    idx = stringMixer.index('<col_rule>')
                except Exception:
                    log.debug('Do not know how to handle column versus input/output')
                    idx = -1
                transpose_coeff = False
                if idx >=0:
                    if stringMixer[idx+1].find("Columns_are_inputs") == -1:
                        log.debug('Transposing the matrix coefficient; you are importing, are not you ?')
                        transpose_coeff = True
                if self.playbackmatrix.readSettings(stringMixer, transpose_coeff):
                    log.debug("Plaback matrix settings modified")
                del stringMixer

        try:
            idxb = readString.index('<outputmatrix>')
            idxe = readString.index('</outputmatrix>')
        except Exception:
            log.debug("No Output matrix settings found")
            idxb = -1
            idxe = -1
        if idxb >= 0:
            if idxe > idxb + 1:
                stringMixer = []
                for s in readString[idxb+1:idxe]:
                    stringMixer.append(s)
                # When trying to import from a different device, the rule for column interpretation is
                # not necessarily the same
                try:
                    idx = stringMixer.index('<col_rule>')
                except Exception:
                    log.debug('Do not know how to handle column versus input/output')
                    idx = -1
                transpose_coeff = False
                if idx >=0:
                    if stringMixer[idx+1].find("Columns_are_inputs") == -1:
                        log.debug('Transposing the matrix coefficient; you are importing, are not you ?')
                        transpose_coeff = True
                if self.outputmatrix.readSettings(stringMixer, transpose_coeff):
                    log.debug("Output matrix settings modified")
                del stringMixer

# vim: et
