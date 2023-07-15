/*
 * ---- Call of Suli ----
 *
 * campaignresultlist.h
 *
 * Created on: 2023. 06. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CampaignResultList
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

#ifndef OFFSETMODEL_H
#define OFFSETMODEL_H

#include <QObject>
#include "qslistmodel.h"
#include "websocket.h"

/**
 * @brief The OffsetModel class
 */

class OffsetModel : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QSListModel *model READ model CONSTANT)

	Q_PROPERTY(WebSocket::API api READ api WRITE setApi NOTIFY apiChanged)
	Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
	Q_PROPERTY(QJsonObject apiData READ apiData WRITE setApiData NOTIFY apiDataChanged)

	Q_PROPERTY(QStringList fields READ fields WRITE setFields NOTIFY fieldsChanged)
	Q_PROPERTY(QString listField READ listField WRITE setListField NOTIFY listFieldChanged)

	Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)
	Q_PROPERTY(bool canFetch READ canFetch WRITE setCanFetch NOTIFY canFetchChanged)

public:
	explicit OffsetModel(QObject *parent = nullptr);
	virtual ~OffsetModel();

	const WebSocket::API &api() const;
	void setApi(const WebSocket::API &newApi);

	const QString &path() const;
	void setPath(const QString &newPath);

	const QJsonObject &apiData() const;
	void setApiData(const QJsonObject &newApiData);

	QSListModel *model() const;

	const QStringList &fields() const;
	void setFields(const QStringList &newFields);

	int limit() const;
	void setLimit(int newLimit);

	const QString &listField() const;
	void setListField(const QString &newListField);

	bool canFetch() const;
	void setCanFetch(bool newCanFetch);

public slots:
	void reload();
	void fetch();

protected:
	virtual QVariantList getListFromJson(const QJsonObject &obj);

signals:
	void apiChanged();
	void pathChanged();
	void apiDataChanged();
	void fieldsChanged();
	void limitChanged();
	void listFieldChanged();
	void canFetchChanged();

	void modelCleared();
	void snapshotAdded();
	void snapshotEmpty();


private:
	void loadFromJson(const QJsonObject &obj);
	QSListModel *m_model = nullptr;
	WebSocket::API m_api = WebSocket::ApiInvalid;
	QString m_path;
	QJsonObject m_apiData;
	QStringList m_fields;
	QString m_listField = QStringLiteral("list");
	bool m_canFetch = true;

	int m_limit = 50;
	int m_offset = 0;

	bool m_fetchActive = false;
};

#endif // OFFSETMODEL_H
