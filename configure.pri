
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
