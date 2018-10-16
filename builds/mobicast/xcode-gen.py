'''
Copyright (C) 2018  Bal Chettri
Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
'''
import os
import sys
import glob
import shutil
import posixpath

from pbxproj import XcodeProject

def xcgen_log(msg):
    print('[xcgen] ' + msg)
    
def xcgen_clean(projPath):
    xcgen_log('Cleaning ' + projPath)
    if os.path.isdir(projPath):
        shutil.rmtree(projPath)

def xcgen_copy_project(templatePath, projPath):
    xcgen_log('Copying from template ' + templatePath)
    shutil.copytree(templatePath, projPath)
    
def xcgen_append_files(fromPath, projPath, list):
    files = glob.glob(fromPath)    
    for filepath in files:
        base, filename = os.path.split(filepath)
        srcpath = posixpath.join(base, filename)
        relpath = posixpath.relpath(srcpath, projPath)          
        list.append(relpath)

def xcgen_build_file_list(paths, projPath, list):
    for path in paths:
        xcgen_append_files(path, projPath, list)

def xcgen_generate(projName, projPath, sourceFiles):
    xcgen_log('Generating project ' + projName)
    
    project = XcodeProject.load(os.path.join(projPath, projName + '.xcodeproj/project.pbxproj'))

    for filename in sourceFiles:
        project.add_file(filename, force=False)

    project.add_other_ldflags('-ObjC')
    
    project.save()

################################################################################

PROJECT_NAME = 'mobicast'

TEMPLATE_PATH = 'proj_xcode_template'
PROJECT_PATH = 'proj_xcode'

SRC_PATHS = [
'../../include/external/tinyxml2/*.cpp',
'../../src/http/*.cpp',
'../../src/mobicast/*.cpp',
'../../src/mobicast/platform/mac/*.m',
'../../src/mobicast/platform/mac/*.mm',
'../../src/mobicast/platform/mac/objc-js/*.mm'
]

def main(argv):
    xcgen_clean(PROJECT_PATH)
    
    xcgen_copy_project(TEMPLATE_PATH, PROJECT_PATH)
    
    sourceFiles = []
    xcgen_build_file_list(SRC_PATHS, PROJECT_PATH, sourceFiles)
    
    xcgen_generate(PROJECT_NAME, PROJECT_PATH, sourceFiles)
    xcgen_log('Done!')
    
if __name__ == '__main__':
    main(sys.argv[1:])
