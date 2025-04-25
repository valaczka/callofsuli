/*
 * ---- Call of Suli ----
 *
 * rpgudpengine_p.h
 *
 * Created on: 2025. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef ABSTRACTUDPENGINE_P_H
#define ABSTRACTUDPENGINE_P_H

#include "qurl.h"
#include <QObject>

#ifndef Q_OS_WASM
#include "qmutex.h"
#include <enet/enet.h>
#endif

class AbstractUdpEngine;


/**
 * @brief The RpgUdpEnginePrivate class
 */

class AbstractUdpEnginePrivate : public QObject
{
	Q_OBJECT

public:
	AbstractUdpEnginePrivate(AbstractUdpEngine *engine)
		: q(engine)
	{}

signals:
	void serverConnected();
	void serverDisconnected();
	void serverConnectFailed();

private:
	void run();
	void sendMessage(QByteArray data, const bool &reliable = true, const int &channel = 0);
	void setUrl(const QUrl &url);

	AbstractUdpEngine *q = nullptr;

	QUrl m_url;

#ifndef Q_OS_WASM
	ENetHost *m_enet_host = nullptr;
	ENetPeer *m_enet_peer = nullptr;

	QMutex m_mutex;
#endif

	bool m_disconnectRequested = false;


	struct Packet {
		QByteArray data;
		int channel = 0;
		bool reliable = false;
	};

	QList<Packet> m_sendList;

	friend class AbstractUdpEngine;
	friend class AbstractUdpEngineThread;
};



#endif // ABSTRACTUDPENGINE_P_H
