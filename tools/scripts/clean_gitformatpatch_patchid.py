#!/usr/bin/env python
import re
import sys

for fileName in sys.argv[1:]:
    input = open(fileName)
    # Replace the SHA1 in the from-line of the mbox with a dummy SHA1 to prevent unneeded changes to the patch file.
    lines = [re.sub('^From \w*', 'From 0000000000000000000000000000000000000000', l) for l in input]
    input.close()
    output = open(fileName, 'w')
    output.writelines(lines)
    output.close()
