/*
 * ---- Call of Suli ----
 *
 * oauth2replyhandler.h
 *
 * Created on: 2023. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OAuth2ReplyHandler
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

#ifndef OAUTH2REPLYHANDLER_H
#define OAUTH2REPLYHANDLER_H

#include "Logger.h"
#include <QAbstractOAuthReplyHandler>
#include <QHostAddress>
#include <QtNetworkAuth/qoauthoobreplyhandler.h>

#ifndef QT_NO_SSL
#include <QSslConfiguration>
#endif


#ifndef QT_NO_SSL
class OAuthHttpServerReplySslHandler;
#endif

class OAuth2Authenticator;

template <typename TData, typename = void>
class OAuth2ReplyHandler;

template <typename T>
using QOAuthOob_SFINAE = typename std::enable_if<std::is_base_of<QOAuthOobReplyHandler, T>::value>::type;




/**
 * @brief The AbstractReplyHandler class
 */

class AbstractReplyHandler
{
public:
	explicit AbstractReplyHandler(OAuth2Authenticator *authenticator);
	virtual ~AbstractReplyHandler();

	void setAuthenticator(OAuth2Authenticator *authenticator) { m_authenticator = authenticator; }
	OAuth2Authenticator *authenticator() const { return m_authenticator; };

	virtual QAbstractOAuthReplyHandler *abstractHandler() const =  0;
	virtual bool listen(const QHostAddress &address, const quint16 &port) = 0;
	virtual bool isListening() const = 0;

protected :
	virtual void onCallbackReceived(const QVariantMap &data);

	OAuth2Authenticator *m_authenticator = nullptr;
};






/**
 * @brief The OAuth2ReplyHandler class
 */

template <typename T>
class OAuth2ReplyHandler<T, QOAuthOob_SFINAE<T>> : public AbstractReplyHandler
{

public:
	explicit OAuth2ReplyHandler(const QHostAddress &address, const quint16 &port, OAuth2Authenticator *authenticator);
	virtual ~OAuth2ReplyHandler();

	QAbstractOAuthReplyHandler *abstractHandler() const { return m_handler; }
	T *handler() const { return m_handler; }
	bool listen(const QHostAddress &address, const quint16 &port) { return m_handler->listen(address, port); }
	bool isListening() const { return m_handler->isListening(); }

private:
	T *m_handler = nullptr;

};




template<typename T>
OAuth2ReplyHandler<T, QOAuthOob_SFINAE<T>>::OAuth2ReplyHandler(const QHostAddress &address, const quint16 &port, OAuth2Authenticator *authenticator)
	: AbstractReplyHandler(authenticator)
	, m_handler(new T(address, port, authenticator))
{
	LOG_CTRACE("oauth2") << "Reply handler created" << this;

	QObject::connect(m_handler, &T::callbackReceived, [this](const QVariantMap &data){ onCallbackReceived(data); });
}



/**
 * @brief OAuth2ReplyHandler::~OAuth2ReplyHandler
 */

template<typename T>
OAuth2ReplyHandler<T, QOAuthOob_SFINAE<T>>::~OAuth2ReplyHandler()
{
	delete m_handler;
	LOG_CTRACE("oauth2") << "Reply handler destroyed" << this;
}





#ifndef QT_NO_SSL

/**
 * @brief The OAuthHttpServerReplySslHandler class
 */

#include <QtNetworkAuth/qoauthglobal.h>
#include <QtNetwork/qhostaddress.h>

class QUrlQuery;
class OAuthHttpServerReplySslHandlerPrivate;


class OAuthHttpServerReplySslHandler : public QOAuthOobReplyHandler
{
	Q_OBJECT

public:
	explicit OAuthHttpServerReplySslHandler(OAuth2Authenticator *authenticator);
	explicit OAuthHttpServerReplySslHandler(OAuth2Authenticator *authenticator, quint16 port);
	explicit OAuthHttpServerReplySslHandler(const QHostAddress &address, quint16 port, OAuth2Authenticator *authenticator);
	~OAuthHttpServerReplySslHandler();

	QString callback() const override;
	QString callbackPath() const;
	void setCallbackPath(const QString &path);
	QString callbackText() const;
	void setCallbackText(const QString &text);
	quint16 port() const;
	bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
	void close();
	bool isListening() const;

	const QSslConfiguration &configuration() const;
	void setConfiguration(const QSslConfiguration &newConfiguration);

private:
	Q_DECLARE_PRIVATE(OAuthHttpServerReplySslHandler)
	OAuth2Authenticator *m_authenticator = nullptr;
	QScopedPointer<OAuthHttpServerReplySslHandlerPrivate> d_ptr;
};




/**
 * @brief The SslServer class
 */

#include <qtcpserver.h>

class SslServer : public QTcpServer
{
	Q_OBJECT

public:
	explicit SslServer(QObject *parent = nullptr) : QTcpServer(parent) {}
	virtual ~SslServer() {}

	const QSslConfiguration &configuration() const { return m_configuration; }
	void setConfiguration(const QSslConfiguration &newConfiguration) { m_configuration = newConfiguration; }

protected:
	virtual void incomingConnection(qintptr socketDescriptor) override;
	QSslConfiguration m_configuration;
};





/**
 * @brief The OAuthHttpServerReplySslHandlerPrivate class
 */

#include <QtNetworkAuth/qoauthglobal.h>
#include <QtNetworkAuth/qoauthhttpserverreplyhandler.h>
#include <private/qobject_p.h>
#include <QtNetwork/qtcpserver.h>

class OAuthHttpServerReplySslHandlerPrivate
{
	Q_DECLARE_PUBLIC(OAuthHttpServerReplySslHandler)

public:
	explicit OAuthHttpServerReplySslHandlerPrivate(OAuthHttpServerReplySslHandler *p);
	~OAuthHttpServerReplySslHandlerPrivate();
	SslServer httpServer;
	QString text;
	QHostAddress listenAddress = QHostAddress::LocalHost;
	QString path;
private:
	void _q_clientConnected();
	void _q_readData(QTcpSocket *socket);
	void _q_answerClient(QTcpSocket *socket, const QUrl &url);
	struct QHttpRequest {
		quint16 port = 0;
		bool readMethod(QTcpSocket *socket);
		bool readUrl(QTcpSocket *socket);
		bool readStatus(QTcpSocket *socket);
		bool readHeader(QTcpSocket *socket);
		enum class State {
			ReadingMethod,
			ReadingUrl,
			ReadingStatus,
			ReadingHeader,
			ReadingBody,
			AllDone
		} state = State::ReadingMethod;
		QByteArray fragment;
		enum class Method {
			Unknown,
			Head,
			Get,
			Put,
			Post,
			Delete,
		} method = Method::Unknown;
		QUrl url;
		QPair<quint8, quint8> version;
		QMap<QByteArray, QByteArray> headers;
	};
	QMap<QTcpSocket *, QHttpRequest> clients;
	OAuthHttpServerReplySslHandler *q_ptr;
};



#endif


#endif // OAUTH2REPLYHANDLER_H
