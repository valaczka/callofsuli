/*
 * ---- Call of Suli ----
 *
 * gamemapnew.h
 *
 * Created on: 2022. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapNew
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

#ifndef GAMEMAPNEW_H
#define GAMEMAPNEW_H

#include <QObject>
#include <QJsonDocument>
#include "gamemapstorage.h"
#include "gamemapchapter.h"
#include "gamemapmission.h"

#define GAMEMAP_CURRENT_VERSION 10

class GameMapNew : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(QList<GameMapStorage *> storages READ storages WRITE setStorages NOTIFY storagesChanged)
	Q_PROPERTY(QList<GameMapChapter *> chapters READ chapters WRITE setChapters NOTIFY chaptersChanged)
	Q_PROPERTY(QList<GameMapMission *> missions READ missions WRITE setMissions NOTIFY missionsChanged)

public:
	explicit GameMapNew(QObject *parent = nullptr);
	virtual ~GameMapNew();

	static GameMapNew* fromBinaryData(const QByteArray &data, QObject *parent = nullptr);
	QByteArray toBinaryData() const;
	QJsonDocument toJsonDocument() const;

	qint32 version() const;

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	const QList<GameMapStorage *> &storages() const;
	void setStorages(const QList<GameMapStorage *> &newStorages);

	const QList<GameMapChapter *> &chapters() const;
	void setChapters(const QList<GameMapChapter *> &newChapters);

	const QList<GameMapMission *> &missions() const;
	void setMissions(const QList<GameMapMission *> &newMissions);

	void storageAdd(GameMapStorage *storage);
	int storageRemove(GameMapStorage *storage);
	GameMapStorage *storage(const qint32 &id) const;

	void chapterAdd(GameMapChapter *chapter);
	int chapterRemove(GameMapChapter *chapter);
	GameMapChapter *chapter(const qint32 &id) const;

	void missionAdd(GameMapMission *mission);
	int missionRemove(GameMapMission *mission);
	//GameMapChapter *chapter(const qint32 &id) const;


signals:
	void uuidChanged();
	void storagesChanged();
	void chaptersChanged();

	void missionsChanged();

private:
	bool storagesFromStream(QDataStream &stream);
	void storagesToStream(QDataStream &stream) const;

	bool chaptersFromStream(QDataStream &stream);
	void chaptersToStream(QDataStream &stream) const;

	bool missionsFromStream(QDataStream &stream);
	void missionsToStream(QDataStream &stream) const;

	qint32 m_version;
	QString m_uuid;
	QList<GameMapStorage *> m_storages;
	QList<GameMapChapter *> m_chapters;
	QList<GameMapMission *> m_missions;
};


#endif // GAMEMAPNEW_H
