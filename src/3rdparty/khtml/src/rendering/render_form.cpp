/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2006 Maksim Orlovich (maksim@kde.org)
 *           (C) 2007-2009 Germain Garand (germain@ebooksfrance.org)
 *           (C) 2007 Mitz Pettel (mitz@webkit.org)
 *           (C) 2007 Charles Samuels (charles@kde.org)
 * Copyright (C) 2018 afarcat <kabak@sina.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "render_form.h"

//AFA #include <kcompletionbox.h>
//AFA #include <kcursor.h>
#include "khtml_debug.h"
//AFA #include <kfind.h>
//AFA #include <kfinddialog.h>
//AFA #include <klocalizedstring.h>
//AFA #include <kmessagebox.h>
//AFA #include <kreplace.h>
//AFA #include <kreplacedialog.h>
//AFA #include <sonnet/dialog.h>
//AFA #include <kurlcompletion.h>
//AFA #include <kwindowsystem.h>
//AFA #include <kstandardaction.h>
//AFA #include <kactioncollection.h>
//AFA #include <kdesktopfile.h>
//AFA #include <kconfiggroup.h>
//AFA #include <kbuildsycocaprogressdialog.h>
//AFA #include <kservicetypetrader.h>
//AFA #include <kservice.h>
//AFA #include <sonnet/backgroundchecker.h>
//AFA #include <sonnet/dialog.h>

#ifdef QT_WIDGETS_LIB
#include <QApplication>
#include <QAbstractItemView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QStyle>
#include <QStyleOptionButton>
#include <QLabel>
#include <QStyleOptionFrameV3>
#include <QListWidget>
#include <QProxyStyle>
#include <QMenu>
#include <QHBoxLayout>
#include <QVBoxLayout>
#endif

#include <QAbstractTextDocumentLayout>
#include <QDir>
#include <QStandardItemModel>

#include <misc/helper.h>
#include <xml/dom2_eventsimpl.h>
#include <html/html_formimpl.h>
#include <html/html_miscimpl.h>

#include <assert.h>

#include <khtmlview.h>
#include <khtml_ext.h>
#include <xml/dom_docimpl.h>

#include <QBitmap>

using namespace khtml;
using namespace DOM;

// ----------------- proxy style used to apply some CSS properties to native Qt widgets -----------------
#ifdef QT_WIDGETS_LIB
struct KHTMLProxyStyle : public QProxyStyle {
    KHTMLProxyStyle(QStyle *parent)
        : QProxyStyle(parent)
    {
        noBorder = false;
        left = right = top = bottom = 0;
        clearButtonOverlay = 0;
    }

    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override
    {
        QRect r = QProxyStyle::subElementRect(element, option, widget);
        switch (element) {
        case QStyle::SE_PushButtonContents:
        case QStyle::SE_LineEditContents:
        case QStyle::SE_ShapedFrameContents:
            r.adjust(left, top, -qMax(0, right - clearButtonOverlay), -bottom);
        default:
            break;
        }
        return r;
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override
    {
        if (element == QStyle::CE_ComboBoxLabel) {
            const QStyleOptionComboBox *o = qstyleoption_cast<const QStyleOptionComboBox *>(option);
            if (o) {
                QStyleOptionComboBox comboOpt = *o;
                comboOpt.currentText = comboOpt.currentText.trimmed();
                // by default combobox label is drawn left justified, vertical centered
                // translate it to reflect padding values
                comboOpt.rect.translate(left, (top - bottom) / 2);
                if (noBorder) {
                    // Need to expand a bit for some styles
                    comboOpt.rect.adjust(-1, -2, 1, 2);
                    comboOpt.state &= ~State_On;
                }
                return QProxyStyle::drawControl(element, &comboOpt, painter, widget);
            }
        }

        QProxyStyle::drawControl(element, option, painter, widget);
    }

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *painter, const QWidget *widget) const override
    {
        if ((cc == QStyle::CC_ComboBox) && noBorder) {
            if (const QStyleOptionComboBox *cbOpt = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
                bool enabled = (cbOpt->state & State_Enabled);
                QColor color = cbOpt->palette.color(QPalette::ButtonText);
                painter->save();
                painter->setBackgroundMode(Qt::TransparentMode);
                painter->setPen(color);
                painter->setRenderHint(QPainter::Antialiasing);
                // Drop down indicator
                QRect arrowRect = QProxyStyle::subControlRect(cc, opt, SC_ComboBoxArrow, widget);
                arrowRect.setTop(cbOpt->rect.top());
                arrowRect.setBottom(cbOpt->rect.bottom());
                arrowRect.setRight(cbOpt->rect.right() - 1);
                if (enabled && (cbOpt->state & State_On)) {
                    arrowRect.translate(1, 1);    // push effect
                }
                //if (!enabled) color = color.lighter();
                painter->setBrush(enabled ? QBrush(color, Qt::SolidPattern) : Qt::NoBrush);
                QPolygon cbArrowDown;
                cbArrowDown.setPoints(6,  3, -2, 4, -2, 0, 2, -4, -2, -3, -2, 0, 1);
                cbArrowDown.translate((arrowRect.x() + (arrowRect.width() >> 1)), (arrowRect.y() + (arrowRect.height() >> 1)));
                painter->drawPolygon(cbArrowDown);
                // Focus rect (from qcleanlooksstyle)
                if (enabled && (cbOpt->state & State_HasFocus)) {
                    QRect focusRect = QProxyStyle::subElementRect(SE_ComboBoxFocusRect, cbOpt, widget);
                    focusRect.adjust(0, -2, 0, 2);
                    painter->setBrush(QBrush(color, Qt::Dense4Pattern));
                    painter->setBrushOrigin(focusRect.topLeft());
                    painter->setPen(Qt::NoPen);
                    const QRect rects[4] = {
                        QRect(focusRect.left(), focusRect.top(), focusRect.width(), 1),    // Top
                        QRect(focusRect.left(), focusRect.bottom(), focusRect.width(), 1), // Bottom
                        QRect(focusRect.left(), focusRect.top(), 1, focusRect.height()),   // Left
                        QRect(focusRect.right(), focusRect.top(), 1, focusRect.height())   // Right
                    };
                    painter->drawRects(rects, 4);
                }
                painter->restore();

                return;
            }
        }

        QProxyStyle::drawComplexControl(cc, opt, painter, widget);
    }

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const override
    {
        // Make sure we give combo popup's enough room to display contents;
        // Qt doesn't do this by default

        if (cc == QStyle::CC_ComboBox && sc == SC_ComboBoxListBoxPopup) {
            const QComboBox *cb = qobject_cast<const QComboBox *>(widget);
            const QStyleOptionComboBox *cbOpt = qstyleoption_cast<const QStyleOptionComboBox *>(opt);

            if (cb && cbOpt) {
                QFontMetrics fm = cb->fontMetrics();
                // Compute content width; Qt uses the usual +4 magic number for icon/text margin
                int maxW = 0;
                for (int c = 0; c < cb->count(); ++c) {
                    int iw = fm.width(cb->itemText(c));
                    if (!cb->itemIcon(c).isNull()) {
                        iw += 4 + cb->iconSize().width();
                    }
                    maxW = qMax(maxW, iw);
                }

                // Now let sizeFromContent add in extra stuff.
                maxW = QProxyStyle::sizeFromContents(QStyle::CT_ComboBox, opt, QSize(maxW, 1), widget).width();

                // How much more room do we need for the text?
                int extraW = maxW > cbOpt->rect.width() ? maxW - cbOpt->rect.width() : 0;

                QRect r = QProxyStyle::subControlRect(cc, opt, sc, widget);
                r.setWidth(r.width() + extraW);
                return r;
            }
        }

        return QProxyStyle::subControlRect(cc, opt, sc, widget);
    }

    int left, right, top, bottom;
    int clearButtonOverlay;
    bool noBorder;
};
#endif

// ---------------------------------------------------------------------

RenderFormElement::RenderFormElement(HTMLGenericFormElementImpl *element)
    : RenderWidget(element)
//     , m_state(0)
#ifdef QT_WIDGETS_LIB
    , m_proxyStyle(nullptr)
#endif
    , m_exposeInternalPadding(false)
    , m_isOxygenStyle(false)
{
    // init RenderObject attributes
    setInline(true);   // our object is Inline
}

RenderFormElement::~RenderFormElement()
{}

void RenderFormElement::setStyle(RenderStyle *_style)
{
    RenderWidget::setStyle(_style);
    setPadding();
    if (!shouldDisableNativeBorders()) {
        // When the widget shows native border, clipping background to border
        // results in a nasty rendering effects
        if (style()->backgroundLayers()->backgroundClip() == BGBORDER) {
            style()->accessBackgroundLayers()->setBackgroundClip(BGPADDING);
        }
#ifdef QT_WIDGETS_LIB
        m_isOxygenStyle = QApplication::style()->objectName().contains("oxygen");
#endif
    }
}

int RenderFormElement::paddingTop() const
{
    return (!includesPadding() || m_exposeInternalPadding) ? RenderWidget::paddingTop() : 0;
}
int RenderFormElement::paddingBottom() const
{
    return (!includesPadding() || m_exposeInternalPadding) ? RenderWidget::paddingBottom() : 0;
}
int RenderFormElement::paddingLeft() const
{
    return (!includesPadding() || m_exposeInternalPadding) ? RenderWidget::paddingLeft() : 0;
}
int RenderFormElement::paddingRight() const
{
    return (!includesPadding() || m_exposeInternalPadding) ? RenderWidget::paddingRight() : 0;
}

bool RenderFormElement::includesPadding() const
{
    return true;
}

void RenderFormElement::setPadding()
{
    if (!includesPadding()) {
        return;
    }
#ifdef QT_WIDGETS_LIB
    KHTMLProxyStyle *style = static_cast<KHTMLProxyStyle *>(getProxyStyle());
    style->left = RenderWidget::paddingLeft();
    style->right = RenderWidget::paddingRight();
    style->top = RenderWidget::paddingTop();
    style->bottom = RenderWidget::paddingBottom();
#endif
}

#ifdef QT_WIDGETS_LIB
QProxyStyle *RenderFormElement::getProxyStyle()
{
    assert(widget());
    if (m_proxyStyle) {
        return m_proxyStyle;
    }
    m_proxyStyle = new KHTMLProxyStyle(widget()->style());
    widget()->setStyle(m_proxyStyle);
    return m_proxyStyle;
}
#endif

short RenderFormElement::baselinePosition(bool f) const
{
    return RenderWidget::baselinePosition(f) - 2 - style()->fontMetrics().descent();
}

void RenderFormElement::setQWidget(QWidget *w)
{
    // Avoid dangling proxy pointer when we switch widgets.
    // the widget will cleanup the proxy, as it is its kid.
#ifdef QT_WIDGETS_LIB
    m_proxyStyle = nullptr;
#endif

    // sets the Qt Object Name for the purposes
    // of setPadding() -- this is because QStyleSheet
    // will propagate children of 'w' even if they are toplevel, like
    // the "find" dialog or the popup menu
    w->setObjectName("RenderFormElementWidget");
    RenderWidget::setQWidget(w);
}

void RenderFormElement::updateFromElement()
{
    if (m_widget) {
#ifdef QT_WIDGETS_LIB
        m_widget->setEnabled(!element()->disabled());
#else
        QWidget *qmlWidget = m_qmlWidget;
        if (qmlWidget) {
            qmlWidget->setEnabled(!element()->disabled());
        }
#endif
    }

    // If we've disabled a focused element, clear its focus,
    // so Qt doesn't do funny stuff like let one type into a disabled
    // line edit.
    if (element()->disabled() && element()->focused()) {
        document()->quietResetFocus();
    }

    RenderWidget::updateFromElement();
}

// Some form widgets apply the padding internally (i.e. as if they were
// some kind of inline-block). Thus we only want to expose that padding
// while layouting (so that width/height calculations are correct), and
// then pretend it does not exist, as it is beyond the replaced edge and
// thus should not affect other calculations.

void RenderFormElement::calcMinMaxWidth()
{
    m_exposeInternalPadding = true;
    RenderWidget::calcMinMaxWidth();
    m_exposeInternalPadding = false;
}

void RenderFormElement::calcWidth()
{
    m_exposeInternalPadding = true;
    RenderWidget::calcWidth();
    m_exposeInternalPadding = false;
}

void RenderFormElement::calcHeight()
{
    m_exposeInternalPadding = true;
    RenderWidget::calcHeight();
    m_exposeInternalPadding = false;
}

void RenderFormElement::layout()
{
    KHTMLAssert(needsLayout());
    KHTMLAssert(minMaxKnown());

    // minimum height
    m_height = 0;
    calcWidth();
    calcHeight();

    if (m_widget)
        resizeWidget(m_width - borderLeft() - borderRight() - paddingLeft() - paddingRight(),
                     m_height - borderTop() - borderBottom() - paddingTop() - paddingBottom());

    setNeedsLayout(false);
}

int RenderFormElement::calcContentWidth(int w) const
{
    if (!shouldDisableNativeBorders()) {
        if (style()->boxSizing() == CONTENT_BOX) {
#ifdef QT_WIDGETS_LIB
            int nativeBorderWidth = m_widget->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, m_widget);
            return RenderBox::calcContentWidth(w) + 2 * nativeBorderWidth;
#endif
        }
    }

    return RenderBox::calcContentWidth(w);
}

int RenderFormElement::calcContentHeight(int h) const
{
    if (!shouldDisableNativeBorders()) {
        if (style()->boxSizing() == CONTENT_BOX) {
#ifdef QT_WIDGETS_LIB
            int nativeBorderWidth = m_widget->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, m_widget);
            return RenderBox::calcContentHeight(h) + 2 * nativeBorderWidth;
#endif
        }
    }

    return RenderBox::calcContentHeight(h);
}

void RenderFormElement::paintOneBackground(QPainter *p, const QColor &c, const BackgroundLayer *bgLayer, QRect clipr, int _tx, int _ty, int w, int height)
{
    int fudge = 0;
    if (!shouldDisableNativeBorders()) {
        fudge = m_isOxygenStyle ? 3 : 1;
    }

    paintBackgroundExtended(p, c, bgLayer, clipr, _tx, _ty, w, height,
                            fudge ? fudge : borderLeft(), fudge ? fudge : borderRight(), RenderWidget::paddingLeft(), RenderWidget::paddingRight(),
                            fudge ? fudge : borderTop(), fudge ? fudge : borderBottom(), RenderWidget::paddingTop(), RenderWidget::paddingBottom());
}

Qt::Alignment RenderFormElement::textAlignment() const
{
    switch (style()->textAlign()) {
    case LEFT:
    case KHTML_LEFT:
        return Qt::AlignLeft;
    case RIGHT:
    case KHTML_RIGHT:
        return Qt::AlignRight;
    case CENTER:
    case KHTML_CENTER:
        return Qt::AlignHCenter;
    case JUSTIFY:
    // Just fall into the auto code for justify.
    case TAAUTO:
        return style()->direction() == RTL ? Qt::AlignRight : Qt::AlignLeft;
    }
    assert(false); // Should never be reached.
    return Qt::AlignLeft;
}

// -------------------------------------------------------------------------

RenderButton::RenderButton(HTMLGenericFormElementImpl *element)
    : RenderFormElement(element)
{
    m_hasTextIndentHack = false;
}

short RenderButton::baselinePosition(bool f) const
{
    int ret = (height() - RenderWidget::paddingTop() - RenderWidget::paddingBottom() + 1) / 2;
    ret += marginTop() + RenderWidget::paddingTop();
    ret += ((fontMetrics(f).ascent()) / 2) - 1;
    return ret;
}

void RenderButton::layout()
{
    RenderFormElement::layout();
    bool needsTextIndentHack = false;
    if (!style()->width().isAuto()) {
        // check if we need to simulate the effect of a popular
        // button text hiding 'trick' that makes use of negative text-indent,
        // which we do not support on form widgets.
        int ti = style()->textIndent().minWidth(containingBlockWidth());
        if (m_widget && m_widget->width() <= qAbs(ti)) {
            needsTextIndentHack = true;
        }
    }
    if (m_hasTextIndentHack != needsTextIndentHack) {
        m_hasTextIndentHack = needsTextIndentHack;
        updateFromElement();
    }
}

void RenderButton::setStyle(RenderStyle *style)
{
    RenderFormElement::setStyle(style);
    if (shouldDisableNativeBorders()) {
        // we paint the borders ourselves on this button,
        // remove the widget's native ones.
#ifdef QT_WIDGETS_LIB
        KHTMLProxyStyle *style = static_cast<KHTMLProxyStyle *>(getProxyStyle());
        style->noBorder = true;
#endif
    }
}

// -------------------------------------------------------------------------------

RenderCheckBox::RenderCheckBox(HTMLInputElementImpl *element)
    : RenderButton(element)
{
#ifdef QT_WIDGETS_LIB
    CheckBoxWidget *b = new CheckBoxWidget(view()->widget());
    //b->setAutoMask(true);
    b->setMouseTracking(true);
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    connect(b, SIGNAL(stateChanged(int)), this, SLOT(slotStateChanged(int)));
#else
    CheckBoxWidget *b = new CheckBoxWidget(view());
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    connect(b, SIGNAL(stateChanged(int)), this, SLOT(slotStateChanged(int)));
#endif

    m_ignoreStateChanged = false;
}

void RenderCheckBox::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());

#ifdef QT_WIDGETS_LIB
    QCheckBox *cb = static_cast<QCheckBox *>(m_widget);
    QSize s(qMin(22, qMax(14, cb->style()->pixelMetric(QStyle::PM_IndicatorWidth))),
            qMin(22, qMax(12, cb->style()->pixelMetric(QStyle::PM_IndicatorHeight))));
    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());
#else
    QCheckBox *cb = static_cast<QCheckBox *>(m_widget);
    QSize s(qMin(22, qMax(14, 22)),
            qMin(22, qMax(12, 22)));
    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());
#endif

    RenderButton::calcMinMaxWidth();
}

void RenderCheckBox::updateFromElement()
{
    if (widget() && widget()->isChecked() != element()->checked()) {
        m_ignoreStateChanged = true;
        widget()->setChecked(element()->checked());
        m_ignoreStateChanged = false;
    }

    RenderButton::updateFromElement();
}

void RenderCheckBox::slotStateChanged(int state)
{
    if (m_ignoreStateChanged) {
        return;
    }
    element()->setChecked(state == Qt::Checked);
}

bool RenderCheckBox::handleEvent(const DOM::EventImpl &ev)
{
    switch (ev.id()) {
    case EventImpl::DOMFOCUSIN_EVENT:
    case EventImpl::DOMFOCUSOUT_EVENT:
    case EventImpl::MOUSEMOVE_EVENT:
    case EventImpl::MOUSEOUT_EVENT:
    case EventImpl::MOUSEOVER_EVENT:
        return RenderButton::handleEvent(ev);
    default:
        break;
    }
    return false;
}

// -------------------------------------------------------------------------------

RenderRadioButton::RenderRadioButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
#ifdef QT_WIDGETS_LIB
    RadioButtonWidget *b = new RadioButtonWidget(view()->widget());
    b->setMouseTracking(true);
    b->setAutoExclusive(false);
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    connect(b, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
#else
    RadioButtonWidget *b = new RadioButtonWidget(view());
    b->setAutoExclusive(false);
    setQWidget(b);

    // prevent firing toggled() signals on initialization
    b->setChecked(element->checked());

    //AFA connect(b, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
#endif
    m_ignoreToggled = false;
}

void RenderRadioButton::updateFromElement()
{
    m_ignoreToggled = true;
    if (widget()) {
        widget()->setChecked(element()->checked());
    }
    m_ignoreToggled = false;

    RenderButton::updateFromElement();
}

void RenderRadioButton::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());

#ifdef QT_WIDGETS_LIB
    QRadioButton *rb = static_cast<QRadioButton *>(m_widget);
    QSize s(qMin(22, qMax(14, rb->style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth))),
            qMin(20, qMax(12, rb->style()->pixelMetric(QStyle::PM_ExclusiveIndicatorHeight))));
    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());
#else
    QRadioButton *rb = static_cast<QRadioButton *>(m_widget);
    QSize s(qMin(22, qMax(14, 22)),
            qMin(22, qMax(12, 22)));
    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());
#endif

    RenderButton::calcMinMaxWidth();
}

void RenderRadioButton::slotToggled(bool /*activated*/)
{
    if (m_ignoreToggled) {
        return;
    }
}

bool RenderRadioButton::handleEvent(const DOM::EventImpl &ev)
{
    switch (ev.id()) {
    case EventImpl::DOMFOCUSIN_EVENT:
    case EventImpl::DOMFOCUSOUT_EVENT:
    case EventImpl::MOUSEMOVE_EVENT:
    case EventImpl::MOUSEOUT_EVENT:
    case EventImpl::MOUSEOVER_EVENT:
        return RenderButton::handleEvent(ev);
    default:
        break;
    }
    return false;
}

// -------------------------------------------------------------------------------

const QLatin1String sBorderNoneSheet("QPushButton{border:none}");

RenderSubmitButton::RenderSubmitButton(HTMLInputElementImpl *element)
    : RenderButton(element)
{
#ifdef QT_WIDGETS_LIB
    PushButtonWidget *p = new PushButtonWidget(view()->widget());
    setQWidget(p);
    //p->setAutoMask(true);
    p->setMouseTracking(true);
    p->setDefault(false);
    p->setAutoDefault(false);
#else
    PushButtonWidget *p = new PushButtonWidget(view());
    setQWidget(p);
#endif
}

static inline void setStyleSheet_helper(const QString &s, QWidget *w)
{
    // ### buggy Qt stylesheets mess with the widget palette.
    // force it again after any stylesheet update.
#ifdef QT_WIDGETS_LIB
    QPalette pal = w->palette();
    w->setStyleSheet(s);
    w->setPalette(pal);
#endif
}

void RenderSubmitButton::setPadding()
{
    // Proxy styling doesn't work well enough for buttons.
    // Use stylesheets instead. tests/css/button-padding-top.html
#ifdef QT_WIDGETS_LIB
    assert(!m_proxyStyle);

    if (!includesPadding()) {
        return;
    }

    if (!RenderWidget::paddingLeft() && !RenderWidget::paddingRight() &&
            !RenderWidget::paddingTop() && !RenderWidget::paddingBottom()) {
        setStyleSheet_helper((shouldDisableNativeBorders() ? sBorderNoneSheet : QString()), widget());
        return;
    }

    setStyleSheet_helper(
        QString("QPushButton{padding-left:%1px; padding-right:%2px; padding-top:%3px; padding-bottom:%4px}")
        .arg(RenderWidget::paddingLeft())
        .arg(RenderWidget::paddingRight())
        .arg(RenderWidget::paddingTop())
        .arg(RenderWidget::paddingBottom()) + (shouldDisableNativeBorders() ? sBorderNoneSheet : QString())
        , widget());
#else
    if (!includesPadding()) {
        return;
    }

    if (!RenderWidget::paddingLeft() && !RenderWidget::paddingRight() &&
            !RenderWidget::paddingTop() && !RenderWidget::paddingBottom()) {
        return;
    }

    QPushButton *pb = static_cast<QPushButton *>(m_widget);
    if (pb && pb->qmlWidget()) {
        pb->qmlWidget()->setProperty("leftPadding", RenderWidget::paddingLeft());
        pb->qmlWidget()->setProperty("rightPadding", RenderWidget::paddingRight());
        pb->qmlWidget()->setProperty("topPadding", RenderWidget::paddingTop());
        pb->qmlWidget()->setProperty("bottomPadding", RenderWidget::paddingBottom());
    }
#endif
}

void RenderSubmitButton::setStyle(RenderStyle *style)
{
    // Proxy styling doesn't work well enough for buttons.
    // Use stylesheets instead. tests/css/button-padding-top.html
#ifdef QT_WIDGETS_LIB
    assert(!m_proxyStyle);
    RenderFormElement::setStyle(style);

    QString s = widget()->styleSheet();
    if (shouldDisableNativeBorders()) {
        // we paint the borders ourselves on this button,
        // remove the widget's native ones.
        if (!s.contains(sBorderNoneSheet)) {
            s.append(sBorderNoneSheet);
            setStyleSheet_helper(s, widget());
        }
    } else {
        setStyleSheet_helper(s.remove(sBorderNoneSheet), widget());
    }
#else
    RenderFormElement::setStyle(style);
#endif
}

QString RenderSubmitButton::rawText()
{
    QString value = element()->valueWithDefault().string();
    value = value.trimmed();
    QString raw;
    for (int i = 0; i < value.length(); i++) {
        raw += value[i];
        if (value[i] == '&') {
            raw += '&';
        }
    }
    return raw;
}

bool RenderSubmitButton::canHaveBorder() const
{
    // ### TODO would be nice to be able to
    // return style()->hasBackgroundImage() here,
    // depending on a config option (e.g. 'favour usability/integration over aspect')
    // so that only buttons with both a custom border
    // and a background image are drawn without native styling.
    //
    // This would go in the same place, gui wise, as a choice of b/w default color scheme,
    // versus native color scheme.

    return true;
}

void RenderSubmitButton::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());

#ifdef QT_WIDGETS_LIB
    QString raw = rawText();
    QPushButton *pb = static_cast<QPushButton *>(m_widget);
    pb->setText(raw);
    pb->setFont(style()->font());

    bool empty = raw.isEmpty();
    if (empty) {
        raw = QLatin1Char('X');
    }
    QFontMetrics fm = pb->fontMetrics();
    QSize ts = fm.size(Qt::TextShowMnemonic, raw);
    //Oh boy.
    QStyleOptionButton butOpt;
    butOpt.init(pb);
    butOpt.text = raw;
    QSize s = pb->style()->sizeFromContents(QStyle::CT_PushButton, &butOpt, ts, pb);

    s = s.expandedTo(QApplication::globalStrut());
    int margin = pb->style()->pixelMetric(QStyle::PM_ButtonMargin) +
                 pb->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;

    int w = ts.width() + margin;
    int h = s.height();

    assert(includesPadding());
    int hpadding = RenderWidget::paddingLeft() + RenderWidget::paddingRight();
    int vpadding = RenderWidget::paddingTop() + RenderWidget::paddingBottom();

    // add 30% margins to the width (heuristics to make it look similar to IE)
    // ### FIXME BASELINE: we could drop this emulation and adopt Mozilla style buttons
    // (+/- padding: 0px 8px 0px 8px) - IE is most often in a separate css
    // code path nowadays, so we have wider buttons than other engines.
    int toAdd = (w * 13 / 10) - w - hpadding;
    toAdd = qMax(0, toAdd);
    w += toAdd;

    if (shouldDisableNativeBorders()) {
        // we paint the borders ourselves, so let's override our height to something saner
        h = ts.height();
    } else {
        h -= vpadding;
    }
    s = QSize(w, h).expandedTo(QApplication::globalStrut());

    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());
#else
    QString raw = rawText();
    QPushButton *pb = static_cast<QPushButton *>(m_widget);
    if (pb->qmlWidget()) {
        pb->setProperty("text", raw);
        pb->setProperty("cont", style()->font());
    }

    bool empty = raw.isEmpty();
    if (empty) {
        raw = QLatin1Char('X');
    }
    QFontMetrics fm(style()->font());
    QSize s = fm.size(Qt::TextShowMnemonic, raw);

    int margin = 12;

    int w = s.width() + margin;
    int h = s.height();

    assert(includesPadding());
    int hpadding = RenderWidget::paddingLeft() + RenderWidget::paddingRight();
    int vpadding = RenderWidget::paddingTop() + RenderWidget::paddingBottom();

    // add 30% margins to the width (heuristics to make it look similar to IE)
    // ### FIXME BASELINE: we could drop this emulation and adopt Mozilla style buttons
    // (+/- padding: 0px 8px 0px 8px) - IE is most often in a separate css
    // code path nowadays, so we have wider buttons than other engines.
    int toAdd = (w * 13 / 10) - w - hpadding;
    toAdd = qMax(0, toAdd);
    w += toAdd;

    if (shouldDisableNativeBorders()) {
        // we paint the borders ourselves, so let's override our height to something saner
        h = s.height();
    } else {
        h -= vpadding;
    }

    s = QSize(w, h);

    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());
#endif

    RenderButton::calcMinMaxWidth();
}

void RenderSubmitButton::updateFromElement()
{
#ifdef QT_WIDGETS_LIB
    QString oldText = static_cast<QPushButton *>(m_widget)->text();
    QString newText = rawText();
    static_cast<QPushButton *>(m_widget)->setText(newText);
#else
    QString oldText ="";
    QString newText = rawText();

    QPushButton *pb = static_cast<QPushButton *>(m_widget);
    if (pb->qmlWidget()) {
        oldText = pb->property("text").toString();
        pb->setProperty("text", newText);
    }
#endif
    if (oldText != newText) {
        setNeedsLayoutAndMinMaxRecalc();
    }

    RenderFormElement::updateFromElement();
}

short RenderSubmitButton::baselinePosition(bool f) const
{
    int ret = (height() - RenderWidget::paddingTop() - RenderWidget::paddingBottom() + 1) / 2;
    ret += marginTop() + RenderWidget::paddingTop();
    ret += ((fontMetrics(f).ascent()) / 2) - 2;
    return ret;
}

// -------------------------------------------------------------------------------

RenderResetButton::RenderResetButton(HTMLInputElementImpl *element)
    : RenderSubmitButton(element)
{
}

// -------------------------------------------------------------------------------

namespace khtml
{
//AFA
//class CompletionWidget: public KCompletionBox
//{
//public:
//    CompletionWidget(QWidget *parent = nullptr) : KCompletionBox(parent) {}
//    QPoint globalPositionHint() const override
//    {
//        QWidget *pw = parentWidget();
//        KHTMLWidget *kwp = dynamic_cast<KHTMLWidget *>(pw);
//        if (!kwp) {
//            qCDebug(KHTML_LOG) << "CompletionWidget has no KHTMLWidget parent";
//            return KCompletionBox::globalPositionHint();
//        }
//        QPoint dest;
//        KHTMLView *v = kwp->m_kwp->rootViewPos(dest);
//        QPoint ret;
//        if (v) {
//            ret = v->mapToGlobal(dest + QPoint(0, pw->height()));
//            int zoomLevel = v->zoomLevel();
//            if (zoomLevel != 100) {
//                ret.setX(ret.x()*zoomLevel / 100);
//                ret.setY(ret.y()*zoomLevel / 100);
//            }
//        }
//        return ret;
//    }
//};

}

LineEditWidget::LineEditWidget(DOM::HTMLInputElementImpl *input, KHTMLView *view, QWidget *parent)
    : QLineEdit(parent), m_input(input), m_view(view)
{
    m_kwp->setIsRedirected(true);
#ifdef QT_WIDGETS_LIB
    setMouseTracking(true);
#endif
    //AFA KActionCollection *ac = new KActionCollection(this);
    //AFA m_spellAction = KStandardAction::spelling(this, SLOT(slotCheckSpelling()), ac);

    //AFA setCompletionBox(new CompletionWidget(this));
    //AFA completionBox()->setObjectName("completion box");
    //AFA completionBox()->setFont(font());
}

LineEditWidget::~LineEditWidget()
{
}

void LineEditWidget::slotCheckSpelling()
{
    //AFA
//    if (text().isEmpty()) {
//        return;
//    }
//    Sonnet::Dialog *spellDialog = new Sonnet::Dialog(new Sonnet::BackgroundChecker(this), nullptr);
//    connect(spellDialog, SIGNAL(replace(QString,int,QString)), this, SLOT(spellCheckerCorrected(QString,int,QString)));
//    connect(spellDialog, SIGNAL(misspelling(QString,int)), this, SLOT(spellCheckerMisspelling(QString,int)));
//    connect(spellDialog, SIGNAL(done(QString)), this, SLOT(slotSpellCheckDone(QString)));
//    connect(spellDialog, SIGNAL(cancel()), this, SLOT(spellCheckerFinished()));
//    connect(spellDialog, SIGNAL(stop()), this, SLOT(spellCheckerFinished()));
//    spellDialog->setBuffer(text());
//    spellDialog->show();
}

void LineEditWidget::spellCheckerMisspelling(const QString &_text, int pos)
{
    highLightWord(_text.length(), pos);
}

void LineEditWidget::setFocus()
{
#ifdef QT_WIDGETS_LIB
    QLineEdit::setFocus();
    end(false);
#endif
}

void LineEditWidget::highLightWord(unsigned int length, unsigned int pos)
{
#ifdef QT_WIDGETS_LIB
    setSelection(pos, length);
#endif
}

void LineEditWidget::spellCheckerCorrected(const QString &old, int pos, const QString &corr)
{
#ifdef QT_WIDGETS_LIB
    if (old != corr) {
        setSelection(pos, old.length());
        insert(corr);
        setSelection(pos, corr.length());
    }
#endif
}

void LineEditWidget::spellCheckerFinished()
{
}

void LineEditWidget::slotSpellCheckDone(const QString &s)
{
#ifdef QT_WIDGETS_LIB
    if (s != text()) {
        setText(s);
    }
#endif
}

namespace khtml
{

/**
  * @internal
  */
class WebShortcutCreator
{
public:
    /**
      * @short Creates a Web Shourtcut without using kdebase SearchProvider class.
      *        It is used by LineEditWidget.
      */
    static bool createWebShortcut(QString query);

private:
    static bool askData(QString &name, QString &keys);
    static void createFile(QString query, QString name, QString keys);
};

bool WebShortcutCreator::createWebShortcut(QString query)
{
    //AFA
    return false;
//    QString name = i18n("New Web Shortcut");
//    QString keys;
//    if (askData(name, keys)) {
//        bool isOk;
//        do { //It's going to be checked if the keys have already been assigned
//            isOk = true;
//            QStringList keyList(keys.split(','));
//            KService::List providers = KServiceTypeTrader::self()->query("SearchProvider");
//            foreach (const KService::Ptr &provider, providers) {
//                if (!isOk) {
//                    break;
//                }
//                foreach (const QString &s, provider->property("Keys").toStringList()) {
//                    if (!isOk) {
//                        break;
//                    }
//                    foreach (const QString &t, keys) {
//                        if (!isOk) {
//                            break;
//                        }
//                        if (s == t) {
//                            KMessageBox::sorry(nullptr, i18n("%1 is already assigned to %2", s, provider->name()), i18n("Error"));
//                            isOk = false;
//                        }
//                    }
//                }
//            }
//            if (!isOk && !askData(name, keys)) {
//                return false;
//            }
//        } while (!isOk);
//        createFile(query, name, keys);
//        return true;
//    } else {
//        return false;
//    }
}

void WebShortcutCreator::createFile(QString query, QString name, QString keys)
{
    // SearchProvider class is part of kdebase, so the file is written as
    // an standard desktop file.
    //AFA
//    QString fileName(keys);
//    QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kservices5/searchproviders";
//    QDir().mkpath(dir);
//    while (QFile::exists(dir + fileName + ".desktop")) {
//        fileName += '_';
//    }
//    KDesktopFile f(dir + fileName + ".desktop");
//    f.desktopGroup().writeEntry("Keys", keys);
//    f.desktopGroup().writeEntry("Type", "Service");
//    f.desktopGroup().writeEntry("ServiceTypes", "SearchProvider");
//    f.desktopGroup().writeEntry("Name", name);
//    f.desktopGroup().writeEntry("Query", query);
//    f.sync();
//    KBuildSycocaProgressDialog::rebuildKSycoca(nullptr);
}

bool WebShortcutCreator::askData(QString &name, QString &keys)
{
    //AFA
    return false;
//    QDialog *dialog = new QDialog();
//    dialog->setWindowTitle(name);
//    QVBoxLayout *mainLayout = new QVBoxLayout();
//    dialog->setLayout(mainLayout);

//    QHBoxLayout *layout = new QHBoxLayout();
//    mainLayout->addLayout(layout);
//    QLabel *label = new QLabel(i18n("Search &provider name:"), dialog);
//    layout->addWidget(label);
//    QLineEdit *nameEdit = new QLineEdit(i18n("New search provider"), dialog);
//    label->setBuddy(nameEdit);
//    layout->addWidget(nameEdit);
//    layout = new QHBoxLayout();
//    mainLayout->addLayout(layout);
//    label = new QLabel(i18n("UR&I shortcuts:"), dialog);
//    layout->addWidget(label);
//    QLineEdit *keysEdit = new QLineEdit(dialog);
//    label->setBuddy(keysEdit);
//    layout->addWidget(keysEdit);

//    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
//    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//    QObject::connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
//    QObject::connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
//    mainLayout->addWidget(buttonBox);

//    bool res = dialog->exec();
//    if (res) {
//        name = nameEdit->text();
//        keys = keysEdit->text();
//    }
//    delete dialog;
//    return res;
}

}

void LineEditWidget::slotCreateWebShortcut()
{
    QString queryName(m_input->name().string());
    HTMLFormElementImpl *form = m_input->form();
    QUrl url(form->action().string());
    QUrl baseUrl(m_view->part()->baseURL().url() + '?');
    if (url.path().isEmpty()) {
        url.setPath(baseUrl.path());
    }
    if (url.host().isEmpty()) {
        url.setScheme(baseUrl.scheme());
        url.setHost(baseUrl.host());
    }
    NodeImpl *node;
    HTMLInputElementImpl *inputNode;
    for (unsigned long i = 0; (node = form->elements()->item(i)); i++) {
        inputNode = dynamic_cast<HTMLInputElementImpl *>(node);
        if (inputNode) {
            if ((!inputNode->name().string().size()) ||
                    (inputNode->name().string() == queryName)) {
                continue;
            } else {
                switch (inputNode->inputType()) {
                case HTMLInputElementImpl::CHECKBOX:
                case HTMLInputElementImpl::RADIO:
                    if (!inputNode->checked()) {
                        break;
                    }
                case HTMLInputElementImpl::TEXT:
                case HTMLInputElementImpl::PASSWORD:
                case HTMLInputElementImpl::HIDDEN:
                    //AFA url.addQueryItem(inputNode->name().string(), inputNode->value().string());
                default:
                    break;
                }
            }
        }
    }
    QString query(url.url());
    if (!query.contains("?")) {
        query += '?'; //This input is the only one of the form
    }
    query += '&' + queryName + "=\\{@}";
    WebShortcutCreator::createWebShortcut(query);
}

void LineEditWidget::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QT_WIDGETS_LIB
    QMenu *popup = createStandardContextMenu();

    if (!popup) {
        return;
    }

    if (m_input->autoComplete()) {
        popup->addSeparator();
        QAction *act = popup->addAction(QIcon::fromTheme("edit-clear-history"), i18n("Clear &History"));
        //AFA act->setEnabled(compObj() && !compObj()->isEmpty());
        connect(act, SIGNAL(triggered()),
                this, SLOT(clearHistoryActivated()));
    }

    if (echoMode() == QLineEdit::Normal &&
            !isReadOnly()) {
        popup->addSeparator();

        popup->addAction(m_spellAction);
        m_spellAction->setEnabled(!text().isEmpty());
    }
    if (!m_view->part()->onlyLocalReferences()) {
        popup->addSeparator();
        QAction *act = popup->addAction(i18n("Create Web Shortcut"));
        connect(act, SIGNAL(triggered()),
                this, SLOT(slotCreateWebShortcut()));
    }

    //AFA emit aboutToShowContextMenu(popup);

    popup->exec(e->globalPos());
    delete popup;
#endif
}

void LineEditWidget::clearHistoryActivated()
{
    m_view->clearCompletionHistory(m_input->name().string());
    //AFA if (compObj()) {
    //AFA     compObj()->clear();
    //AFA }
}

bool LineEditWidget::event(QEvent *e)
{
    if (QLineEdit::event(e)) {
        return true;
    }
#if 0
    if (e->type() == QEvent::AccelAvailable && isReadOnly()) {
        QKeyEvent *ke = (QKeyEvent *) e;
        if (ke->modifiers() & Qt::ControlModifier) {
            switch (ke->key()) {
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Home:
            case Qt::Key_End:
                ke->accept();
            default:
                break;
            }
        }
    }
#endif
    return false;
}

void LineEditWidget::mouseMoveEvent(QMouseEvent *e)
{
    // hack to prevent Qt from calling setCursor on the widget
#ifdef QT_WIDGETS_LIB
    setDragEnabled(false);
    QLineEdit::mouseMoveEvent(e);
    setDragEnabled(true);
#endif
}

// -----------------------------------------------------------------------------

RenderLineEdit::RenderLineEdit(HTMLInputElementImpl *element)
    : RenderFormElement(element), m_blockElementUpdates(false)
{
#ifdef QT_WIDGETS_LIB
    LineEditWidget *edit = new LineEditWidget(element, view(), view()->widget());
    connect(edit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    connect(edit, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));

    if (element->inputType() == HTMLInputElementImpl::PASSWORD) {
        edit->setEchoMode(QLineEdit::Password);
    }

    if (element->autoComplete()) {
        QStringList completions = view()->formCompletionItems(element->name().string());
        if (completions.count()) {
            //AFA edit->completionObject()->setItems(completions);
            edit->setContextMenuPolicy(Qt::NoContextMenu);
            //AFA edit->completionBox()->setTabHandling(false);
        }
    }

    setQWidget(edit);
#else
    LineEditWidget *edit = new LineEditWidget(element, view(), view());
    //AFA connect(edit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
    //AFA connect(edit, SIGNAL(textChanged()), this, SLOT(slotTextChanged(QString)));

    if (element->inputType() == HTMLInputElementImpl::PASSWORD) {
        edit->setEchoMode(QQuickTextInput::Password);
    }

    if (element->autoComplete()) {
        QStringList completions = view()->formCompletionItems(element->name().string());
        if (completions.count()) {
            //AFA edit->completionObject()->setItems(completions);
            //AFA edit->setContextMenuPolicy(Qt::NoContextMenu);
            //AFA edit->completionBox()->setTabHandling(false);
        }
    }

    setQWidget(edit);
#endif
}

short RenderLineEdit::baselinePosition(bool f) const
{
#ifdef QT_WIDGETS_LIB
    bool hasFrame = static_cast<LineEditWidget *>(widget())->hasFrame();
#else
    bool hasFrame = false;
#endif
    int bTop = hasFrame ? 0 : borderTop();
    int bBottom = hasFrame ? 0 : borderBottom();
    int ret = (height() - RenderWidget::paddingTop() - RenderWidget::paddingBottom() - bTop - bBottom + 1) / 2;
    ret += marginTop() + RenderWidget::paddingTop() + bTop;
    ret += ((fontMetrics(f).ascent()) / 2) - 2;
    return ret;
}

void RenderLineEdit::setStyle(RenderStyle *_style)
{
    RenderFormElement::setStyle(_style);

#ifdef QT_WIDGETS_LIB
    if (widget()->alignment() != textAlignment()) {
        widget()->setAlignment(textAlignment());
    }
#endif

    bool showClearButton = (!shouldDisableNativeBorders() && !_style->hasBackgroundImage());
    //AFA
//    if (!showClearButton && widget()->isClearButtonShown()) {
//        widget()->setClearButtonShown(false);
//    } else if (showClearButton && !widget()->isClearButtonShown()) {
//        widget()->setClearButtonShown(true);
//        QObjectList children = widget()->children();
//        foreach (QObject *object, children) {
//            QWidget *w = qobject_cast<QWidget *>(object);
//            if (w && !w->isWindow() && (w->objectName() == "QLineEditButton")) {
//                // this duplicates KHTMLView's handleWidget but this widget
//                // is created on demand, so it might not be here at ChildPolished time
//                w->installEventFilter(view());
//            }
//        }
//    }

//    if (m_proxyStyle) {
//        static_cast<KHTMLProxyStyle *>(m_proxyStyle)->clearButtonOverlay = qMax(0, widget()->clearButtonUsedSize().width());
//    }
}

void RenderLineEdit::highLightWord(unsigned int length, unsigned int pos)
{
    LineEditWidget *w = static_cast<LineEditWidget *>(m_widget);
    if (w) {
        w->highLightWord(length, pos);
    }
}

void RenderLineEdit::slotReturnPressed()
{
    // don't submit the form when return was pressed in a completion-popup
    //AFA
//    KCompletionBox *box = widget()->completionBox(false);

//    if (box && box->isVisible() && box->currentRow() != -1) {
//        box->hide();
//        return;
//    }

    // Emit onChange if necessary
    // Works but might not be enough, dirk said he had another solution at
    // hand (can't remember which) - David
    handleFocusOut();

    HTMLFormElementImpl *fe = element()->form();
    if (fe) {
        fe->submitFromKeyboard();
    }
}

void RenderLineEdit::handleFocusOut()
{
#ifdef QT_WIDGETS_LIB
    if (widget() && widget()->isModified()) {
        element()->onChange();
        widget()->setModified(false);
    }
#endif
}

void RenderLineEdit::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());

    const QFontMetrics &fm = style()->fontMetrics();
    QSize s;

    int size = (element()->size() > 0) ? (element()->size() + 1) : 17; // "some"

    int h = fm.lineSpacing();
    int w = (fm.height() * size) / 2; // on average a character cell is twice as tall as it is wide

#ifdef QT_WIDGETS_LIB
    QStyleOptionFrame opt;
    opt.initFrom(widget());
    if (widget()->hasFrame()) {
        opt.lineWidth = widget()->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, widget());
    }

    s = QSize(w, qMax(h, 14));
    s = widget()->style()->sizeFromContents(QStyle::CT_LineEdit, &opt, s, widget());
    s = s.expandedTo(QApplication::globalStrut());

    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());
#else
    s = QSize(w, qMax(h, 14));

    setIntrinsicWidth(s.width());
    setIntrinsicHeight(s.height());
#endif

    RenderFormElement::calcMinMaxWidth();
}

void RenderLineEdit::updateFromElement()
{
    int ml = element()->maxLength();
    if (ml < 0) {
        ml = 32767;
    }

    if (widget()) {
#ifdef QT_WIDGETS_LIB
        if (widget()->maxLength() != ml)  {
            widget()->setMaxLength(ml);
        }

        if (element()->value().string() != widget()->text()) {
            m_blockElementUpdates = true;  // Do not block signals here (#188374)
            int pos = widget()->cursorPosition();
            widget()->setText(element()->value().string());
            widget()->setCursorPosition(pos);
            m_blockElementUpdates = false;
        }
        widget()->setReadOnly(element()->readOnly());

        widget()->setPlaceholderText(element()->placeholder().string().remove(QLatin1Char('\n')).remove(QLatin1Char('\r')));
#endif
    }

    RenderFormElement::updateFromElement();
}

void RenderLineEdit::slotTextChanged(const QString &string)
{
    if (m_blockElementUpdates) {
        return;
    }

    // don't use setValue here!
    element()->m_value = string.isNull() ? DOMString("") : string;
    element()->m_unsubmittedFormChange = true;
}

void RenderLineEdit::select()
{
    if (m_widget) {
#ifdef QT_WIDGETS_LIB
        static_cast<LineEditWidget *>(m_widget)->selectAll();
#endif
    }
}

long RenderLineEdit::selectionStart()
{
    if (m_widget) {
#ifdef QT_WIDGETS_LIB
        LineEditWidget *w = static_cast<LineEditWidget *>(m_widget);
        if (!w->selectedText().isEmpty()) {
            return w->selectionStart();
        } else {
            return w->cursorPosition();
        }
#endif
    }
    else
        return 0;
}

long RenderLineEdit::selectionEnd()
{
    if (m_widget) {
#ifdef QT_WIDGETS_LIB
        LineEditWidget *w = static_cast<LineEditWidget *>(m_widget);
        if (!w->selectedText().isEmpty()) {
            return w->selectionStart() + w->selectedText().length();
        } else {
            return w->cursorPosition();
        }
#endif
    }
    else
        return 0;
}

void RenderLineEdit::setSelectionStart(long pos)
{
    LineEditWidget *w = static_cast<LineEditWidget *>(m_widget);
    //See whether we have a non-empty selection now.
    long end = selectionEnd();
#ifdef QT_WIDGETS_LIB
    if (end > pos) {
        w->setSelection(pos, end - pos);
    }
    w->setCursorPosition(pos);
#endif
}

void RenderLineEdit::setSelectionEnd(long pos)
{
    LineEditWidget *w = static_cast<LineEditWidget *>(m_widget);
    //See whether we have a non-empty selection now.
    long start = selectionStart();
#ifdef QT_WIDGETS_LIB
    if (start < pos) {
        w->setSelection(start, pos - start);
    }
    w->setCursorPosition(pos);
#endif
}

void RenderLineEdit::setSelectionRange(long start, long end)
{
    LineEditWidget *w = static_cast<LineEditWidget *>(m_widget);
#ifdef QT_WIDGETS_LIB
    w->setCursorPosition(end);
    w->setSelection(start, end - start);
#endif
}

// ---------------------------------------------------------------------------

RenderFieldset::RenderFieldset(HTMLGenericFormElementImpl *element)
    : RenderBlock(element)
{
    m_intrinsicWidth = 0;
}

void RenderFieldset::calcMinMaxWidth()
{
    RenderBlock::calcMinMaxWidth();
    if (style()->htmlHacks()) {
        if (RenderObject *legend = findLegend()) {
            int legendMinWidth = legend->minWidth();

            Length legendMarginLeft = legend->style()->marginLeft();
            Length legendMarginRight = legend->style()->marginLeft();

            if (legendMarginLeft.isFixed()) {
                legendMinWidth += legendMarginLeft.value();
            }

            if (legendMarginRight.isFixed()) {
                legendMinWidth += legendMarginRight.value();
            }

            m_intrinsicWidth = qMax((int)m_minWidth, legendMinWidth + paddingLeft() + paddingRight() + borderLeft() + borderRight());
        }
    }
}

RenderObject *RenderFieldset::layoutLegend(bool relayoutChildren)
{
    RenderObject *legend = findLegend();
    if (legend) {
        if (relayoutChildren) {
            legend->setNeedsLayout(true);
        }
        legend->layoutIfNeeded();

        int xPos = borderLeft() + paddingLeft() + legend->marginLeft();
        if (style()->direction() == RTL) {
            xPos = m_width - paddingRight() - borderRight() - legend->width() - legend->marginRight();
        }
        int b = borderTop();
        int h = legend->height();
        legend->setPos(xPos, qMax((b - h) / 2, 0));
        m_height = qMax(b, h) + paddingTop();
    }
    return legend;
}

RenderObject *RenderFieldset::findLegend() const
{
    for (RenderObject *legend = firstChild(); legend; legend = legend->nextSibling()) {
        if (!legend->isFloatingOrPositioned() && legend->element() &&
                legend->element()->id() == ID_LEGEND) {
            return legend;
        }
    }
    return nullptr;
}

void RenderFieldset::paintBoxDecorations(PaintInfo &pI, int _tx, int _ty)
{
    //qCDebug(KHTML_LOG) << renderName() << "::paintDecorations()";

    RenderObject *legend = findLegend();
    if (!legend) {
        return RenderBlock::paintBoxDecorations(pI, _tx, _ty);
    }

    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    int yOff = (legend->yPos() > 0) ? 0 : (legend->height() - borderTop()) / 2;
    int legendBottom = _ty + legend->yPos() + legend->height();
    h -= yOff;
    _ty += yOff - borderTopExtra();

    QRect cr = QRect(_tx, _ty, w, h).intersected(pI.r);
    paintOneBackground(pI.p, style()->backgroundColor(), style()->backgroundLayers(), cr, _tx, _ty, w, h);

    if (style()->hasBorder()) {
        paintBorderMinusLegend(pI.p, _tx, _ty, w, h, style(), legend->xPos(), legend->width(), legendBottom);
    }
}

void RenderFieldset::paintBorderMinusLegend(QPainter *p, int _tx, int _ty, int w, int h,
        const RenderStyle *style, int lx, int lw, int lb)
{

    const QColor &tc = style->borderTopColor();
    const QColor &bc = style->borderBottomColor();

    EBorderStyle ts = style->borderTopStyle();
    EBorderStyle bs = style->borderBottomStyle();
    EBorderStyle ls = style->borderLeftStyle();
    EBorderStyle rs = style->borderRightStyle();

    bool render_t = ts > BHIDDEN;
    bool render_l = ls > BHIDDEN;
    bool render_r = rs > BHIDDEN;
    bool render_b = bs > BHIDDEN;

    int borderLeftWidth = style->borderLeftWidth();
    int borderRightWidth = style->borderRightWidth();

    if (render_t) {
        if (lx >= borderLeftWidth)
            drawBorder(p, _tx, _ty, _tx + lx, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                       (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE) ? style->borderLeftWidth() : 0), 0);
        if (lx + lw <=  w - borderRightWidth)
            drawBorder(p, _tx + lx + lw, _ty, _tx + w, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                       0, (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE) ? style->borderRightWidth() : 0));
    }

    if (render_b)
        drawBorder(p, _tx, _ty + h - style->borderBottomWidth(), _tx + w, _ty + h, BSBottom, bc, style->color(), bs,
                   (render_l && (ls == DOTTED || ls == DASHED || ls == DOUBLE) ? style->borderLeftWidth() : 0),
                   (render_r && (rs == DOTTED || rs == DASHED || rs == DOUBLE) ? style->borderRightWidth() : 0));

    if (render_l) {
        const QColor &lc = style->borderLeftColor();

        bool ignore_top =
            (tc == lc) &&
            (ls >= OUTSET) &&
            (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

        bool ignore_bottom =
            (bc == lc) &&
            (ls >= OUTSET) &&
            (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        int startY = _ty;
        if (lx < borderLeftWidth && lx + lw > 0) {
            // The legend intersects the border.
            ignore_top = true;
            startY = lb;
        }

        drawBorder(p, _tx, startY, _tx + borderLeftWidth, _ty + h, BSLeft, lc, style->color(), ls,
                   ignore_top ? 0 : style->borderTopWidth(),
                   ignore_bottom ? 0 : style->borderBottomWidth());
    }

    if (render_r) {
        const QColor &rc = style->borderRightColor();

        bool ignore_top =
            (tc == rc) &&
            (rs >= DOTTED || rs == INSET) &&
            (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

        bool ignore_bottom =
            (bc == rc) &&
            (rs >= DOTTED || rs == INSET) &&
            (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        int startY = _ty;
        if (lx < w && lx + lw > w - borderRightWidth) {
            // The legend intersects the border.
            ignore_top = true;
            startY = lb;
        }

        drawBorder(p, _tx + w - borderRightWidth, startY, _tx + w, _ty + h, BSRight, rc, style->color(), rs,
                   ignore_top ? 0 : style->borderTopWidth(),
                   ignore_bottom ? 0 : style->borderBottomWidth());
    }
}

void RenderFieldset::setStyle(RenderStyle *_style)
{
    RenderBlock::setStyle(_style);

    // WinIE renders fieldsets with display:inline like they're inline-blocks.  For us,
    // an inline-block is just a block element with replaced set to true and inline set
    // to true.  Ensure that if we ended up being inline that we set our replaced flag
    // so that we're treated like an inline-block.
    if (isInline()) {
        setReplaced(true);
    }
}

// -------------------------------------------------------------------------

RenderFileButton::RenderFileButton(HTMLInputElementImpl *element)
    : RenderFormElement(element)
{
#ifdef QT_WIDGETS_LIB
    FileButtonWidget *w = new FileButtonWidget(view()->widget());

    //AFA
//    w->setMode(KFile::File | KFile::ExistingOnly);
//    w->lineEdit()->setCompletionBox(new CompletionWidget(w));
//    w->completionObject()->setDir(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));

//    connect(w->lineEdit(), SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
//    connect(w->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
//    connect(w, SIGNAL(urlSelected(QUrl)), this, SLOT(slotUrlSelected()));

    setQWidget(w);
#else
    FileButtonWidget *w = new FileButtonWidget(view());

    //AFA
//    w->setMode(KFile::File | KFile::ExistingOnly);
//    w->lineEdit()->setCompletionBox(new CompletionWidget(w));
//    w->completionObject()->setDir(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));

//    connect(w->lineEdit(), SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
//    connect(w->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
//    connect(w, SIGNAL(urlSelected(QUrl)), this, SLOT(slotUrlSelected()));

    setQWidget(w);
#endif
    m_haveFocus = false;
}

short RenderFileButton::baselinePosition(bool f) const
{
    int bTop = borderTop();
    int bBottom = borderBottom();
    int ret = (height() - paddingTop() - paddingBottom() - bTop - bBottom + 1) / 2;
    ret += marginTop() + paddingTop() + bTop;
    ret += ((fontMetrics(f).ascent()) / 2) - 2;
    return ret;
}

void RenderFileButton::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());

    const QFontMetrics &fm = style()->fontMetrics();
    int size = (element()->size() > 0) ? (element()->size() + 1) : 17; // "some"

    int h = fm.lineSpacing();
    int w = (fm.height() * size) / 2; // on average a character cell is twice as tall as it is wide
    //AFA QLineEdit *edit = widget()->lineEdit();
    QWidget *edit = widget();

#ifdef QT_WIDGETS_LIB
    QStyleOptionFrame opt;
    opt.initFrom(edit);
    if (false/*AFA edit->hasFrame()*/) {
        opt.lineWidth = edit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, edit);
    }

    QSize s(w, qMax(h, 14));
    s = edit->style()->sizeFromContents(QStyle::CT_LineEdit, &opt, s, edit);
    s = s.expandedTo(QApplication::globalStrut());

    QSize bs = widget()->minimumSizeHint() - edit->minimumSizeHint();

    setIntrinsicWidth(s.width() + bs.width());
    setIntrinsicHeight(qMax(s.height(), bs.height()));
#else
    QSize s(w, qMax(h, 14));

    QSize bs = s;

    setIntrinsicWidth(s.width() + bs.width());
    setIntrinsicHeight(qMax(s.height(), bs.height()));
#endif

    RenderFormElement::calcMinMaxWidth();
}

void RenderFileButton::handleFocusOut()
{
    //AFA
//    if (widget()->lineEdit() && widget()->lineEdit()->isModified()) {
//        element()->onChange();
//        widget()->lineEdit()->setModified(false);
//    }
}

void RenderFileButton::updateFromElement()
{
    //AFA
//    QLineEdit *edit = widget()->lineEdit();
//    bool blocked = edit->blockSignals(true);
//    edit->setText(element()->value().string());
//    edit->blockSignals(blocked);
//    edit->setModified(false);

    RenderFormElement::updateFromElement();
}

void RenderFileButton::slotReturnPressed()
{
    // don't submit the form when return was pressed in a completion-popup
    //AFA
//    KCompletionBox *box = widget()->lineEdit()->completionBox(false);
//    if (box && box->isVisible() && box->currentRow() != -1) {
//        box->hide();
//        return;
//    }

    handleFocusOut();

    if (element()->form()) {
        element()->form()->submitFromKeyboard();
    }
}

void RenderFileButton::slotTextChanged(const QString &/*string*/)
{
    //AFA element()->m_value = QUrl(widget()->url()).toDisplayString(QUrl::PreferLocalFile);
}

void RenderFileButton::slotUrlSelected()
{
    element()->onChange();
}

void RenderFileButton::select()
{
    //AFA widget()->lineEdit()->selectAll();
}

// -------------------------------------------------------------------------

RenderLabel::RenderLabel(HTMLGenericFormElementImpl *element)
    : RenderFormElement(element)
{

}

// -------------------------------------------------------------------------

RenderLegend::RenderLegend(HTMLGenericFormElementImpl *element)
    : RenderBlock(element)
{
}

// -------------------------------------------------------------------------------

bool ListBoxWidget::event(QEvent *event)
{
    // accept all wheel events so that they are not propagated to the view
    // once either end of the list is reached.
    bool ret = QListWidget::event(event);
    if (event->type() == QEvent::Wheel) {
        event->accept();
        ret = true;
    }
    return ret;
}

ComboBoxWidget::ComboBoxWidget(QWidget *parent)
    : QComboBox(/*false, */parent)
{
    m_kwp->setIsRedirected(true);
    //setAutoMask(true);
#ifdef QT_WIDGETS_LIB
    if (view()) {
        view()->installEventFilter(this);
    }
    setMouseTracking(true);
#endif
}

void ComboBoxWidget::showPopup()
{
#ifdef QT_WIDGETS_LIB
    QPoint p = pos();
    QPoint dest(p);
    QWidget *parent = parentWidget();
    KHTMLView *v = m_kwp->rootViewPos(dest);
    int zoomLevel = v ? v->zoomLevel() : 100;
    if (zoomLevel != 100) {
        if (v) {
            // we need to place the popup even lower on the screen, take in count the widget is bigger
            // now, so we add also the difference between the original height, and the zoomed height
            dest.setY(dest.y() + (sizeHint().height() * zoomLevel / 100 - sizeHint().height()));
        }
    }
    bool blocked = blockSignals(true);
    if (v != parent) {
        setParent(v);
    }
    move(dest);
    blockSignals(blocked);

    QComboBox::showPopup();

    blocked = blockSignals(true);
    if (v != parent) {
        setParent(parent);
        // undo side effect of setParent()
        show();
    }
    move(p);
    blockSignals(blocked);
#endif
}

void ComboBoxWidget::hidePopup()
{
#ifdef QT_WIDGETS_LIB
    QComboBox::hidePopup();
#endif
}

bool ComboBoxWidget::event(QEvent *e)
{
    if (QComboBox::event(e)) {
        return true;
    }
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        switch (ke->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            showPopup();
            ke->accept();
            return true;
        default:
            return false;
        }
    }
    return false;
}

bool ComboBoxWidget::eventFilter(QObject *dest, QEvent *e)
{
#ifdef QT_WIDGETS_LIB
    if (dest == view() &&  e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        bool forward = false;
        switch (ke->key()) {
        case Qt::Key_Tab:
            forward = true;
        // fall through
        case Qt::Key_Backtab:
            // ugly hack. emulate popdownlistbox() (private in QComboBox)
            // we re-use ke here to store the reference to the generated event.
            ke = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            QCoreApplication::sendEvent(dest, ke);
            focusNextPrevChild(forward);
            delete ke;
            return true;
        default:
            return QComboBox::eventFilter(dest, e);
        }
    }
#endif
    return QComboBox::eventFilter(dest, e);
}

void ComboBoxWidget::keyPressEvent(QKeyEvent *e)
{
    // Normally, widgets are not sent Tab keys this way in the first
    // place as they are handled by QWidget::event() for focus handling
    // already. But we get our events via EventPropagator::sendEvent()
    // directly. Ignore them so that HTMLGenericFormElementImpl::
    // defaultEventHandler() can call focusNextPrev().
    if (e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) {
        e->ignore();
        return;
    }
#ifdef QT_WIDGETS_LIB
    QComboBox::keyPressEvent(e);
#endif
}

// -------------------------------------------------------------------------

RenderSelect::RenderSelect(HTMLSelectElementImpl *element)
    : RenderFormElement(element)
{
    m_ignoreSelectEvents = false;
    m_multiple = element->multiple();
    m_size = element->size();
    m_useListBox = (m_multiple || m_size > 1);
    m_selectionChanged = true;
    m_optionsChanged = true;

    if (m_useListBox) {
        setQWidget(createListBox());
    } else {
        setQWidget(createComboBox());
#ifdef QT_WIDGETS_LIB
        getProxyStyle(); // We always need it to make sure popups are big enough
#endif
    }
}

void RenderSelect::clearItemFlags(int index, Qt::ItemFlags flags)
{
    if (m_useListBox) {
#ifdef QT_WIDGETS_LIB
        QListWidgetItem *item = static_cast<QListWidget *>(m_widget)->item(index);
        item->setFlags(item->flags() & ~flags);
#endif
    } else {
        QComboBox *combo = static_cast<QComboBox *>(m_widget);
#ifdef QT_WIDGETS_LIB
        if (QStandardItemModel *model = qobject_cast<QStandardItemModel *>(combo->model())) {
            QStandardItem *item = model->item(index);
            item->setFlags(item->flags() & ~flags);
        }
#endif
    }
}

void RenderSelect::setStyle(RenderStyle *_style)
{
    RenderFormElement::setStyle(_style);
    if (!m_useListBox) {
#ifdef QT_WIDGETS_LIB
        KHTMLProxyStyle *proxyStyle = static_cast<KHTMLProxyStyle *>(getProxyStyle());
        proxyStyle->noBorder = shouldDisableNativeBorders();
#endif
    }
}

void RenderSelect::updateFromElement()
{
    m_ignoreSelectEvents = true;

    // change widget type
    bool oldMultiple = m_multiple;
    unsigned oldSize = m_size;
    bool oldListbox = m_useListBox;

    m_multiple = element()->multiple();
    m_size = element()->size();
    m_useListBox = (m_multiple || m_size > 1);

    if (oldMultiple != m_multiple || oldSize != m_size) {
        if (m_useListBox != oldListbox) {
            // type of select has changed
            if (m_useListBox) {
                setQWidget(createListBox());
            } else {
                setQWidget(createComboBox());
            }

            // Call setStyle() to fix unwanted font size change (#142722)
            // and to update our proxy style properties
            setStyle(style());
        }

        if (m_useListBox && oldMultiple != m_multiple) {
#ifdef QT_WIDGETS_LIB
            static_cast<QListWidget *>(m_widget)->setSelectionMode(m_multiple ?
                    QListWidget::ExtendedSelection
                    : QListWidget::SingleSelection);
#endif
        }
        m_selectionChanged = true;
        m_optionsChanged = true;
    }

    // update contents listbox/combobox based on options in m_element
    if (m_optionsChanged) {
        if (element()->m_recalcListItems) {
            element()->recalcListItems();
        }
        const QVector<HTMLGenericFormElementImpl *> listItems = element()->listItems();
        int listIndex;
#ifdef QT_WIDGETS_LIB
        if (m_useListBox) {
            static_cast<QListWidget *>(m_widget)->clear();
        } else {
            static_cast<QComboBox *>(m_widget)->clear();
        }
#endif
        for (listIndex = 0; listIndex < int(listItems.size()); listIndex++) {
            if (listItems[listIndex]->id() == ID_OPTGROUP) {
                DOMString text = listItems[listIndex]->getAttribute(ATTR_LABEL);
                if (text.isNull()) {
                    text = "";
                }

                text = text.implementation()->collapseWhiteSpace(false, false);
#ifdef QT_WIDGETS_LIB
                if (m_useListBox) {
                    QListWidgetItem *item = new QListWidgetItem(QString(text.implementation()->s, text.implementation()->l));
                    static_cast<QListWidget *>(m_widget)->insertItem(listIndex, item);
                } else {
                    static_cast<QComboBox *>(m_widget)->insertItem(listIndex, QString(text.implementation()->s, text.implementation()->l));
                }
#endif
                bool disabled = !listItems[listIndex]->getAttribute(ATTR_DISABLED).isNull();
                if (disabled) {
                    clearItemFlags(listIndex, Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                } else {
                    clearItemFlags(listIndex, Qt::ItemIsSelectable);
                }
            } else if (listItems[listIndex]->id() == ID_OPTION) {
                HTMLOptionElementImpl *optElem = static_cast<HTMLOptionElementImpl *>(listItems[listIndex]);

                DOMString domText = optElem->text();
                // Prefer label if set
                DOMString label = optElem->getAttribute(ATTR_LABEL);
                if (!label.isEmpty()) {
                    domText = label;
                }
                domText = domText.implementation()->collapseWhiteSpace(false, false);

                QString text;

                ElementImpl *parentOptGroup = optElem->parentNode()->id() == ID_OPTGROUP ?
                                              static_cast<ElementImpl *>(optElem->parentNode()) : nullptr;

                if (parentOptGroup) {
                    text = QLatin1String("    ") + domText.string();
                } else {
                    text = domText.string();
                }
#ifdef QT_WIDGETS_LIB
                if (m_useListBox) {
                    static_cast<QListWidget *>(m_widget)->insertItem(listIndex, text);
                } else {
                    static_cast<QComboBox *>(m_widget)->insertItem(listIndex, text);
                }
#endif
                bool disabled = !optElem->getAttribute(ATTR_DISABLED).isNull();
                if (parentOptGroup) {
                    disabled = disabled || !parentOptGroup->getAttribute(ATTR_DISABLED).isNull();
                }

                if (disabled) {
                    clearItemFlags(listIndex, Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                }
            } else {
                KHTMLAssert(false);
            }

            m_selectionChanged = true;
        }

        // QComboBox caches the size hint unless you call setFont (ref: TT docu)
        if (!m_useListBox) {
#ifdef QT_WIDGETS_LIB
            QComboBox *that = static_cast<QComboBox *>(m_widget);
            that->setFont(that->font());
#endif
        }
        setNeedsLayoutAndMinMaxRecalc();
        m_optionsChanged = false;
    }

    // update selection
    if (m_selectionChanged) {
        updateSelection();
    }

    m_ignoreSelectEvents = false;

    RenderFormElement::updateFromElement();
}

short RenderSelect::baselinePosition(bool f) const
{
    if (m_useListBox) {
        return RenderFormElement::baselinePosition(f);
    }

    int bTop = shouldDisableNativeBorders() ? borderTop() : 0;
    int bBottom = shouldDisableNativeBorders() ? borderBottom() : 0;
    int ret = (height() - RenderWidget::paddingTop() - RenderWidget::paddingBottom() - bTop - bBottom + 1) / 2;
    ret += marginTop() + RenderWidget::paddingTop() + bTop;
    ret += ((fontMetrics(f).ascent()) / 2) - 2;
    return ret;
}

void RenderSelect::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());

    if (m_optionsChanged) {
        updateFromElement();
    }

    // ### ugly HACK FIXME!!!
    setMinMaxKnown();
    layoutIfNeeded();
    setNeedsLayoutAndMinMaxRecalc();
    // ### end FIXME

    RenderFormElement::calcMinMaxWidth();
}

void RenderSelect::layout()
{
    KHTMLAssert(needsLayout());
    KHTMLAssert(minMaxKnown());

    // ### maintain selection properly between type/size changes, and work
    // out how to handle multiselect->singleselect (probably just select
    // first selected one)
    // calculate size
#ifdef QT_WIDGETS_LIB
    if (m_useListBox) {
        QListWidget *w = static_cast<QListWidget *>(m_widget);

        int width = 0;
        int height = 0;

        QAbstractItemModel *m = w->model();
        QAbstractItemDelegate *d = w->itemDelegate();
        QStyleOptionViewItem so;
        so.font = w->font();

        for (int rowIndex = 0; rowIndex < w->count(); rowIndex++) {
            QModelIndex mi = m->index(rowIndex, 0);
            QSize s = d->sizeHint(so, mi);
            width = qMax(width, s.width());
            height = qMax(height, s.height());
        }

        if (!height) {
            height = w->fontMetrics().height();
        }
        if (!width) {
            width = w->fontMetrics().width('x');
        }

        int size = m_size;
        // check if multiple and size was not given or invalid
        // Internet Exploder sets size to qMin(number of elements, 4)
        // Netscape seems to simply set it to "number of elements"
        // the average of that is IMHO qMin(number of elements, 10)
        // so I did that ;-)
        if (size < 1) {
            size = qMin(w->count(), 10);
        }

        QStyleOptionFrameV3 opt;
        opt.initFrom(w);
        opt.lineWidth = w->lineWidth();
        opt.midLineWidth = w->midLineWidth();
        opt.frameShape = w->frameShape();
        QRect r = w->style()->subElementRect(QStyle::SE_ShapedFrameContents, &opt, w);
        QRect o = opt.rect;
        int hfw = (r.left() - o.left()) + (o.right() - r.right());
        int vfw = (r.top() - o.top()) + (o.bottom() - r.bottom());

        width += hfw + w->verticalScrollBar()->sizeHint().width();
        // FIXME BASELINE: the 3 lines below could be removed.
        int lhs = m_widget->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
        if (lhs > 0) {
            width += lhs;
        }
        height = size * height + vfw;

        assert(includesPadding());
        width -= RenderWidget::paddingLeft() + RenderWidget::paddingRight();
        height -= RenderWidget::paddingTop() + RenderWidget::paddingBottom();

        setIntrinsicWidth(width);
        setIntrinsicHeight(height);
    } else {
        QSize s(m_widget->sizeHint());
        int w = s.width();
        int h = s.height();

        if (shouldDisableNativeBorders()) {
            const int dfw = 2 * m_widget->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, m_widget);
            w -= dfw;
            h -= dfw;
        }

        setIntrinsicWidth(w);
        setIntrinsicHeight(h);
    }
#else
    //AFA-FIXME
#endif

    /// uuh, ignore the following line..
    setNeedsLayout(true);
    RenderFormElement::layout();

    // and now disable the widget in case there is no <option> given
    const QVector<HTMLGenericFormElementImpl *> listItems = element()->listItems();

    bool foundOption = false;
    for (int i = 0; i < listItems.size() && !foundOption; i++) {
        foundOption = (listItems[i]->id() == ID_OPTION);
    }

#ifdef QT_WIDGETS_LIB
    m_widget->setEnabled(foundOption && ! element()->disabled());
#else
    QWidget *qmlWidget = m_qmlWidget;
    if (qmlWidget) {
        qmlWidget->setEnabled(foundOption && ! element()->disabled());
    }
#endif
}

void RenderSelect::slotSelected(int index) // emitted by the combobox only
{
    if (m_ignoreSelectEvents) {
        return;
    }

    KHTMLAssert(!m_useListBox);

    const QVector<HTMLGenericFormElementImpl *> listItems = element()->listItems();
    if (index >= 0 && index < int(listItems.size())) {
        bool found = (listItems[index]->id() == ID_OPTION);

        if (!found) {
            // this one is not selectable,  we need to find an option element
            while (index < listItems.size()) {
                if (listItems[index]->id() == ID_OPTION) {
                    found = true;
                    break;
                }
                ++index;
            }

            if (!found) {
                while (index >= 0) {
                    if (listItems[index]->id() == ID_OPTION) {
                        found = true;
                        break;
                    }
                    --index;
                }
            }
        }

        if (found) {
            bool changed = false;

            for (int i = 0; i < listItems.size(); ++i)
                if (listItems[i]->id() == ID_OPTION && i !=  index) {
                    HTMLOptionElementImpl *opt = static_cast<HTMLOptionElementImpl *>(listItems[i]);
                    changed |= (opt->m_selected == true);
                    opt->m_selected = false;
                }

            HTMLOptionElementImpl *opt = static_cast<HTMLOptionElementImpl *>(listItems[index]);
            changed |= (opt->m_selected == false);
            opt->m_selected = true;
#ifdef QT_WIDGETS_LIB
            if (index != static_cast<ComboBoxWidget *>(m_widget)->currentIndex()) {
                static_cast<ComboBoxWidget *>(m_widget)->setCurrentIndex(index);
            }
#endif
            // When selecting an optgroup item, and we move forward to we
            // shouldn't emit onChange. Hence this bool, the if above doesn't do it.
            if (changed) {
                ref();
                element()->onChange();
                deref();
            }
        }
    }
}

void RenderSelect::slotSelectionChanged() // emitted by the listbox only
{
    if (m_ignoreSelectEvents) {
        return;
    }

    // don't use listItems() here as we have to avoid recalculations - changing the
    // option list will make use update options not in the way the user expects them
    const QVector<HTMLGenericFormElementImpl *> listItems = element()->m_listItems;
    for (int i = 0; i < listItems.count(); i++)
        // don't use setSelected() here because it will cause us to be called
        // again with updateSelection.
        if (listItems[i]->id() == ID_OPTION)
#ifdef QT_WIDGETS_LIB
            static_cast<HTMLOptionElementImpl *>(listItems[i])
            ->m_selected = static_cast<QListWidget *>(m_widget)->item(i)->isSelected();
#else
            ;
#endif

    ref();
    element()->onChange();
    deref();
}

void RenderSelect::setOptionsChanged(bool _optionsChanged)
{
    m_optionsChanged = _optionsChanged;
}

void RenderSelect::setPadding()
{
    RenderFormElement::setPadding();
}

ListBoxWidget *RenderSelect::createListBox()
{
#ifdef QT_WIDGETS_LIB
    ListBoxWidget *lb = new ListBoxWidget(view()->widget());
    lb->setSelectionMode(m_multiple ? QListWidget::ExtendedSelection : QListWidget::SingleSelection);
    connect(lb, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectionChanged()));
    m_ignoreSelectEvents = false;
    lb->setMouseTracking(true);

    return lb;
#else
    ListBoxWidget *lb = new ListBoxWidget(view());
    //lb->setSelectionMode(m_multiple ? QListWidget::ExtendedSelection : QListWidget::SingleSelection);
    //AFA connect(lb, SIGNAL(currentIndexChanged()), this, SLOT(slotSelectionChanged()));
    m_ignoreSelectEvents = false;
    //lb->setMouseTracking(true);

    return lb;
#endif
}

ComboBoxWidget *RenderSelect::createComboBox()
{
#ifdef QT_WIDGETS_LIB
    ComboBoxWidget *cb = new ComboBoxWidget(view()->widget());
    connect(cb, SIGNAL(activated(int)), this, SLOT(slotSelected(int)));
    return cb;
#else
    ComboBoxWidget *cb = new ComboBoxWidget(view());
    //AFA connect(cb, SIGNAL(activated(int)), this, SLOT(slotSelected(int)));
    return cb;
#endif
}

void RenderSelect::updateSelection()
{
    const QVector<HTMLGenericFormElementImpl *> listItems = element()->listItems();
    int i;
    if (m_useListBox) {
        // if multi-select, we select only the new selected index
        QListWidget *listBox = static_cast<QListWidget *>(m_widget);
        for (i = 0; i < int(listItems.size()); i++)
#ifdef QT_WIDGETS_LIB
            listBox->item(i)->setSelected(listItems[i]->id() == ID_OPTION &&
                                          static_cast<HTMLOptionElementImpl *>(listItems[i])->selectedBit());
#else
            ;
#endif
    } else {
        bool found = false;
        int firstOption = i = listItems.size();
        while (i--)
            if (listItems[i]->id() == ID_OPTION) {
                if (found) {
                    static_cast<HTMLOptionElementImpl *>(listItems[i])->m_selected = false;
                } else if (static_cast<HTMLOptionElementImpl *>(listItems[i])->selectedBit()) {
#ifdef QT_WIDGETS_LIB
                    static_cast<QComboBox *>(m_widget)->setCurrentIndex(i);
#else
                    ;
#endif
                    found = true;
                }
                firstOption = i;
            }

        if (!found && firstOption != listItems.size()) {
            // select first option (IE7/Gecko behaviour)
            static_cast<HTMLOptionElementImpl *>(listItems[firstOption])->m_selected = true;
#ifdef QT_WIDGETS_LIB
            static_cast<QComboBox *>(m_widget)->setCurrentIndex(firstOption);
#else
            ;
#endif
        }
    }

    m_selectionChanged = false;
}

// -------------------------------------------------------------------------

TextAreaWidget::TextAreaWidget(int wrap, QWidget *parent)
    : QTextEdit(parent)
{
    m_kwp->setIsRedirected(true);

#ifdef QT_WIDGETS_LIB
    if (wrap != DOM::HTMLTextAreaElementImpl::ta_NoWrap) {
        setLineWrapMode(QTextEdit::WidgetWidth);
    } else {
        setLineWrapMode(QTextEdit::NoWrap);
    }

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    //AFA KCursor::setAutoHideCursor(viewport(), true);
    setAcceptRichText(false);
    setMouseTracking(true);
#else
    if (wrap != DOM::HTMLTextAreaElementImpl::ta_NoWrap) {
        setWrapMode(QQuickTextEdit::Wrap);
    } else {
        setWrapMode(QQuickTextEdit::NoWrap);
    }
    setTextFormat(QQuickTextEdit::RichText);
#endif
}

TextAreaWidget::~TextAreaWidget()
{
}

void TextAreaWidget::scrollContentsBy(int dx, int dy)
{
#ifdef QT_WIDGETS_LIB
    QTextEdit::scrollContentsBy(dx, dy);
    update();
#endif
}

bool TextAreaWidget::event(QEvent *e)
{
#if 0
    if (e->type() == QEvent::AccelAvailable && isReadOnly()) {
        QKeyEvent *ke = (QKeyEvent *) e;
        if (ke->modifiers() & Qt::ControlModifier) {
            switch (ke->key()) {
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Home:
            case Qt::Key_End:
                ke->accept();
            default:
                break;
            }
        }
    }
#endif
    // accept all wheel events so that they are not propagated to the view
    // once either end of the widget is reached.
    bool ret = QTextEdit::event(e);
    if (e->type() == QEvent::Wheel) {
        e->accept();
        ret = true;
    }
    return ret;
}

void TextAreaWidget::keyPressEvent(QKeyEvent *e)
{
    // The ComboBoxWidget::keyPressEvent() comment about having to
    // deal with events coming from EventPropagator::sendEvent()
    // directly applies here, too.
#ifdef QT_WIDGETS_LIB
    if ((e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) &&
            tabChangesFocus()) {
        e->ignore();
        return;
    }
    QTextEdit::keyPressEvent(e);
#endif
}

// -------------------------------------------------------------------------

RenderTextArea::RenderTextArea(HTMLTextAreaElementImpl *element)
    : RenderFormElement(element)
{
    TextAreaWidget *edit = new TextAreaWidget(element->wrap(), view());
    setQWidget(edit);
    const KHTMLSettings *settings = view()->part()->settings();
    //AFA edit->setCheckSpellingEnabled(settings->autoSpellCheck());
#ifdef QT_WIDGETS_LIB
    edit->setTabChangesFocus(! settings->allowTabulation());
#endif
    //AFA connect(edit, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));

    setText(element->value().string());
#ifdef QT_WIDGETS_LIB
    m_textAlignment = edit->alignment();
#endif
}

RenderTextArea::~RenderTextArea()
{
    element()->m_value = text();
}

void RenderTextArea::handleFocusOut()
{
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);

    if (w && element()->m_changed) {
        element()->m_changed = false;
        element()->onChange();
    }
}

void RenderTextArea::calcMinMaxWidth()
{
    KHTMLAssert(!minMaxKnown());

#ifdef QT_WIDGETS_LIB
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
    const QFontMetrics &m = style()->fontMetrics();
    w->setTabStopWidth(8 * m.width(" "));
    int lvs = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));
    int lhs = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing));
    int llm = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutLeftMargin));
    int lrm = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutRightMargin));
    int lbm = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
    int ltm = qMax(0, w->style()->pixelMetric(QStyle::PM_LayoutTopMargin));

    QStyleOptionFrameV3 opt;
    opt.initFrom(w);
    opt.lineWidth = w->lineWidth();
    opt.midLineWidth = w->midLineWidth();
    opt.frameShape = w->frameShape();
    QRect r = w->style()->subElementRect(QStyle::SE_ShapedFrameContents, &opt, w);
    QRect o = opt.rect;
    int hfw = (r.left() - o.left()) + (o.right() - r.right());
    int vfw = (r.top() - o.top()) + (o.bottom() - r.bottom());

    QSize size(qMax(element()->cols(), 1L)*m.width('x') + hfw + llm + lrm +
               w->verticalScrollBar()->sizeHint().width() + lhs,
               qMax(element()->rows(), 1L)*m.lineSpacing() + vfw + lbm + ltm +
               (w->lineWrapMode() == QTextEdit::NoWrap ?
                w->horizontalScrollBar()->sizeHint().height() + lvs : 0)
              );

    assert(includesPadding());
    size.rwidth() -= RenderWidget::paddingLeft() + RenderWidget::paddingRight();
    size.rheight() -= RenderWidget::paddingTop() + RenderWidget::paddingBottom();

    setIntrinsicWidth(size.width());
    setIntrinsicHeight(size.height());
#else
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
    const QFontMetrics &m = style()->fontMetrics();

    int lvs = 0;
    int lhs = 0;
    int llm = 0;
    int lrm = 0;
    int lbm = 0;
    int ltm = 0;

    QRect r;
    QRect o;
    int hfw = (r.left() - o.left()) + (o.right() - r.right());
    int vfw = (r.top() - o.top()) + (o.bottom() - r.bottom());

    QSize size(qMax(element()->cols(), 1L)*m.width('x') + hfw + llm + lrm + lhs,
               qMax(element()->rows(), 1L)*m.lineSpacing() + vfw + lbm + ltm + lvs);

    assert(includesPadding());
    size.rwidth() -= RenderWidget::paddingLeft() + RenderWidget::paddingRight();
    size.rheight() -= RenderWidget::paddingTop() + RenderWidget::paddingBottom();

    setIntrinsicWidth(size.width());
    setIntrinsicHeight(size.height());
#endif

    RenderFormElement::calcMinMaxWidth();
}

void RenderTextArea::setStyle(RenderStyle *_style)
{
    RenderFormElement::setStyle(_style);

#ifdef QT_WIDGETS_LIB
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);

    if (m_textAlignment != textAlignment()) {
        m_textAlignment = textAlignment();
        bool unsubmittedFormChange = element()->m_unsubmittedFormChange;
        bool blocked = w->blockSignals(true);
        int cx = w->horizontalScrollBar()->value();
        int cy = w->verticalScrollBar()->value();
        QTextCursor tc = w->textCursor();
        // Set alignment on all textarea's paragraphs
        w->selectAll();
        w->setAlignment(m_textAlignment);
        w->setTextCursor(tc);
        w->horizontalScrollBar()->setValue(cx);
        w->verticalScrollBar()->setValue(cy);
        w->blockSignals(blocked);
        element()->m_unsubmittedFormChange = unsubmittedFormChange;
    }

    if (style()->overflowX() == OSCROLL) {
        w->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    } else if (style()->overflowX() == OHIDDEN) {
        w->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        w->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    if (style()->overflowY() == OSCROLL) {
        w->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    } else if (style()->overflowY() == OHIDDEN) {
        w->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        w->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
#endif
}

short RenderTextArea::scrollWidth() const
{
    return RenderObject::scrollWidth();
}

int RenderTextArea::scrollHeight() const
{
#ifdef QT_WIDGETS_LIB
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
    int contentHeight = qRound(w->document()->size().height());
    return qMax(contentHeight, RenderObject::clientHeight());
#else
    return 0;
#endif
}

void RenderTextArea::setText(const QString &newText)
{
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);

    // When this is called, m_value in the element must have just
    // been set to new value --- see if we have any work to do
#ifdef QT_WIDGETS_LIB
    QString oldText = text();
    int oldTextLen = oldText.length();
    int newTextLen = newText.length();
    if (newTextLen != oldTextLen || newText != oldText) {
        bool blocked = w->blockSignals(true);
        int cx = w->horizontalScrollBar()->value();
        int cy = w->verticalScrollBar()->value();
        // Not using setPlaintext as it resets text alignment property
        int minLen = qMin(newTextLen, oldTextLen);
        int ex = 0;
        while (ex < minLen && (newText.at(ex) == oldText.at(ex))) {
            ++ex;
        }
        QTextCursor tc = w->textCursor();
        tc.setPosition(ex, QTextCursor::MoveAnchor);
        tc.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        tc.insertText(newText.right(newTextLen - ex));

        if (oldTextLen == 0) {
            tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        } else {
            tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        }
        w->setTextCursor(tc);
        w->horizontalScrollBar()->setValue(cx);
        w->verticalScrollBar()->setValue(cy);
        w->blockSignals(blocked);
    }
#endif
}

void RenderTextArea::updateFromElement()
{
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
    w->setReadOnly(element()->readOnly());
    //AFA w->setClickMessage(element()->placeholder().string());
    RenderFormElement::updateFromElement();
}

QString RenderTextArea::text()
{
    // ### We may want to cache this when physical, since the DOM no longer caches,
    // but seeing how text() has always been called on textChanged(), it's probably not needed

    QString txt;
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
#ifdef QT_WIDGETS_LIB
#ifdef __GNUC__
#warning "Physical wrap mode needs testing (also in ::selection*)"
#endif
    if (element()->wrap() == DOM::HTMLTextAreaElementImpl::ta_Physical) {
        QTextCursor tc(w->document());
        while (!tc.atEnd()) {
            tc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            txt += tc.selectedText();
            if (tc.movePosition(QTextCursor::Right)) {
                txt += QLatin1String("\n");
                tc.movePosition(QTextCursor::StartOfLine);
            } else {
                break;
            }
        }
    } else {
        txt = w->toPlainText();
    }
#endif
    return txt;
}

void RenderTextArea::highLightWord(unsigned int length, unsigned int pos)
{
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
    if (w) {
        //AFA w->highlightWord(length, pos);
    }
}

void RenderTextArea::slotTextChanged()
{
    element()->m_changed    = true;
    if (element()->m_value != text()) {
        element()->m_unsubmittedFormChange = true;
    }
}

void RenderTextArea::select()
{
    if (m_widget) {
#ifdef QT_WIDGETS_LIB
        static_cast<TextAreaWidget *>(m_widget)->selectAll();
#endif
    }
}

long RenderTextArea::selectionStart()
{
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
#ifdef QT_WIDGETS_LIB
    return w->textCursor().selectionStart();
#else
    return 0;
#endif
}

long RenderTextArea::selectionEnd()
{
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
#ifdef QT_WIDGETS_LIB
    return w->textCursor().selectionEnd();
#else
    return 0;
#endif
}

static void setPhysWrapPos(QTextCursor &otc, bool selStart, int idx)
{
    QTextCursor tc = otc;
    tc.setPosition(0);
    tc.movePosition(QTextCursor::EndOfLine);
    while (!tc.atEnd()) {
        if (tc.movePosition(QTextCursor::Down) && tc.position() < idx) {
            --idx;
        }
        if (tc.position() >= idx) {
            break;
        }
    }
    otc.setPosition(idx, selStart ? QTextCursor::MoveAnchor : QTextCursor::KeepAnchor);
}

void RenderTextArea::setSelectionStart(long offset)
{
#ifdef QT_WIDGETS_LIB
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
    QTextCursor tc = w->textCursor();
    if (element()->wrap() == DOM::HTMLTextAreaElementImpl::ta_Physical) {
        setPhysWrapPos(tc, true /*selStart*/, offset);
    } else {
        tc.setPosition(offset);
    }
    w->setTextCursor(tc);
#endif
}

void RenderTextArea::setSelectionEnd(long offset)
{
#ifdef QT_WIDGETS_LIB
    TextAreaWidget *w = static_cast<TextAreaWidget *>(m_widget);
    QTextCursor tc = w->textCursor();
    if (element()->wrap() == DOM::HTMLTextAreaElementImpl::ta_Physical) {
        setPhysWrapPos(tc, false /*selStart*/, offset);
    } else {
        tc.setPosition(offset, QTextCursor::KeepAnchor);
    }
    w->setTextCursor(tc);
#endif
}

void RenderTextArea::setSelectionRange(long start, long end)
{
    setSelectionStart(start);
    setSelectionEnd(end);
}
// ---------------------------------------------------------------------------

