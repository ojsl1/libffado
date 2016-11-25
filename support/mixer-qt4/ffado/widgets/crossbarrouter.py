#
# Copyright (C) 2009 by Arnold Krille
#               2013 by Philippe Carriere
#
# This file is part of FFADO
# FFADO = Free Firewire (pro-)audio drivers for linux
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

from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import pyqtSignal
from PyQt4.QtGui import QFrame, QPainter, QGridLayout, QLabel, QComboBox
from PyQt4.QtGui import QWidget, QVBoxLayout, QHBoxLayout, QPushButton
import dbus, math

import logging
log = logging.getLogger("crossbarrouter")

class VuMeter(QFrame):
    def __init__(self, interface, output, input=None, parent=None):
        QFrame.__init__(self, parent)
        self.setLineWidth(1)
        self.setFrameStyle(QFrame.Panel|QFrame.Sunken)
        self.setMinimumSize(20, 20)

        self.level = 0

        self.interface = interface

        self.output = output

    def updateLevel(self, value):
        self.level = value
        self.update()

    def paintEvent(self, event):
        p = QPainter(self)
        value = self.level/4096
        r = self.rect()
        r.setHeight(r.height() * math.sqrt(value))
        r.moveBottom(self.rect().height())
        p.fillRect(r, self.palette().highlight())

class OutputSwitcher(QFrame):
    """
The name is a bit misleading. This widget selectes sources for a specified
destination.

In mixer-usage this widget is at the top of the input-channel. Because the input
of the mixer is an available output from the routers point.
"""
    MixerRoutingChanged = pyqtSignal()
    def __init__(self, interface, outname, parent):
        QFrame.__init__(self, parent)
        self.interface = interface
        self.outname = outname
        self.lastin = ""

        self.setLineWidth(1)
        self.setFrameStyle(QFrame.Sunken|QFrame.Panel)

        self.layout = QGridLayout(self)
        self.setLayout(self.layout)

        self.lbl = QLabel(self.outname, self)
        self.lbl.setToolTip("The name of the destination that is to be controlled here.")
        self.layout.addWidget(self.lbl, 0, 0)

        self.vu = VuMeter(self.interface, outname, parent=self)
        self.layout.addWidget(self.vu, 0, 1)

        sources = self.interface.getSourceNames()

        self.combo = QComboBox(self)
        self.combo.setToolTip("<qt>Select the source for this destination.<br>Each destination can only receive sound from one source at a time. But one source can send sound to multiple destinations.</qt>")
        self.layout.addWidget(self.combo, 1, 0, 1, 2)
        self.combo.addItem("Disconnected")
        self.combo.addItems(sources)
        src = self.interface.getSourceForDestination(self.outname)
        self.lastin = str(src)
        if src != "":
            self.combo.setCurrentIndex(self.combo.findText(src))
        else:
            self.combo.setCurrentIndex(0)
        self.combo.activated.connect(self.comboCurrentChanged)


    def peakValue(self, value):
        self.vu.updateLevel(value)
        pass

    def comboCurrentChanged(self, inname):
        #log.debug("comboCurrentChanged( %s )" % inname)
        if inname == self.lastin:
            return
        if self.lastin != "":
            self.interface.setConnectionState(self.lastin, self.outname, False)

        if inname != "Disconnected":
            if self.interface.setConnectionState(str(inname), self.outname, True):
                if self.outname[:5] == "Mixer" or self.lastin[:5] == "Mixer" or str(inname)[:5] == "Mixer":
                    self.MixerRoutingChanged.emit()
                self.lastin = str(inname)
            else:
                log.warning(" Failed to connect %s to %s" % (inname, self.outname))
        else:
            self.lastin = ""


class CrossbarRouter(QWidget):
    MixerRoutingChanged = pyqtSignal(name='MixerRoutingChanged')
    def __init__(self, servername, basepath, parent=None):
        QWidget.__init__(self, parent);
        self.bus = dbus.SessionBus()
        self.dev = self.bus.get_object(servername, basepath)
        self.interface = dbus.Interface(self.dev, dbus_interface="org.ffado.Control.Element.CrossbarRouter")

        self.settings = QtCore.QSettings(self)

        self.destinations = self.interface.getDestinationNames()
        self.outgroups = []
        for ch in self.destinations:
            tmp = str(ch).split(":")[0]
            if not tmp in self.outgroups:
                self.outgroups.append(tmp)

        self.biglayout = QVBoxLayout(self)
        self.setLayout(self.biglayout)

        self.toplayout = QHBoxLayout()
        self.biglayout.addLayout(self.toplayout)

        self.vubtn = QPushButton("Switch peak meters", self)
        self.vubtn.setCheckable(True)
        self.vubtn.toggled.connect(self.runVu)
        self.toplayout.addWidget(self.vubtn)

        self.layout = QGridLayout()
        self.biglayout.addLayout(self.layout)

        self.switchers = {}
        for out in self.destinations:
            btn = OutputSwitcher(self.interface, out, self)
            self.layout.addWidget(btn, int(out.split(":")[-1]) + 1, self.outgroups.index(out.split(":")[0]))
            self.switchers[out] = btn
            self.switchers[out].MixerRoutingChanged.connect(self.updateMixerRouting)

        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(200)
        self.timer.timeout.connections(self.updateLevels)

        self.vubtn.setChecked(self.settings.value("crossbarrouter/runvu", False).toBool())

    def __del__(self):
        print "CrossbarRouter.__del__()"
        self.settings.setValue("crossbarrouter/runvu", self.vubtn.isChecked())

    def runVu(self, run=True):
        #log.debug("CrossbarRouter.runVu( %i )" % run)
        if run:
            self.timer.start()
        else:
            self.timer.stop()
            for sw in self.switchers:
                self.switchers[sw].peakValue(0)

    def updateLevels(self):
        #log.debug("CrossbarRouter.updateLevels()")
        peakvalues = self.interface.getPeakValues()
        #log.debug("Got %i peaks" % len(peakvalues))
        for peak in peakvalues:
            #log.debug("peak = [%s,%s]" % (str(peak[0]),str(peak[1])))
            if peak[0] >= 0:
                self.switchers[peak[0]].peakValue(peak[1])

    def updateMixerRouting(self):
        self.MixerRoutingChanged.emit()

    def saveSettings(self, indent):
        routerSaveString = []
        routerSaveString.append('%s<dest_number>\n' % indent)
        routerSaveString.append('%s  %d\n' % (indent, len(self.destinations)))
        routerSaveString.append('%s</dest_number>\n' % indent)
        routerSaveString.append('%s<srcperdest>\n' % indent)
        routerSaveString.append('%s  2\n' % indent)
        routerSaveString.append('%s</srcperdest>\n' % indent)
        routerSaveString.append('%s<connections>\n' % indent)
        for out in self.destinations:
            routerSaveString.append('%s  ' % indent + out + ' ')
            routerSaveString.append(self.interface.getSourceForDestination(out) + '\n')
        routerSaveString.append('%s</connections>\n' % indent)
        return routerSaveString        

    def readSettings(self, routerReadString):
        sources = str(self.interface.getSourceNames())
        if routerReadString[0].find('<dest_number>') == -1:
            log.debug("Number of router destinations must be specified\n")
            return False
        if routerReadString[2].find('</dest_number>') == -1:
            log.debug("Incompatible xml file\n")
            return False
        n_dest = int(routerReadString[1])
        if n_dest != len(self.destinations):
            log.debug("Caution: numbers of destinations mismatch")
        if routerReadString[3].find('<srcperdest>') == -1:
            log.debug("Number of sources per destinations must be specified\n")
            return False
        if routerReadString[5].find('</srcperdest>') == -1:
            log.debug("Incompatible xml file\n")
            return False
        n_spd = int(routerReadString[4])
        if n_spd != 2:
            log.debug("Unable to handle more than one source for each destination;")
        try:
            idxb = routerReadString.index('<connections>')
            idxe = routerReadString.index('</connections>')
        except Exception:
            log.debug("Router connections not specified\n")
            idxb = -1
            idxe = -1
            return False
        if idxb >= 0:
            if idxe > idxb + 1:
                for s in routerReadString[idxb+1:idxe]:
                    destination = s.split()[0]
                    if str(self.destinations).find(destination) != -1:
                        source = s.split()[1]
                        if sources.find(source) != -1:                        
                            idx = self.switchers[destination].combo.findText(source)
                            self.switchers[destination].combo.setCurrentIndex(idx)
                            self.switchers[destination].comboCurrentChanged(source)
        return True
#
# vim: sw=4 ts=4 et
