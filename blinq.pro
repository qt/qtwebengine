TEMPLATE = subdirs

CONFIG += ordered

# The first two subdirs contain dummy .pro files that are used by qmake
# to generate a corresponding .gyp file
SUBDIRS = lib \
          process \
          build \ # This is where we use the generated qt_generated.gypi and run gyp
          example \

