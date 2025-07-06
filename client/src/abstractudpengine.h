/*
 * ---- Call of Suli ----
 *
 * abstractudpengine.h
 *
 * Created on: 2025. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractUdpEngine
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

#ifndef ABSTRACTUDPENGINE_H
#define ABSTRACTUDPENGINE_H

#include <QObject>
#include <QThread>
#include <QCborMap>

#ifndef Q_OS_WASM
#include "qlambdathreadworker.h"
#endif


class AbstractUdpEnginePrivate;


/**
 * @brief The AbstractUdpEngine class
 */

class AbstractUdpEngine : public QObject
{
	Q_OBJECT

public:
	explicit AbstractUdpEngine(QObject *parent = nullptr);
	virtual ~AbstractUdpEngine();

	void sendMessage(const QByteArray &data, const bool &reliable = true, const bool &sign = true);

	void setUrl(const QUrl &url);

	QByteArray connectionToken() const;
	void setConnectionToken(const QByteArray &token);

	int currentRtt() const;
	void setCurrentRtt(const int &rtt);

signals:
	void serverConnected();
	void serverDisconnected();
	void serverConnectFailed(const QString &error);
	void serverConnectionLost();

protected:
	virtual void binaryDataReceived(const QList<QPair<QByteArray, unsigned int> > &list) = 0;

private:
#ifndef Q_OS_WASM
	std::unique_ptr<QLambdaThreadWorker> m_worker;
#endif
	AbstractUdpEnginePrivate *d = nullptr;

	friend class AbstractUdpEnginePrivate;
};


#endif // ABSTRACTUDPENGINE_H
