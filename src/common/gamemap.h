/*
 * ---- Call of Suli ----
 *
 * gamemap.h
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

#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <QVariant>
#include <QDebug>

class GameMap
{

public:

	class Storage {
	public:
		Storage(const quint16 &id, const QByteArray &module, const QVariantMap &data);

		quint16 id() const { return m_id; };
		QByteArray module() const { return m_module; };
		QVariantMap data() const { return m_data; };

	private:
		quint16 m_id;
		QByteArray m_module;
		QVariantMap m_data;
	};


	class Objective {
	public:
		Objective(const QByteArray &uuid, const QByteArray &module, Storage *storage, const QVariantMap &data);

		QByteArray uuid() const { return m_uuid; };
		QByteArray module() const { return m_module; };
		Storage *storage() const { return m_storage; };
		QVariantMap data() const { return m_data; };

	private:
		QByteArray m_uuid;
		QByteArray m_module;
		Storage *m_storage;
		QVariantMap m_data;
	};


	class Chapter {
	public:
		Chapter(const quint16 &id, const QString &name);
		~Chapter();

		quint16 id() const { return m_id; };
		QString name() const { return m_name; };
		QVector<Storage *> storages() const { return m_storages; };
		QVector<Objective *> objectives() const { return m_objectives; };

	private:
		quint16 m_id;
		QString m_name;
		QVector<Storage *> m_storages;
		QVector<Objective *> m_objectives;
	};


	class BlockChapterMap {
	public:
		BlockChapterMap(const quint16 &maxObjective);

		QVector<quint16> blocks() const { return m_blocks; };
		QVector<Chapter *> chapters() const { return m_chapters; };
		QVector<Objective *> favorites() const { return m_favorites; };
		quint16 maxObjective() const { return m_maxObjective; };

	private:
		QVector<quint16> m_blocks;
		QVector<Chapter *> m_chapters;
		QVector<Objective *> m_favorites;
		quint16 m_maxObjective;
	};


	class Inventory {
	public:
		Inventory(const quint16 &block, const QByteArray &module, const quint16 &count);

		quint16 block() const { return m_block; };
		QByteArray module() const { return m_module; };
		quint16 count() const { return m_count; };

	private:
		quint16 m_block;
		QByteArray m_module;
		quint16 m_count;
	};


	class MissionLevel {
	public:
		MissionLevel(const QByteArray &missionUuid, const quint16 &level, const QByteArray &terrain,
					 const quint16 &startHP, const quint16 &duration, const quint16 &startBlock);
		~MissionLevel();

		QByteArray missionUuid() const { return m_missionUuid; };
		quint16 level() const { return m_level; };
		QByteArray terrain() const { return m_terrain; };
		quint16 startHP() const { return m_startHP; };
		quint16 duration() const { return m_duration; };
		quint16 startBlock() const { return m_startBlock; };
		QVector<BlockChapterMap *> blockChapterMaps() const { return m_blockChapterMaps; };
		QVector<Inventory *> inventories() const { return m_inventories; };

	private:
		QByteArray m_missionUuid;
		quint16 m_level;
		QByteArray m_terrain;
		quint16 m_startHP;
		quint16 m_duration;
		quint16 m_startBlock;
		QVector<BlockChapterMap *> m_blockChapterMaps;
		QVector<Inventory *> m_inventories;
	};


	class Mission {
	public:
		Mission(const QByteArray &uuid, const bool &mandatory, const QString &name);
		~Mission();

		QByteArray uuid() const { return m_uuid;};
		bool mandatory() const { return m_mandatory;};
		QString name() const { return m_name;};
		QVector<MissionLevel *> levels() const { return m_levels; };
		QVector<Mission *> locksMission() const { return m_locksMission; };
		QVector<MissionLevel *> locksMissionLevel() const { return m_locksMissionLevel; };

	private:
		QByteArray m_uuid;
		bool m_mandatory;
		QString m_name;
		QVector<MissionLevel *> m_levels;
		QVector<Mission *> m_locksMission;
		QVector<MissionLevel *> m_locksMissionLevel;
	};


	class Campaign {
	public:
		Campaign(const quint16 &id, const QString &name);
		~Campaign();

		quint16 id() const { return m_id;};
		QString name() const { return m_name;};
		QVector<Campaign *> locks() const { return m_locks;};
		QVector<Mission *> missions() const { return m_missions;};

	private:
		quint16 m_id;
		QString m_name;
		QVector<Campaign *> m_locks;
		QVector<Mission *> m_missions;
	};


	class Image {
	public:
		Image(const QString &folder, const QString &file, const QByteArray &data);

		QString folder() const { return m_folder;};
		QString file() const { return  m_file; };
		QByteArray data() const { return m_data; };

	private:
		QString m_folder;
		QString m_file;
		QByteArray m_data;
	};


	GameMap(const QByteArray &uuid = QByteArray(), const QString &name = QString());
	~GameMap();

	static GameMap* fromBinaryData(const QByteArray &data);
	QByteArray toBinaryData() const;

	static GameMap* fromDb();

	QByteArray uuid() const { return m_uuid; }
	QString name() const { return m_name; }
	QVector<Campaign *> campaigns() const { return m_campaigns; }
	QVector<Chapter *> chapters() const { return m_chapters; }
	QVector<Image *> images() const { return m_images; }

private:
	void chaptersToStream(QDataStream &stream) const;
	void chaptersFromStream(QDataStream &stream);

	QByteArray m_uuid;
	QString m_name;
	QVector<Campaign *> m_campaigns;
	QVector<Chapter *> m_chapters;
	QVector<Image *> m_images;
};




#endif // GAMEMAP_H
