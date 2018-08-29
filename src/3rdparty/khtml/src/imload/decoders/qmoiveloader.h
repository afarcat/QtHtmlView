#ifndef QMOVIE_LOADER_H
#define QMOVIE_LOADER_H

#include "imageloaderprovider.h"

namespace khtmlImLoad
{

class ImageLoader;

class QMovieLoaderProvider: public ImageLoaderProvider
{
public:
    virtual ~QMovieLoaderProvider() {}
    Type type() override;

    ImageLoader *loaderFor(const QByteArray &prefix) override;
};

}

#endif
