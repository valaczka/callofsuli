/*
 * ---- Call of Suli ----
 *
 * cosdownloader.cpp
 *
 * Created on: 2020. 12. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CosDownloader
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "cosdownloader.h"
#include "abstractactivity.h"

CosDownloader::CosDownloader(AbstractActivity *activity,
							 const CosMessage::CosClass &cosClass,
							 const QString &func,
							 QObject *parent)
	: QObject(parent)
	, m_activity(activity)
	, m_list()
	, m_class(cosClass)
	, m_func(func)
	, m_jsonKeyFileName("filename")
	, m_jsonKeyMd5("md5")
	, m_running(false)
{
	Q_ASSERT(activity);

	connect(Client::clientInstance(), &Client::messageReceived, this, &CosDownloader::onMessageReceived);
	connect(Client::clientInstance(), &Client::messageFrameReceived, this, &CosDownloader::onMessageFrameReceived);

	connect(this, &CosDownloader::downloadFailed, this, [=](){ m_running = false; });
	connect(this, &CosDownloader::downloadFinished, this, [=](){ m_running = false; });

	//QWebSocket *socket = client->socket();
	//connect(socket, &QWebSocket::disconnected, this, &CosDownloader::onSocketDisconnected);
}


/**
 * @brief CosDownloader::~CosDownloader
 */

CosDownloader::~CosDownloader()
{

}


/**
 * @brief CosDownloader::append
 * @param item
 */

void CosDownloader::append(const CosDownloaderItem &item)
{
	m_list.append(item);
	emit listChanged(m_list);
	emit countChanged(count());
	emit fullSizeChanged(fullSize());
	emit downloadedSizeChanged(downloadedSize());
	setActiveDownload("");
}



/**
 * @brief CosDownloader::append
 * @param remote
 * @param local
 * @param downloaded
 * @param prg
 */

void CosDownloader::append(const QString &remote, const QString &local, const int &size, const QString &md5, const bool &downloaded, const qreal &progress)
{
	append(CosDownloaderItem(remote, local, size, md5, downloaded, progress));
}


/**
 * @brief CosDownloader::append
 * @param map
 */

void CosDownloader::append(const QVariantMap &map)
{
	append(CosDownloaderItem(
			   map.value("remoteFile").toString(),
			   map.value("localFile").toString(),
			   map.value("size", 0).toInt(),
			   map.value("md5", "").toString(),
			   map.value("downloaded", false).toBool(),
			   map.value("progress", 0.0).toReal()
			   ));
}


/**
 * @brief CosDownloader::append
 * @param list
 */

void CosDownloader::append(const QVariantList &list)
{
	foreach (QVariant v, list)
		append(v.toMap());
}



/**
 * @brief CosDownloader::clear
 */

void CosDownloader::clear()
{
	m_list.clear();
	emit listChanged(m_list);
	emit countChanged(count());
	emit fullSizeChanged(fullSize());
	emit downloadedSizeChanged(downloadedSize());
	setActiveDownload("");
}


/**
 * @brief CosDownloader::get
 * @param remote
 * @return
 */

int CosDownloader::get(const QString &remote)
{
	for (int i=0; i<m_list.size(); ++i) {
		if (m_list.at(i).remoteFile == remote)
			return i;
	}

	return -1;
}



/**
 * @brief CosDownloader::fullSize
 * @return
 */

int CosDownloader::fullSize() const
{
	int size = 0;

	foreach(CosDownloaderItem item, m_list) {
		if (!item.isDownloaded)
			size += item.size;
	}

	return size;
}


/**
 * @brief CosDownloader::downloadedSize
 * @return
 */

int CosDownloader::downloadedSize() const
{
	int size = 0;

	foreach(CosDownloaderItem item, m_list) {
		if (item.isDownloaded)
			size += item.size;
		else
			size += item.size*std::min(item.progress, 1.0);
	}

	return size;
}


/**
 * @brief CosDownloader::hasDownloadable
 * @return
 */

bool CosDownloader::hasDownloadable() const
{
	foreach(CosDownloaderItem item, m_list)
		if (!item.isDownloaded)
			return true;

	return false;
}


/**
 * @brief CosDownloader::downloadProgress
 * @return
 */

qreal CosDownloader::downloadProgress() const
{
	qreal f = fullSize();
	qreal d = downloadedSize();
	if (f>0) {
		return d/f;
	}

	return 0;
}




/**
 * @brief CosDownloader::start
 */

void CosDownloader::start()
{
	if (m_running) {
		qWarning() << tr("Already running!") << this;
		return;
	}

	m_running = true;

	qDebug() << "DOWNLOADER" << this << "START";
	if (!hasDownloadable()) {
		qDebug() << "DOWNLOADER" << this << "FINISHED";
		emit downloadFinished();
		return;
	}

	emit downloadStarted();

	downloadNext();
}


/**
 * @brief CosDownloader::abort
 */

void CosDownloader::abort()
{
	emit downloadFailed();
	Client::clientInstance()->socket()->abort();
}



/**
 * @brief CosDownloader::setActiveDownload
 * @param activeDownload
 */

void CosDownloader::setActiveDownload(QString activeDownload)
{
	if (m_activeDownload == activeDownload)
		return;

	m_activeDownload = activeDownload;
	emit activeDownloadChanged(m_activeDownload);
}


/**
 * @brief CosDownloader::onMessageFrameReceived
 * @param message
 */

void CosDownloader::onMessageFrameReceived(const CosMessage &message)
{
	QString filename = message.jsonData().value(m_jsonKeyFileName).toString();

	if (filename.isEmpty())
		return;

	int idx = get(filename);

	if (idx == -1)
		return;

	setActiveDownload(filename);

	CosDownloaderItem &item = m_list[idx];

	qreal ratio = message.receivedDataRatio();
	if (ratio >= 1.0)
		ratio = 0.999999999;

	item.progress = ratio;

	emit downloadedSizeChanged(downloadedSize());
	emit downloadProgressChanged(downloadProgress());
}


/**
 * @brief CosDownloader::onMessageReceived
 * @param message
 */

void CosDownloader::onMessageReceived(const CosMessage &message)
{
	QString func = message.cosFunc();
	QJsonObject d = message.jsonData();

	if (message.cosClass() == CosMessage::ClassUserInfo && func == m_func) {
		QString filename = message.jsonData().value(m_jsonKeyFileName).toString();
		QString md5 = message.jsonData().value(m_jsonKeyMd5).toString();

		if (message.messageType() != CosMessage::MessageBinaryData) {
			Client::clientInstance()->sendMessageError(tr("Letöltés"), tr("Érvénytelen adat érkezett"));
			emit downloadFailed();
			return;
		}

		if (filename.isEmpty()) {
			Client::clientInstance()->sendMessageError(tr("Letöltés"), tr("Érvénytelen fájl érkezett"));
			emit downloadFailed();
			return;
		}

		QByteArray data = message.binaryData();
		QString localmd5 = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

		if (!md5.isEmpty() && localmd5 != md5) {
			Client::clientInstance()->sendMessageError(tr("Letöltés"), tr("Hibás adat érkezett"));
			emit downloadFailed();
			return;
		}

		int idx = get(filename);

		if (idx == -1) {
			Client::clientInstance()->sendMessageError(tr("Letöltés"), tr("Váratlan fájl érkezett: ")+filename);
			emit downloadFailed();
			return;
		}

		CosDownloaderItem &item = m_list[idx];


		if (!md5.isEmpty() && !item.md5.isEmpty() && md5 != item.md5) {
			Client::clientInstance()->sendMessageError(tr("Letöltés"), tr("Hibás adat érkezett"));
			emit downloadFailed();
			return;
		}

		item.progress = 1.0;
		item.isDownloaded = true;

		if (!item.localFile.isEmpty()) {
			QString newFile = item.localFile;
			QFile f(newFile);

			if (!f.open(QIODevice::WriteOnly)) {
				Client::clientInstance()->sendMessageError(tr("Fájl létrehozási hiba"), f.errorString(), newFile);
				emit downloadFailed();
				return;
			}

			if (f.write(data) == -1) {
				Client::clientInstance()->sendMessageError(tr("Fájl írási hiba"), f.errorString(), newFile);
				f.close();
				emit downloadFailed();
				return;
			}

			f.close();

			qDebug() << tr("Fájl létrehozva") << newFile;
		}

		qDebug() << "DOWNLOADER" << this << "RECEIVED" << item.localFile;

		emit oneDownloadFinished(item, data, d);
		emit downloadProgressChanged(downloadProgress());
		emit downloadedSizeChanged(downloadedSize());

		if (hasDownloadable())
			downloadNext();
		else
			emit downloadFinished();

	}
}


/**
 * @brief CosDownloader::downloadNext
 */

void CosDownloader::downloadNext()
{
	qDebug() << "DOWNLOADER" << this << "NEXT";
	for (int i=0; i<m_list.size(); ++i) {
		CosDownloaderItem &item = m_list[i];

		if (!item.isDownloaded) {
			qDebug() << "Download" << item.localFile;

			item.progress = 0.0;

			QJsonObject o;
			o[m_jsonKeyFileName] = item.remoteFile;
			m_activity->send(m_class, m_func, o);
			break;
		}
	}
}











