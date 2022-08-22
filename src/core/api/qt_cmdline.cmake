# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_commandline_option(webengine-embedded-build TYPE boolean)
qt_commandline_option(webengine-pepper-plugins TYPE boolean)
qt_commandline_option(webengine-printing-and-pdf TYPE boolean)
qt_commandline_option(webengine-proprietary-codecs TYPE boolean)
qt_commandline_option(webengine-spellchecker TYPE boolean)
qt_commandline_option(webengine-native-spellchecker TYPE boolean)
qt_commandline_option(webengine-webrtc TYPE boolean)
qt_commandline_option(webengine-full-debug-info TYPE boolean)
qt_commandline_option(webengine-sanitizer TYPE boolean)

qt_commandline_option(webengine-jumbo-build TYPE jumbo)
function(qt_commandline_jumbo arg val nextok)
    if ("${val}" STREQUAL "")
        qtConfGetNextCommandlineArg(val)
    endif()
    if ("${val}" STREQUAL "no")
        qtConfCommandlineSetInput(webengine_jumbo_file_merge_limit 0)
    elseif("${val}" STREQUAL "")
        qtConfCommandlineSetInput(webengine_jumbo_file_merge_limit 8)
    elseif (val MATCHES "[0-9]+")
        qtConfCommandlineSetInput(webengine_jumbo_file_merge_limit ${val})
    else()
        qtConfAddError("Invalid argument '${val}' to command line parameter '${arg}'")
    endif()
endfunction()

set(systemLibs alsa ffmpeg freetype harfbuzz icu lcms2 libevent libjpeg
    libpng libvpx libxml libwebp minizip opus pulseaudio re2 snappy zlib)
foreach(slib ${systemLibs})
    qt_commandline_option(webengine-${slib} TYPE enum NAME webengine-system-${slib} VALUES yes no system)
endforeach()
