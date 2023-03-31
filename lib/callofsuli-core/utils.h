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

#ifdef CLIENT_UTILS
#include <QOlm/QOlm.hpp>
#endif

#ifdef Q_OS_ANDROID
#include <jni.h>
#include <QtAndroidExtras/QAndroidJniObject>
#endif


/**
 * @brief The Utils class
 */

class Utils : public QObject
{
	Q_OBJECT

public:
	explicit Utils(QObject *parent = nullptr);
	virtual ~Utils();

	Q_INVOKABLE static QByteArray fileContent(const QString &filename, bool *error = nullptr);
	Q_INVOKABLE static QString fileBaseName(const QString &filename);

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

	Q_INVOKABLE static QByteArray generateRandomString(quint8 length);
	Q_INVOKABLE static QByteArray generateRandomString(quint8 length, const char *characters);

	Q_INVOKABLE static QVariant settingsGet(const QString &key, const QVariant &defaultValue = QVariant());
	Q_INVOKABLE static void settingsSet(const QString &key, const QVariant &value);

#ifdef CLIENT_UTILS
	Q_INVOKABLE static int selectedCount(qolm::QOlmBase *list);

	template<typename QEnum>
	static std::string enumToString(const QEnum value)
	{
		return std::string(QMetaEnum::fromType<QEnum>().valueToKey(value));
	}

	template<typename QEnum>
	static QString enumToQString(const QEnum value)
	{
		return QString(QMetaEnum::fromType<QEnum>().valueToKey(value));
	}
#endif

	Q_INVOKABLE static quint32 versionMajor();
	Q_INVOKABLE static quint32 versionMinor();
	Q_INVOKABLE static quint32 versionCode();
	Q_INVOKABLE static quint32 versionCode(const int &major, const int &minor);

	Q_INVOKABLE void checkStoragePermissions();
	Q_INVOKABLE void checkMediaPermissions();

signals:
	void storagePermissionsGranted();
	void storagePermissionsDenied();
	void mediaPermissionsGranted();
	void mediaPermissionsDenied();

private:
	static const quint32 m_versionMajor;
	static const quint32 m_versionMinor;

#ifdef Q_OS_ANDROID
	jint m_screenOrientationRequest = -1;
#endif

};


#endif // UTILS_H
