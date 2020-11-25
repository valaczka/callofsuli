/*
 * ---- Call of Suli ----
 *
 * gamemap.cpp
 *
 * Created on: 2020. 11. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMap
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gamemap.h"

#include <QDataStream>

GameMap::GameMap(const QByteArray &uuid, const QString &name)
	: m_uuid(uuid)
	, m_name(name)
	, m_campaigns()
	, m_chapters()
{

}


/**
 * @brief GameMap::~GameMap
 */

GameMap::~GameMap()
{
	qDeleteAll(m_campaigns.begin(), m_campaigns.end());
	m_campaigns.clear();

	qDeleteAll(m_chapters.begin(), m_chapters.end());
	m_chapters.clear();
}


/**
 * @brief GameMap::fromBinaryData
 * @param data
 * @return
 */

GameMap *GameMap::fromBinaryData(const QByteArray &data)
{
	QDataStream stream(data);

	quint32 magic;
	QByteArray str;

	stream >> magic >> str;

	if (magic != 0x434F53 || str != "MAP") {			// COS
		qWarning() << "Invalid map data";
		return nullptr;
	}

	quint16 version;

	stream >> version;

	stream.setVersion(QDataStream::Qt_5_11);

	QByteArray uuid;
	QString	name;

	stream >> uuid >> name;

	GameMap *map = new GameMap(uuid, name);
	map->chaptersFromStream(stream);

	map->m_chapters.append(new Chapter(map->m_chapters.size()+1, "legujabb"));

	return map;
}


/**
 * @brief GameMap::toBinaryData
 * @return
 */

QByteArray GameMap::toBinaryData() const
{
	QByteArray s;
	QDataStream stream(&s, QIODevice::WriteOnly);

	stream << (quint32) 0x434F53;			// COS
	stream << QByteArray("MAP");
	stream << (quint16) 1;

	stream.setVersion(QDataStream::Qt_5_11);

	stream << m_uuid;
	stream << m_name;

	chaptersToStream(stream);

	return s;
}



/**
 * @brief GameMap::fromDb
 * @return
 */

GameMap *GameMap::fromDb()
{
	GameMap *map = new GameMap("----sdfsdf---", "weuroiweuroiwer");

	map->m_chapters.append(new GameMap::Chapter(5, "Fejezet 1"));
	map->m_chapters.append(new GameMap::Chapter(15, "Fejezet 12"));
	map->m_chapters.append(new GameMap::Chapter(4, "Fejezet 3"));

	return map;
}





/**
 * @brief GameMap::chaptersToStream
 * @param stream
 */

void GameMap::chaptersToStream(QDataStream &stream) const
{
	stream << (quint32) m_chapters.size();

	foreach (GameMap::Chapter *ch, m_chapters) {
		stream << ch->id();
		stream << ch->name();
	}
}


/**
 * @brief GameMap::chaptersFromStream
 * @param stream
 */

void GameMap::chaptersFromStream(QDataStream &stream)
{
	quint32 size;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		quint16 id;
		QString name;
		stream >> id >> name;

		Chapter *ch = new Chapter(id, name);

		m_chapters.append(ch);
	}
}





/**
 * @brief GameMap::Storage::Storage
 * @param id
 * @param module
 * @param data
 */

GameMap::Storage::Storage(const quint16 &id, const QByteArray &module, const QVariantMap &data)
	: m_id(id)
	, m_module(module)
	, m_data(data)
{

}


/**
 * @brief GameMap::Objective::Objective
 * @param uuid
 * @param module
 * @param storage
 * @param data
 */

GameMap::Objective::Objective(const QByteArray &uuid, const QByteArray &module, GameMap::Storage *storage, const QVariantMap &data)
	: m_uuid(uuid)
	, m_module(module)
	, m_storage(storage)
	, m_data(data)
{

}



/**
 * @brief GameMap::Chapter::Chapter
 * @param id
 * @param name
 */

GameMap::Chapter::Chapter(const quint16 &id, const QString &name)
	: m_id(id)
	, m_name(name)
	, m_storages()
	, m_objectives()
{

}


/**
 * @brief GameMap::Chapter::~Chapter
 */

GameMap::Chapter::~Chapter()
{
	qDeleteAll(m_storages.begin(), m_storages.end());
	m_storages.clear();

	qDeleteAll(m_objectives.begin(), m_objectives.end());
	m_objectives.clear();
}


/**
 * @brief GameMap::BlockChapterMap::BlockChapterMap
 * @param block
 * @param maxObjective
 */

GameMap::BlockChapterMap::BlockChapterMap(const quint16 &maxObjective)
	: m_blocks()
	, m_chapters()
	, m_favorites()
	, m_maxObjective(maxObjective)
{

}


/**
 * @brief GameMap::Inventory::Inventory
 * @param block
 * @param module
 * @param count
 */

GameMap::Inventory::Inventory(const quint16 &block, const QByteArray &module, const quint16 &count)
	: m_block(block)
	, m_module(module)
	, m_count(count)
{

}


/**
 * @brief GameMap::MissionLevel::MissionLevel
 * @param missionUuid
 * @param level
 * @param terrain
 * @param startHP
 * @param duration
 * @param startBlock
 */

GameMap::MissionLevel::MissionLevel(const QByteArray &missionUuid, const quint16 &level, const QByteArray &terrain,
									const quint16 &startHP, const quint16 &duration, const quint16 &startBlock)
	: m_missionUuid(missionUuid)
	, m_level(level)
	, m_terrain(terrain)
	, m_startHP(startHP)
	, m_duration(duration)
	, m_startBlock(startBlock)
	, m_blockChapterMaps()
	, m_inventories()

{

}


/**
 * @brief GameMap::MissionLevel::~MissionLevel
 */

GameMap::MissionLevel::~MissionLevel()
{
	qDeleteAll(m_blockChapterMaps.begin(), m_blockChapterMaps.end());
	m_blockChapterMaps.clear();

	qDeleteAll(m_inventories.begin(), m_inventories.end());
	m_inventories.clear();
}



/**
 * @brief GameMap::Mission::Mission
 * @param uuid
 * @param mandatory
 * @param name
 */

GameMap::Mission::Mission(const QByteArray &uuid, const bool &mandatory, const QString &name)
	: m_uuid(uuid)
	, m_mandatory(mandatory)
	, m_name(name)
	, m_levels()
	, m_locksMission()
	, m_locksMissionLevel()
{

}


/**
 * @brief GameMap::Mission::~Mission
 */

GameMap::Mission::~Mission()
{
	qDeleteAll(m_levels.begin(), m_levels.end());
	m_levels.clear();
}


/**
 * @brief GameMap::Campaign::Campaign
 * @param id
 * @param name
 */

GameMap::Campaign::Campaign(const quint16 &id, const QString &name)
	: m_id(id)
	, m_name(name)
	, m_locks()
	, m_missions()
{

}


/**
 * @brief GameMap::Campaign::~Campaign
 */

GameMap::Campaign::~Campaign()
{
	qDeleteAll(m_missions.begin(), m_missions.end());
	m_missions.clear();
}




/**
 * @brief GameMap::Image::Image
 * @param folder
 * @param file
 * @param data
 */

GameMap::Image::Image(const QString &folder, const QString &file, const QByteArray &data)
	: m_folder(folder)
	, m_file(file)
	, m_data(data)
{

}
