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
#include <QObject>
#include <QSerializer>

#ifndef Q_OS_WASM
#include "qlambdathreadworker.h"
#endif



/**
 * @brief The DynamicContent class
 */

class DynamicContent : public QSerializer
{
	Q_GADGET

public:
	DynamicContent()
		: QSerializer()
		, size(0)
	{}


	bool operator== (const DynamicContent &other) const {
		return other.file == file &&
				other.md5 == md5 &&
				other.size == size;
	}

	QByteArray md5AsByteArray() const { return QByteArray::fromHex(md5.toLatin1()); }

	QS_SERIALIZABLE

	QS_FIELD(QString, file)
	QS_FIELD(QString, md5)
	QS_FIELD(qint64, size)
};



/**
 * @brief The DynamicContentList class
 */


class DynamicContentList : public QSerializer
{
	Q_GADGET

public:
	DynamicContentList() : QSerializer() {}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, DynamicContent, list)
};





/**
 * @brief The Downloader class
 */

class Downloader : public QObject
{
	Q_OBJECT

	Q_PROPERTY(qint64 fullSize READ fullSize WRITE setFullSize NOTIFY fullSizeChanged FINAL)
	Q_PROPERTY(qint64 downloadedSize READ downloadedSize WRITE setDownloadedSize NOTIFY downloadedSizeChanged FINAL)
	Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged FINAL)
	Q_PROPERTY(int downloadedCount READ downloadedCount WRITE setDownloadedCount NOTIFY downloadedCountChanged FINAL)

public:
	explicit Downloader(QObject *parent = nullptr);
	virtual ~Downloader();

	enum State {
		StateInvalid = 0,
		StateDownloadRequired,
		StateUpdateRequired,
		StateContentReady,
		StateError
	};

	Q_ENUM(State);

	Q_INVOKABLE bool check();
	Q_INVOKABLE void download();

	static std::optional<QDir> sharedContentDir();
	static std::optional<QByteArray> fileChecksum(const QString &fileName, const QCryptographicHash::Algorithm &alg,
												  qint64 *sizePtr = nullptr);

	void clear();

	void contentClear();
	void contentAdd(const DynamicContent &content);
	void contentAdd(const QList<DynamicContent> &contentList);
	void contentRemove(const DynamicContent &content);
	void contentRemove(const QString &name);
	void contentUpdate(const DynamicContent &content, const qreal &progress);
	void contentUpdate(const QString &name, const qreal &progress);

	bool contains(const DynamicContent &content) const;
	bool contains(const QString &name) const;

	bool dynamicContentCheck(QVector<DynamicContent> *listPtr);
	void unloadDynamicContents();

	void loadDynamicContent();
	void loadDynamicContent(const QString &filename);
	bool dynamicContentSave(const QString &name, const QByteArray &data);

	qint64 fullSize() const;
	void setFullSize(qint64 newFullSize);

	qint64 downloadedSize() const;
	void setDownloadedSize(qint64 newDownloadedSize);

	int count() const;
	void setCount(int newCount);

	int downloadedCount() const;
	void setDownloadedCount(int newDownloadedCount);

	const QHash<QString, QString> &contentDict() const;
	void setContentDict(const QHash<QString, QString> &newContentDict);
	void contentDictAdd(const QString &tsx, const QString &res);
	void contentDictAdd(const QJsonObject &json);
	void contentDictClear();

	State state() const;

signals:
	void contentDownloaded();
	void downloadError();

	void fullSizeChanged();
	void downloadedSizeChanged();
	void countChanged();
	void downloadedCountChanged();
	void stateChanged();

private:
	void recalculate();
	void setState(const State &newState);

	State m_state = StateInvalid;

	qint64 m_fullSize = 0;
	qint64 m_downloadedSize = 0;
	int m_count = 0;
	int m_downloadedCount = 0;

	struct ReplyData {
		DynamicContent content;
		QPointer<HttpReply> reply;
		qreal progress = 0.;
	};

	QVector<ReplyData> m_contentList;
	QSet<QString> m_loadedContent;

	QHash<QString, QString> m_contentDict;

	// Dynamic content

#ifndef Q_OS_WASM
	QLambdaThreadWorker m_worker;
	QRecursiveMutex m_mutex;
#endif


	auto find(const DynamicContent &content) {
		return std::find_if(m_contentList.begin(), m_contentList.end(), [content](const ReplyData &d) {
			return d.content == content;
		});
	}

	auto find(const DynamicContent &content) const {
		return std::find_if(m_contentList.cbegin(), m_contentList.cend(), [content](const ReplyData &d) {
			return d.content == content;
		});
	}

	auto find(const QString &name) {
		return std::find_if(m_contentList.begin(), m_contentList.end(), [name](const ReplyData &d) {
			return d.content.file == name;
		});
	}

	auto find(const QString &name) const {
		return std::find_if(m_contentList.cbegin(), m_contentList.cend(), [name](const ReplyData &d) {
			return d.content.file == name;
		});
	}
	Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged FINAL)
};

#endif // DOWNLOADER_H
