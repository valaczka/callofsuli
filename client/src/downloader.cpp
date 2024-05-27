/*
 * ---- Call of Suli ----
 *
 * downloader.cpp
 *
 * Created on: 2024. 05. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Downloader
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

#include "downloader.h"
#include "application.h"

Downloader::Downloader(QObject *parent)
	: QObject{parent}
{

}

Downloader::Downloader(Server *server, QObject *parent)
	: QObject(parent)
	, m_server(server)
{

}




/**
 * @brief Downloader::download
 */

void Downloader::download()
{
	check();

	if (m_contentList.isEmpty()) {
		LOG_CDEBUG("client") << "Downloader content empty";
		emit contentDownloaded();
		return;
	}

	Client *client = Application::instance()->client();

	bool hasDownloadable = false;

	for (const ReplyData &data : m_contentList) {
		if (data.progress < 1.0 && !data.reply) {
			hasDownloadable = true;

			QString fname = data.content.name;

			HttpReply *r = client->httpConnection()->get(QStringLiteral("/content/")+fname)
						   ->fail(this, [](const QString &err){
				Application::instance()->messageWarning(err, tr("Letöltési hiba"));
			})
						   ->done(this, [this, fname](const QByteArray &data){
				if (!m_server) {
					LOG_CERROR("client") << "Missing server";
					Application::instance()->messageError(tr("Fájl mentése sikertelen: %1").arg(fname));
					contentUpdate(fname, 0.);
					emit downloadError();
					return;
				}


				if (!m_server->dynamicContentSaveAndLoad(fname, data)) {
					Application::instance()->messageError(tr("Fájl mentése sikertelen: %1").arg(fname));
					contentUpdate(fname, 0.);
					emit downloadError();
					return;
				}

				contentUpdate(fname, 1.);
				recalculate();

				if (m_downloadedCount >= m_count)
					emit contentDownloaded();
			});

			connect(r, &HttpReply::downloadProgress, this, [this, fname](const qreal &percent) {
				contentUpdate(fname, percent);
			});
		}
	}

	if (!hasDownloadable) {
		LOG_CDEBUG("client") << "All downloader content ready";
		emit contentDownloaded();
		return;
	}

}


/**
 * @brief Downloader::server
 * @return
 */

Server *Downloader::server() const
{
	return m_server;
}

void Downloader::setServer(Server *newServer)
{
	if (m_server == newServer)
		return;
	m_server = newServer;
	emit serverChanged();
}


/**
 * @brief Downloader::contentClear
 */

void Downloader::contentClear()
{
	m_contentList.clear();
	recalculate();
}


/**
 * @brief Downloader::contentAdd
 * @param content
 */

void Downloader::contentAdd(const Server::DynamicContent &content)
{
	if (!contains(content))
		m_contentList.append(ReplyData{content, nullptr, 0.});
	recalculate();
}


/**
 * @brief Downloader::contentRemove
 * @param content
 */

void Downloader::contentRemove(const Server::DynamicContent &content)
{
	auto it = find(content);

	if (it != m_contentList.cend()) {
		m_contentList.erase(it);
		recalculate();
	}
}


/**
 * @brief Downloader::contentRemove
 * @param name
 */

void Downloader::contentRemove(const QString &name)
{
	auto it = find(name);

	if (it != m_contentList.cend()) {
		m_contentList.erase(it);
		recalculate();
	}
	recalculate();
}


/**
 * @brief Downloader::contentUpdate
 * @param content
 * @param progress
 */

void Downloader::contentUpdate(const Server::DynamicContent &content, const qreal &progress)
{
	auto it = find(content);

	if (it == m_contentList.end())
		return;

	it->progress = progress;

	recalculate();
}


/**
 * @brief Downloader::contentUpdate
 * @param name
 * @param progress
 */

void Downloader::contentUpdate(const QString &name, const qreal &progress)
{
	auto it = find(name);

	if (it == m_contentList.end())
		return;

	it->progress = progress;

	recalculate();
}



/**
 * @brief Downloader::contains
 * @param content
 * @return
 */

bool Downloader::contains(const Server::DynamicContent &content) const
{
	return find(content) != m_contentList.constEnd();
}


/**
 * @brief Downloader::contains
 * @param name
 * @return
 */

bool Downloader::contains(const QString &name) const
{
	return find(name) != m_contentList.constEnd();
}



/**
 * @brief Downloader::recalculate
 */

void Downloader::recalculate()
{
	qint64 fullSize = 0;
	qint64 downloadedSize = 0;
	int downloadedCount = 0;

	for (auto it = m_contentList.constBegin(); it != m_contentList.constEnd(); ++it) {
		const auto &s = it->content.size;
		fullSize += s;
		downloadedSize += it->progress * s;

		if (it->progress >= 1.)
			++downloadedCount;
	}

	setFullSize(fullSize);
	setCount(m_contentList.size());
	setDownloadedSize(downloadedSize);
	setDownloadedCount(downloadedCount);
}


int Downloader::downloadedCount() const
{
	return m_downloadedCount;
}

void Downloader::setDownloadedCount(int newDownloadedCount)
{
	if (m_downloadedCount == newDownloadedCount)
		return;
	m_downloadedCount = newDownloadedCount;
	emit downloadedCountChanged();
}


/**
 * @brief Downloader::check
 */

void Downloader::check()
{
	if (!m_server) {
		LOG_CERROR("client") << "Missing server";
		return;
	}

	QVector<Server::DynamicContent> list;

	for (const ReplyData &d : m_contentList) {
		list.append(d.content);
	}

	m_server->dynamicContentCheck(&list);

	for (ReplyData &data : m_contentList) {
		if (list.contains(data.content))
			data.progress = 0.;
		else
			data.progress = 1.;
	}

	recalculate();
}


/**
 * @brief Downloader::count
 * @return
 */

int Downloader::count() const
{
	return m_count;
}

void Downloader::setCount(int newCount)
{
	if (m_count == newCount)
		return;
	m_count = newCount;
	emit countChanged();
}

qint64 Downloader::downloadedSize() const
{
	return m_downloadedSize;
}

void Downloader::setDownloadedSize(qint64 newDownloadedSize)
{
	if (m_downloadedSize == newDownloadedSize)
		return;
	m_downloadedSize = newDownloadedSize;
	emit downloadedSizeChanged();
}

qint64 Downloader::fullSize() const
{
	return m_fullSize;
}

void Downloader::setFullSize(qint64 newFullSize)
{
	if (m_fullSize == newFullSize)
		return;
	m_fullSize = newFullSize;
	emit fullSizeChanged();
}
