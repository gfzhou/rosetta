#!/usr/bin/env python2

""" Configure a new build with the given options.

Equivalent to:
  $ rm -rf build_option1_option2   # Only if the -f flag is given.
  $ mkdir -p build_option1_option2
  $ cd build_option1_option2
  $ cmake ../build [-G generator]
  
This script is meant to work with `build/CMakeLists.txt', which itself is 
meant to make the cmake build system more flexible and less redundant.  One 
problem with the existing system is that each new build option leads to a 
combinatorial explosion of build directories.  Consider these options for a 
rosetta build:

Compile Type: release or debug
MPI Enabled: yes or no
MySQL Enabled: yes or no

To support all possible combinations of these options, 8 build directories 
would have to be created.  If a new option were to be added later on, another 
8 directories would need to be created.  Even worse, all of these directories 
are highly redundant.

The solution to this problem is to create a central configuration file which 
allows options to be independently enabled or disabled.  This is the role of 
`build/CMakeLists.txt'.  

However, one issue with this setup is that, as far as I can tell, there's no 
way to set the build directory from within cmake based on the given options.  
This information must be provided via command line arguments to cmake.
Unfortunately, cmake is not very well set up for compiling the same project 
to different directories when different options are given.  The only way to 
do this is to make the directory in advance and to invoke cmake from that 
directory, but this is cumbersome and hard to remember. 

The purpose of this script is to automate this process.  It takes options 
from the command line, creates a new build directory if necessary, and 
populates it with the build files generated by cmake.

Note that this script also provides a simple API for converting a list of 
options (as would be specified on the command line here) into the name of a 
build directory.  Perhaps this will be of use in some other script.
"""

def build_from_options(options, use_branch=False):
    options = set(options)
    prefix = ['build']

    if 'debug' in options:
        prefix.append('debug')
        options.remove('debug')
    else:
        prefix.append('release')
        options.discard('release')

    if use_branch is True:
        branch = current_branch_name()
        prefix.insert(1, branch)
    elif use_branch:
        prefix.insert(1, use_branch)
    else:
        pass

    return str.join('_', prefix + sorted(options))

def current_branch_name():
    import subprocess, shlex
    command = shlex.split('git symbolic-ref --short -q HEAD')
    return subprocess.check_output(command).strip()


if __name__ == '__main__':
    import argparse
    import os, shutil, subprocess

    parser = argparse.ArgumentParser(
            description=__doc__.strip(),
            formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument('options', nargs='*')
    parser.add_argument('--use-branch', '-b', action='store_true')
    parser.add_argument('--generator', '-G', default='Ninja')
    parser.add_argument('--force', '-f', action='store_true')
    parser.add_argument('--dry-run', '-n', action='store_true')
    arguments = parser.parse_args()

    path = build_from_options(arguments.options, arguments.use_branch)

    command = ['cmake', '../build', '-G', arguments.generator]
    for option in arguments.options:
        command += ['-D{}=ON'.format(option.upper())]
    
    if arguments.dry_run:
        print path

    else:
        if arguments.force:
            shutil.rmtree(path)
        if not os.path.exists(path):
            os.mkdir(path)
        os.chdir(path)
        subprocess.call(command)

