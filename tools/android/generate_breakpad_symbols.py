#!/usr/bin/env python

"""A tool to generate symbols for a binary suitable for breakpad.

Wrapper for components/crash/content/tools/generate_breakpad_symbols.py for
Android which takes the apk or aab and generates symbols for all libs
"""

import glob
import multiprocessing
import optparse
import os
import Queue
import re
import sys
import subprocess
import threading
import zipfile

CONCURRENT_TASKS=multiprocessing.cpu_count()

def GetDumpSymsBinary(build_dir=None):
  """Returns the path to the dump_syms binary."""
  DUMP_SYMS = 'dump_syms'
  dump_syms_bin = os.path.join(os.path.expanduser(build_dir), DUMP_SYMS)
  if not os.access(dump_syms_bin, os.X_OK):
    print 'Cannot find %s.' % dump_syms_bin
    return None

  return dump_syms_bin

def InvokeChromiumGenerateSymbols(options, lib_paths):
    """Invokes Chromium's script
    components/crash/content/tools/generate_breakpad_symbols.py for each lib
    of lib_paths."""

    queue = Queue.Queue()
    print_lock = threading.Lock()

    at_least_one_failed = multiprocessing.Value('b', False)

    chromium_script = os.path.join(options.src_root, 'components/crash/content/tools/generate_breakpad_symbols.py')


    def _Worker():
        while True:
            lib_path = queue.get()

            try:
                # Invoke the original Chromium script
                args = ['python',
                        chromium_script,
                        '--build-dir=' + options.build_dir,
                        '--symbols-dir=' + options.symbols_dir,
                        '--binary=' + lib_path,
                        '--platform=android',
                        '--verbose'
                        ]

                ret = subprocess.call(args)

                if (ret != 0):
                    # Lets fail just not to ignore something important
                    at_least_one_failed.value = True

            except Exception, e: # pylint: disable=broad-except
                if options.verbose:
                    with print_lock:
                        print(type(e))
                        print(e)
            finally:
                queue.task_done()

    for lib_path in lib_paths:
        queue.put(lib_path)

    for _ in range(options.jobs):
        t = threading.Thread(target=_Worker)
        t.daemon = True
        t.start()

    queue.join()

    if at_least_one_failed.value:
        return 1

    return 0

def main():
    parser = optparse.OptionParser()
    parser.add_option('', '--build-dir', default='',
                    help='The build output directory.')
    parser.add_option('', '--symbols-dir', default='',
                    help='The directory where to write the symbols file.')
    parser.add_option('', '--package-path', default='',
                    help='The path of the apk or aab package to generate symbols for libs from it.')
    parser.add_option('', '--clear', default=False, action='store_true',
                    help='Clear the symbols directory before writing new '
                         'symbols.')
    parser.add_option('', '--src-root', default='',
                    help='The path of the root src Chromium\'s folder.')
    parser.add_option('-j', '--jobs', default=CONCURRENT_TASKS, action='store',
                    type='int', help='Number of parallel tasks to run.')
    parser.add_option('-v', '--verbose', action='store_true',
                    help='Print verbose status output.')

    (options, _) = parser.parse_args()

    if not options.symbols_dir:
        print "Required option --symbols-dir missing."
        return 1

    if not options.build_dir:
        print "Required option --build-dir missing."
        return 1

    if not options.package_path:
        print "Required option --package-path missing."
        return 1

    if not options.src_root:
        print "Required option --src-root missing."
        return 1


    if not GetDumpSymsBinary(options.build_dir):
        return 1

    file_name, extension = os.path.splitext(options.package_path)

    if extension != '.apk' and extension != '.aab':
        print('Input file is not apk or aab');
        return 1

    with zipfile.ZipFile(options.package_path, 'r') as zf:
        names = zf.namelist()

    libs = []
    if extension == '.aab':
        AAB_BASE_LIB_RE = re.compile('base/lib/[^/]*/\S*[.]so')
        libs = [s for s in zf.namelist() if AAB_BASE_LIB_RE.match(s)]
    elif extension == '.apk':
        APK_LIB_RE = re.compile('lib/[^/]*/\S*[.]so')
        libs = [s for s in zf.namelist() if APK_LIB_RE.match(s)]

    # Set of libs which were not created by us
    ignored_libs = {
        'libarcore_sdk_c.so' # Augmented reality lib from Android SDK
        }

    # Additional ABIs
    additional_abi_dirs = glob.glob(os.path.join(options.build_dir, 'android_clang_*'))

    if len(additional_abi_dirs) > 1:
        # We have more than one additional ABIs. This may be in theory, but
        # we never have seen this in real. Just in case fail the build to
        # investigate this when happens.
        return 1

    lib_names = set()
    for lib in libs:
        # Cut out 'crazy.' if exists, see //chrome/android/chrome_public_apk_tmpl.gni
        lib_name = os.path.basename(lib).replace('crazy.', '')
        if not lib_name in ignored_libs:
            lib_names.add(lib_name)

    libs_result = []
    for lib_name in lib_names:
        lib_path = os.path.join(options.build_dir, 'lib.unstripped', lib_name)
        if os.path.exists(lib_path):
            libs_result.append(lib_path)
        else:
            lib_path = os.path.join(options.build_dir, lib_name)
            if os.path.exists(lib_path):
                libs_result.append(lib_path)
            else:
                # Something is wrong, we met the lib in aab/apk, for which we
                # cannot find lib in the build dir
                print('Seeing lib don\'t know the location for: ' + lib_name);
                return 1

        if additional_abi_dirs:
            lib_path = os.path.join(additional_abi_dirs[0], 'lib.unstripped', lib_name)
            if os.path.exists(lib_path):
                libs_result.append(lib_path)

    # Remove the duplicates, can have in the case when apk/aab contains two abis
    libs_result = list(set(libs_result))

    # Clear the dir
    if options.clear:
        try:
            shutil.rmtree(options.symbols_dir)
        except: # pylint: disable=bare-except
            pass

    InvokeChromiumGenerateSymbols(options, libs_result)

    return 0


if __name__ == '__main__':
    sys.exit(main())
