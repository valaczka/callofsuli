/*
 * ---- Call of Suli ----
 *
 * storageseed.h
 *
 * Created on: 2025. 07. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StorageSeed
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

#ifndef STORAGESEED_H
#define STORAGESEED_H

#include "qdebug.h"
#include <QString>
#include <QList>


typedef QHash<int, QList<int>> StorageSeedData;
typedef QHash<int, StorageSeedData> StorageSeedStorageData;


class StorageSeedPrivate;


/**
 * @brief The StorageSeed class
 */

class StorageSeed
{

public:
	StorageSeed(const QString &file = {});
	StorageSeed(const StorageSeed &other);
	virtual ~StorageSeed();

	static void addSeedToMap(QVariantMap *dst, const int &main, const int &sub, const int &storage = -1, const QString &map = {});
	static void addSeedToMap(QVariantMap *dst, const int &mainA, const int &subA, const int &mainB, const int &subB);

	QString currentMap() const;
	void setCurrentMap(const QString &newCurrentMap);

	int currentStorage() const;
	void setCurrentStorage(int newCurrentStorage);

	QList<int> getDataFromCurrent(const int &main) const;
	QList<int> getDataFromCurrent(const int &storage, const int &main) const;
	QList<int> getData(const QString &map, const int &storage, const int &main) const;

	void setData(const QVariantMap &question, const int &storage = -1, const QString &map = {});

	QDebug debug(QDebug debug) const;

	bool operator==(const StorageSeed &other) const;
	StorageSeed& operator=(const StorageSeed &other);

private:
	StorageSeedPrivate *d;

	QString m_currentMap;
	int m_currentStorage = -1;

	friend class StorageSeedPrivate;
	friend class SeedHelper;
	friend class SeedDuplexHelper;
};


QDebug operator<<(QDebug debug, const StorageSeed &c);



/**
 * @brief The SeedHelper class
 */

class SeedHelper
{
public:
	SeedHelper(StorageSeed *seed, const int &main);
	SeedHelper(StorageSeed *seed, const int &storage, const int &main);
	SeedHelper(StorageSeed *seed, const QString &map, const int &storage, const int &main);

	SeedHelper &append(const QVariantMap &map, const int &sub = -1, const int &main = -1);
	SeedHelper &operator<< (const QVariantMap &map) { return append(map); }

	QVariantList getVariantList(const bool &autoClean = false);

private:
	SeedHelper(const int &main, StorageSeed *seed);

	StorageSeed *const m_seed;
	const int m_main;

	QString m_map;
	int m_storage = -1;

	QList<int> m_data;
	std::vector<QVariantMap> m_itemUsed;
	std::vector<QVariantMap> m_itemReady;
};




/**
 * @brief The SeedDuplexHelper class
 */

class SeedDuplexHelper
{
public:
	SeedDuplexHelper(StorageSeed *seed, const int &mainA, const int &mainB);
	SeedDuplexHelper(StorageSeed *seed, const int &storage, const int &mainA, const int &mainB);
	SeedDuplexHelper(StorageSeed *seed, const QString &map, const int &storage, const int &mainA, const int &mainB);

	SeedDuplexHelper &append(const QVariantMap &map, const int &subA, const int &subB, const int &mainA = -1, const int &mainB = -1);

	int getSubB(const int &from, const int &to) const;
	int getSubB(const int &to) const { return getSubB(0, to); }
	int getSubBOffset(const int &to, const int &offset) const { return getSubB(offset, offset+to)-offset; }
	int getSubB(const QList<int> &list) const;

	QVariantList getVariantList(const bool &autoClean = false);

private:
	SeedDuplexHelper(const int &mainA, const int &mainB, StorageSeed *seed);

	StorageSeed *const m_seed;
	const int m_mainA;
	const int m_mainB;

	QString m_map;
	int m_storage = -1;

	QList<int> m_dataA;
	QList<int> m_dataB;
	std::vector<QVariantMap> m_itemUsedDouble;
	std::vector<QVariantMap> m_itemUsedA;
	std::vector<QVariantMap> m_itemUsedB;
	std::vector<QVariantMap> m_itemReady;
};


#endif // STORAGESEED_H
