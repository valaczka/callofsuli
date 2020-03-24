/*
 * ---- Call of Suli ----
 *
 * handler.cpp
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Handler
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  Call of Suli is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QJsonDocument>
#include <QDebug>
#include "handler.h"

Handler::Handler(CosSql *database, QWebSocket *socket, QObject *parent)
	: QObject(parent)
	, m_db(database)
	, m_socket(socket)
{
	Q_ASSERT(socket);

	qInfo().noquote() << tr("Client connected: ") << m_socket->peerAddress().toString() << m_socket->peerPort();

	connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(m_socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onBinaryMessageReceived(QByteArray)));
}


/**
 * @brief Handler::~Handler
 */

Handler::~Handler()
{

}


/**
 * @brief Handler::onDisconnected
 */

void Handler::onDisconnected()
{
	qInfo().noquote() << tr("Client disconnected:") << m_socket->peerAddress().toString() << m_socket->peerPort();
	emit disconnected();
}


/**
 * @brief Handler::onBinaryMessageReceived
 * @param message
 */

void Handler::onBinaryMessageReceived(const QByteArray &message)
{
	QDataStream ds(message);
	ds.setVersion(QDataStream::Qt_5_14);

	quint64 siz;
	QString txt;
	bool test;
	QByteArray json;
	QByteArray doc;
	QByteArray hash;

	ds >> siz >> txt >> test >> json >> hash >> doc;

	qDebug() << siz << txt << test;
	qDebug() << QJsonDocument::fromJson(json) << hash.toHex();

	QFile f("out.db");
	f.open(QIODevice::WriteOnly);
	f.write(doc);
	f.close();
}

