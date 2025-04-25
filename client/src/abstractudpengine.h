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


class AbstractUdpEngine;
class AbstractUdpEnginePrivate;



/**
 * @brief The RpgUdpEngineThread class
 */

#ifndef Q_OS_WASM
class AbstractUdpEngineThread : public QThread
{
	Q_OBJECT

public:
	AbstractUdpEngineThread(AbstractUdpEngine *engine)
		: QThread()
		, q(engine)
	{  }

	void run() override;

private:
	AbstractUdpEngine *q;
};

#endif


/**
 * @brief The AbstractUdpEngine class
 */

class AbstractUdpEngine : public QObject
{
	Q_OBJECT

public:
	explicit AbstractUdpEngine(QObject *parent = nullptr);
	virtual ~AbstractUdpEngine();

	void sendMessage(QByteArray data, const bool &reliable = true, const int &channel = 0);

	void setUrl(const QUrl &url);

protected:
	virtual void packetReceived(const QCborMap &data, const unsigned int rtt) = 0;

private:
#ifndef Q_OS_WASM
	AbstractUdpEngineThread *m_dThread = nullptr;
#endif
	AbstractUdpEnginePrivate *d = nullptr;

	friend class AbstractUdpEnginePrivate;
	friend class AbstractUdpEngineThread;
};


#endif // ABSTRACTUDPENGINE_H
