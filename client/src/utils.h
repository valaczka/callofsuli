/*
 * ---- Call of Suli ----
 *
 * utils.h
 *
 * Created on: 2022. 12. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Utils
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

#ifndef UTILS_H
#define UTILS_H

#include "qloggingcategory.h"
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QColor>

class Client;

class Utils : public QObject
{
	Q_OBJECT

public:
	explicit Utils(Client *client);
	virtual ~Utils();

	Client *client() const;

	Q_INVOKABLE static QJsonDocument byteArrayToJsonDocument(const QByteArray &data);
	Q_INVOKABLE static QJsonObject byteArrayToJsonObject(const QByteArray &data, bool *error = nullptr);
	Q_INVOKABLE static QJsonArray byteArrayToJsonArray(const QByteArray &data, bool *error = nullptr);

	Q_INVOKABLE static QJsonDocument fileToJsonDocument(const QString &filename, bool *error = nullptr);
	Q_INVOKABLE static QJsonObject fileToJsonObject(const QString &filename, bool *error = nullptr);
	Q_INVOKABLE static QJsonArray fileToJsonArray(const QString &filename, bool *error = nullptr);

	Q_INVOKABLE static QColor colorSetAlpha(QColor color, const qreal &alpha);

private:
	Client *const m_client = nullptr;

};


Q_DECLARE_LOGGING_CATEGORY(lcUtils)

#endif // UTILS_H
