TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = lib \ # Contains a dummy lib target that is used to generate qt_generated.gypi
          build \ # This is where we use the generated qt_generated.gypi and run gyp
          example \

