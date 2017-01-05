equals(QMAKE_HOST.os, Windows): EXE_SUFFIX = .exe

defineTest(qtConfTest_detectNinja) {
    ninja = $$qtConfFindInPath("ninja$$EXE_SUFFIX")
    !isEmpty(ninja) {
        qtLog("Found ninja from path: $$ninja")
        qtRunLoggedCommand("$$ninja --version", version)|return(false)
        contains(version, "1.*"): return(true)
        qtLog("Ninja version too old")
    }
    qtLog("Building own ninja")
    return(false)
}

defineTest(qtConfTest_detectGn) {
    gn = $$qtConfFindInPath("gn$$EXE_SUFFIX")
    !isEmpty(gn) {
        qtRunLoggedCommand("$$gn --version", version)|return(false)
        #accept all for now
        contains(version, ".*"): return(true)
        qtLog("Gn version too old")
    }
    qtLog("Building own gn")
    return(false)
}
