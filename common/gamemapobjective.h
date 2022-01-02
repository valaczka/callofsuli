/*
 * ---- Call of Suli ----
 *
 * gamemapobjective.h
 *
 * Created on: 2022. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapObjective
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

#ifndef GAMEMAPOBJECTIVE_H
#define GAMEMAPOBJECTIVE_H

#include <QObject>
#include <QJsonObject>
#include "gamemapstorage.h"

class GameMapChapter;

class GameMapObjective : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(QString module READ module WRITE setModule NOTIFY moduleChanged)
	Q_PROPERTY(GameMapStorage *storage READ storage WRITE setStorage NOTIFY storageChanged)
	Q_PROPERTY(qint32 storageCount READ storageCount WRITE setStorageCount NOTIFY storageCountChanged)
	Q_PROPERTY(QVariantMap data READ data WRITE setData NOTIFY dataChanged)
	Q_PROPERTY(GameMapChapter *chapter READ chapter WRITE setChapter NOTIFY chapterChanged)

public:
	explicit GameMapObjective(QObject *parent = nullptr);
	virtual ~GameMapObjective();

	QJsonObject toJsonObject() const;

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	const QString &module() const;
	void setModule(const QString &newModule);

	GameMapStorage *storage() const;
	void setStorage(GameMapStorage *newStorage);

	qint32 storageCount() const;
	void setStorageCount(qint32 newStorageCount);

	const QVariantMap &data() const;
	void setData(const QVariantMap &newData);

	GameMapChapter *chapter() const;
	void setChapter(GameMapChapter *newChapter);

signals:
	void uuidChanged();
	void moduleChanged();
	void storageChanged();
	void storageCountChanged();
	void dataChanged();

	void chapterChanged();

private:
	QString m_uuid;
	QString m_module;
	GameMapStorage *m_storage;
	qint32 m_storageCount;
	QVariantMap m_data;
	GameMapChapter *m_chapter;

};

#endif // GAMEMAPOBJECTIVE_H
