#include "updatenotification.h"
#include "ui_updatenotification.h"
#include "utils.h"
#include <QtNetwork/QNetworkReply>

QString platform;

UpdateNotification::UpdateNotification(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpdateNotification)
{
    ui->setupUi(this);
    ui->downloadLinkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->downloadLinkLabel->setOpenExternalLinks(true);
    /* Add link here, not in designer, so our translators don't need to touch HTML */

    #ifdef Q_OS_MAC
    platform = QString("mac");
    #endif
    #ifdef Q_OS_WIN32 || #ifdef Q_OS_WIN
    platform = QString("windows");
    #endif
    #ifdef Q_OS_LINUX
    platform = QString("linux");
    #endif
    QString appendURL = "<a href=\"http://osmc.tv/download/" + platform + "\"><span style=\" text-decoration: underline; color:#f0f0f0;\">http://osmc.tv/download/" + platform + "</span></a></p></body></html>";
    ui->downloadLinkLabel->setText(QString(ui->downloadLinkLabel->text() + appendURL));
    #ifdef Q_OS_LINUX
    /*TODO: Only display if /usr/bin/apt-get exists */
    ui->platformtipLabel->setText(QString("You can also do this with \"apt-get upgrade\""));
    #endif
}

UpdateNotification::~UpdateNotification()
{
    delete ui;
}

void UpdateNotification::isUpdateAvailable()
{
    utils::writeLog("Checking for updates");
    int currentBuild = utils::getBuildNumber();
    QString buildURL = QString("http://download.osmc.tv/installers/latest_" + platform);
    utils::writeLog("Checking for updates by downloading " + buildURL);
    accessManager = new QNetworkAccessManager(this);
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    QNetworkRequest request(buildURL);
    accessManager->get(request);
}

void UpdateNotification::replyFinished(QNetworkReply *reply)
{
    QVariant mirrorRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    QUrl redirectURL = mirrorRedirectUrl.toUrl();
    if (!redirectURL.isEmpty())
    {
        utils::writeLog("Redirected to mirror file " + redirectURL.toString());
        QNetworkRequest request(redirectURL);
        this->accessManager->get(request);
    }
    else
    {
        utils::writeLog("Acquired mirror file");
        int latestBuild = QString::fromUtf8(reply->readAll()).toInt();
        if (utils::getBuildNumber() < latestBuild)
        {
            utils::writeLog("A new update is available");
            emit hasUpdate();
        }
        else
        {
            utils::writeLog("No new update is available");
        }
    }
    reply->deleteLater();
}
