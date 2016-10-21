
defineTest(qtConfTest_detectNinja) {
    ninja = $$qtConfFindInPath("ninja")
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
    gn = $$qtConfFindInPath("gn")
    !isEmpty(gn) {
        qtRunLoggedCommand("$$gn --version", version)|return(false)
        #accept all for now
        contains(version, ".*"): return(true)
        qtLog("Gn version too old")
    }
    qtLog("Building own gn")
    return(false)
}
