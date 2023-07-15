/*
 * ---- Call of Suli ----
 *
 * panel.h
 *
 * Created on: 2023. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Panel
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

#ifndef PANEL_H
#define PANEL_H

#include <QObject>
#include <QPointer>
#include "httpEventStream.h"
#include "qjsonobject.h"
#include "qtimer.h"

class ServerService;


/**
 * @brief The Panel class
 */

class Panel : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)
	Q_PROPERTY(QString owner READ owner WRITE setOwner NOTIFY ownerChanged)
	Q_PROPERTY(HttpEventStream *stream READ stream WRITE setStream NOTIFY streamChanged)
	Q_PROPERTY(QJsonObject config READ config WRITE setConfig NOTIFY configChanged)

public:
	explicit Panel(ServerService *service);
	virtual ~Panel();

	int id() const;
	void setId(int newId);

	const QString &owner() const;
	void setOwner(const QString &newOwner);

	HttpEventStream *stream() const;
	void setStream(HttpEventStream *newStream);

	const QJsonObject &config() const;
	void setConfig(const QJsonObject &newConfig);

	const QDateTime &lastUpdated() const;

	QJsonObject getEventData() const;

public slots:
	void sendConfig();

private slots:
	void onDeleteTimerTimeout();

signals:
	void idChanged();
	void ownerChanged();
	void streamChanged();
	void configChanged();

private:
	ServerService *const m_service;
	int m_id = -1;
	QString m_owner;
	QPointer<HttpEventStream> m_stream = nullptr;
	QJsonObject m_config;
	QTimer m_deleteTimer;
	QDateTime m_lastUpdated = QDateTime::currentDateTime();
};

#endif // PANEL_H
