# Shared configuration for all our supported platforms

# Trigger Qt-specific build conditions.
GYP_CONFIG += use_qt=1
# We do not want to ship more external binary blobs, so let v8 embed its startup data.
GYP_CONFIG += v8_use_external_startup_data=0
# Disable printing since we don't support it yet
GYP_CONFIG += enable_basic_printing=1 enable_print_preview=0
# WebSpeech requires Google API keys and adds dependencies on speex and flac.
GYP_CONFIG += enable_web_speech=0
# We do not use or even include the extensions
GYP_CONFIG += enable_extensions=0
