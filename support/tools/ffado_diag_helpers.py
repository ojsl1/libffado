#
# Copyright (C) 2008 Pieter Palmers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

#
# Test for common FFADO problems
#

# Make "print(..., end='')" functionality available under Python2
from __future__ import print_function

import glob
import sys
import os
import errno
import logging
import subprocess

## logging setup
logging.basicConfig()
log = logging.getLogger('diag')

## helper routines

# kernel
def get_kernel_version():
    return run_command (('uname', '-r')).rstrip ()

def get_kernel_rt_patched():
    l = run_command (('uname', '-v'))
    return "PREEMPT RT" in l

def get_kernel_preempt():
    l = run_command (('uname', '-v'))
    return (" PREEMPT " in l) and (not " RT " in l)

# modules
def check_for_module_loaded(modulename, procfile):
    log.info("Checking if module '%s' is present in %s... " % (modulename, procfile))
    with open (procfile) as f:
        for l in f:
            if modulename in l or modulename.replace('-', '_') in l:
                log.info(" found")
                return True
    log.info(" not found")
    return False

def check_for_module_present(modulename):
    log.info("Checking if module '%s' is present... " % modulename)
    kver = get_kernel_version()
    outtext = run_command (('find', '/lib/modules/' + kver, '-name', modulename + '.ko'))
    if not modulename in outtext:
        log.info(" not found")
        return False
    else:
        log.info(" found")
        return True

def check_1394oldstack_active():
    return check_for_module_loaded('ohci1394', '/proc/interrupts')

def check_1394oldstack_linked():
    return os.access('/sys/module/ohci1394', os.F_OK) and \
           os.access('/sys/module/raw1394',  os.F_OK)

def check_1394oldstack_loaded():
    for modulename in ('ieee1394', 'ohci1394', 'raw1394'):
        if not check_for_module_loaded(modulename, '/proc/modules'):
            return False
    return True

def check_1394oldstack_present():
    for modulename in ('ieee1394', 'ohci1394', 'raw1394'):
        if not check_for_module_present(modulename):
            return False
    return True

def check_1394newstack_active():
    return check_for_module_loaded('firewire_ohci', '/proc/interrupts')

def check_1394newstack_linked():
    return os.access('/sys/module/firewire_ohci', os.F_OK)

def check_1394newstack_loaded():
    for modulename in ('firewire_core', 'firewire_ohci'):
        if not check_for_module_loaded(modulename, '/proc/modules'):
            return False
    return True

def check_1394newstack_present():
    for modulename in ('firewire-core', 'firewire-ohci'):
        if not check_for_module_present(modulename):
            return False
    return True

def check_1394oldstack_devnode_present():
    return os.path.exists('/dev/raw1394')

def check_1394oldstack_devnode_permissions():
    try:
        with open('/dev/raw1394', 'w'):
            return True
    except:
        return False

# Raise an exception for any problem.
def run_command (cmd):
    outtext = subprocess.check_output (cmd)
    outtext = outtext.decode ('utf8')
    log.debug("%s outputs: %s" % (str (cmd), outtext))
    return outtext

# Wrapper intercepting common exceptions and returning a string nevertheless.
def run_command_string (cmd):
    try:
        return run_command (cmd)
    except OSError, ( errorcode, emsg ):
        if (errorcode == errno.ENOENT):
            msg = "Not found"
        else:
            msg = "Failed to execute %s" % str (cmd)
            log.warning (msg)
        return msg
    # Other errors are raised.
    except subprocess.CalledProcessError:
        msg = "Command %s returned a non-zero exit status" % str (cmd)
        log.warning (msg)
        return msg

# package versions
def get_package_version(name):
    return run_command_string (('pkg-config', '--modversion', name))

def get_package_flags(name):
    return run_command_string (('pkg-config', '--cflags', '--libs', name))

def get_version_first_line(cmd):
    outtext = run_command_string (cmd)
    i = outtext.find ("\n")
    if i == -1:
        return outtext
    else:
        return outtext [:i]

def list_host_controllers():
    try:
        lspci_cmd = run_command (('which', 'lspci')).rstrip ()
    except subprocess.CalledProcessError:
        lspci_cmd = "/sbin/lspci"
    outtext = run_command ((lspci_cmd,))
    for c in outtext.split("\n"):
        if '1394' in c:
            tmp = c.split()
            if len(tmp) > 0:
                cmd = (lspci_cmd, '-vv', '-nn', '-s', tmp[0])
                print( run_command(cmd) )

def get_juju_permissions():
    return run_command_string (['ls', '-lh'] + glob.glob ('/dev/fw*'))

def get_user_ids():
    return run_command_string (('id',));

def usage ():
    print ("")
    print ("Usage: %s [verboselevel]" % sys.argv [0])
    print ("  verboselevel : verbosity level.")
    print ("")
    sys.exit (0)

def parse_command_line ():
    num_args = len(sys.argv)
    if num_args > 2:
        usage ()
    elif num_args == 2:
        loglevel = int (sys.argv [1])
        if loglevel == 1:
            log.setLevel(logging.INFO)
        elif loglevel == 2:
            log.setLevel(logging.DEBUG)

def check_libraries ():
    print("   gcc ............... %s" % get_version_first_line(('gcc', '--version')))
    print("   g++ ............... %s" % get_version_first_line(('g++', '--version')))
    print("   PyQt4 (by pyuic4) . %s" % get_version_first_line(('pyuic4', '--version')))
    print("   PyQt5 (by pyuic5) . %s" % get_version_first_line(('pyuic5', '--version')))
    print("   jackd ............. %s" % get_version_first_line(('jackd', '--version')))
    print("     path ............ %s" % run_command_string (('which', 'jackd')), end='')
    print("     flags ........... %s" % get_package_flags("jack"))
    print("   libraw1394 ........ %s" % get_package_version("libraw1394"))
    print("     flags ........... %s" % get_package_flags("libraw1394"))
    print("   libavc1394 ........ %s" % get_package_version("libavc1394"))
    print("     flags ........... %s" % get_package_flags("libavc1394"))
    print("   libiec61883 ....... %s" % get_package_version("libiec61883"))
    print("     flags ........... %s" % get_package_flags("libiec61883"))
    print("   libxml++-2.6 ...... %s" % get_package_version("libxml++-2.6"))
    print("     flags ........... %s" % get_package_flags("libxml++-2.6"))
    print("   dbus-1 ............ %s" % get_package_version("dbus-1"))
    print("     flags ........... %s" % get_package_flags("dbus-1"))
