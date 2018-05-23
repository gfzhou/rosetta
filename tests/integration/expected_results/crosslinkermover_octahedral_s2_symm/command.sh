
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

cd /local/vmullig/rosetta_git_devcopy3/Rosetta/main/tests/integration/new/crosslinkermover_octahedral_s2_symm

[ -x /local/vmullig/rosetta_git_devcopy3/Rosetta/main/source/bin/rosetta_scripts.default.linuxgccrelease ] || exit 1
/local/vmullig/rosetta_git_devcopy3/Rosetta/main/source/bin/rosetta_scripts.default.linuxgccrelease  @flags -database /local/vmullig/rosetta_git_devcopy3/Rosetta/main/database -testing:INTEGRATION_TEST  2>&1 \
    | egrep -vf ../../ignore_list \
    > log

test "${PIPESTATUS[0]}" != '0' && exit 1 || true  # Check if the first executable in pipe line return error and exit with error code if so

# The following should ensure that at least one PDB file was written
ls -1 *.pdb >/dev/null 2>/dev/null
test "${PIPESTATUS[0]}" != '0' && exit 1 || true  # Check if the first executable in pipe line return error and exit with error code if so

#
# After that, do whatever you want.
# Files will be diffed verbatim, so if you want to log output and compare it,
# you'll need to filter out lines that change randomly (e.g. timings).
# Prefixing your tests with "nice" is probably good form as well.
# Don't forget to use -testing:INTEGRATION_TEST  so results are reproducible.
# Here's a typical test for a Mini binary, assuming there's a "flags" file
# in this directory too:
#
## /local/vmullig/rosetta_git_devcopy3/Rosetta/main/source/bin/MY_MINI_PROGRAM.default.linuxgccrelease  @flags -database /local/vmullig/rosetta_git_devcopy3/Rosetta/main/database -testing:INTEGRATION_TEST  2>&1 \
##     | egrep -v 'Finished.+in [0-9]+ seconds.' \
##     | egrep -v 'Dunbrack library took .+ seconds to load' \
##     > log
#
# Or if you don't care whether the logging output changes:
#
## /local/vmullig/rosetta_git_devcopy3/Rosetta/main/source/bin/MY_MINI_PROGRAM.default.linuxgccrelease  @flags -database /local/vmullig/rosetta_git_devcopy3/Rosetta/main/database -testing:INTEGRATION_TEST  2>&1 \
##     > /dev/null
#