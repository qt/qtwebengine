include(common.pri)

qtConfig(build-qtwebengine-core):qtConfig(webengine-spellchecker) {
    qtConfig(webengine-native-spellchecker): gn_args += use_browser_spellchecker=true
    else: gn_args += use_browser_spellchecker=false
} else {
    gn_args += use_browser_spellchecker=false
}
