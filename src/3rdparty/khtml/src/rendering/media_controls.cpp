/*
 * Copyright (C) 2009 Michael Howell <mhowell123@gmail.com>.
 * Copyright (C) 2009 Germain Garand <germain@ebooksfrance.org>
 * Parts copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2018 afarcat <kabak@sina.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "media_controls.h"
#ifdef QT_WIDGETS_LIB
#include <QHBoxLayout>
#endif
//AFA #include <phonon/seekslider.h>
//AFA #include <phonon/mediaobject.h>
#include <rendering/render_media.h>
//AFA #include <phonon/videowidget.h>
//AFA #include <ktogglefullscreenaction.h>
//AFA #include <kglobalaccel.h>
//AFA #include <klocalizedstring.h>

namespace khtml
{

MediaControls::MediaControls(MediaPlayer *mediaPlayer, QWidget *parent) : QWidget(parent)
{
    m_mediaPlayer = mediaPlayer;
#ifdef QT_WIDGETS_LIB
    //AFA Phonon::MediaObject *mediaObject = m_mediaPlayer->mediaObject();
    setLayout(new QHBoxLayout(this));
    m_play = new QPushButton(QIcon::fromTheme("media-playback-start"), i18n("Play"), this);
    //AFA connect(m_play, SIGNAL(clicked()), mediaObject, SLOT(play()));
    layout()->addWidget(m_play);
    m_pause = new QPushButton(QIcon::fromTheme("media-playback-pause"), i18n("Pause"), this);
    //AFA connect(m_pause, SIGNAL(clicked()), mediaObject, SLOT(pause()));
    layout()->addWidget(m_pause);
    //AFA layout()->addWidget(new Phonon::SeekSlider(mediaObject, this));
    //AFA QAction *fsac = new KToggleFullScreenAction(this);
    //AFA fsac->setObjectName("KHTMLMediaPlayerFullScreenAction"); // needed for global shortcut activation.
    m_fullscreen = new QToolButton(this);
    //AFA m_fullscreen->setDefaultAction(fsac);
    m_fullscreen->setCheckable(true);
    //AFA connect(fsac, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
    layout()->addWidget(m_fullscreen);
#else
    m_play = nullptr;
    m_pause = nullptr;
    m_fullscreen = nullptr;
#endif

    //AFA slotStateChanged(mediaObject->state());
    //AFA connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), SLOT(slotStateChanged(Phonon::State)));
}

void MediaControls::slotToggled(bool t)
{
    if (t) {
        //AFA m_mediaPlayer->videoWidget()->enterFullScreen();
        //AFA KGlobalAccel::self()->setShortcut(m_fullscreen->defaultAction(), QList<QKeySequence>() << Qt::Key_Escape);
    } else {
        //AFA m_mediaPlayer->videoWidget()->exitFullScreen();
        //AFA KGlobalAccel::self()->removeAllShortcuts(m_fullscreen->defaultAction());
    }
}

//AFA
//void MediaControls::slotStateChanged(Phonon::State state)
//{
//    if (state == Phonon::PlayingState) {
//        m_play->hide();
//        m_pause->show();
//    } else {
//        m_pause->hide();
//        m_play->show();
//    }
//}

}

