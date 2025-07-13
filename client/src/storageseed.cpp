/*
 * ---- Call of Suli ----
 *
 * storageseed.cpp
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

#include "storageseed.h"
#include "Logger.h"
#include <QFile>
#include <QCborMap>
#include <QCborArray>
#include <random>
#include <QRandomGenerator>
#include "utils_.h"


class StorageSeedPrivate
{
private:
	StorageSeedPrivate(StorageSeed *seed)
		: q(seed)
	{}

	bool loadFromFile();
	bool saveToFile();

	void setData(const QVariantMap &question, const int &storage = -1, const QString &map = {});
	static void extract(const QVariantMap &question, int *storage, QString *map, int *mainA, int *subA, int *mainB, int *subB);

	void record(const QString &map, const int &storage, const int &main, const int &sub);

	void clear(const QString &map, const int &storage, const int &main);

	StorageSeed *const q;

	QString m_fileName;

	QHash<QString, StorageSeedStorageData> m_data;


	friend class StorageSeed;
	friend class SeedHelper;
	friend class SeedDuplexHelper;
};



/**
 * @brief StorageSeed::StorageSeed
 */

StorageSeed::StorageSeed(const QString &file)
	: d(new StorageSeedPrivate(this))
{
	d->m_fileName = file;
	d->loadFromFile();
}




StorageSeed::~StorageSeed()
{
	d->saveToFile();
	delete d;
	d = nullptr;
}


/**
 * @brief StorageSeed::addSeed
 * @param dst
 * @param main
 * @param sub
 * @param storage
 * @param map
 */

void StorageSeed::addSeedToMap(QVariantMap *dst, const int &main, const int &sub, const int &storage, const QString &map)
{
	if (!dst) {
		LOG_CERROR("utils") << "Missing QVariantMap";
		return;
	}

	if (main > 0)
		dst->insert(QStringLiteral("seedMain"), main);

	if (sub > 0)
		dst->insert(QStringLiteral("seedSub"), sub);

	if (storage > 0)
		dst->insert(QStringLiteral("seedStorage"), storage);

	if (!map.isEmpty())
		dst->insert(QStringLiteral("seedMap"), map);

}



/**
 * @brief StorageSeed::addSeedToMap
 * @param dst
 * @param mainA
 * @param subA
 * @param mainB
 * @param subB
 */

void StorageSeed::addSeedToMap(QVariantMap *dst, const int &mainA, const int &subA, const int &mainB, const int &subB)
{
	if (!dst) {
		LOG_CERROR("utils") << "Missing QVariantMap";
		return;
	}

	if (mainA > 0)
		dst->insert(QStringLiteral("seedMain"), mainA);

	if (subA > 0)
		dst->insert(QStringLiteral("seedSub"), subA);

	if (mainB > 0)
		dst->insert(QStringLiteral("seedMainB"), mainB);

	if (subB > 0)
		dst->insert(QStringLiteral("seedSubB"), subB);
}




QString StorageSeed::currentMap() const
{
	return m_currentMap;
}

void StorageSeed::setCurrentMap(const QString &newCurrentMap)
{
	m_currentMap = newCurrentMap;
}

int StorageSeed::currentStorage() const
{
	return m_currentStorage;
}

void StorageSeed::setCurrentStorage(int newCurrentStorage)
{
	m_currentStorage = newCurrentStorage;
}



/**
 * @brief StorageSeed::getDataFromCurrent
 * @param main
 * @return
 */

QList<int> StorageSeed::getDataFromCurrent(const int &main) const
{
	if (m_currentStorage <= 0)
		return {};

	return getDataFromCurrent(m_currentStorage, main);
}



/**
 * @brief StorageSeed::getDataFromCurrent
 * @param storage
 * @param main
 * @return
 */

QList<int> StorageSeed::getDataFromCurrent(const int &storage, const int &main) const
{
	if (m_currentMap.isEmpty())
		return {};

	return getData(m_currentMap, storage, main);
}



/**
 * @brief StorageSeed::getData
 * @param map
 * @param storage
 * @param main
 * @return
 */

QList<int> StorageSeed::getData(const QString &map, const int &storage, const int &main) const
{
	return d->m_data.value(map).value(storage).value(main);
}


/**
 * @brief StorageSeed::setData
 * @param question
 * @param storage
 * @param map
 */

void StorageSeed::setData(const QVariantMap &question, const int &storage, const QString &map)
{
	d->setData(question, storage, map);
}



/**
 * @brief StorageSeedPrivate::loadFromFile
 * @return
 */

bool StorageSeedPrivate::loadFromFile()
{
	if (m_fileName.isEmpty())
		return false;

	const auto &ptr = Utils::fileContent(m_fileName);

	if (!ptr) {
		LOG_CWARNING("utils") << "File read error" << m_fileName;
		return false;
	}

	m_data.clear();

	QCborMap map = QCborValue::fromCbor(ptr.value()).toMap();

	for (const auto &[map, slist] : map) {
		for (const auto &[st, mlist] : slist.toMap()) {
			for (const auto &[main, list] : mlist.toMap()) {
				for (const auto &sub : list.toArray())
					record(map.toString(), st.toInteger(), main.toInteger(), sub.toInteger());
			}
		}
	}

	LOG_CDEBUG("utils") << "Storage seed loaded" << m_fileName;

	return true;
}





/**
 * @brief StorageSeedPrivate::saveToFile
 * @return
 */

bool StorageSeedPrivate::saveToFile()
{
	if (m_fileName.isEmpty())
		return false;

	QFile f(m_fileName);

	if (!f.open(QIODevice::WriteOnly)) {
		LOG_CERROR("utils") << "File write error" << m_fileName;
		return false;
	}

	QCborMap map;

	for (const auto &[m, d] : m_data.asKeyValueRange()) {
		QCborMap mapSt;

		for (const auto &[st, sd] : d.asKeyValueRange()) {
			QCborMap mapD;

			for (const auto &[main, subl] : sd.asKeyValueRange()) {
				QCborArray sub;

				for (const int &s : subl)
					sub.append(s);

				// main = [ subs ]
				mapD.insert(main, sub);
			}

			// storage = { mains }
			mapSt.insert(st, mapD);
		}

		// mapUuid = { storages }
		map.insert(m, mapSt);
	}

	LOG_CDEBUG("utils") << "Storage seed saved" << m_fileName;

	f.write(map.toCborValue().toCbor());

	f.close();

	return true;
}


/**
 * @brief StorageSeedPrivate::setData
 * @param question
 * @param storage
 * @param map
 */

void StorageSeedPrivate::setData(const QVariantMap &question, const int &storage, const QString &map)
{
	int main = -1;
	int sub = -1;
	int mainB = -1;
	int subB = -1;
	int eStorage = -1;
	QString eMap;

	QString cMap = map;
	int cStorage = storage;

	if (cMap.isEmpty())
		cMap = q->m_currentMap;

	if (cStorage <= 0)
		cStorage = q->m_currentStorage;


	extract(question, &eStorage, &eMap, &main, &sub, &mainB, &subB);

	if (main <= 0 || sub <= 0) {
		LOG_CTRACE("utils") << "Map seed invalid data" << question;
		return;
	}

	if (cMap.isEmpty()) {
		if (eMap.isEmpty()) {
			LOG_CERROR("utils") << "Map seed data missing" << question;
			return;
		}

		cMap = eMap;
	} else if (!eMap.isEmpty() && cMap != eMap) {
		LOG_CWARNING("utils") << "Map seed data mismatch" << cMap << question;
		cMap = eMap;
	}

	if (cStorage <= 0) {
		if (eStorage <= 0) {
			LOG_CERROR("utils") << "Storage seed data missing" << question;
			return;
		}

		cStorage = eStorage;
	} else if (eStorage > 0 && cStorage != eStorage) {
		LOG_CERROR("utils") << "Storage seed data mismatch" << cStorage << question;
		cStorage = eStorage;
	}

	record(cMap, cStorage, main, sub);

	if (mainB > 0 && subB > 0)
		record(cMap, cStorage, mainB, subB);
}



/**
 * @brief StorageSeedPrivate::extract
 * @param question
 * @param storage
 * @param map
 * @param mainA
 * @param subA
 * @param mainB
 * @param subB
 */

void StorageSeedPrivate::extract(const QVariantMap &question, int *storage, QString *map, int *mainA, int *subA, int *mainB, int *subB)
{
	Q_ASSERT(storage);
	Q_ASSERT(map);
	Q_ASSERT(mainA);
	Q_ASSERT(mainB);
	Q_ASSERT(subA);
	Q_ASSERT(subB);

	if (question.contains(QStringLiteral("seedMain")))
		*mainA = question.value(QStringLiteral("seedMain")).toInt();

	if (question.contains(QStringLiteral("seedSub")))
		*subA = question.value(QStringLiteral("seedSub")).toInt();

	if (question.contains(QStringLiteral("seedStorage")))
		*storage = question.value(QStringLiteral("seedStorage")).toInt();

	if (question.contains(QStringLiteral("seedMap")))
		*map = question.value(QStringLiteral("seedMap")).toString();

	if (question.contains(QStringLiteral("seedMainB")))
		*mainB = question.value(QStringLiteral("seedMainB")).toInt();

	if (question.contains(QStringLiteral("seedSubB")))
		*subB = question.value(QStringLiteral("seedSubB")).toInt();
}




/**
 * @brief StorageSeedPrivate::record
 * @param map
 * @param storage
 * @param main
 * @param sub
 */

void StorageSeedPrivate::record(const QString &map, const int &storage, const int &main, const int &sub)
{
	QList<int> &list = m_data[map][storage][main];
	list.removeAll(sub);
	list.append(sub);
}


/**
 * @brief StorageSeedPrivate::clear
 * @param main
 */

void StorageSeedPrivate::clear(const QString &map, const int &storage, const int &main)
{
	m_data[(map.isEmpty() ? q->m_currentMap : map)]
			[(storage <= 0 ? q->m_currentStorage : storage)]
			[main]
			.clear();
}






/**
 * @brief SeedHelper::SeedHelper
 * @param main
 * @param seed
 */

SeedHelper::SeedHelper(const int &main, StorageSeed *seed)
	: m_seed(seed)
	, m_main(main)
{

}


/**
 * @brief SeedHelper::SeedHelper
 * @param seed
 * @param main
 */


SeedHelper::SeedHelper(StorageSeed *seed, const int &main)
	: SeedHelper(main, seed)
{
	if (seed)
		m_data = seed->getDataFromCurrent(main);
}


/**
 * @brief SeedHelper::SeedHelper
 * @param seed
 * @param storage
 * @param main
 */

SeedHelper::SeedHelper(StorageSeed *seed, const int &storage, const int &main)
	: SeedHelper(main, seed)
{
	if (seed)
		m_data = seed->getDataFromCurrent(storage, main);

	m_storage = storage;
}

/**
 * @brief SeedHelper::SeedHelper
 * @param seed
 * @param map
 * @param storage
 * @param main
 */

SeedHelper::SeedHelper(StorageSeed *seed, const QString &map, const int &storage, const int &main)
	: SeedHelper(main, seed)
{
	if (seed)
		m_data = seed->getData(map, storage, main);

	m_map = map;
	m_storage = storage;
}





/**
 * @brief SeedHelper::append
 * @param map
 * @param sub
 * @param main
 * @return
 */

SeedHelper &SeedHelper::append(const QVariantMap &map, const int &sub, const int &main)
{
	if (m_data.contains(sub))
		StorageSeed::addSeedToMap(&m_itemUsed.emplace_back(map), main > -1 ? main : m_main, sub);
	else
		StorageSeed::addSeedToMap(&m_itemReady.emplace_back(map), main > -1 ? main : m_main, sub);

	return *this;
}


/**
 * @brief SeedHelper::getVariantList
 * @return
 */

QVariantList SeedHelper::getVariantList(const bool &autoClean)
{
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(m_itemReady.begin(), m_itemReady.end(), g);
	std::shuffle(m_itemUsed.begin(), m_itemUsed.end(), g);

	QVariantList list;


	// Valójában csak egyet adunk vissza, mert ha seed-del hívtuk meg, akkor úgyis mindig újragyártjuk és csak az elsőt használjuk fel

	if (m_seed) {
		if (!m_itemReady.empty())
			list.append(m_itemReady.front());

		if (list.empty() && !m_itemUsed.empty())
			list.append(m_itemUsed.front());

	} else {
		for (const QVariantMap &m : m_itemReady)
			list.append(m);


		for (const QVariantMap &m : m_itemUsed)
			list.append(m);
	}

	if (autoClean && m_itemReady.empty() && !m_itemUsed.empty()) {
		m_data.clear();

		if (m_seed)
			m_seed->d->clear(m_map, m_storage, m_main);
	}

	return list;
}






/**
 * @brief SeedDuplexHelper::SeedDuplexHelper
 * @param mainA
 * @param mainB
 * @param seed
 */

SeedDuplexHelper::SeedDuplexHelper(const int &mainA, const int &mainB, StorageSeed *seed)
	: m_seed(seed)
	, m_mainA(mainA)
	, m_mainB(mainB)
{
}


/**
 * @brief SeedDuplexHelper::SeedDuplexHelper
 * @param seed
 * @param mainA
 * @param mainB
 */

SeedDuplexHelper::SeedDuplexHelper(StorageSeed *seed, const int &mainA, const int &mainB)
	: SeedDuplexHelper(mainA, mainB, seed)
{
	if (seed) {
		m_dataA = seed->getDataFromCurrent(mainA);
		m_dataB = seed->getDataFromCurrent(mainB);
	}
}



/**
 * @brief SeedDuplexHelper::SeedDuplexHelper
 * @param seed
 * @param storage
 * @param mainA
 * @param mainB
 */

SeedDuplexHelper::SeedDuplexHelper(StorageSeed *seed, const int &storage, const int &mainA, const int &mainB)
	: SeedDuplexHelper(mainA, mainB, seed)
{
	if (seed) {
		m_dataA = seed->getDataFromCurrent(storage, mainA);
		m_dataB = seed->getDataFromCurrent(storage, mainB);
	}

	m_storage = storage;
}



/**
 * @brief SeedDuplexHelper::SeedDuplexHelper
 * @param seed
 * @param map
 * @param storage
 * @param mainA
 * @param mainB
 */

SeedDuplexHelper::SeedDuplexHelper(StorageSeed *seed, const QString &map, const int &storage, const int &mainA, const int &mainB)
	: SeedDuplexHelper(mainA, mainB, seed)
{
	if (seed) {
		m_dataA = seed->getData(map, storage, mainA);
		m_dataB = seed->getData(map, storage, mainB);
	}

	m_map = map;
	m_storage = storage;
}




/**
 * @brief SeedDuplexHelper::append
 * @param map
 * @param mainA
 * @param subA
 * @param mainB
 * @param subB
 * @return
 */

SeedDuplexHelper &SeedDuplexHelper::append(const QVariantMap &map, const int &subA, const int &subB, const int &mainA, const int &mainB)
{
	if (m_dataA.contains(subA) && m_dataB.contains(subB))
		StorageSeed::addSeedToMap(&m_itemUsedDouble.emplace_back(map),
								  mainA > -1 ? mainA : m_mainA, subA,
								  mainB > -1 ? mainB : m_mainB, subB);

	else if (m_dataA.contains(subA))
		StorageSeed::addSeedToMap(&m_itemUsedA.emplace_back(map),
								  mainA > -1 ? mainA : m_mainA, subA,
								  mainB > -1 ? mainB : m_mainB, subB);

	else if (m_dataB.contains(subB))
		StorageSeed::addSeedToMap(&m_itemUsedB.emplace_back(map),
								  mainA > -1 ? mainA : m_mainA, subA,
								  mainB > -1 ? mainB : m_mainB, subB);

	else
		StorageSeed::addSeedToMap(&m_itemReady.emplace_back(map),
								  mainA > -1 ? mainA : m_mainA, subA,
								  mainB > -1 ? mainB : m_mainB, subB);

	return *this;
}



/**
 * @brief SeedDuplexHelper::getSubB
 * @param from
 * @param to
 * @return
 */

int SeedDuplexHelper::getSubB(const int &from, const int &to) const
{
	Q_ASSERT(to>from);

	if (m_dataB.empty())
		return QRandomGenerator::global()->bounded(from, to);

	std::vector<int> list;

	for (int i=from; i<to; ++i) {
		if (!m_dataB.contains(i))
			list.emplace_back(i);
	}

	if (list.empty())
		return QRandomGenerator::global()->bounded(from, to);

	return list.at(QRandomGenerator::global()->bounded((int) list.size()));
}





/**
 * @brief SeedDuplexHelper::getSubB
 * @param list
 * @return
 */

int SeedDuplexHelper::getSubB(const QList<int> &list) const
{
	Q_ASSERT(!list.isEmpty());

	std::vector<int> l;

	for (const int &i : list) {
		if (!m_dataB.contains(i))
			l.emplace_back(i);
	}

	if (l.empty())
		return list.at(QRandomGenerator::global()->bounded((int) list.size()));

	return l.at(QRandomGenerator::global()->bounded((int) l.size()));
}





/**
 * @brief SeedDuplexHelper::getVariantList
 * @param autoClean
 * @return
 */

QVariantList SeedDuplexHelper::getVariantList(const bool &autoClean)
{
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(m_itemReady.begin(), m_itemReady.end(), g);
	std::shuffle(m_itemUsedA.begin(), m_itemUsedA.end(), g);
	std::shuffle(m_itemUsedB.begin(), m_itemUsedB.end(), g);
	std::shuffle(m_itemUsedDouble.begin(), m_itemUsedDouble.end(), g);

	QVariantList list;


	// Valójában csak egyet adunk vissza, mert ha seed-del hívtuk meg, akkor úgyis mindig újragyártjuk és csak az elsőt használjuk fel

	if (m_seed) {
		if (!m_itemReady.empty())
			list.append(m_itemReady.front());

		if (list.empty() && !m_itemUsedB.empty())
			list.append(m_itemUsedB.front());


		if (list.empty() && !m_itemUsedA.empty())
			list.append(m_itemUsedA.front());

		if (list.empty() && !m_itemUsedDouble.empty())
			list.append(m_itemUsedDouble.front());


	} else {
		for (const QVariantMap &m : m_itemReady)
			list.append(m);

		for (const QVariantMap &m : m_itemUsedB)
			list.append(m);

		for (const QVariantMap &m : m_itemUsedA)
			list.append(m);

		for (const QVariantMap &m : m_itemUsedDouble)
			list.append(m);
	}


	if (autoClean && m_itemReady.empty() && (!m_itemUsedA.empty() || !m_itemUsedB.empty() || !m_itemUsedDouble.empty())) {

		if (m_itemUsedB.empty()) {
			m_dataA.clear();

			if (m_seed)
				m_seed->d->clear(m_map, m_storage, m_mainA);
		}

		if (m_itemUsedA.empty()) {
			m_dataB.clear();

			if (m_seed)
				m_seed->d->clear(m_map, m_storage, m_mainB);
		}
	}

	return list;
}

