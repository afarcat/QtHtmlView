# Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
#   Use of this source code is governed by a Apache license that can be
#   found in the LICENSE file.

INCLUDEPATH += $$PWD/kde/ $$PWD/kde/kio

#kio
HEADERS += \
    $$PWD/kde/kio/global.h \
    $$PWD/kde/kio/kjob.h \
    $$PWD/kde/kio/kjob_p.h \
    $$PWD/kde/kio/job.h \
    $$PWD/kde/kio/job_p.h \
    $$PWD/kde/kio/metadata.h

SOURCES += \
    $$PWD/kde/kio/global.cpp \
    $$PWD/kde/kio/kjob.cpp \
    $$PWD/kde/kio/job.cpp \
    $$PWD/kde/kio/metadata.cpp

#kparts
HEADERS += \
    $$PWD/kde/kparts/browserarguments.h \
    $$PWD/kde/kparts/browserextension.h \
    $$PWD/kde/kparts/browserinterface.h \
    $$PWD/kde/kparts/event.h \
    $$PWD/kde/kparts/historyprovider.h \
    $$PWD/kde/kparts/htmlsettingsinterface.h \
    $$PWD/kde/kparts/openurlarguments.h \
    $$PWD/kde/kparts/part.h \
    $$PWD/kde/kparts/part_p.h \
    $$PWD/kde/kparts/partmanager.h \
    $$PWD/kde/kparts/readonlypart.h \
    $$PWD/kde/kparts/readonlypart_p.h \
    $$PWD/kde/kparts/windowargs.h

SOURCES += \
    $$PWD/kde/kparts/browserarguments.cpp \
    $$PWD/kde/kparts/browserextension.cpp \
    $$PWD/kde/kparts/browserinterface.cpp \
    $$PWD/kde/kparts/event.cpp \
    $$PWD/kde/kparts/historyprovider.cpp \
    $$PWD/kde/kparts/htmlsettingsinterface.cpp \
    $$PWD/kde/kparts/openurlarguments.cpp \
    $$PWD/kde/kparts/part.cpp \
    $$PWD/kde/kparts/partmanager.cpp \
    $$PWD/kde/kparts/readonlypart.cpp \
    $$PWD/kde/kparts/windowargs.cpp

ENABLE_KJS {
HEADERS += \
    $$PWD/kde/kparts/liveconnectextension.h \
    $$PWD/kde/kparts/scriptableextension.h \
    $$PWD/kde/kparts/scriptableextension_p.h

SOURCES += \
    $$PWD/kde/kparts/liveconnectextension.cpp \
    $$PWD/kde/kparts/scriptableextension.cpp
}
