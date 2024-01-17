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





