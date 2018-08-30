TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += htmlview

greaterThan(QT_MAJOR_VERSION, 4) {
    qtHaveModule(quick) {
        SUBDIRS += htmlqml
    }
}
