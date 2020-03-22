/*
 * ---- Call of Suli ----
 *
 * handler.h
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


#ifndef HANDLER_H
#define HANDLER_H

#include <QWebSocket>
#include <QObject>

#include "../common/cossql.h"

class Handler : public QObject
{
	Q_OBJECT

public:
	explicit Handler(CosSql *database,
					 QWebSocket *socket,
					 QObject *parent = nullptr);
	virtual ~Handler();

private slots:
	void onDisconnected();
	void onBinaryMessageReceived(const QByteArray &message);

signals:
	void disconnected();

private:
	CosSql *m_db;
	QWebSocket *m_socket;

};

#endif // HANDLER_H
