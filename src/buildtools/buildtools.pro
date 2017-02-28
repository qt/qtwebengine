TEMPLATE = subdirs

ninja.file = ninja.pro
SUBDIRS += ninja

use?(gn) {
    gn.file = gn.pro
    gn.depends = ninja
    SUBDIRS += gn
}
