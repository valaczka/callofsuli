/*
 * ---- Call of Suli ----
 *
 * database.h
 *
 * Created on: 2022. 12. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Database
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

#ifndef DATABASE_H
#define DATABASE_H


#include "Logger.h"
#include <QDeferred>
#include <QLambdaThreadWorker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QObject>
#include <QSqlQuery>
#include "utils_.h"



/**
 * @brief The Database class
 */


class Database
{
public:
	Database(const QString &dbName);
	virtual ~Database();

	const QString &dbName() const;

	virtual bool databaseOpen(const QString &path);
	virtual void databaseClose();

	QRecursiveMutex *mutex() const;

	// Methods starts with "_" must be called from m_thread

	bool _batchFromData(const QString &data);
	bool _batchFromFile(const QString &filename);

	QLambdaThreadWorker *worker() const;


	// Database upgrade

	struct Upgrade
	{
		enum Type {
			UpgradeFromData,
			UpgradeFromFile
		};

		int fromVersionMajor = 0;
		int fromVersionMinor = 0;
		int toVersionMajor = 0;
		int toVersionMinor = 0;
		Type type = UpgradeFromData;
		QString content;
	};

	virtual bool performUpgrade(const QVector<Upgrade> &upgrades, const QString &defaultUpgradeData,
								const quint32 &fromVersionMajor, const quint32 &fromVersionMinor,
								const quint32 &toVersionMajor = Utils::versionMajor(),
								const quint32 &toVersionMinor = Utils::versionMinor());

protected:
	bool databaseInit();

	std::unique_ptr<QLambdaThreadWorker> m_worker;
	QString m_dbName;
	std::unique_ptr<QRecursiveMutex> m_mutex;
};




#endif // DATABASE_H
