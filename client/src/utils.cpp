/*
 * ---- Call of Suli ----
 *
 * utils.cpp
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

#include "utils.h"
#include "client.h"
#include "qjsondocument.h"

Q_LOGGING_CATEGORY(lcUtils, "app.utils")

/**
 * @brief Utils::Utils
 * @param client
 */

Utils::Utils(Client *client)
	: QObject{client}
	, m_client(client)
{

}


/**
 * @brief Utils::~Utils
 */

Utils::~Utils()
{

}


/**
 * @brief Utils::client
 * @return
 */

Client *Utils::client() const
{
	return m_client;
}


/**
 * @brief Utils::byteArrayToJsonDocument
 * @param data
 * @return
 */

QJsonDocument Utils::byteArrayToJsonDocument(const QByteArray &data)
{
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(data, &error);

	if (error.error != QJsonParseError::NoError) {
		qCWarning(lcUtils).noquote() << tr("JSON feldolgozási hiba: %1 (%2)").arg(error.errorString()).arg(error.error);
	}

	return doc;
}


/**
 * @brief Utils::byteArrayToJsonObject
 * @param data
 * @return
 */

QJsonObject Utils::byteArrayToJsonObject(const QByteArray &data, bool *error)
{
	const QJsonDocument &doc = byteArrayToJsonDocument(data);

	if (error)
		*error = doc.isNull();

	return doc.object();
}


/**
 * @brief Utils::byteArrayToJsonArray
 * @param data
 * @return
 */

QJsonArray Utils::byteArrayToJsonArray(const QByteArray &data, bool *error)
{
	const QJsonDocument &doc = byteArrayToJsonDocument(data);

	if (error)
		*error = doc.isNull();

	return doc.array();
}


/**
 * @brief Utils::fileToJsonDocument
 * @param filename
 * @return
 */

QJsonDocument Utils::fileToJsonDocument(const QString &filename, bool *error)
{
	QFile f(filename);

	if (!f.exists()) {
		qCWarning(lcUtils).noquote() << tr("A fájl nem olvasható:") << filename;

		if (error)
			*error = true;

		return QJsonDocument();
	}

	if (!f.open(QIODevice::ReadOnly)) {
		qCWarning(lcUtils).noquote() << tr("Nem lehet megnyitni a fájlt:") << filename;

		if (error)
			*error = true;

		return QJsonDocument();
	}

	QByteArray data = f.readAll();

	f.close();

	return byteArrayToJsonDocument(data);
}


/**
 * @brief Utils::fileToJsonObject
 * @param filename
 * @param error
 * @return
 */

QJsonObject Utils::fileToJsonObject(const QString &filename, bool *error)
{
	bool myerror = false;

	const QJsonDocument &doc = fileToJsonDocument(filename, &myerror);

	if (myerror) {
		if (error)
			*error = true;

		return QJsonObject();
	}

	if (error)
		*error = doc.isNull();

	return doc.object();
}



/**
 * @brief Utils::fileToJsonArray
 * @param filename
 * @param error
 * @return
 */

QJsonArray Utils::fileToJsonArray(const QString &filename, bool *error)
{
	bool myerror = false;

	const QJsonDocument &doc = fileToJsonDocument(filename, &myerror);

	if (myerror) {
		if (error)
			*error = true;

		return QJsonArray();
	}

	if (error)
		*error = doc.isNull();

	return doc.array();
}


/**
 * @brief Utils::colorSetAlpha
 * @param color
 * @param alpha
 * @return
 */

QColor Utils::colorSetAlpha(QColor color, const qreal &alpha)
{
	color.setAlphaF(alpha);
	return color;
}
