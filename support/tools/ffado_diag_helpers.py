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

import glob
import sys
import os
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

def run_command(cmd):
    try:
        outtext = subprocess.check_output (cmd).decode ()
    except:
        return ""
    log.debug("%s outputs: %s" % (str (cmd), outtext))
    return outtext

# package versions
def get_package_version(name):
    cmd = ('pkg-config', '--modversion', name)
    return run_command(cmd)

def get_package_flags(name):
    cmd = ('pkg-config', '--cflags', '--libs', name)
    return run_command(cmd)

def get_command_path(name):
    cmd = ('which', name)
    return run_command(cmd)

def get_version_first_line(cmd):
    ver = run_command(cmd).split("\n")
    if len(ver) == 0:
        ver = ["None"]
    if "sh: " in ver[0]:
        ver = ["Not found"]
    return ver[0]

def list_host_controllers():
    lspci_cmd = get_command_path("lspci")
    if lspci_cmd == "":
        lspci_cmd = "/sbin/lspci"
    outtext = run_command ((lspci_cmd,))
    for c in outtext.split("\n"):
      if '1394' in c:
        tmp = c.split()
        if len(tmp) > 0:
            cmd = (lspci_cmd, '-vv', '-nn', '-s', tmp[0])
            print( run_command(cmd) )

def get_juju_permissions():
    return run_command(('ls', '-lh') + tuple(glob.glob ('/dev/fw*')))

def get_user_ids():
    return run_command(('id',));

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
    print("     path ............ %s" % get_command_path('jackd'))
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
