/*
 * ---- Call of Suli ----
 *
 * userimporter.h
 *
 * Created on: 2023. 07. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * UserImporter
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

#ifndef USERIMPORTER_H
#define USERIMPORTER_H

#include "qjsonarray.h"
#include <QObject>

class Client;

#ifdef Q_OS_WASM
class OnlineClient;
#else
#include "qlambdathreadworker.h"
#endif

/**
 * @brief The UserImporter class
 */

class UserImporter : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QJsonArray records READ records NOTIFY recordsChanged)
	Q_PROPERTY(QJsonArray errorRecords READ errorRecords NOTIFY errorRecordsChanged)

public:
	explicit UserImporter(QObject *parent = nullptr);
	virtual ~UserImporter();

	enum Field {
		Invalid,
		Username,
		FamilyName,
		GivenName,
		NickName,
		Picture,
		OAuth2,
		Password
	};

	Q_ENUM(Field)

	const QJsonArray &records() const;
	const QJsonArray &errorRecords() const;

public slots:
	void downloadTemplate(const QUrl &file);
	void upload(const QUrl &file);

#ifdef Q_OS_WASM
	void wasmDownloadTemplate();
	void wasmUpload();
#endif

signals:
	void loadStarted();
	void loadFinished();
	void emptyDocument();

	void recordsChanged();
	void errorRecordsChanged();

private:
	void _generateTemplate();
	void _load(const QByteArray &data);

	Client *const m_client;

	QByteArray m_templateContent;
	static const QHash<Field, QString> m_fieldMap;

	QJsonArray m_records;
	QJsonArray m_errorRecords;

#ifdef Q_OS_WASM
	OnlineClient *m_onlineClient = nullptr;
#else
	QLambdaThreadWorker *m_worker = nullptr;
#endif
};

#endif // USERIMPORTER_H
