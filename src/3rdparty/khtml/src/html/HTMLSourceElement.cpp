/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
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

#include "HTMLSourceElement.h"
#include "HTMLDocument.h"

namespace khtml
{

HTMLSourceElement::HTMLSourceElement(Document *doc)
    : HTMLElement(doc)
{
}

HTMLSourceElement::~HTMLSourceElement()
{
}

DOM::NodeImpl::Id HTMLSourceElement::id() const
{
    return ID_SOURCE;
}

DOMString HTMLSourceElement::src() const
{
    return document()->completeURL(getAttribute(ATTR_SRC).string());
}

void HTMLSourceElement::setSrc(const DOMString &url)
{
    setAttribute(ATTR_SRC, url);
}

DOMString HTMLSourceElement::media() const
{
    return getAttribute(ATTR_MEDIA);
}

void HTMLSourceElement::setMedia(const DOMString &media)
{
    setAttribute(ATTR_MEDIA, media);
}

DOMString HTMLSourceElement::type() const
{
    return getAttribute(ATTR_TYPE);
}

void HTMLSourceElement::setType(const DOMString &type)
{
    setAttribute(ATTR_TYPE, type);
}

}
