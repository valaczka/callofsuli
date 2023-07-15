/*
 * ---- Call of Suli ----
 *
 * scorelist.h
 *
 * Created on: 2023. 06. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ScoreList
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

#ifndef SCORELIST_H
#define SCORELIST_H

#include "qjsonobject.h"
#include "websocket.h"
#include "fetchmodel.h"
#include <QObject>

class ScoreList : public FetchModel
{
	Q_OBJECT

	Q_PROPERTY(SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
	Q_PROPERTY(int filterClassId READ filterClassId WRITE setFilterClassId NOTIFY filterClassIdChanged)

	Q_PROPERTY(WebSocket::API api READ api WRITE setApi NOTIFY apiChanged)
	Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
	Q_PROPERTY(QJsonObject apiData READ apiData WRITE setApiData NOTIFY apiDataChanged)

	Q_PROPERTY(EventStream *eventStream READ eventStream WRITE setEventStream NOTIFY eventStreamChanged)

public:
	explicit ScoreList(QObject *parent = nullptr);

	enum SortOrder {
		SortNone,
		SortXPdesc,
		SortXP,
		SortFullname,
		SortFullNickname,
		SortStreak
	};

	Q_ENUM(SortOrder);

	const SortOrder &sortOrder() const;
	void setSortOrder(const SortOrder &newSortOrder);


	const WebSocket::API &api() const;
	void setApi(const WebSocket::API &newApi);

	const QString &path() const;
	void setPath(const QString &newPath);

	const QJsonObject &apiData() const;
	void setApiData(const QJsonObject &newApiData);

	int filterClassId() const;
	void setFilterClassId(int newFilterClassId);

	EventStream *eventStream() const;
	void setEventStream(EventStream *newEventStream);

public slots:
	void reload() override;
	void reloadFromVariantList(const QVariantList &list) override;
	void refresh();

private slots:
	void onEventJsonReceived(const QString &, const QJsonObject &json);

signals:
	void sortOrderChanged();
	void apiChanged();
	void pathChanged();
	void apiDataChanged();
	void filterClassIdChanged();
	void eventStreamChanged();

private:
	void loadFromJson(const QJsonObject &obj);

	SortOrder m_sortOrder = SortNone;
	WebSocket::API m_api = WebSocket::ApiGeneral;
	QString m_path = QStringLiteral("score");
	QJsonObject m_apiData;
	int m_filterClassId = -1;

	QVariantList m_originalModel;

	EventStream *m_eventStream = nullptr;
};




#endif // SCORELIST_H
