#include "UpdateChecker.h"
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QVersionNumber>

namespace ClinicalCore {

UpdateChecker::UpdateChecker(const QString& currentVersion, QObject* parent)
    : QObject(parent), m_currentVersion(currentVersion)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this, &UpdateChecker::onReplyFinished);
}

void UpdateChecker::checkForUpdates()
{
    if (m_checking) return;
    m_checking = true;
    emit checkingChanged();

    QUrl url(QStringLiteral("https://api.github.com/repos/Travishulse/therapist-documentation-tool/releases/latest"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("TherapistDocumentationCompanion"));

    m_nam.get(request);
}

void UpdateChecker::onReplyFinished(QNetworkReply* reply)
{
    m_checking = false;
    emit checkingChanged();

    if (!reply || reply->error() != QNetworkReply::NoError) {
        if (reply) reply->deleteLater();
        emit updateChecked(false);
        return;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit updateChecked(false);
        return;
    }

    const QJsonObject obj = doc.object();
    QString tagName = obj.value(QStringLiteral("tag_name")).toString();
    if (tagName.startsWith(QLatin1Char('v')) || tagName.startsWith(QLatin1Char('V'))) {
        tagName = tagName.mid(1);
    }

    m_latestVersion = tagName;
    m_releaseNotes = obj.value(QStringLiteral("name")).toString();

    // Default browser fallback URL
    m_downloadUrl = obj.value(QStringLiteral("html_url")).toString();

    // Look for installer asset if available
    const QJsonArray assets = obj.value(QStringLiteral("assets")).toArray();
    for (const QJsonValue& val : assets) {
        const QJsonObject assetObj = val.toObject();
        const QString name = assetObj.value(QStringLiteral("name")).toString();
        if (name.endsWith(QStringLiteral("-Setup.exe"), Qt::CaseInsensitive) ||
            name.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive)) {
            m_downloadUrl = assetObj.value(QStringLiteral("browser_download_url")).toString();
            break;
        }
    }

    const QVersionNumber currentVer = QVersionNumber::fromString(m_currentVersion);
    const QVersionNumber latestVer = QVersionNumber::fromString(m_latestVersion);

    m_updateAvailable = (latestVer > currentVer);
    emit updateChecked(m_updateAvailable);
}

void UpdateChecker::openDownloadUrl()
{
    if (!m_downloadUrl.isEmpty()) {
        QDesktopServices::openUrl(QUrl(m_downloadUrl));
    }
}

} // namespace ClinicalCore

