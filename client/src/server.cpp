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

	for (const QJsonValue &v : std::as_const(list)) {
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

	foreach (const QSslError::SslError &e, m_ignoredSslErrors)
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
	QJsonWebToken token;

	if (!token.setToken(jwt)) {
		LOG_CWARNING("client") << "Invalid token:" << jwt;
		return false;
	}

	const QJsonObject &object = token.getPayloadJDoc().object();

	if (JSON_TO_INTEGER(object.value(QStringLiteral("exp"))) <= QDateTime::currentSecsSinceEpoch()) {
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
	foreach (const Rank &r, m_rankList)
		if (r.id() == id)
			return r;

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
 * @brief Server::dynamicContentReady
 * @return
 */

bool Server::dynamicContentReady() const
{
	return m_dynamicContentReady;
}


/**
 * @brief Server::dynamicContentReset
 */

void Server::dynamicContentReset(const QJsonArray &list)
{
#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, list, ret]() mutable {
		QMutexLocker locker(&m_mutex);
#endif

		unloadDynamicContents();

		m_contentList.clear();

		for (const QJsonValue &v : list) {
			const QJsonObject &o = v.toObject();

			DynamicContent content;
			content.name = o.value(QStringLiteral("file")).toString();
			content.md5 = o.value(QStringLiteral("md5")).toString();
			content.size = JSON_TO_INTEGER(o.value(QStringLiteral("size")));
			m_contentList.append(content);
		}

#ifndef Q_OS_WASM
		ret.resolve();
	});

	QDefer::await(ret);
#endif
}



void Server::setDynamicContentReady(bool newDynamicContentReady)
{
	if (m_dynamicContentReady == newDynamicContentReady)
		return;
	m_dynamicContentReady = newDynamicContentReady;
	emit dynamicContentReadyChanged();
}

QVector<Server::DynamicContent> Server::dynamicContentList() const
{
	return m_contentList;
}




/**
 * @brief Server::dynamicContentCheck
 * @return
 */

bool Server::dynamicContentCheck()
{
	static const QString subdir = "content";
	QDir dir = m_directory;

	if ((!dir.exists(subdir) && !dir.mkdir(subdir)) || !dir.cd(subdir)) {
		Application::instance()->messageError(tr("Belső hiba"));
		return false;
	}


#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, dir, ret]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		for (auto it = m_contentList.cbegin(); it != m_contentList.cend(); ) {
			if (it->name.isEmpty()) {
				++it;
				continue;
			}

			const QString &filename = dir.absoluteFilePath(it->name);

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
				loadDynamicContent(filename);
				it = m_contentList.erase(it);
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
 * @param name
 * @param data
 * @return
 */

bool Server::dynamicContentRemove(const QString &name, const QByteArray &data)
{
	const QString &md5 = QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());
	const qint64 size = data.size();
	bool found = false;

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, name, md5, size, &found, ret]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		for (auto it = m_contentList.cbegin(); it != m_contentList.cend(); ) {
			if (it->name == name && it->md5 == md5 && it->size == size) {
				it = m_contentList.erase(it);
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
	static const QString subdir = "content";
	QDir dir = m_directory;

	if ((!dir.exists(subdir) && !dir.mkdir(subdir)) || !dir.cd(subdir)) {
		Application::instance()->messageError(tr("Belső hiba"));
		return false;
	}


#ifndef Q_OS_WASM
	QDefer ret;
	m_worker.execInThread([this, dir, ret, name, data]() mutable {
		QMutexLocker locker(&m_mutex);
#endif
		const QString &filename = dir.absoluteFilePath(name);

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
			if (!QResource::unregisterResource(s, QStringLiteral("/content"))) {
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
		if (!QResource::registerResource(filename, QStringLiteral("/content"))) {
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





