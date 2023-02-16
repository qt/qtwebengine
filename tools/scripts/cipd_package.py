# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import glob
import os
import re
import subprocess
import sys
import version_resolver as resolver

androidx_package_name = 'chromium/third_party/androidx'

def subprocessCall(args):
    print(args)
    return subprocess.call(args)

def subprocessCheckOutput(args):
    print(args)
    return subprocess.check_output(args).decode()

class PackageDEPSParser(resolver.DEPSParser):
    def __init__(self):
        super().__init__()

    def createEntitiesFromScope(self, scope):
        entities = []
        for dep in scope:
            if (type(scope[dep]) == dict and 'packages' in scope[dep] \
            and 'dep_type' in scope[dep] and scope[dep]['dep_type'] == 'cipd'):
                subdir = self.subdir(dep)
                if subdir is None:
                    continue
                entity = CIPDEntity(subdir, sp=self.topmost_supermodule_path_prefix)
                path = entity.pathRelativeToTopMostSupermodule()
                for pkg in scope[dep]['packages']:
                   p = Package(pkg['package'], pkg['version'], path)
                   entity.packages.append(p)
                entities.append(entity)
        return entities

    def parse(self, deps_content, module_whitelist = []):
        exec(deps_content, self.global_scope, self.local_scope)
        entities = []
        entities.extend(self.createEntitiesFromScope(self.local_scope['deps']))
        return entities

class Package:
    def __init__(self, name, version, path):
        self.name = name
        self.version = version
        self.path = path
        self.fileName = name.replace('/','.') + '.pkg'

    def fetchAndDeploy(self):
        if os.path.isdir(self.path):
            currentDir = os.getcwd()
            os.chdir(self.path)
            subprocessCall(['cipd', 'pkg-fetch', self.name ,'-out' , self.fileName, '-version', self.version ])
            subprocessCall(['cipd', 'pkg-deploy', self.fileName , '-root', '.' ])
            os.chdir(currentDir)
        else:
            print('-- missing directory' + self.path + ' skipping')

    def listFiles(self):
        if os.path.isdir(self.path):
            currentDir = os.getcwd()
            os.chdir(self.path)
            files = []
            if os.path.isfile(self.fileName):
                files = subprocessCheckOutput(['cipd', 'pkg-inspect', self.fileName]).splitlines()
                files = map( lambda x: x.replace( ' F ', self.path + '/'), files)
            else:
                print('-- missing package file ' + self.path + ' skipping')
            os.chdir(currentDir)
            return files
        else:
            print('-- missing directory' + self.path + ' skipping')
        return []

class CIPDEntity:
    def __init__(self, path='', packages=[], os=[], sp=''):
        self.path = path
        self.packages = []
        self.topmost_supermodule_path_prefix = sp

    def pathRelativeToTopMostSupermodule(self):
        return os.path.normpath(os.path.join(self.topmost_supermodule_path_prefix, self.path))

    def findPackage(self, package):
        pkg = [p for p in self.packages if p.name == package]
        if len(pkg) > 1:
            raise Exception(package + " is ambiguous package name for" + self.path)
        return pkg[0] if pkg else None

    def readEntities(self):
        cipd_packages = []
        cipd_packages = resolver.read(PackageDEPSParser)
        print('DEPS file provides the following packages:')
        for cipd_package in cipd_packages:
            print(cipd_package.pathRelativeToTopMostSupermodule() + ':')
            for package in cipd_package.packages:
                print(' * {:<80}'.format(package.name) + package.version)
        return cipd_packages
