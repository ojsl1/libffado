#
# Copyright (C) 2005-2008 by Pieter Palmers
#               2007-2008 by Arnold Krille
#
# This file is part of FFADO
# FFADO = Free Firewire (pro-)audio drivers for linux
#
# FFADO is based upon FreeBoB.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
import ffado.config
from ffado.configuration import *

import dbus
try:
    # First try the PyQt4 module name
    from dbus.mainloop.qt import DBusQtMainLoop
except ImportError:
    from dbus.mainloop.pyqt5 import DBusQtMainLoop

DBusQtMainLoop(set_as_default=True)

import logging
log = logging.getLogger('dbus')

class ControlInterface:
    def __init__(self, servername, basepath):
        self.basepath=basepath
        self.servername=servername
        if ffado.config.bypassdbus:
            self.devices = DeviceList(ffado.config.SYSTEM_CONFIG_FILE)
            self.devices.updateFromFile(ffado.config.USER_CONFIG_FILE)
        else:
            self.bus=dbus.SessionBus()

    def setContignuous(self, subpath, v, idx=None):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would set Continuous %s on server %s" % (path, self.servername))
            return
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Continuous')
            if idx == None:
                dev_cont.setValue(v)
            else:
                dev_cont.setValueIdx(idx,v)
        except:
            log.error("Failed to set Continuous %s on server %s" % (path, self.servername))

    def getContignuous(self, subpath, idx=None):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would get Continuous %s on server %s" % (path, self.servername))
            return 0
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Continuous')
            if idx == None:
                return dev_cont.getValue()
            else:
                return dev_cont.getValueIdx(idx)
        except:
            log.error("Failed to get Continuous %s on server %s" % (path, self.servername))
            return 0

    def setDiscrete(self, subpath, v, idx=None):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would set Discrete %s on server %s" % (path, self.servername))
            return
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Discrete')
            if idx == None:
                dev_cont.setValue(v)
            else:
                dev_cont.setValueIdx(v, idx)
        except:
            log.error("Failed to set Discrete %s on server %s" % (path, self.servername))

    def getDiscrete(self, subpath, idx=None):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
             log.info("bypassdbus set, would get Discrete %s on server %s" % (path, self.servername))
             return 0
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Discrete')
            if idx == None:
                return dev_cont.getValue()
            else:
                return dev_cont.getValueIdx(idx)
        except:
            log.error("Failed to get Discrete %s on server %s" % (path, self.servername))
            return 0

    def setText(self, subpath, v):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("DEBUG_BYPASSDBUS set, would set Text %s on server %s" % (path, self.servername))
            return
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Text')
            dev_cont.setValue(v)
        except:
            log.error("Failed to set Text %s on server %s" % (path, self.servername))

    def getText(self, subpath):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would get get Text %s on server %s" % (path, self.servername))
            return ""
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Text')
            return dev_cont.getValue()
        except:
            log.error("Failed to get Text %s on server %s" % (path, self.servername))
            return 0

    def setMatrixMixerValue(self, subpath, row, col, v):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would set MatrixMixer %s on server %s" % (path, self.servername))
            return
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.MatrixMixer')
            dev_cont.setValue(row, col, v)
        except:
            log.error("Failed to set MatrixMixer %s on server %s" % (path, self.servername))

    def getMatrixMixerValue(self, subpath, row, col):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would get MatrixMixer %s on server %s" % (path, self.servername))
            return 0
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.MatrixMixer')
            return dev_cont.getValue(row, col)
        except:
            log.error("Failed to get MatrixMixer %s on server %s" % (path, self.servername))
            return 0

    def enumSelect(self, subpath, v):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would select %s on server %s" % (path, self.servername))
            return
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Enum')
            dev_cont.select(v)
        except:
            log.error("Failed to select %s on server %s" % (path, self.servername))

    def enumSelected(self, subpath):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would get selected enum %s on server %s" % (path, self.servername))
            return 0
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Enum')
            return dev_cont.selected()
        except:
            log.error("Failed to get selected enum %s on server %s" % (path, self.servername))
            return 0

    def enumGetLabel(self, subpath, v):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would get enum label %s on server %s" % (path, self.servername))
            return 0
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Enum')
            return dev_cont.getEnumLabel(v)
        except:
            log.error("Failed to get enum label %s on server %s" % (path, self.servername))
            return 0

    def enumCount(self, subpath):
        path = self.basepath + subpath
        if ffado.config.bypassdbus:
            log.info("bypassdbus set, would get enum count %s on server %s" % (path, self.servername))
            return 0
        try:
            dev = self.bus.get_object(self.servername, path)
            dev_cont = dbus.Interface(dev, dbus_interface='org.ffado.Control.Element.Enum')
            return dev_cont.count()
        except:
            log.error("Failed to get enum count %s on server %s" % (path, self.servername))
            return 0

class DeviceManagerInterface:
    """ Implementation of the singleton """
    def __init__(self, servername, basepath):
        self.basepath=basepath + '/DeviceManager'
        self.servername=servername
        if ffado.config.bypassdbus:
            self.devices = DeviceList(ffado.config.SYSTEM_CONFIG_FILE)
            self.devices.updateFromFile(ffado.config.USER_CONFIG_FILE)
        else:
            self.bus=dbus.SessionBus()
            self.dev = self.bus.get_object(self.servername, self.basepath)
            self.iface = dbus.Interface(self.dev, dbus_interface='org.ffado.Control.Element.Container')

        self.updateSignalHandlers = []
        self.updateSignalHandlerArgs = {}
        self.preUpdateSignalHandlers = []
        self.preUpdateSignalHandlerArgs = {}
        self.postUpdateSignalHandlers = []
        self.postUpdateSignalHandlerArgs = {}
        self.destroyedSignalHandlers = []
        self.destroyedSignalHandlerArgs = {}

        if ffado.config.bypassdbus:
            return

        # signal reception does not work yet since we need a mainloop for that
        # and qt3 doesn't provide one for python/dbus
        try:
            log.debug("connecting to: Updated on %s (server: %s)" % (self.basepath, self.servername))
            self.dev.connect_to_signal("Updated", self.updateSignal, \
                                    dbus_interface="org.ffado.Control.Element.Container")
            self.dev.connect_to_signal("PreUpdate", self.preUpdateSignal, \
                                    dbus_interface="org.ffado.Control.Element.Container")
            self.dev.connect_to_signal("PostUpdate", self.postUpdateSignal, \
                                    dbus_interface="org.ffado.Control.Element.Container")
            self.dev.connect_to_signal("Destroyed", self.destroyedSignal, \
                                    dbus_interface="org.ffado.Control.Element.Container")

        except dbus.DBusException:
            traceback.print_exc()

    def registerPreUpdateCallback(self, callback, arg=None):
        if not callback in self.preUpdateSignalHandlers:
            self.preUpdateSignalHandlers.append(callback)
        # always update the argument
        self.preUpdateSignalHandlerArgs[callback] = arg

    def registerPostUpdateCallback(self, callback, arg=None):
        if not callback in self.postUpdateSignalHandlers:
            self.postUpdateSignalHandlers.append(callback)
        # always update the argument
        self.postUpdateSignalHandlerArgs[callback] = arg

    def registerUpdateCallback(self, callback, arg=None):
        if not callback in self.updateSignalHandlers:
            self.updateSignalHandlers.append(callback)
        # always update the argument
        self.updateSignalHandlerArgs[callback] = arg

    def registerDestroyedCallback(self, callback, arg=None):
        if not callback in self.destroyedSignalHandlers:
            self.destroyedSignalHandlers.append(callback)
        # always update the argument
        self.destroyedSignalHandlerArgs[callback] = arg

    def updateSignal(self):
        log.debug("Received update signal")
        for handler in self.updateSignalHandlers:
            arg = self.updateSignalHandlerArgs[handler]
            try:
                if arg:
                    handler(arg)
                else:
                    handler()
            except:
                log.error("Failed to execute handler %s" % handler)

    def preUpdateSignal(self):
        log.debug("Received pre-update signal")
        for handler in self.preUpdateSignalHandlers:
            arg = self.preUpdateSignalHandlerArgs[handler]
            try:
                if arg:
                    handler(arg)
                else:
                    handler()
            except:
                log.error("Failed to execute handler %s" % handler)

    def postUpdateSignal(self):
        log.debug("Received post-update signal")
        for handler in self.postUpdateSignalHandlers:
            arg = self.postUpdateSignalHandlerArgs[handler]
            try:
                if arg:
                    handler(arg)
                else:
                    handler()
            except:
                log.error("Failed to execute handler %s" % handler)

    def destroyedSignal(self):
        log.debug("Received destroyed signal")
        for handler in self.destroyedSignalHandlers:
            arg = self.destroyedSignalHandlerArgs[handler]
            try:
                if arg:
                    handler(arg)
                else:
                    handler()
            except:
                log.error("Failed to execute handler %s" % handler)

    def getNbDevices(self):
        if ffado.config.bypassdbus:
            return len(self.devices.devices)
        return self.iface.getNbElements()
    def getDeviceName(self, idx):
        if ffado.config.bypassdbus:
            return str(idx)
        return self.iface.getElementName(idx)

class ConfigRomInterface:
    def __init__(self, servername, devicepath):
        self.basepath=devicepath + '/ConfigRom'
        self.servername=servername
        if ffado.config.bypassdbus:
            self.devices = DeviceList(ffado.config.SYSTEM_CONFIG_FILE)
            self.devices.updateFromFile(ffado.config.USER_CONFIG_FILE)
            self.idx = int(devicepath)
        else:
            self.bus=dbus.SessionBus()
            self.dev = self.bus.get_object(self.servername, self.basepath)
            self.iface = dbus.Interface(self.dev, dbus_interface='org.ffado.Control.Element.ConfigRomX')
    def getGUID(self):
        if ffado.config.bypassdbus:
            return str(self.idx)
        return self.iface.getGUID()
    def getVendorName(self):
        if ffado.config.bypassdbus:
            return self.devices.devices[self.idx]['vendorname']
        return self.iface.getVendorName()
    def getModelName(self):
        if ffado.config.bypassdbus:
            return self.devices.devices[self.idx]['modelname']
        return self.iface.getModelName()
    def getVendorId(self):
        if ffado.config.bypassdbus:
            return int(self.devices.devices[self.idx]['vendorid'], 16)
        return self.iface.getVendorId()
    def getModelId(self):
        if ffado.config.bypassdbus:
            return int(self.devices.devices[self.idx]['modelid'], 16)
        return self.iface.getModelId()
    def getUnitVersion(self):
        if ffado.config.bypassdbus:
            return 0
        return self.iface.getUnitVersion()

class ClockSelectInterface:
    def __init__(self, servername, devicepath):
        self.basepath=devicepath + '/Generic/ClockSelect'
        self.servername=servername
        if ffado.config.bypassdbus:
            self.devices = DeviceList(ffado.config.SYSTEM_CONFIG_FILE)
            self.devices.updateFromFile(ffado.config.USER_CONFIG_FILE)
            self.idx = devicepath
        else:
            self.bus=dbus.SessionBus()
            self.dev = self.bus.get_object(self.servername, self.basepath)
            self.iface = dbus.Interface(self.dev, dbus_interface='org.ffado.Control.Element.AttributeEnum')
            self.iface_element = dbus.Interface(self.dev, dbus_interface='org.ffado.Control.Element.Element')
    def count(self):
        if ffado.config.bypassdbus:
            return 1
        return self.iface.count()
    def select(self, idx):
        if ffado.config.bypassdbus:
            return 1
        return self.iface.select(idx)
    def selected(self):
        if ffado.config.bypassdbus:
            return True
        return self.iface.selected()
    def getEnumLabel(self, idx):
        if ffado.config.bypassdbus:
            return 'enumlabel ' + str(idx)
        return self.iface.getEnumLabel(idx)
    def attributeCount(self):
        if ffado.config.bypassdbus:
            return 1
        return self.iface.attributeCount()
    def getAttributeValue(self, idx):
        if ffado.config.bypassdbus:
            return 1
        return self.iface.getAttributeValue(idx)
    def getAttributeName(self, idx):
        if ffado.config.bypassdbus:
            return 'attrib ' + str(idx)
        return self.iface.getAttributeName(idx)
    def canChangeValue(self):
        if ffado.config.bypassdbus:
            return 1
        return self.iface_element.canChangeValue()

class EnumInterface:
    def __init__(self, servername, basepath):
        self.basepath = basepath
        self.servername = servername
        if ffado.config.bypassdbus:
            self.devices = DeviceList(ffado.config.SYSTEM_CONFIG_FILE)
            self.devices.updateFromFile(ffado.config.USER_CONFIG_FILE)
        else:
            self.bus = dbus.SessionBus()
            self.dev = self.bus.get_object(self.servername, self.basepath)
            self.iface = dbus.Interface(self.dev, dbus_interface='org.ffado.Control.Element.Enum')
            self.iface_element = dbus.Interface(self.dev, dbus_interface='org.ffado.Control.Element.Element')
    def count(self):
        if ffado.config.bypassdbus:
            return 1
        return self.iface.count()
    def select(self, idx):
        if ffado.config.bypassdbus:
            return 1
        return self.iface.select(idx)
    def selected(self):
        if ffado.config.bypassdbus:
            return True
        return self.iface.selected()
    def getEnumLabel(self, idx):
        if ffado.config.bypassdbus:
            # Can't include text here since some code uses int() to extract
            # a value from the enum label
            return '0'
        return self.iface.getEnumLabel(idx)
    def canChangeValue(self):
        if ffado.config.bypassdbus:
            return True
        return self.iface_element.canChangeValue()
    def devConfigChanged(self, idx):
        if ffado.config.bypassdbus:
            return True
        return self.iface.devConfigChanged(idx)

class SamplerateSelectInterface(EnumInterface):
    def __init__(self, servername, devicepath):
        EnumInterface.__init__(self, servername, devicepath + '/Generic/SamplerateSelect')

class StreamingStatusInterface(EnumInterface):
    def __init__(self, servername, devicepath):
        EnumInterface.__init__(self, servername, devicepath + '/Generic/StreamingStatus')

class TextInterface:
    def __init__(self, servername, basepath):
        self.basepath=basepath
        self.servername=servername
        if ffado.config.bypassdbus:
            self.devices = DeviceList(ffado.config.SYSTEM_CONFIG_FILE)
            self.devices.updateFromFile(ffado.config.USER_CONFIG_FILE)
        else:
            self.bus=dbus.SessionBus()
            self.dev = self.bus.get_object( self.servername, self.basepath )
            self.iface = dbus.Interface( self.dev, dbus_interface="org.ffado.Control.Element.Text" )
            self.iface_element = dbus.Interface(self.dev, dbus_interface='org.ffado.Control.Element.Element')
    def text(self):
        if ffado.config.bypassdbus:
            return "text"
        return self.iface.getValue()
    def setText(self,text):
        if ffado.config.bypassdbus:
            return
        self.iface.setValue(text)
    def canChangeValue(self):
        if ffado.config.bypassdbus:
            return True
        return self.iface_element.canChangeValue()

# vim: et
