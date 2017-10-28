
#
# This is a command file.
#
# To make a new test, all you have to do is:
#   1.  Make a new directory under tests/
#   2.  Put a file like this (named "command") into that directory.
#
# The contents of this file will be passed to the shell (Bash or SSH),
# so any legal shell commands can go in this file.
# Or comments like this one, for that matter.
#
# Variable substiution is done using Python's printf format,
# meaning you need a percent sign, the variable name in parentheses,
# and the letter 's' (for 'string').
#
# Available variables include:
#   workdir     the directory where test input files have been copied,
#               and where test output files should end up.
#   minidir     the base directory where Mini lives
#   database    where the Mini database lives
#   bin         where the Mini binaries live
#   binext      the extension on binary files, like ".linuxgccrelease"
#
# The most important thing is that the test execute in the right directory.
# This is especially true when we're using SSH to execute on other hosts.
# All command files should start with this line:
#

cd /Users/rhiju/src/rosetta/main/tests/integration/new/swa_rna_gagu_01_append

#
# After that, do whatever you want.
# It's nice to test that the executable exists first, to ensure the test actually runs:
#
[ -x /Users/rhiju/src/rosetta/main/source/bin/swa_rna_main.default.macosgccrelease ] || exit 1
#
# Files will be diffed verbatim, so if you want to log output and compare it,
# you'll need to filter out lines that change randomly (e.g. timings).
# Prefixing your tests with "nice" is probably good form as well.
# Don't forget to use -testing:INTEGRATION_TEST  so results are reproducible.
# Here's a typical test for a Mini binary, assuming there's a "flags" file
# in this directory too:
#

/Users/rhiju/src/rosetta/main/source/bin/swa_rna_main.default.macosgccrelease  @flags -database /Users/rhiju/src/rosetta/main/database -testing:INTEGRATION_TEST  2>&1 \
    | egrep -vf ../../ignore_list \
    > log

test "${PIPESTATUS[0]}" != '0' && exit 1 || true  # Check if the first executable in pipe line return error and exit with error code if so
