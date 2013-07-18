# This is a dummy .pro file used to prepare chromium .pak resource files.
# These files will then be bundled using the Qt Resource System.
TEMPLATE = subdirs

system(python ../build/scripts/build_resources.py)


