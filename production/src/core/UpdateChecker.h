#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace ClinicalCore {

class UpdateChecker : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updateChecked)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY updateChecked)
    Q_PROPERTY(QString downloadUrl READ downloadUrl NOTIFY updateChecked)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY updateChecked)
    Q_PROPERTY(bool checking READ checking NOTIFY checkingChanged)

public:
    explicit UpdateChecker(const QString& currentVersion, QObject* parent = nullptr);

    bool updateAvailable() const { return m_updateAvailable; }
    QString latestVersion() const { return m_latestVersion; }
    QString downloadUrl() const { return m_downloadUrl; }
    QString releaseNotes() const { return m_releaseNotes; }
    bool checking() const { return m_checking; }

    Q_INVOKABLE void checkForUpdates();
    Q_INVOKABLE void openDownloadUrl();

signals:
    void updateChecked(bool available);
    void checkingChanged();

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QString m_currentVersion;
    bool m_updateAvailable{false};
    bool m_checking{false};
    QString m_latestVersion;
    QString m_downloadUrl;
    QString m_releaseNotes;
    QNetworkAccessManager m_nam;
};

} // namespace ClinicalCore

