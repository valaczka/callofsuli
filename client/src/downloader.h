/*
 * ---- Call of Suli ----
 *
 * downloader.h
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

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "httpconnection.h"
#include "server.h"
#include <QObject>

class Downloader : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Server *server READ server WRITE setServer NOTIFY serverChanged FINAL)
	Q_PROPERTY(qint64 fullSize READ fullSize WRITE setFullSize NOTIFY fullSizeChanged FINAL)
	Q_PROPERTY(qint64 downloadedSize READ downloadedSize WRITE setDownloadedSize NOTIFY downloadedSizeChanged FINAL)
	Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged FINAL)
	Q_PROPERTY(int downloadedCount READ downloadedCount WRITE setDownloadedCount NOTIFY downloadedCountChanged FINAL)

public:
	explicit Downloader(QObject *parent = nullptr);
	Downloader(Server *server, QObject *parent = nullptr);

	Q_INVOKABLE void download();

	Server *server() const;
	void setServer(Server *newServer);

	void contentClear();
	void contentAdd(const Server::DynamicContent &content);
	void contentRemove(const Server::DynamicContent &content);
	void contentRemove(const QString &name);
	void contentUpdate(const Server::DynamicContent &content, const qreal &progress);
	void contentUpdate(const QString &name, const qreal &progress);

	bool contains(const Server::DynamicContent &content) const;
	bool contains(const QString &name) const;

	qint64 fullSize() const;
	void setFullSize(qint64 newFullSize);

	qint64 downloadedSize() const;
	void setDownloadedSize(qint64 newDownloadedSize);

	int count() const;
	void setCount(int newCount);

	int downloadedCount() const;
	void setDownloadedCount(int newDownloadedCount);

signals:
	void contentDownloaded();
	void downloadError();

	void serverChanged();
	void fullSizeChanged();
	void downloadedSizeChanged();
	void countChanged();
	void downloadedCountChanged();

private:
	void check();
	void recalculate();

	Server *m_server = nullptr;
	qint64 m_fullSize = 0;
	qint64 m_downloadedSize = 0;
	int m_count = 0;
	int m_downloadedCount = 0;

	struct ReplyData {
		Server::DynamicContent content;
		QPointer<HttpReply> reply;
		qreal progress = 0.;
	};

	QVector<ReplyData> m_contentList;

	auto find(const Server::DynamicContent &content) {
		return std::find_if(m_contentList.begin(), m_contentList.end(), [content](const ReplyData &d) {
			return d.content == content;
		});
	}

	auto find(const Server::DynamicContent &content) const {
		return std::find_if(m_contentList.cbegin(), m_contentList.cend(), [content](const ReplyData &d) {
			return d.content == content;
		});
	}

	auto find(const QString &name) {
		return std::find_if(m_contentList.begin(), m_contentList.end(), [name](const ReplyData &d) {
			return d.content.name == name;
		});
	}

	auto find(const QString &name) const {
		return std::find_if(m_contentList.cbegin(), m_contentList.cend(), [name](const ReplyData &d) {
			return d.content.name == name;
		});
	}
};

#endif // DOWNLOADER_H
