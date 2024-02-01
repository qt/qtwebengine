#!/usr/bin/env python3
# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import glob
import os
import subprocess
import sys
import importlib
import errno
import shutil

from distutils.version import StrictVersion
import git_submodule as GitSubmodule
import cipd_package as CIPDPackage

qtwebengine_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
os.chdir(qtwebengine_root)

def isInGitBlacklist(file_path):
    # We need all the git files.
    if ( '.gitignore' in file_path
        or '.gitmodules' in file_path
        or '.gitattributes' in file_path
        or '.DEPS' in file_path ):
        return True

def isInChromiumBlacklist(file_path):
    # Filter out empty submodule directories.
    if (os.path.isdir(file_path)):
        return True
    # We do need all the gn files.
    if file_path.endswith('.gn') or file_path.endswith('.gni') or file_path.endswith('.typemap') or \
    file_path.endswith('.mojom'):
        return False
    # Add android dependencies info so gn build tree can be parsed
    if file_path.endswith('.info') or file_path.endswith('.pydeps'):
        return False
    if file_path.endswith('.mailmap') or file_path.endswith('.tat.gz.sha1'):
        return True
    if (file_path.startswith('android_webview')
        or file_path.startswith('apps/')
        or file_path.startswith('ash/')
        or file_path.startswith('buildtools/clang_format/script')
        or file_path.startswith('buildtools/third_party/libc++')
        or file_path.startswith('buildtools/third_party/libc++abi')
        or file_path.startswith('buildtools/third_party/libunwind')
        or (file_path.startswith('chrome/')
          and not file_path.startswith('chrome/VERSION')
          and not file_path.startswith('chrome/app/resources/')
          and not file_path.startswith('chrome/app/theme/')
          and not file_path.startswith('chrome/browser/chrome_notification_types.h')
          and not file_path.startswith('chrome/browser/accessibility/')
          and not file_path.startswith('chrome/browser/custom_handlers/')
          and not file_path.startswith('chrome/browser/devtools/')
          and not file_path.startswith('chrome/browser/extensions/api/')
          and not file_path.startswith('chrome/browser/gcm/')
          and not file_path.startswith('chrome/browser/media/webrtc/')
          and not file_path.startswith('chrome/browser/net/')
          and not file_path.startswith('chrome/browser/prefs/')
          and not file_path.startswith('chrome/browser/printing/')
          and not file_path.startswith('chrome/browser/profiles/incognito_helpers')
          and not file_path.startswith('chrome/browser/profiles/profile_keyed_service_factory')
          and not file_path.startswith('chrome/browser/profiles/profile_selections')
          and not file_path.startswith('chrome/browser/push_messaging/')
          and not file_path.startswith('chrome/browser/renderer_host/')
          and not file_path.startswith('chrome/browser/share/core/')
          and not file_path.startswith('chrome/browser/share/proto/')
          and not file_path.startswith('chrome/browser/signin/')
          and not file_path.startswith('chrome/browser/spellchecker')
          and not file_path.startswith('chrome/browser/tab_contents/')
          and not file_path.startswith('chrome/browser/ui/webui/')
          and not (file_path.startswith('chrome/browser/resources/')
            and not '/chromeos/' in file_path
            and not '/settings/' in file_path
            and not '/mediarouter/' in file_path)
          and not (file_path.startswith('chrome/common/')
            and not file_path.startswith('chrome/common/extensions/docs'))
          and not file_path.startswith('chrome/renderer/')
          and not file_path.startswith('chrome/tools/convert_dict/')
          and not file_path.endswith('.grd')
          and not file_path.endswith('.grdp')
          and not file_path.endswith('chrome_version.rc.version')
          and not file_path.endswith('service_sandbox_type.h'))
        or file_path.startswith('chrome_elf')
        or file_path.startswith('chromecast')
        or file_path.startswith('chromeos')
        or file_path.startswith('cloud_print')
        or (file_path.startswith('components/')
          and (file_path.startswith('components/chrome_apps/')
            or file_path.startswith('components/cronet/')
            or file_path.startswith('components/drive/')
            or file_path.startswith('components/invalidation/')
            or file_path.startswith('components/nacl/')
            or file_path.startswith('components/omnibox/')
            or file_path.startswith('components/policy/resources/')
            or file_path.startswith('components/proximity_auth/')
            or (file_path.startswith('components/resources/terms/')
              and not file_path.endswith('terms_chromium.html'))
            or file_path.startswith('components/rlz/')
            or (file_path.startswith('components/sync/')
              and not file_path.endswith('ordinal.h'))
            or file_path.startswith('components/test/')
            or file_path.startswith('components/test_runner/')
            or file_path.startswith('components/translate/')
        ))
        or file_path.startswith('content/public/android/java')
        or file_path.startswith('content/shell/android/')
        or file_path.startswith('content/shell/browser/')
        or file_path.startswith('courgette')
        or file_path.startswith('docs/website/')
        or file_path.startswith('google_update')
        or file_path.startswith('ios')
        or file_path.startswith('media/base/android/java')
        or file_path.startswith('native_client_sdk')
        or file_path.startswith('net/android/java')
        or (file_path.startswith('net/data/')
          and '_unittest/' in file_path)
        or file_path.startswith('net/data/fuzzer_data/')
        or (file_path.startswith('remoting')
            and not file_path.endswith('VERSION')
            and not file_path.endswith('branding_Chromium')
            and not file_path.endswith('remove_spaces.py'))
        or file_path.startswith('rlz')
        or file_path.startswith('testing/android')
        or file_path.startswith('testing/buildbot')
        or (file_path.startswith('third_party/')
          and (file_path.startswith('third_party/WebKit/LayoutTests')
            or file_path.startswith('third_party/accessibility')
            or file_path.startswith('third_party/afl')
            or file_path.startswith('third_party/android_')
            or file_path.startswith('third_party/androidx')
            or file_path.startswith('third_party/angle/third_party/deqp')
            or file_path.startswith('third_party/angle/third_party/glmark2')
            or file_path.startswith('third_party/angle/third_party/VK-GL-CTS')
            or file_path.startswith('third_party/angle/third_party/vulkan-validation-layers')
            or file_path.startswith('third_party/apache-')
            or file_path.startswith('third_party/arcode-android-sdk')
            or file_path.startswith('third_party/ashmem')
            or file_path.startswith('third_party/binutils')
            or file_path.startswith('third_party/blink/perf_tests/')
            or file_path.startswith('third_party/blink/web_tests/')
            or file_path.startswith('third_party/breakpad/src/processor/testdata/')
            or file_path.startswith('third_party/boringssl/crypto_test_data.cc')
            or (file_path.startswith('third_party/cacheinvalidation')
              and not file_path.endswith('isolate'))
            or (file_path.startswith('third_party/catapult/')
              and not file_path.startswith('third_party/catapult/catapult_build')
              and not file_path.startswith('third_party/catapult/common')
              and not file_path.startswith('third_party/catapult/third_party/beautifulsoup4')
              and not file_path.startswith('third_party/catapult/third_party/html5lib-python')
              and not file_path.startswith('third_party/catapult/third_party/polymer')
              and not file_path.startswith('third_party/catapult/third_party/six')
              and not file_path.startswith('third_party/catapult/tracing'))
            or file_path.startswith('third_party/catapult/tracing/test_data/')
            or file_path.startswith('third_party/chromevox')
            or file_path.startswith('third_party/chromite')
            or file_path.startswith('third_party/colorama')
            or file_path.startswith('third_party/depot_tools')
            or file_path.startswith('third_party/devtools-frontend/src/third_party/image_diff')
            or (file_path.startswith('third_party/node/node_modules/')
              and not file_path.startswith('third_party/node/node_modules/@babel/')
              and not file_path.startswith('third_party/node/node_modules/@types/d3')
              and not file_path.startswith('third_party/node/node_modules/@types/trusted-types/')
              and not file_path.startswith('third_party/node/node_modules/ansi-styles/')
              and not file_path.startswith('third_party/node/node_modules/balanced-match/')
              and not file_path.startswith('third_party/node/node_modules/brace-expansion/')
              and not file_path.startswith('third_party/node/node_modules/cancel-token/')
              and not file_path.startswith('third_party/node/node_modules/chalk/')
              and not file_path.startswith('third_party/node/node_modules/color-convert/')
              and not file_path.startswith('third_party/node/node_modules/color-name/')
              and not file_path.startswith('third_party/node/node_modules/commander/')
              and not file_path.startswith('third_party/node/node_modules/concat-map/')
              and not file_path.startswith('third_party/node/node_modules/cssbeautify/')
              and not file_path.startswith('third_party/node/node_modules/debug/')
              and not file_path.startswith('third_party/node/node_modules/escape-string-regexp/')
              and not file_path.startswith('third_party/node/node_modules/esutils/')
              and not file_path.startswith('third_party/node/node_modules/function-bind/')
              and not file_path.startswith('third_party/node/node_modules/globals/')
              and not file_path.startswith('third_party/node/node_modules/has-ansi/')
              and not file_path.startswith('third_party/node/node_modules/has-flag/')
              and not file_path.startswith('third_party/node/node_modules/has/')
              and not file_path.startswith('third_party/node/node_modules/indent/')
              and not file_path.startswith('third_party/node/node_modules/is-core-module/')
              and not file_path.startswith('third_party/node/node_modules/is-windows/')
              and not file_path.startswith('third_party/node/node_modules/@jridgewell/')
              and not file_path.startswith('third_party/node/node_modules/js-tokens/')
              and not file_path.startswith('third_party/node/node_modules/jsesc/')
              and not file_path.startswith('third_party/node/node_modules/jsonschema/')
              and not file_path.startswith('third_party/node/node_modules/lodash.camelcase/')
              and not file_path.startswith('third_party/node/node_modules/lodash.sortby/')
              and not file_path.startswith('third_party/node/node_modules/minimatch/')
              and not file_path.startswith('third_party/node/node_modules/ms/')
              and not file_path.startswith('third_party/node/node_modules/path-is-inside/')
              and not file_path.startswith('third_party/node/node_modules/polymer-analyzer/')
              and not file_path.startswith('third_party/node/node_modules/polymer-css-build/')
              and not file_path.startswith('third_party/node/node_modules/resolve/')
              and not file_path.startswith('third_party/node/node_modules/rollup/')
              and not file_path.startswith('third_party/node/node_modules/shady-css-parser/')
              and not file_path.startswith('third_party/node/node_modules/source-map/')
              and not file_path.startswith('third_party/node/node_modules/stable/')
              and not file_path.startswith('third_party/node/node_modules/supports-color/')
              and not file_path.startswith('third_party/node/node_modules/terser/')
              and not file_path.startswith('third_party/node/node_modules/to-fast-properties/')
              and not file_path.startswith('third_party/node/node_modules/tr46/')
              and not file_path.startswith('third_party/node/node_modules/typescript/')
              and not file_path.startswith('third_party/node/node_modules/vscode-uri/')
              and not file_path.startswith('third_party/node/node_modules/webidl-conversions/')
              and not file_path.startswith('third_party/node/node_modules/whatwg-url/'))
            or file_path.startswith('third_party/fuschsia-sdk/')
            or file_path.startswith('third_party/glslang/src/Test/')
            or file_path.startswith('third_party/google_')
            or file_path.startswith('third_party/grpc/')
            or file_path.startswith('third_party/hunspell_dictionaries')
            or file_path.startswith('third_party/icu/cast')
            or file_path.startswith('third_party/icu/chromeos')
            or file_path.startswith('third_party/instrumented_libraries')
            or file_path.startswith('third_party/jsr-305')
            or file_path.startswith('third_party/junit')
            or file_path.startswith('third_party/lcov')
            or file_path.startswith('third_party/libaddressinput/src/testdata')
            or file_path.startswith('third_party/libaddressinput/src/common/src/test')
            or file_path.startswith('third_party/libFuzzer')
            or file_path.startswith('third_party/liblouis')
            or file_path.startswith('third_party/libphonenumber')
            or file_path.startswith('third_party/libunwindstack')
            or file_path.startswith('third_party/logilab')
            or file_path.startswith('third_party/markdown')
            or file_path.startswith('third_party/openh264/src/res')
            or file_path.startswith('third_party/openscreen/src/third_party/boringssl/')
            or file_path.startswith('third_party/pdfium/testing/resources')
            or file_path.startswith('third_party/pdfium/tools')
            or file_path.startswith('third_party/perl')
            or file_path.startswith('third_party/pylint')
            or file_path.startswith('third_party/sfntly/src/cpp/data/fonts')
            or file_path.startswith('third_party/sfntly/src/java')
            or file_path.startswith('third_party/skia/docs/')
            or file_path.startswith('third_party/skia/infra')
            or file_path.startswith('third_party/skia/site/docs/dev/tools/calendar.mskp')
            or file_path.startswith('third_party/sqlite/sqlite-src-')
            or file_path.startswith('third_party/spirv-cross/spirv-cross/reference/')
            or file_path.startswith('third_party/swiftshader/third_party/')
            or file_path.startswith('third_party/tflite/')
            or file_path.startswith('third_party/unrar')
            or file_path.startswith('third_party/wayland')
            or file_path.startswith('third_party/webgl')
            or file_path.startswith('third_party/webrtc/resources/')
            or file_path.startswith('third_party/webrtc/third_party/boringssl/crypto_test_data.cc')
        ))
        or (file_path.startswith('tools/')
          and (file_path.startswith('tools/android')
            or file_path.startswith('tools/luci_go')
            or file_path.startswith('tools/memory_inspector')
            or file_path.startswith('tools/perf')
            or file_path.startswith('tools/swarming_client')
        ))
        or (file_path.startswith('ui/')
          and (file_path.startswith('ui/android/java')
            or file_path.startswith('ui/app_list')
            or file_path.startswith('ui/base/ime/chromeos')
            or file_path.startswith('ui/chromeos')
            or file_path.startswith('ui/display/chromeos')
            or file_path.startswith('ui/events/ozone/chromeos')
            or file_path.startswith('ui/file_manager')
            or file_path.startswith('ui/gfx/chromeos')
        ))
        or '/android/java/' in file_path
        or ('/fuzz' in file_path
          and ('/fuzz/' in file_path
            or '/fuzzer/' in file_path
            or '/fuzzers/' in file_path
            or '/fuzzing/' in file_path
        ))
        or ('/test' in file_path
         and ('/testdata/' in file_path
           or '/tests/' in file_path
           or ('/test/' in file_path
             and not '/webrtc/' in file_path
             and not file_path.startswith('net/test/')
             and not file_path.endswith('test_hook.h')
             and not file_path.endswith('perftimer.h')
             and not file_path.endswith('test-torque.tq')
             and not 'ozone' in file_path
             and not 'clang_coverage' in file_path
             and not 'crypto/test/trampoline' in file_path
             and not 'core/mojo/test/' in file_path
             and not file_path.startswith('extensions/browser/')
             and
               (not file_path.startswith('base/test/')
                 or file_path.startswith('base/test/android/')
                 or file_path.startswith('base/test/data/')
                 or file_path.startswith('base/test/ios/')
                 or file_path.startswith('base/test/launcher/')
                 or file_path.startswith('base/test/library_loader/')
                 or file_path.startswith('base/test/metrics/')
               )
           )
        ))):
            return True
    return False

def printProgress(current, total):
    sys.stdout.write("\r{} of {}".format(current, total))
    sys.stdout.flush()

def copyFile(src, dst, use_link = True, force_remove = False):
    src = os.path.abspath(src)
    dst = os.path.abspath(dst)
    dst_dir = os.path.dirname(dst)

    if not os.path.isdir(dst_dir):
        os.makedirs(dst_dir)

    if force_remove or os.path.exists(dst):
        os.remove(dst)

    try:
        if use_link:
            os.link(src, dst)
        else:
            shutil.copy(src,dst)
        # Qt uses LF-only but Chromium isn't.
        subprocess.call(['dos2unix', '--keep-bom', '--quiet', dst])
    except OSError as exception:
        if exception.errno == errno.ENOENT:
            print('file does not exist: ' + src)
        else:
            raise

third_party_upstream = os.path.join(qtwebengine_root, 'src/3rdparty_upstream')
third_party = os.path.join(qtwebengine_root, 'src/3rdparty')

def clearDirectory(directory):
    currentDir = os.getcwd()
    os.chdir(directory)
    print('clearing the directory:' + directory)
    for direntry in os.listdir(directory):
        if not direntry == '.git' and os.path.isdir(direntry):
            print('clearing:' + direntry)
            shutil.rmtree(direntry)
    os.chdir(currentDir)

def listFilesInCurrentRepository(use_deps=False):
    currentRepo = GitSubmodule.Submodule(os.getcwd())
    files = subprocess.check_output(['git', 'ls-files']).splitlines()
    submodules = currentRepo.readSubmodules(use_deps)
    for submodule in submodules:
        submodule_files = submodule.listFiles()
        for submodule_file in submodule_files:
            files.append(os.path.join(submodule.pathRelativeToTopMostSupermodule(), submodule_file))
    return files

def listPackageFilesInCurrentRepositoryForPackage(packageName):
    cipd = CIPDPackage.CIPDEntity(os.getcwd())
    cipd_entities = cipd.readEntities()
    files = []
    for e in cipd_entities:
        pkg = e.findPackage(CIPDPackage.androidx_package_name)
        if pkg:
            files.extend(pkg.listFiles())
    return files

def exportGn():
    third_party_upstream_gn = os.path.join(third_party_upstream, 'gn')
    third_party_gn = os.path.join(third_party, 'gn')
    os.makedirs(third_party_gn);
    print('exporting contents of:' + third_party_upstream_gn)
    os.chdir(third_party_upstream_gn)
    files = listFilesInCurrentRepository()
    print('copying files to ' + third_party_gn)
    for i in range(len(files)):
        printProgress(i+1, len(files))
        f = files[i].decode()
        if not isInGitBlacklist(f):
            copyFile(f, os.path.join(third_party_gn, f))
    print("")

def exportNinja():
    third_party_upstream_ninja = os.path.join(third_party_upstream, 'ninja')
    third_party_ninja = os.path.join(third_party, 'ninja')
    os.makedirs(third_party_ninja);
    print('exporting contents of:' + third_party_upstream_ninja)
    os.chdir(third_party_upstream_ninja)
    files = listFilesInCurrentRepository()
    print('copying files to ' + third_party_ninja)
    for i in range(len(files)):
        printProgress(i+1, len(files))
        f = files[i].decode()
        if not isInGitBlacklist(f):
            copyFile(f, os.path.join(third_party_ninja, f))
    print("")

def exportChromium():
    third_party_upstream_chromium = os.path.join(third_party_upstream, 'chromium')
    third_party_chromium = os.path.join(third_party, 'chromium')
    os.makedirs(third_party_chromium);
    print('exporting contents of:' + third_party_upstream_chromium)
    os.chdir(third_party_upstream_chromium)
    files = listFilesInCurrentRepository(True)
    # Add LASTCHANGE files which are not tracked by git.
    files.append(b'build/util/LASTCHANGE')
    files.append(b'build/util/LASTCHANGE.committime')
    files.append(b'skia/ext/skia_commit_hash.h')
    files.append(b'gpu/config/gpu_lists_version.h')

    files.extend(listPackageFilesInCurrentRepositoryForPackage(CIPDPackage.androidx_package_name))

    for root, directories, local_files in os.walk(third_party_upstream_chromium + '/third_party/node/node_modules'):
        for name in local_files:
            f = os.path.relpath(os.path.join(root, name))
            files.append(f)

    symlinks = []
    print('copying files to ' + third_party_chromium)
    for i in range(len(files)):
        printProgress(i+1, len(files))
        if isinstance(files[i], bytes):
            f = files[i].decode()
        else:
            f = files[i]
        if not isInChromiumBlacklist(f) and not isInGitBlacklist(f):
            d = os.path.join(third_party_chromium, f)
            copyFile(f,d)
            # make sure we did not make a hardlink of symlink which is broken afterwards
            if os.path.islink(f):
              symlinks.append((f,d))
    # this is mostly used for files coming from cipd packages
    for s in symlinks:
        if not os.path.exists(s[1]):
            print('fixing ivalid link ' + s[1])
            copyFile(s[0],s[1], use_link = False, force_remove = True)

    # We need to gzip transport_security_state_static.json since it is otherwise too big for our git configuration:
    subprocess.call(['gzip', '-n',  third_party_chromium + '/net/http/transport_security_state_static.json'])
    print("")

commandNotFound = subprocess.call(['which', 'dos2unix'])

if not commandNotFound:
    dos2unixVersion, err = subprocess.Popen(['dos2unix', '-V', '| true'], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    if not dos2unixVersion:
        raise Exception("You need dos2unix version 6.0.6 minimum.")
    dos2unixVersion = StrictVersion(dos2unixVersion.splitlines()[0].split()[1].decode())

if commandNotFound or dos2unixVersion < StrictVersion('6.0.6'):
    raise Exception("You need dos2unix version 6.0.6 minimum.")

os.chdir(third_party)
ignore_case_setting = subprocess.Popen(['git', 'config', '--get', 'core.ignorecase'], stdout=subprocess.PIPE).communicate()[0]
if b'true' in ignore_case_setting:
    raise Exception("Your 3rdparty repository is configured to ignore case. "
                    "A snapshot created with these settings would cause problems on case sensitive file systems.")

clearDirectory(third_party)

exportGn()
exportNinja()
exportChromium()

print('done.')

