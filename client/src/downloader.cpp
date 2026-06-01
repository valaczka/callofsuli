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
#include "utils_.h"

Downloader::Downloader(QObject *parent)
	: QObject{parent}
{

}




/**
 * @brief Downloader::~Downloader
 */

Downloader::~Downloader()
{
	unloadDynamicContents();
}




/**
 * @brief Downloader::download
 */

void Downloader::download()
{
	if (m_state == StateError) {
		emit downloadError();
		return;
	}


	if (m_contentList.isEmpty()) {
		LOG_CERROR("client") << "Downloader content empty";
		emit downloadError();
		setState(StateError);
		return;
	}


	if (m_contentDict.isEmpty()) {
		LOG_CERROR("client") << "Downloader content dictionary empty";
		emit downloadError();
		setState(StateError);
		return;
	}

	check();

	Client *client = Application::instance()->client();

	bool hasDownloadable = false;

	for (const ReplyData &data : m_contentList) {
		if (data.progress < 1.0 && !data.reply) {
			hasDownloadable = true;

			QString fname = data.content.file;

			HttpReply *r = client->httpConnection()->getUrl(client->rpgServerUrl(fname))
						   ->fail(this, [](const QString &err){
				Application::instance()->messageWarning(err, tr("Letöltési hiba"));
			})
						   ->done(this, [this, fname](const QByteArray &data){


				if (!dynamicContentSave(fname, data)) {
					Application::instance()->messageError(tr("Fájl mentése sikertelen: %1").arg(fname));
					contentUpdate(fname, 0.);
					emit downloadError();
					setState(StateError);
					return;
				}

				contentUpdate(fname, 1.);
				recalculate();

				if (m_downloadedCount >= m_count) {
					emit contentDownloaded();
					setState(StateContentReady);
				}
			});

			connect(r, &HttpReply::downloadProgress, this, [this, fname](const qreal &percent) {
				contentUpdate(fname, percent);
			});
		}
	}

	if (!hasDownloadable) {
		LOG_CDEBUG("client") << "All downloader content ready";
		emit contentDownloaded();
		setState(StateContentReady);
		return;
	}

}



/**
 * @brief Downloader::sharedContentDir
 * @return
 */

std::optional<QDir> Downloader::sharedContentDir()
{
	static const QString subdir = "shared";

#ifdef Q_OS_WASM
	QDir dir = QStringLiteral("/");
#else
	QDir dir = Utils::standardPath();

	if ((!dir.exists(subdir) && !dir.mkdir(subdir)) || !dir.cd(subdir)) {
		Application::instance()->messageError(tr("Belső hiba"));
		return std::nullopt;
	}
#endif

	return dir;
}



/**
 * @brief Downloader::fileChecksum
 * @param fileName
 * @param alg
 * @return
 */

std::optional<QByteArray> Downloader::fileChecksum(const QString &fileName, const QCryptographicHash::Algorithm &alg, qint64 *sizePtr)
{
	QCryptographicHash hash(alg);

	QFile f(fileName);

	if (f.open(QFile::ReadOnly)) {
		if (sizePtr)
			*sizePtr = f.size();

		QCryptographicHash hash(alg);
		if (hash.addData(&f)) {
			return hash.result();
		}
	}

	if (sizePtr)
		*sizePtr = 0;

	return std::nullopt;
}



/**
 * @brief Downloader::clear
 */

void Downloader::clear()
{
	contentClear();
	contentDictClear();
	setState(StateInvalid);
}


/**
 * @brief Downloader::contentClear
 */

void Downloader::contentClear()
{
	m_contentList.clear();
	setState(StateInvalid);
	recalculate();
}


/**
 * @brief Downloader::contentAdd
 * @param content
 */

void Downloader::contentAdd(const DynamicContent &content)
{
	if (content.file.isEmpty())
		return;

	if (!contains(content))
		m_contentList.append(ReplyData{content, nullptr, 0.});

	recalculate();
}



/**
 * @brief Downloader::contentAdd
 * @param contentList
 */

void Downloader::contentAdd(const QList<DynamicContent> &contentList)
{
	for (const DynamicContent &c : contentList)
		contentAdd(c);
}



/**
 * @brief Downloader::contentRemove
 * @param content
 */

void Downloader::contentRemove(const DynamicContent &content)
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

void Downloader::contentUpdate(const DynamicContent &content, const qreal &progress)
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

bool Downloader::contains(const DynamicContent &content) const
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



/**
 * @brief Downloader::state
 * @return
 */

Downloader::State Downloader::state() const
{
	return m_state;
}

void Downloader::setState(const State &newState)
{
	if (m_state == newState)
		return;
	m_state = newState;
	emit stateChanged();
}



/**
 * @brief Downloader::contentDict
 * @return
 */

const QHash<QString, QString> &Downloader::contentDict() const
{
	return m_contentDict;
}

void Downloader::setContentDict(const QHash<QString, QString> &newContentDict)
{
	m_contentDict = newContentDict;
}


/**
 * @brief Downloader::contentDictAdd
 * @param tsx
 * @param res
 */

void Downloader::contentDictAdd(const QString &tsx, const QString &res)
{
	if (!tsx.isEmpty() && !res.isEmpty())
		m_contentDict[QStringLiteral(":/").append(tsx)] = res;
}


/**
 * @brief Downloader::contentDictAdd
 * @param json
 */

void Downloader::contentDictAdd(const QJsonObject &json)
{
	for (const auto &[key, value] : json.asKeyValueRange()) {
		contentDictAdd(key.toString(), value.toString());
	}
}


/**
 * @brief Downloader::contentDictClear
 */

void Downloader::contentDictClear()
{
	m_contentDict.clear();
	setState(StateInvalid);
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

bool Downloader::check()
{
	QVector<DynamicContent> list;
	list.reserve(m_contentList.size());

	const auto &dir = sharedContentDir();

	if (!dir) {
		LOG_CERROR("client") << "Invalid shared content directory";
		setState(StateError);
		return false;
	}

	if (m_contentDict.isEmpty() || m_contentList.isEmpty())
		return false;


	bool hasAny = false;

	for (const ReplyData &d : m_contentList) {
		list.append(d.content);

		if (!hasAny && dir->exists(d.content.file))
			hasAny = true;
	}

	dynamicContentCheck(&list);

	for (ReplyData &data : m_contentList) {
		if (list.contains(data.content))
			data.progress = 0.;
		else
			data.progress = 1.;
	}

	recalculate();

	if (!list.isEmpty())
		setState(hasAny ? StateUpdateRequired : StateDownloadRequired);

	return list.isEmpty();
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















/**
 * @brief Downloader::dynamicContentCheck
 * @param listPtr
 * @return
 */

bool Downloader::dynamicContentCheck(QVector<DynamicContent> *listPtr)
{
	Q_ASSERT(listPtr);

	const auto &dir = sharedContentDir();

	if (!dir) {
		LOG_CERROR("client") << "Invalid shared content directory";
		setState(StateError);
		return false;
	}

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, dir, ret, listPtr]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		for (auto it = listPtr->begin(); it != listPtr->end(); ) {
			if (it->file.isEmpty()) {
				++it;
				continue;
			}

			const QString &filename = dir->absoluteFilePath(it->file);

			LOG_CTRACE("client") << "Check:" << filename;

			if (!QFile::exists(filename)) {
				++it;
				continue;
			}

			qint64 size = 0;
			const auto &hash = fileChecksum(filename, QCryptographicHash::Md5, &size);

			if (!hash) {
				++it;
				continue;
			}

			if (it->md5AsByteArray() == hash.value() && it->size == size) {
				LOG_CTRACE("client") << "Check success:" << qPrintable(filename);

				if (m_loadedContent.contains(filename)) {
					LOG_CTRACE("client") << "Content already loaded:" << filename;
					//emit loadableContentOneDownloaded(filename);
					it = listPtr->erase(it);
					continue;
				}

				//loadDynamicContent(filename);
				it = listPtr->erase(it);
			} else {
				++it;
			}
		}

#ifndef Q_OS_WASM
		ret.resolve();
	});

	QDefer::await(ret);
#endif

	return true;
}







/**
 * @brief Downloader::dynamicContentSaveAndLoad
 * @param name
 * @param data
 * @return
 */

bool Downloader::dynamicContentSave(const QString &name, const QByteArray &data)
{
	const auto &dir = sharedContentDir();

	if (!dir) {
		LOG_CERROR("client") << "Invalid shared content directory";
		setState(StateError);
		return false;
	}

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, dir, ret, name, data]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		const QString &filename = dir->absoluteFilePath(name);

		LOG_CINFO("client") << "Save dynamic content:" << qPrintable(filename);

		{
			QFile f(filename);
			if (!f.open(QIODevice::WriteOnly)) {
				LOG_CERROR("client") << "Save failed:" << qPrintable(filename);
#ifndef Q_OS_WASM
				ret.reject();
				return;
#else
				return false;
#endif
			}
			f.write(data);
			f.close();
		}

		///loadDynamicContent(filename);

#ifdef Q_OS_WASM
		return true;
#else
		ret.resolve();
	});

	QDefer::await(ret);
	return ret.state() == QDeferredState::RESOLVED;
#endif

}







/**
 * @brief Downloader::unloadDynamicContents
 */

void Downloader::unloadDynamicContents()
{
	LOG_CTRACE("client") << "Unload dynamic contents";

	for (const QString &s : m_loadedContent) {
		if (!QResource::unregisterResource(s)) {
			LOG_CERROR("client") << "Unregister resource failed:" << qPrintable(s);
		} else {
			LOG_CTRACE("client") << "Unload dynamic content:" << qPrintable(s);
		}
	}

	m_loadedContent.clear();


}





/**
 * @brief Downloader::loadDynamicContent
 */

void Downloader::loadDynamicContent()
{
	for (const ReplyData &d : m_contentList) {
		if (d.content.file.endsWith(QStringLiteral(".cres")))
			loadDynamicContent(d.content.file);
	}
}








/**
 * @brief Downloader::loadDynamicContent
 * @param filename
 */

void Downloader::loadDynamicContent(const QString &filename)
{
	const auto dir = sharedContentDir();

	if (!dir) {
		LOG_CERROR("client") << "Invalid shared content directory";
		setState(StateError);
		return;
	}

	const QString full = dir->absoluteFilePath(filename);

	LOG_CDEBUG("client") << "Load dynamic content:" << qPrintable(full);

	if (m_loadedContent.contains(full)) {
		LOG_CTRACE("client") << "Content already loaded" << qPrintable(full);
		return;
	}

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, full, ret]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		if (!QResource::registerResource(full)) {
			LOG_CERROR("client") << "Register resource failed:" << qPrintable(full);
		} else {
			m_loadedContent.insert(full);
		}

#ifndef Q_OS_WASM
		ret.resolve();
	});

	QDefer::await(ret);
#endif
}





