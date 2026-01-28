/*
 * ---- Call of Suli ----
 *
 * server.cpp
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Server
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

#include "server.h"
#include "Logger.h"
#include "application.h"
#include "qjsonarray.h"
#include "qjsonobject.h"
#include "utils_.h"
#include "rpggame.h"

Server::Server(QObject *parent)
	: SelectableObject{parent}
	, m_user(new User())
{


}


Server::~Server()
{

}


/**
 * @brief Server::fromJson
 * @param data
 * @param parent
 * @return
 */

Server *Server::fromJson(const QJsonObject &data, QObject *parent)
{
	if (data.isEmpty()) {
		LOG_CTRACE("client") << "Empty JsonObject";
		return nullptr;
	}

	Server *s = new Server(parent);
	//s->setDirectory(realname.section('/', 0, -2));
	s->setUrl(data.value(QStringLiteral("url")).toString());
	s->setAutoConnect(data.value(QStringLiteral("autoConnect")).toVariant().toBool());
	s->user()->setUsername(data.value(QStringLiteral("username")).toString());
	s->setToken(data.value(QStringLiteral("token")).toString());
	s->setCertificate(data.value(QStringLiteral("certificate")).toString().toUtf8());
	s->setServerName(data.value(QStringLiteral("serverName")).toString());

#ifndef QT_NO_SSL
	const QJsonArray &list = data.value(QStringLiteral("ignoredSslErrors")).toArray();

	QList<QSslError::SslError> errList;

	for (const QJsonValue &v : list) {
		QSslError::SslError e = v.toVariant().value<QSslError::SslError>();
		if (!errList.contains(e))
			errList.append(e);
	}

	s->setIgnoredSslErrors(errList);
#endif

	return s;
}



/**
 * @brief Server::toJson
 * @return
 */

QJsonObject Server::toJson() const
{
	QJsonObject o;

	o[QStringLiteral("url")] = m_url.toString();
	o[QStringLiteral("autoConnect")] = m_autoConnect;
	o[QStringLiteral("username")] = m_user->username();
	o[QStringLiteral("serverName")] = m_serverName;
	o[QStringLiteral("token")] = m_token;
	o[QStringLiteral("certificate")] = QString::fromUtf8(m_certificate);

#ifndef QT_NO_SSL
	QJsonArray list;

	for (const QSslError::SslError &e : m_ignoredSslErrors)
		list.append(e);

	o[QStringLiteral("ignoredSslErrors")] = list;
#endif

	return o;
}



const QUrl &Server::url() const
{
	return m_url;
}

void Server::setUrl(const QUrl &newUrl)
{
	if (m_url == newUrl)
		return;
	m_url = newUrl;
	emit urlChanged();
}

const QDir &Server::directory() const
{
	return m_directory;
}

void Server::setDirectory(const QDir &newDirectory)
{
	if (m_directory == newDirectory)
		return;
	m_directory = newDirectory;
	emit directoryChanged();
}

bool Server::autoConnect() const
{
	return m_autoConnect;
}

void Server::setAutoConnect(bool newAutoConnect)
{
	if (m_autoConnect == newAutoConnect)
		return;
	m_autoConnect = newAutoConnect;
	emit autoConnectChanged();
}


const QString &Server::token() const
{
	return m_token;
}

void Server::setToken(const QString &newToken)
{
	if (m_token == newToken)
		return;
	m_token = newToken;
	emit tokenChanged();
}

const QByteArray &Server::certificate() const
{
	return m_certificate;
}

void Server::setCertificate(const QByteArray &newCertificate)
{
	if (m_certificate == newCertificate)
		return;
	m_certificate = newCertificate;
	emit certificateChanged();
}


#ifndef QT_NO_SSL

const QList<QSslError::SslError> &Server::ignoredSslErrors() const
{
	return m_ignoredSslErrors;
}

void Server::setIgnoredSslErrors(const QList<QSslError::SslError> &newIgnoredSslErrors)
{
	if (m_ignoredSslErrors == newIgnoredSslErrors)
		return;
	m_ignoredSslErrors = newIgnoredSslErrors;
	emit ignoredSslErrorsChanged();
}

#endif

const QString &Server::name() const
{
	return m_name;
}

void Server::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

const QString &Server::serverName() const
{
	return m_serverName;
}

void Server::setServerName(const QString &newServerName)
{
	if (m_serverName == newServerName)
		return;
	m_serverName = newServerName;
	emit serverNameChanged();
}




const QJsonObject &Server::config() const
{
	return m_config;
}

void Server::setConfig(const QJsonObject &newConfig)
{
	if (m_config == newConfig)
		return;
	m_config = newConfig;
	emit configChanged();
}

User *Server::user() const
{
	return m_user.get();
}



void Server::setRankList(const RankList &newRankList)
{
	if (m_rankList == newRankList)
		return;
	m_rankList = newRankList;
	emit rankListChanged();
}


/**
 * @brief Server::isTokenValid
 * @param token
 * @return
 */

bool Server::isTokenValid(const QString &jwt)
{
	Token token(jwt.toUtf8());

	if (token.payload().isEmpty()) {
		LOG_CWARNING("client") << "Invalid token:" << jwt;
		return false;
	}

	if (token.payload().value(QStringLiteral("exp")).toInteger() <= QDateTime::currentSecsSinceEpoch()) {
		LOG_CWARNING("client") << "Expired token:" << jwt;
		return false;
	}

	return true;
}


/**
 * @brief Server::rank
 * @param id
 * @return
 */

Rank Server::rank(const int &id) const
{
	if (const auto it = std::find_if(m_rankList.cbegin(), m_rankList.cend(), [&id](const Rank &r){
									 return r.id() == id;
}); it != m_rankList.cend())
		return *it;

	return Rank();
}



/**
 * @brief Server::temporary
 * @return
 */

bool Server::temporary() const
{
	return m_temporary;
}

void Server::setTemporary(bool newTemporary)
{
	if (m_temporary == newTemporary)
		return;
	m_temporary = newTemporary;
	emit temporaryChanged();
}

int Server::maxUploadSize() const
{
	return m_maxUploadSize;
}

void Server::setMaxUploadSize(int newMaxUploadSize)
{
	if (m_maxUploadSize == newMaxUploadSize)
		return;
	m_maxUploadSize = newMaxUploadSize;
	emit maxUploadSizeChanged();
}




/**
 * @brief Server::dynamicContentCheck
 * @return
 */

bool Server::dynamicContentCheck(QVector<DynamicContent> *listPtr)
{
	Q_ASSERT(listPtr);

	const auto &dir = getContentDir();

	if (!dir)
		return false;

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, dir, ret, listPtr]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		for (auto it = listPtr->begin(); it != listPtr->end(); ) {
			if (it->name.isEmpty()) {
				++it;
				continue;
			}

			const QString &filename = dir->absoluteFilePath(it->name);

			LOG_CTRACE("client") << "Check:" << filename;

			if (!QFile::exists(filename)) {
				++it;
				continue;
			}

			const auto &content = Utils::fileContent(filename);

			if (!content) {
				++it;
				continue;
			}

			if (it->md5 == QString::fromLatin1(QCryptographicHash::hash(*content, QCryptographicHash::Md5).toHex()) &&
					it->size == content->size()) {
				LOG_CTRACE("client") << "Check success:" << qPrintable(filename);

				if (m_loadedContentList.contains(filename)) {
					LOG_CTRACE("client") << "Content already loaded:" << filename;
					//emit loadableContentOneDownloaded(filename);
					it = listPtr->erase(it);
					continue;
				}

				loadDynamicContent(filename);
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
 * @brief Server::dynamicContentRemove
 * @param listPtr
 * @param name
 * @param data
 * @return
 */

bool Server::dynamicContentRemove(QVector<DynamicContent> *listPtr, const QString &name, const QByteArray &data)
{
	Q_ASSERT(listPtr);

	const QString &md5 = QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());
	const qint64 size = data.size();
	bool found = false;

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, name, md5, size, &found, ret, listPtr]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		for (auto it = listPtr->begin(); it != listPtr->end(); ) {
			if (it->name == name && it->md5 == md5 && it->size == size) {
				it = listPtr->erase(it);
				found = true;
			} else {
				++it;
			}
		}

#ifndef Q_OS_WASM
		ret.resolve();
	});

	QDefer::await(ret);
#endif

	return found;
}



/**
 * @brief Server::dynamicContentSaveAndLoad
 * @param name
 * @param data
 * @return
 */

bool Server::dynamicContentSaveAndLoad(const QString &name, const QByteArray &data)
{
	const auto &dir = getContentDir();

	if (!dir)
		return false;

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

		loadDynamicContent(filename);

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
 * @brief Server::dynamicContentUnload
 * @param name
 * @return
 */

bool Server::dynamicContentUnload(const QString &name)
{
	const auto &dir = getContentDir();

	if (!dir)
		return false;

	const QString &filename = dir->absoluteFilePath(name);

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, ret, filename]() mutable {
		QMutexLocker locker(&m_mutex);
#endif

		if (!m_loadedContentList.contains(filename)) {
			LOG_CERROR("client") << "Unload failed:" << qPrintable(filename);
#ifndef Q_OS_WASM
			ret.reject();
			return;
#else
			return false;
#endif
		}

		if (!QResource::unregisterResource(filename)) {
			LOG_CERROR("client") << "Unregister resource failed:" << qPrintable(filename);
#ifndef Q_OS_WASM
			ret.reject();
			return;
#else
			return false;
#endif
		} else {
			LOG_CTRACE("client") << "Unload dynamic content:" << qPrintable(filename);
		}

		m_loadedContentList.removeAll(filename);

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
 * @brief Server::unloadDynamicContents
 */

void Server::unloadDynamicContents()
{
	LOG_CTRACE("client") << "Unload dynamic contents";

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, ret]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		for (const QString &s : m_loadedContentList) {
			if (!QResource::unregisterResource(s)) {
				LOG_CERROR("client") << "Unregister resource failed:" << qPrintable(s);
			} else {
				LOG_CTRACE("client") << "Unload dynamic content:" << qPrintable(s);
			}
		}

		m_loadedContentList.clear();

#ifndef Q_OS_WASM
		ret.resolve();
	});

	QDefer::await(ret);
#endif
}





/**
 * @brief Server::loadDynamicContent
 * @param filename
 */

void Server::loadDynamicContent(const QString &filename)
{
	LOG_CDEBUG("client") << "Load dynamic content:" << qPrintable(filename);

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, filename, ret]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		if (!QResource::registerResource(filename)) {
			LOG_CERROR("client") << "Register resource failed:" << qPrintable(filename);
		} else {
			m_loadedContentList.append(filename);
		}

#ifndef Q_OS_WASM
		ret.resolve();
	});

	QDefer::await(ret);
#endif
}




/**
 * @brief Server::getContentDir
 * @return
 */

std::optional<QDir> Server::getContentDir() const
{
	static const QString subdir = "content";

#ifdef Q_OS_WASM
	QDir dir = QStringLiteral("/");
#else
	if (m_isStatic) {
		if (m_staticTmpDir.isValid()) {
			QDir dir;
			dir.setPath(m_staticTmpDir.path());
			return dir;
		} else {
			LOG_CERROR("client") << "Invalid temporary directory";
			Application::instance()->messageError(tr("Belső hiba"));
			return std::nullopt;
		}
	}

	QDir dir = m_directory;

	if ((!dir.exists(subdir) && !dir.mkdir(subdir)) || !dir.cd(subdir)) {
		Application::instance()->messageError(tr("Belső hiba"));
		return std::nullopt;
	}
#endif

	return dir;
}


/**
 * @brief Server::isStatic
 * @return
 */

bool Server::isStatic() const
{
	return m_isStatic;
}

void Server::setIsStatic(bool newIsStatic)
{
	if (m_isStatic == newIsStatic)
		return;
	m_isStatic = newIsStatic;
	emit isStaticChanged();
}


/**
 * @brief Server::checkNotification
 */

void Server::checkNotification()
{
	const auto &dir = getContentDir();

	if (!dir)
		return;

	const QString &filename = dir->absoluteFilePath(QStringLiteral("notification.json"));

	const auto &ptr = Utils::fileToJsonObject(filename);

	QJsonArray characterList;
	QJsonArray mapList;

	if (ptr) {
		characterList = ptr->value(QStringLiteral("characters")).toArray();
		mapList = ptr->value(QStringLiteral("maps")).toArray();
	}


	QStringList newCharacterList;
	QStringList newMapList;


	for (const auto &[id, config] : RpgGame::characters().asKeyValueRange()) {
		if (!characterList.contains(id)) {
			characterList.append(id);
			newCharacterList.append(config.name);
		}
	}

	for (const auto &[id, config] : RpgGame::terrains().asKeyValueRange()) {
		if (!mapList.contains(id)) {
			mapList.append(id);
			newMapList.append(config.name);
		}
	}

	if (!newMapList.isEmpty()) {
		m_notificationContent.insert(qMakePair(NotificationMap, 1), mapList);
		emit notificationActivated(NotificationMap, 1, tr("Új világ elérhető: <b>%1</b>").arg(newMapList.join(QStringLiteral(", "))));
	} else if (!newCharacterList.isEmpty()) {
		m_notificationContent.insert(qMakePair(NotificationCharacter, 1), characterList);
		emit notificationActivated(NotificationCharacter, 1, tr("Új karakter elérhető: <b>%1</b>").arg(newCharacterList.join(QStringLiteral(", "))));
	}


}




/**
 * @brief Server::closeNotification
 * @param type
 * @param id
 */

void Server::closeNotification(const NotificationType &type, const int &id)
{
	if (type == NotificationInvalid || id <= 0)
		return;

	QJsonArray list = m_notificationContent.take(qMakePair(type, id)).toArray();

	if (list.isEmpty())
		return;

	const auto &dir = getContentDir();

	if (!dir)
		return;

	const QString &filename = dir->absoluteFilePath(QStringLiteral("notification.json"));

	QJsonObject data = Utils::fileToJsonObject(filename).value_or(QJsonObject());

	if (type == NotificationMap) {
		data[QStringLiteral("maps")] = list;
	} else if (type == NotificationCharacter) {
		data[QStringLiteral("characters")] = list;
	}

	Utils::jsonObjectToFile(data, filename);
}



QByteArray Server::sessionId() const
{
	return m_sessionId;
}

void Server::setSessionId(const QByteArray &newSessionId)
{
	if (m_sessionId == newSessionId)
		return;
	m_sessionId = newSessionId;
	emit sessionIdChanged();
}
