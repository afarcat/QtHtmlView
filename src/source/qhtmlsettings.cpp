/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "qhtmlsettings.h"
#include "khtmldefaults.h"

#include <kparts/htmlsettingsinterface.h>
#include "khtml_debug.h"
#include <qhtmlfilter_p.h>

#include <kjob.h>
#include <kio/job.h>

#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QByteArray>
#include <QUrl>
#include <qstandardpaths.h>
#include <QFontDatabase>
#include <QFontInfo>

/**
 * @internal
 * Contains all settings which are both available globally and per-domain
 */
struct KPerDomainSettings {
    bool m_bEnableJava : 1;
    bool m_bEnableJavaScript : 1;
    bool m_bEnablePlugins : 1;
    // don't forget to maintain the bitfields as the enums grow
    QHTMLSettings::KJSWindowOpenPolicy m_windowOpenPolicy : 2;
    QHTMLSettings::KJSWindowStatusPolicy m_windowStatusPolicy : 1;
    QHTMLSettings::KJSWindowFocusPolicy m_windowFocusPolicy : 1;
    QHTMLSettings::KJSWindowMovePolicy m_windowMovePolicy : 1;
    QHTMLSettings::KJSWindowResizePolicy m_windowResizePolicy : 1;

#ifdef DEBUG_SETTINGS
    void dump(const QString &infix = QString()) const
    {
        // qCDebug(KHTML_LOG) << "KPerDomainSettings " << infix << " @" << this << ":";
        // qCDebug(KHTML_LOG) << "  m_bEnableJava: " << m_bEnableJava;
        // qCDebug(KHTML_LOG) << "  m_bEnableJavaScript: " << m_bEnableJavaScript;
        // qCDebug(KHTML_LOG) << "  m_bEnablePlugins: " << m_bEnablePlugins;
        // qCDebug(KHTML_LOG) << "  m_windowOpenPolicy: " << m_windowOpenPolicy;
        // qCDebug(KHTML_LOG) << "  m_windowStatusPolicy: " << m_windowStatusPolicy;
        // qCDebug(KHTML_LOG) << "  m_windowFocusPolicy: " << m_windowFocusPolicy;
        // qCDebug(KHTML_LOG) << "  m_windowMovePolicy: " << m_windowMovePolicy;
        // qCDebug(KHTML_LOG) << "  m_windowResizePolicy: " << m_windowResizePolicy;
    }
#endif
};

QString *QHTMLSettings::avFamilies = nullptr;
typedef QMap<QString, KPerDomainSettings> PolicyMap;

// The "struct" that contains all the data. Must be copiable (no pointers).
class QHTMLSettingsData
{
public:
    bool m_bChangeCursor : 1;
    bool m_bOpenMiddleClick : 1;
    bool m_underlineLink : 1;
    bool m_hoverLink : 1;
    bool m_bEnableJavaScriptDebug : 1;
    bool m_bEnableJavaScriptErrorReporting : 1;
    bool enforceCharset : 1;
    bool m_bAutoLoadImages : 1;
    bool m_bUnfinishedImageFrame : 1;
    bool m_formCompletionEnabled : 1;
    bool m_autoDelayedActionsEnabled : 1;
    bool m_jsErrorsEnabled : 1;
    bool m_follow_system_colors : 1;
    bool m_allowTabulation : 1;
    bool m_autoSpellCheck : 1;
    bool m_adFilterEnabled : 1;
    bool m_hideAdsEnabled : 1;
    bool m_jsPopupBlockerPassivePopup : 1;
    bool m_accessKeysEnabled : 1;

    // the virtual global "domain"
    KPerDomainSettings global;

    int m_fontSize;
    int m_minFontSize;
    int m_maxFormCompletionItems;
    QHTMLSettings::KAnimationAdvice m_showAnimations;
    QHTMLSettings::KSmoothScrollingMode m_smoothScrolling;
    QHTMLSettings::KDNSPrefetch m_dnsPrefetch;

    QString m_encoding;
    QString m_userSheet;

    QColor m_textColor;
    QColor m_baseColor;
    QColor m_linkColor;
    QColor m_vLinkColor;

    PolicyMap domainPolicy;
    QStringList fonts;
    QStringList defaultFonts;

    khtml::FilterSet adBlackList;
    khtml::FilterSet adWhiteList;
    QList< QPair< QString, QChar > > m_fallbackAccessKeysAssignments;
};

class QHTMLSettingsPrivate : public QObject, public QHTMLSettingsData
{
    Q_OBJECT
public:

    void adblockFilterLoadList(const QString &filename)
    {
        // qCDebug(KHTML_LOG) << "Loading filter list from" << filename;
        /** load list file and process each line */
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream ts(&file);
            QString line = ts.readLine();
#ifndef NDEBUG /// only count when compiled for debugging
            int whiteCounter = 0, blackCounter = 0;
#endif // NDEBUG
            while (!line.isEmpty()) {
                /** white list lines start with "@@" */
                if (line.startsWith(QLatin1String("@@"))) {
#ifndef NDEBUG
                    ++whiteCounter;
#endif // NDEBUG
                    adWhiteList.addFilter(line);
                } else {
#ifndef NDEBUG
                    ++blackCounter;
#endif // NDEBUG
                    adBlackList.addFilter(line);
                }

                line = ts.readLine();
            }
            file.close();

#ifndef NDEBUG
            // qCDebug(KHTML_LOG) << "Filter list loaded" << whiteCounter << "white list entries and" << blackCounter << "black list entries";
#endif // NDEBUG
        }
    }

public Q_SLOTS:
//    void adblockFilterResult(KJob *job)
//    {
//        KIO::StoredTransferJob *tJob = qobject_cast<KIO::StoredTransferJob *>(job);
//        Q_ASSERT(tJob);

//        if (tJob->error()) {
//            // qCDebug(KHTML_LOG) << "Failed to download" << tJob->url() << "with message:" << tJob->errorText();
//        } else if (tJob->isErrorPage()) { // 4XX error code
//            // qCDebug(KHTML_LOG) << "Failed to fetch filter list" << tJob->url();
//        } else {
//            const QByteArray byteArray = tJob->data();
//            const QString localFileName = tJob->property("khtmlsettings_adBlock_filename").toString();

//            QFile file(localFileName);
//            if (file.open(QFile::WriteOnly)) {
//                bool success = file.write(byteArray) == byteArray.size();
//                file.close();
//                if (success) {
//                    adblockFilterLoadList(localFileName);
//                } else {
//                    // qCDebug(KHTML_LOG) << "Could not write" << byteArray.size() << "to file" << localFileName;
//                }
//            } else {
//                // qCDebug(KHTML_LOG) << "Cannot open file" << localFileName << "for filter list";
//            }
//        }

//    }
};

/** Returns a writeable per-domains settings instance for the given domain
  * or a deep copy of the global settings if not existent.
  */
static KPerDomainSettings &setup_per_domain_policy(
    QHTMLSettingsPrivate *const d,
    const QString &domain)
{
    if (domain.isEmpty()) {
        qCWarning(KHTML_LOG) << "setup_per_domain_policy: domain is empty";
    }
    const QString ldomain = domain.toLower();
    PolicyMap::iterator it = d->domainPolicy.find(ldomain);
    if (it == d->domainPolicy.end()) {
        // simply copy global domain settings (they should have been initialized
        // by this time)
        it = d->domainPolicy.insert(ldomain, d->global);
    }
    return *it;
}

QHTMLSettings::KJavaScriptAdvice QHTMLSettings::strToAdvice(const QString &_str)
{
    return static_cast<KJavaScriptAdvice>(KParts::HtmlSettingsInterface::textToJavascriptAdvice(_str));
}

const char *QHTMLSettings::adviceToStr(KJavaScriptAdvice _advice)
{
    return KParts::HtmlSettingsInterface::javascriptAdviceToText(static_cast<KParts::HtmlSettingsInterface::JavaScriptAdvice>(_advice));
}

void QHTMLSettings::splitDomainAdvice(const QString &configStr, QString &domain,
                                      KJavaScriptAdvice &javaAdvice, KJavaScriptAdvice &javaScriptAdvice)
{
    KParts::HtmlSettingsInterface::JavaScriptAdvice jAdvice, jsAdvice;
    //AFA KParts::HtmlSettingsInterface::splitDomainAdvice(configStr, domain, jAdvice, jsAdvice);
    javaAdvice = static_cast<KJavaScriptAdvice>(jAdvice);
    javaScriptAdvice = static_cast<KJavaScriptAdvice>(jsAdvice);
}

void QHTMLSettings::readDomainSettings(const QSettings &config, bool reset,
                                       bool global, KPerDomainSettings &pd_settings)
{
//    QString jsPrefix = global ? QString()
//                       : QString::fromLatin1("javascript.");
//    QString javaPrefix = global ? QString()
//                         : QString::fromLatin1("java.");
//    QString pluginsPrefix = global ? QString()
//                            : QString::fromLatin1("plugins.");

//    // The setting for Java
//    QString key = javaPrefix + QLatin1String("EnableJava");
//    if ((global && reset) || config->contains(key)) {
//        pd_settings.m_bEnableJava = config.value(key, false);
//    } else if (!global) {
//        pd_settings.m_bEnableJava = d->global.m_bEnableJava;
//    }

//    // The setting for Plugins
//    key = pluginsPrefix + QLatin1String("EnablePlugins");
//    if ((global && reset) || config->contains(key)) {
//        pd_settings.m_bEnablePlugins = config.value(key, true);
//    } else if (!global) {
//        pd_settings.m_bEnablePlugins = d->global.m_bEnablePlugins;
//    }

//    // The setting for JavaScript
//    key = jsPrefix + QLatin1String("EnableJavaScript");
//    if ((global && reset) || config->contains(key)) {
//        pd_settings.m_bEnableJavaScript = config.value(key, true);
//    } else if (!global) {
//        pd_settings.m_bEnableJavaScript = d->global.m_bEnableJavaScript;
//    }

//    // window property policies
//    key = jsPrefix + QLatin1String("WindowOpenPolicy");
//    if ((global && reset) || config->contains(key))
//        pd_settings.m_windowOpenPolicy = (KJSWindowOpenPolicy)
//                                         config.value(key, uint(KJSWindowOpenSmart));
//    else if (!global) {
//        pd_settings.m_windowOpenPolicy = d->global.m_windowOpenPolicy;
//    }

//    key = jsPrefix + QLatin1String("WindowMovePolicy");
//    if ((global && reset) || config->contains(key))
//        pd_settings.m_windowMovePolicy = (KJSWindowMovePolicy)
//                                         config.value(key, uint(KJSWindowMoveAllow));
//    else if (!global) {
//        pd_settings.m_windowMovePolicy = d->global.m_windowMovePolicy;
//    }

//    key = jsPrefix + QLatin1String("WindowResizePolicy");
//    if ((global && reset) || config->contains(key))
//        pd_settings.m_windowResizePolicy = (KJSWindowResizePolicy)
//                                           config.value(key, uint(KJSWindowResizeAllow));
//    else if (!global) {
//        pd_settings.m_windowResizePolicy = d->global.m_windowResizePolicy;
//    }

//    key = jsPrefix + QLatin1String("WindowStatusPolicy");
//    if ((global && reset) || config->contains(key))
//        pd_settings.m_windowStatusPolicy = (KJSWindowStatusPolicy)
//                                           config.value(key, uint(KJSWindowStatusAllow));
//    else if (!global) {
//        pd_settings.m_windowStatusPolicy = d->global.m_windowStatusPolicy;
//    }

//    key = jsPrefix + QLatin1String("WindowFocusPolicy");
//    if ((global && reset) || config->contains(key))
//        pd_settings.m_windowFocusPolicy = (KJSWindowFocusPolicy)
//                                          config.value(key, uint(KJSWindowFocusAllow));
//    else if (!global) {
//        pd_settings.m_windowFocusPolicy = d->global.m_windowFocusPolicy;
//    }

}

QHTMLSettings::QHTMLSettings()
    : d(new QHTMLSettingsPrivate())
{
    init();
}

QHTMLSettings::QHTMLSettings(const QHTMLSettings &other)
    : d(new QHTMLSettingsPrivate())
{
    QHTMLSettingsData *data = d;
    *data = *other.d;
}

QHTMLSettings::~QHTMLSettings()
{
    delete d;
}

bool QHTMLSettings::changeCursor() const
{
    return d->m_bChangeCursor;
}

bool QHTMLSettings::underlineLink() const
{
    return d->m_underlineLink;
}

bool QHTMLSettings::hoverLink() const
{
    return d->m_hoverLink;
}

void QHTMLSettings::init()
{
    QSettings global("khtmlrc", QSettings::IniFormat);
    init(&global, true);

//    KSharedConfig::Ptr local = KSharedConfig::openConfig();
//    if (!local) {
//        return;
//    }

//    init(local.data(), false);
}

void QHTMLSettings::init(QSettings *config, bool reset)
{
    config->beginGroup("MainView Settings");
    if (reset || config->contains("OpenMiddleClick")) {
        d->m_bOpenMiddleClick = config->value("OpenMiddleClick", true).toBool();
    }
    config->endGroup();

//    KConfigGroup cgAccess(config, "Access Keys");
//    if (reset || cgAccess.exists()) {
//        d->m_accessKeysEnabled = cgAccess.value("Enabled", true);
//    }

//    KConfigGroup cgFilter(config, "Filter Settings");

//    if (reset || cgFilter.exists()) {
//        d->m_adFilterEnabled = cgFilter.value("Enabled", false);
//        d->m_hideAdsEnabled = cgFilter.value("Shrink", false);

//        d->adBlackList.clear();
//        d->adWhiteList.clear();

//        if (d->m_adFilterEnabled) {

//            /** read maximum age for filter list files, minimum is one day */
//            int htmlFilterListMaxAgeDays = cgFilter.value(QString("HTMLFilterListMaxAgeDays")).toInt();
//            if (htmlFilterListMaxAgeDays < 1) {
//                htmlFilterListMaxAgeDays = 1;
//            }

//            QMap<QString, QString> entryMap = cgFilter.entryMap();
//            QMap<QString, QString>::ConstIterator it;
//            for (it = entryMap.constBegin(); it != entryMap.constEnd(); ++it) {
//                int id = -1;
//                QString name = it.key();
//                QString url = it.value();

//                if (name.startsWith("Filter")) {
//                    if (url.startsWith(QLatin1String("@@"))) {
//                        d->adWhiteList.addFilter(url);
//                    } else {
//                        d->adBlackList.addFilter(url);
//                    }
//                } else if (name.startsWith("HTMLFilterListName-") && (id = name.mid(19).toInt()) > 0) {
//                    /** check if entry is enabled */
//                    bool filterEnabled = cgFilter.value(QString("HTMLFilterListEnabled-").append(QString::number(id))) != QLatin1String("false");

//                    /** get url for HTMLFilterList */
//                    QUrl url(cgFilter.value(QString("HTMLFilterListURL-").append(QString::number(id))));

//                    if (filterEnabled && url.isValid()) {
//                        /** determine where to cache HTMLFilterList file */
//                        QString localFile = cgFilter.value(QString("HTMLFilterListLocalFilename-").append(QString::number(id)));
//                        localFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + "khtml/" + localFile;

//                        /** determine existence and age of cache file */
//                        QFileInfo fileInfo(localFile);

//                        /** load cached file if it exists, irrespective of age */
//                        if (fileInfo.exists()) {
//                            d->adblockFilterLoadList(localFile);
//                        }

//                        /** if no cache list file exists or if it is too old ... */
//                        if (!fileInfo.exists() || fileInfo.lastModified().daysTo(QDateTime::currentDateTime()) > htmlFilterListMaxAgeDays) {
//                            /** ... in this case, refetch list asynchronously */
//                            // qCDebug(KHTML_LOG) << "Asynchronously fetching filter list from" << url << "to" << localFile;

//                            KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
//                            QObject::connect(job, SIGNAL(result(KJob*)), d, SLOT(adblockFilterResult(KJob*)));
//                            /** for later reference, store name of cache file */
//                            job->setProperty("khtmlsettings_adBlock_filename", localFile);
//                        }
//                    }
//                }
//            }
//        }

//    }

    config->beginGroup("HTML Settings");
    if (reset || !config->group().isEmpty()) {
        // Fonts and colors
        if (reset) {
            d->defaultFonts = QStringList();
            d->defaultFonts.append(config->value("StandardFont", QFontDatabase::systemFont(QFontDatabase::GeneralFont).family()).toString());
            d->defaultFonts.append(config->value("FixedFont", QFontDatabase::systemFont(QFontDatabase::FixedFont).family()).toString());
            // Resolve generic font family names
            const QString serifFont = QFontInfo(QFont(QLatin1String(HTML_DEFAULT_VIEW_SERIF_FONT))).family();
            const QString sansSerifFont = QFontInfo(QFont(QLatin1String(HTML_DEFAULT_VIEW_SANSSERIF_FONT))).family();
            const QString cursiveFont = QFontInfo(QFont(QLatin1String(HTML_DEFAULT_VIEW_CURSIVE_FONT))).family();
            const QString fantasyFont = QFontInfo(QFont(QLatin1String(HTML_DEFAULT_VIEW_FANTASY_FONT))).family();
            d->defaultFonts.append(config->value("SerifFont", serifFont).toString());
            d->defaultFonts.append(config->value("SansSerifFont", sansSerifFont).toString());
            d->defaultFonts.append(config->value("CursiveFont", cursiveFont).toString());
            d->defaultFonts.append(config->value("FantasyFont", fantasyFont).toString());
            d->defaultFonts.append(QString("0"));     // font size adjustment
        }

        if (reset || config->contains("MinimumFontSize")) {
            d->m_minFontSize = config->value("MinimumFontSize", HTML_DEFAULT_MIN_FONT_SIZE).toInt();
        }

        if (reset || config->contains("MediumFontSize")) {
            d->m_fontSize = config->value("MediumFontSize", 12).toInt();
        }

        d->fonts = config->value("Fonts", QStringList()).toStringList();
        const int fontsListLength = d->fonts.length();
        // Resolve generic font family names
        for (int i = 0; i < fontsListLength; ++i) {
            const QString fontFamily = d->fonts.at(i);
            if (!fontFamily.isEmpty()) {
                d->fonts[i] = QFontInfo(QFont(fontFamily)).family();
                //qCWarning(KHTML_LOG) << "Font family name:" << fontFamily << "resolved to:" << d->fonts.at(i);
            }
        }

        if (reset || config->contains("DefaultEncoding")) {
            d->m_encoding = config->value("DefaultEncoding", "").toString();
        }

        if (reset || config->contains("EnforceDefaultCharset")) {
            d->enforceCharset = config->value("EnforceDefaultCharset", false).toBool();
        }

        // Behavior
        if (reset || config->contains("ChangeCursor")) {
            d->m_bChangeCursor = config->value("ChangeCursor", true).toBool();
        }

        if (reset || config->contains("UnderlineLinks")) {
            d->m_underlineLink = config->value("UnderlineLinks", true).toBool();
        }

        if (reset || config->contains("HoverLinks")) {
            if ((d->m_hoverLink = config->value("HoverLinks", false).toBool())) {
                d->m_underlineLink = false;
            }
        }

        if (reset || config->contains("AllowTabulation")) {
            d->m_allowTabulation = config->value("AllowTabulation", false).toBool();
        }

        if (reset || config->contains("AutoSpellCheck")) {
            d->m_autoSpellCheck = config->value("AutoSpellCheck", true).toBool();
        }

        // Other
        if (reset || config->contains("AutoLoadImages")) {
            d->m_bAutoLoadImages = config->value("AutoLoadImages", true).toBool();
        }

        if (reset || config->contains("UnfinishedImageFrame")) {
            d->m_bUnfinishedImageFrame = config->value("UnfinishedImageFrame", true).toBool();
        }

        if (reset || config->contains("ShowAnimations")) {
            QString value = config->value("ShowAnimations").toString().toLower();
            if (value == "disabled") {
                d->m_showAnimations = KAnimationDisabled;
            } else if (value == "looponce") {
                d->m_showAnimations = KAnimationLoopOnce;
            } else {
                d->m_showAnimations = KAnimationEnabled;
            }
        }

        if (reset || config->contains("SmoothScrolling")) {
            QString value = config->value("SmoothScrolling", "whenefficient").toString().toLower();
            if (value == "disabled") {
                d->m_smoothScrolling = KSmoothScrollingDisabled;
            } else if (value == "whenefficient") {
                d->m_smoothScrolling = KSmoothScrollingWhenEfficient;
            } else {
                d->m_smoothScrolling = KSmoothScrollingEnabled;
            }
        }

        if (reset || config->contains("DNSPrefetch")) {
            // Enabled, Disabled, OnlyWWWAndSLD
            QString value = config->value("DNSPrefetch", "Enabled").toString().toLower();
            if (value == "enabled") {
                d->m_dnsPrefetch = KDNSPrefetchEnabled;
            } else if (value == "onlywwwandsld") {
                d->m_dnsPrefetch = KDNSPrefetchOnlyWWWAndSLD;
            } else {
                d->m_dnsPrefetch = KDNSPrefetchDisabled;
            }
        }

        if (config->value("UserStyleSheetEnabled", false) == true) {
            if (reset || config->contains("UserStyleSheet")) {
                d->m_userSheet = config->value("UserStyleSheet", "").toString();
            }
        }

        d->m_formCompletionEnabled = config->value("FormCompletion", true).toBool();
        d->m_maxFormCompletionItems = config->value("MaxFormCompletionItems", 10).toInt();
        d->m_autoDelayedActionsEnabled = config->value("AutoDelayedActions", true).toBool();
        d->m_jsErrorsEnabled = config->value("ReportJSErrors", true).toBool();
        const QStringList accesskeys = config->value("FallbackAccessKeysAssignments", QStringList()).toStringList();
        d->m_fallbackAccessKeysAssignments.clear();
        for (QStringList::ConstIterator it = accesskeys.begin(); it != accesskeys.end(); ++it)
            if ((*it).length() > 2 && (*it)[ 1 ] == ':') {
                d->m_fallbackAccessKeysAssignments.append(qMakePair((*it).mid(2), (*it)[ 0 ]));
            }
    }
    config->endGroup();

//    // Colors
//    //In which group ?????
//    if (reset || cg->contains("FollowSystemColors")) {
//        d->m_follow_system_colors = cg.value("FollowSystemColors", false);
//    }

    config->beginGroup("General");
    if (reset || !config->group().isEmpty()) {
        if (reset || config->contains("foreground")) {
            QColor def(HTML_DEFAULT_TXT_COLOR);
            d->m_textColor = config->value("foreground", def).value<QColor>();
        }

        if (reset || config->contains("linkColor")) {
            QColor def(HTML_DEFAULT_LNK_COLOR);
            d->m_linkColor = config->value("linkColor", def).value<QColor>();
        }

        if (reset || config->contains("visitedLinkColor")) {
            QColor def(HTML_DEFAULT_VLNK_COLOR);
            d->m_vLinkColor = config->value("visitedLinkColor", def).value<QColor>();
        }

        if (reset || config->contains("background")) {
            QColor def(HTML_DEFAULT_BASE_COLOR);
            d->m_baseColor = config->value("background", def).value<QColor>();
        }
    }
    config->endGroup();

//    KConfigGroup cgJava(config, "Java/JavaScript Settings");
//    if (reset || cgJava.exists()) {
//        // The global setting for JavaScript debugging
//        // This is currently always enabled by default
//        if (reset || cgJava->contains("EnableJavaScriptDebug")) {
//            d->m_bEnableJavaScriptDebug = cgJava.value("EnableJavaScriptDebug", false);
//        }

//        // The global setting for JavaScript error reporting
//        if (reset || cgJava->contains("ReportJavaScriptErrors")) {
//            d->m_bEnableJavaScriptErrorReporting = cgJava.value("ReportJavaScriptErrors", false);
//        }

//        // The global setting for popup block passive popup
//        if (reset || cgJava->contains("PopupBlockerPassivePopup")) {
//            d->m_jsPopupBlockerPassivePopup = cgJava.value("PopupBlockerPassivePopup", true);
//        }

//        // Read options from the global "domain"
//        readDomainSettings(cgJava, reset, true, d->global);
//#ifdef DEBUG_SETTINGS
//        d->global.dump("init global");
//#endif

//        // The domain-specific settings.

//        static const char *const domain_keys[] = {  // always keep order of keys
//            "ECMADomains", "JavaDomains", "PluginDomains"
//        };
//        bool check_old_ecma_settings = true;
//        bool check_old_java_settings = true;
//        // merge all domains into one list
//        QMap<QString, int> domainList;  // why can't Qt have a QSet?
//        for (unsigned i = 0; i < sizeof domain_keys / sizeof domain_keys[0]; ++i) {
//            if (reset || cgJava->contains(domain_keys[i])) {
//                if (i == 0) {
//                    check_old_ecma_settings = false;
//                } else if (i == 1) {
//                    check_old_java_settings = false;
//                }
//                const QStringList dl = cgJava.value(domain_keys[i], QStringList());
//                const QMap<QString, int>::Iterator notfound = domainList.end();
//                QStringList::ConstIterator it = dl.begin();
//                const QStringList::ConstIterator itEnd = dl.end();
//                for (; it != itEnd; ++it) {
//                    const QString domain = (*it).toLower();
//                    QMap<QString, int>::Iterator pos = domainList.find(domain);
//                    if (pos == notfound) {
//                        domainList.insert(domain, 0);
//                    }
//                }/*next it*/
//            }
//        }/*next i*/

//        if (reset) {
//            d->domainPolicy.clear();
//        }

//        {
//            QMap<QString, int>::ConstIterator it = domainList.constBegin();
//            const QMap<QString, int>::ConstIterator itEnd = domainList.constEnd();
//            for (; it != itEnd; ++it) {
//                const QString domain = it.key();
//                config->beginGroup(domain);
//                readDomainSettings(cg, reset, false, d->domainPolicy[domain]);
//#ifdef DEBUG_SETTINGS
//                d->domainPolicy[domain].dump("init " + domain);
//#endif
//            }
//        }

//        bool check_old_java = true;
//        if ((reset || cgJava->contains("JavaDomainSettings"))
//                && check_old_java_settings) {
//            check_old_java = false;
//            const QStringList domainList = cgJava.value("JavaDomainSettings", QStringList());
//            QStringList::ConstIterator it = domainList.constBegin();
//            const QStringList::ConstIterator itEnd = domainList.constEnd();
//            for (; it != itEnd; ++it) {
//                QString domain;
//                KJavaScriptAdvice javaAdvice;
//                KJavaScriptAdvice javaScriptAdvice;
//                splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
//                setup_per_domain_policy(d, domain).m_bEnableJava =
//                    javaAdvice == KJavaScriptAccept;
//#ifdef DEBUG_SETTINGS
//                setup_per_domain_policy(d, domain).dump("JavaDomainSettings 4 " + domain);
//#endif
//            }
//        }

//        bool check_old_ecma = true;
//        if ((reset || cgJava->contains("ECMADomainSettings"))
//                && check_old_ecma_settings) {
//            check_old_ecma = false;
//            const QStringList domainList = cgJava.value("ECMADomainSettings", QStringList());
//            QStringList::ConstIterator it = domainList.constBegin();
//            const QStringList::ConstIterator itEnd = domainList.constEnd();
//            for (; it != itEnd; ++it) {
//                QString domain;
//                KJavaScriptAdvice javaAdvice;
//                KJavaScriptAdvice javaScriptAdvice;
//                splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
//                setup_per_domain_policy(d, domain).m_bEnableJavaScript =
//                    javaScriptAdvice == KJavaScriptAccept;
//#ifdef DEBUG_SETTINGS
//                setup_per_domain_policy(d, domain).dump("ECMADomainSettings 4 " + domain);
//#endif
//            }
//        }

//        if ((reset || cgJava->contains("JavaScriptDomainAdvice"))
//                && (check_old_java || check_old_ecma)
//                && (check_old_ecma_settings || check_old_java_settings)) {
//            const QStringList domainList = cgJava.value("JavaScriptDomainAdvice", QStringList());
//            QStringList::ConstIterator it = domainList.constBegin();
//            const QStringList::ConstIterator itEnd = domainList.constEnd();
//            for (; it != itEnd; ++it) {
//                QString domain;
//                KJavaScriptAdvice javaAdvice;
//                KJavaScriptAdvice javaScriptAdvice;
//                splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
//                if (check_old_java)
//                    setup_per_domain_policy(d, domain).m_bEnableJava =
//                        javaAdvice == KJavaScriptAccept;
//                if (check_old_ecma)
//                    setup_per_domain_policy(d, domain).m_bEnableJavaScript =
//                        javaScriptAdvice == KJavaScriptAccept;
//#ifdef DEBUG_SETTINGS
//                setup_per_domain_policy(d, domain).dump("JavaScriptDomainAdvice 4 " + domain);
//#endif
//            }

//            //save all the settings into the new keywords if they don't exist
//#if 0
//            if (check_old_java) {
//                QStringList domainConfig;
//                PolicyMap::Iterator it;
//                for (it = d->javaDomainPolicy.begin(); it != d->javaDomainPolicy.end(); ++it) {
//                    QByteArray javaPolicy = adviceToStr(it.value());
//                    QByteArray javaScriptPolicy = adviceToStr(KJavaScriptDunno);
//                    domainConfig.append(QString::fromLatin1("%1:%2:%3").arg(it.key()).arg(javaPolicy).arg(javaScriptPolicy));
//                }
//                cg.writeEntry("JavaDomainSettings", domainConfig);
//            }

//            if (check_old_ecma) {
//                QStringList domainConfig;
//                PolicyMap::Iterator it;
//                for (it = d->javaScriptDomainPolicy.begin(); it != d->javaScriptDomainPolicy.end(); ++it) {
//                    QByteArray javaPolicy = adviceToStr(KJavaScriptDunno);
//                    QByteArray javaScriptPolicy = adviceToStr(it.value());
//                    domainConfig.append(QString::fromLatin1("%1:%2:%3").arg(it.key()).arg(javaPolicy).arg(javaScriptPolicy));
//                }
//                cg.writeEntry("ECMADomainSettings", domainConfig);
//            }
//#endif
//        }
//    }
}

/** Local helper for retrieving per-domain settings.
  *
  * In case of doubt, the global domain is returned.
  */
static const KPerDomainSettings &lookup_hostname_policy(
    const QHTMLSettingsPrivate *const d,
    const QString &hostname)
{
#ifdef DEBUG_SETTINGS
    // qCDebug(KHTML_LOG) << "lookup_hostname_policy(" << hostname << ")";
#endif
    if (hostname.isEmpty()) {
#ifdef DEBUG_SETTINGS
        d->global.dump("global");
#endif
        return d->global;
    }

    const PolicyMap::const_iterator notfound = d->domainPolicy.constEnd();

    // First check whether there is a perfect match.
    PolicyMap::const_iterator it = d->domainPolicy.find(hostname);
    if (it != notfound) {
#ifdef DEBUG_SETTINGS
        // qCDebug(KHTML_LOG) << "perfect match";
        (*it).dump(hostname);
#endif
        // yes, use it (unless dunno)
        return *it;
    }

    // Now, check for partial match.  Chop host from the left until
    // there's no dots left.
    QString host_part = hostname;
    int dot_idx = -1;
    while ((dot_idx = host_part.indexOf(QChar('.'))) >= 0) {
        host_part.remove(0, dot_idx);
        it = d->domainPolicy.find(host_part);
        Q_ASSERT(notfound == d->domainPolicy.end());
        if (it != notfound) {
#ifdef DEBUG_SETTINGS
            // qCDebug(KHTML_LOG) << "partial match";
            (*it).dump(host_part);
#endif
            return *it;
        }
        // assert(host_part[0] == QChar('.'));
        host_part.remove(0, 1); // Chop off the dot.
    }

    // No domain-specific entry: use global domain
#ifdef DEBUG_SETTINGS
    // qCDebug(KHTML_LOG) << "no match";
    d->global.dump("global");
#endif
    return d->global;
}

bool QHTMLSettings::isOpenMiddleClickEnabled()
{
    return d->m_bOpenMiddleClick;
}

bool QHTMLSettings::isBackRightClickEnabled()
{
    return false; // ## the feature moved to konqueror
}

bool QHTMLSettings::accessKeysEnabled() const
{
    return d->m_accessKeysEnabled;
}

bool QHTMLSettings::isAdFilterEnabled() const
{
    return d->m_adFilterEnabled;
}

bool QHTMLSettings::isHideAdsEnabled() const
{
    return d->m_hideAdsEnabled;
}

bool QHTMLSettings::isAdFiltered(const QString &url) const
{
    if (d->m_adFilterEnabled) {
        if (!url.startsWith("data:")) {
            // Check the blacklist, and only if that matches, the whitelist
            return d->adBlackList.isUrlMatched(url) && !d->adWhiteList.isUrlMatched(url);
        }
    }
    return false;
}

QString QHTMLSettings::adFilteredBy(const QString &url, bool *isWhiteListed) const
{
    QString m = d->adWhiteList.urlMatchedBy(url);
    if (!m.isEmpty()) {
        if (isWhiteListed != nullptr) {
            *isWhiteListed = true;
        }
        return (m);
    }

    m = d->adBlackList.urlMatchedBy(url);
    if (!m.isEmpty()) {
        if (isWhiteListed != nullptr) {
            *isWhiteListed = false;
        }
        return (m);
    }

    return (QString());
}

void QHTMLSettings::addAdFilter(const QString &url)
{
//    KConfigGroup config = KSharedConfig::openConfig("khtmlrc", KConfig::NoGlobals)->group("Filter Settings");

//    QRegExp rx;

//    // Try compiling to avoid invalid stuff. Only support the basic syntax here...
//    // ### refactor somewhat
//    if (url.length() > 2 && url[0] == '/' && url[url.length() - 1] == '/') {
//        QString inside = url.mid(1, url.length() - 2);
//        rx.setPattern(inside);
//    } else {
//        rx.setPatternSyntax(QRegExp::Wildcard);
//        rx.setPattern(url);
//    }

//    if (rx.isValid()) {
//        int last = config.value("Count", 0);
//        QString key = "Filter-" + QString::number(last);
//        config.writeEntry(key, url);
//        config.writeEntry("Count", last + 1);
//        config.sync();
//        if (url.startsWith(QLatin1String("@@"))) {
//            d->adWhiteList.addFilter(url);
//        } else {
//            d->adBlackList.addFilter(url);
//        }
//    } else {
//        KMessageBox::error(nullptr,
//                           rx.errorString(),
//                           i18n("Filter error"));
//    }
}

bool QHTMLSettings::isJavaEnabled(const QString &hostname) const
{
    return lookup_hostname_policy(d, hostname.toLower()).m_bEnableJava;
}

bool QHTMLSettings::isJavaScriptEnabled(const QString &hostname) const
{
    return lookup_hostname_policy(d, hostname.toLower()).m_bEnableJavaScript;
}

bool QHTMLSettings::isJavaScriptDebugEnabled(const QString & /*hostname*/) const
{
    // debug setting is global for now, but could change in the future
    return d->m_bEnableJavaScriptDebug;
}

bool QHTMLSettings::isJavaScriptErrorReportingEnabled(const QString & /*hostname*/) const
{
    // error reporting setting is global for now, but could change in the future
    return d->m_bEnableJavaScriptErrorReporting;
}

bool QHTMLSettings::isPluginsEnabled(const QString &hostname) const
{
    return lookup_hostname_policy(d, hostname.toLower()).m_bEnablePlugins;
}

QHTMLSettings::KJSWindowOpenPolicy QHTMLSettings::windowOpenPolicy(
    const QString &hostname) const
{
    return lookup_hostname_policy(d, hostname.toLower()).m_windowOpenPolicy;
}

QHTMLSettings::KJSWindowMovePolicy QHTMLSettings::windowMovePolicy(
    const QString &hostname) const
{
    return lookup_hostname_policy(d, hostname.toLower()).m_windowMovePolicy;
}

QHTMLSettings::KJSWindowResizePolicy QHTMLSettings::windowResizePolicy(
    const QString &hostname) const
{
    return lookup_hostname_policy(d, hostname.toLower()).m_windowResizePolicy;
}

QHTMLSettings::KJSWindowStatusPolicy QHTMLSettings::windowStatusPolicy(
    const QString &hostname) const
{
    return lookup_hostname_policy(d, hostname.toLower()).m_windowStatusPolicy;
}

QHTMLSettings::KJSWindowFocusPolicy QHTMLSettings::windowFocusPolicy(
    const QString &hostname) const
{
    return lookup_hostname_policy(d, hostname.toLower()).m_windowFocusPolicy;
}

int QHTMLSettings::mediumFontSize() const
{
    return d->m_fontSize;
}

int QHTMLSettings::minFontSize() const
{
    return d->m_minFontSize;
}

QString QHTMLSettings::settingsToCSS() const
{
    // lets start with the link properties
    QString str = "a:link {\ncolor: ";
    str += d->m_linkColor.name();
    str += ';';
    if (d->m_underlineLink) {
        str += "\ntext-decoration: underline;";
    }

    if (d->m_bChangeCursor) {
        str += "\ncursor: pointer;";
        str += "\n}\ninput[type=image] { cursor: pointer;";
    }
    str += "\n}\n";
    str += "a:visited {\ncolor: ";
    str += d->m_vLinkColor.name();
    str += ';';
    if (d->m_underlineLink) {
        str += "\ntext-decoration: underline;";
    }

    if (d->m_bChangeCursor) {
        str += "\ncursor: pointer;";
    }
    str += "\n}\n";

    if (d->m_hoverLink) {
        str += "a:link:hover, a:visited:hover { text-decoration: underline; }\n";
    }

    return str;
}

const QString &QHTMLSettings::availableFamilies()
{
    if (!avFamilies) {
        avFamilies = new QString;
        QFontDatabase db;
        QStringList families = db.families();
        QStringList s;
        QRegExp foundryExp(" \\[.+\\]");

        //remove foundry info
        QStringList::Iterator f = families.begin();
        const QStringList::Iterator fEnd = families.end();

        for (; f != fEnd; ++f) {
            (*f).replace(foundryExp, "");
            if (!s.contains(*f)) {
                s << *f;
            }
        }
        s.sort();

        *avFamilies = ',' + s.join(",") + ',';
    }

    return *avFamilies;
}

QString QHTMLSettings::lookupFont(int i) const
{
    QString font;
    if (d->fonts.count() > i) {
        font = d->fonts[i];
    }
    if (font.isEmpty()) {
        font = d->defaultFonts[i];
    }
    return font;
}

QString QHTMLSettings::stdFontName() const
{
    return lookupFont(0);
}

QString QHTMLSettings::fixedFontName() const
{
    return lookupFont(1);
}

QString QHTMLSettings::serifFontName() const
{
    return lookupFont(2);
}

QString QHTMLSettings::sansSerifFontName() const
{
    return lookupFont(3);
}

QString QHTMLSettings::cursiveFontName() const
{
    return lookupFont(4);
}

QString QHTMLSettings::fantasyFontName() const
{
    return lookupFont(5);
}

void QHTMLSettings::setStdFontName(const QString &n)
{
    while (d->fonts.count() <= 0) {
        d->fonts.append(QString());
    }
    d->fonts[0] = n;
}

void QHTMLSettings::setFixedFontName(const QString &n)
{
    while (d->fonts.count() <= 1) {
        d->fonts.append(QString());
    }
    d->fonts[1] = n;
}

QString QHTMLSettings::userStyleSheet() const
{
    return d->m_userSheet;
}

bool QHTMLSettings::isFormCompletionEnabled() const
{
    return d->m_formCompletionEnabled;
}

int QHTMLSettings::maxFormCompletionItems() const
{
    return d->m_maxFormCompletionItems;
}

const QString &QHTMLSettings::encoding() const
{
    return d->m_encoding;
}

bool QHTMLSettings::followSystemColors() const
{
    return d->m_follow_system_colors;
}

const QColor &QHTMLSettings::textColor() const
{
    return d->m_textColor;
}

const QColor &QHTMLSettings::baseColor() const
{
    return d->m_baseColor;
}

const QColor &QHTMLSettings::linkColor() const
{
    return d->m_linkColor;
}

const QColor &QHTMLSettings::vLinkColor() const
{
    return d->m_vLinkColor;
}

bool QHTMLSettings::autoLoadImages() const
{
    return d->m_bAutoLoadImages;
}

bool QHTMLSettings::unfinishedImageFrame() const
{
    return d->m_bUnfinishedImageFrame;
}

QHTMLSettings::KAnimationAdvice QHTMLSettings::showAnimations() const
{
    return d->m_showAnimations;
}

QHTMLSettings::KSmoothScrollingMode QHTMLSettings::smoothScrolling() const
{
    return d->m_smoothScrolling;
}

QHTMLSettings::KDNSPrefetch QHTMLSettings::dnsPrefetch() const
{
    return d->m_dnsPrefetch;
}

bool QHTMLSettings::isAutoDelayedActionsEnabled() const
{
    return d->m_autoDelayedActionsEnabled;
}

bool QHTMLSettings::jsErrorsEnabled() const
{
    return d->m_jsErrorsEnabled;
}

void QHTMLSettings::setJSErrorsEnabled(bool enabled)
{
    d->m_jsErrorsEnabled = enabled;
    // save it
//    KConfigGroup cg(KSharedConfig::openConfig(), "HTML Settings");
//    cg.writeEntry("ReportJSErrors", enabled);
//    cg.sync();
}

bool QHTMLSettings::allowTabulation() const
{
    return d->m_allowTabulation;
}

bool QHTMLSettings::autoSpellCheck() const
{
    return d->m_autoSpellCheck;
}

QList< QPair< QString, QChar > > QHTMLSettings::fallbackAccessKeysAssignments() const
{
    return d->m_fallbackAccessKeysAssignments;
}

void QHTMLSettings::setJSPopupBlockerPassivePopup(bool enabled)
{
    d->m_jsPopupBlockerPassivePopup = enabled;
    // save it
//    KConfigGroup cg(KSharedConfig::openConfig(), "Java/JavaScript Settings");
//    cg.writeEntry("PopupBlockerPassivePopup", enabled);
//    cg.sync();
}

bool QHTMLSettings::jsPopupBlockerPassivePopup() const
{
    return d->m_jsPopupBlockerPassivePopup;
}

#include "qhtmlsettings.moc"
