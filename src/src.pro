TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += htmlview.pro

greaterThan(QT_MAJOR_VERSION, 4) {
    qtHaveModule(quick) {
        #SUBDIRS += htmlqml.pro
    }
}
