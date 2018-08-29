/*
    Animation image load library -- QMovie decoder

    Copyright (C) 2018 afarcat <kabak@sina.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
    AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "qmoiveloader.h"

#include "animprovider.h"

#include "imageloader.h"
#include "imagemanager.h"
#include "pixmapplane.h"
#include "updater.h"

#include <QByteArray>
#include <QPainter>
#include <QVector>
#include <QBuffer>
#include <QImageReader>
#include <QMovie>
#include "khtml_debug.h"

#include <stdlib.h>

namespace khtmlImLoad
{

struct MovieFrameInfo {
    QImage  image;
    int     delay;
};

/**
 An anim provider for the animated QMovies. We keep a backing store for
 the screen.
*/
class QMovieAnimProvider : public AnimProvider
{
protected:
    QVector<MovieFrameInfo> frameInfos;
    int                   frame; // refers to the /current/ frame
    bool                  firstTime;
#ifdef FRAME_CACHE
    QMap<QString, QImage> frameAlter;
#endif

public:
    QMovieAnimProvider(PixmapPlane *plane, Image *img, QVector<MovieFrameInfo> frames):
        AnimProvider(plane, img), firstTime(true)
    {
        frameInfos = frames;
        frame     = 0;
    }

    void paint(int dx, int dy, QPainter *p, int sx, int sy, int width, int height) override
    {
    }

    void paint(int dx, int dy, int dw, int dh, QPainter *p, int sx, int sy, int sw, int sh) override
    {
        if (!dw || !dh) {
            return;    //Nothing to draw.
        }

        // Move over to next frame if need be, incorporating
        // the change effect of current one onto the screen.
        if (shouldSwitchFrame) {
            ++frame;
            if (frame >= frameInfos.count()) {
                if (animationAdvice == KHTMLSettings::KAnimationLoopOnce) {
                    animationAdvice = KHTMLSettings::KAnimationDisabled;
                }
                frame = 0;
            }
            nextFrame();
        }

        // Request next frame to be drawn...
        if (shouldSwitchFrame || firstTime) {
            shouldSwitchFrame = false;
            firstTime         = false;

            if (frameInfos.count() > 1) {
                ImageManager::animTimer()->nextFrameIn(this, frameInfos[frame].delay);
            }
        }

        // Render the currently active frame
#ifdef FRAME_CACHE
        QImage image = frameInfos[frame].image;
        //save diff size to speed
        if (image.width() != sw && image.height() != sh) {
            QString key = QString("%1-%2x%3").arg(frame).arg(sw).arg(sh);
            if (frameAlter.contains(key)) {
                image = frameAlter[key];
            }
            else {
                image = frameInfos[frame].image.scaled(sw, sh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                frameAlter[key] = image;
            }
        }
#else
        QImage image = frameInfos[frame].image.scaled(sw, sh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
#endif
        p->drawImage(dx, dy, image, sx, sy, dw, dh);
    }

    AnimProvider *clone(PixmapPlane *plane) override
    {
        if (frame0->height == 0 || frame0->width == 0 ||
                plane->height == 0 || plane->width == 0) {
            return nullptr;
        }

        return new QMovieAnimProvider(plane, image, frameInfos);
    }
};

class QMovieLoader : public ImageLoader
{
    QByteArray buffer;
public:
    QMovieLoader()
    {
    }

    ~QMovieLoader()
    {
    }

    int processData(uchar *data, int length) override
    {
        //Collect data in the buffer
        int pos = buffer.size();
        buffer.resize(buffer.size() + length);
        memcpy(buffer.data() + pos, data, length);
        return length;
    }

    int processEOF() override
    {
        QBuffer dataBuffer(&buffer);
        dataBuffer.open(QIODevice::ReadOnly);

        QMovie movie;
        movie.setCacheMode(QMovie::CacheAll);
        movie.setDevice(&dataBuffer);
        if (!movie.isValid()) {
            return Error;
        }

        QVector<MovieFrameInfo> frameInfos;

        // Check each frame to be within the size limit policy
        for (int frame = 0; frame < movie.frameCount(); ++frame) {
            movie.jumpToFrame(frame);

            QImage image = movie.currentImage();
            int w = image.width();
            int h = image.height();

            if (!ImageManager::isAcceptableSize(w, h)) {
                return Error;
            }

            if (frame == 0) {
                notifyImageInfo(w, h);
            }

            ImageFormat format;
            if (!imageFormat(image, format)) {
                return Error;
            }

            //Now we can declare frame format
            notifyAppendFrame(w, h, format);

            //
            MovieFrameInfo frameInfo;
            frameInfo.image = image;
            frameInfo.delay = movie.nextFrameDelay();

            frameInfos.append(frameInfo);
        }

        //need animation provider
        PixmapPlane *frame0  = requestFrame0();
        frame0->animProvider = new QMovieAnimProvider(frame0, image, frameInfos);

        return Done;
    }

    bool imageFormat(QImage &image, ImageFormat &format)
    {
        switch (image.format()) {
        case QImage::Format_RGB32:
            format.type  = ImageFormat::Image_RGB_32;
            break;
        case QImage::Format_ARGB32:
            format.type  = ImageFormat::Image_ARGB_32_DontPremult;
            break;
        case QImage::Format_ARGB32_Premultiplied:
            format.type  = ImageFormat::Image_ARGB_32;
            break;
        case QImage::Format_Indexed8:
            format.type  = ImageFormat::Image_Palette_8;
            format.palette = image.colorTable();
            break;
        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
            image = image.convertToFormat(QImage::Format_Indexed8);
            format.type  = ImageFormat::Image_Palette_8;
            format.palette = image.colorTable();
            break;
        case QImage::Format_Invalid:
        default:
            // unsupported formats
            return false;
        }
        return true;
    }
};

ImageLoaderProvider::Type QMovieLoaderProvider::type()
{
    return Efficient;
}

ImageLoader *QMovieLoaderProvider::loaderFor(const QByteArray &prefix)
{
    QByteArray pref = prefix;
    QBuffer prefixBuffer(&pref);
    prefixBuffer.open(QIODevice::ReadOnly);
    bool animation = QImageReader(&prefixBuffer, QByteArray()).supportsOption(QImageIOHandler::Animation);
    prefixBuffer.close();

    if (animation) {
        return new QMovieLoader;
    }

    return nullptr;
}

}

