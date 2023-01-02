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

#ifdef Q_OS_ANDROID
#include <jni.h>
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#endif

class Utils : public QObject
{
	Q_OBJECT

public:
	explicit Utils(QObject *parent = nullptr);
	virtual ~Utils();

	Q_INVOKABLE static QByteArray fileContent(const QString &filename, bool *error = nullptr);

	Q_INVOKABLE static bool jsonDocumentToFile(const QJsonDocument &doc, const QString &filename,
											   const QJsonDocument::JsonFormat &format = QJsonDocument::Indented);
	Q_INVOKABLE static bool jsonObjectToFile(const QJsonObject &object, const QString &filename,
											 const QJsonDocument::JsonFormat &format = QJsonDocument::Indented);
	Q_INVOKABLE static bool jsonArrayToFile(const QJsonArray &array, const QString &filename,
											const QJsonDocument::JsonFormat &format = QJsonDocument::Indented);

	Q_INVOKABLE static QJsonDocument byteArrayToJsonDocument(const QByteArray &data);
	Q_INVOKABLE static QJsonObject byteArrayToJsonObject(const QByteArray &data, bool *error = nullptr);
	Q_INVOKABLE static QJsonArray byteArrayToJsonArray(const QByteArray &data, bool *error = nullptr);

	Q_INVOKABLE static QJsonDocument fileToJsonDocument(const QString &filename, bool *error = nullptr);
	Q_INVOKABLE static QJsonObject fileToJsonObject(const QString &filename, bool *error = nullptr);
	Q_INVOKABLE static QJsonArray fileToJsonArray(const QString &filename, bool *error = nullptr);

	Q_INVOKABLE static QColor colorSetAlpha(QColor color, const qreal &alpha);


	Q_INVOKABLE static QString formatMSecs(const int &msec, const int &decimals = 0, const bool &withMinute = true);

	Q_INVOKABLE static void openUrl(const QUrl &url);

	Q_INVOKABLE static QString standardPath(const QString &path = "");
	Q_INVOKABLE static QString genericDataPath(const QString &path = "");

private:

#ifdef Q_OS_ANDROID
	jint m_screenOrientationRequest = -1;
#endif

};


Q_DECLARE_LOGGING_CATEGORY(lcUtils)

#endif // UTILS_H
