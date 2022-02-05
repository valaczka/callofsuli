/*
 * ---- Call of Suli ----
 *
 * mapeditoraction.h
 *
 * Created on: 2022. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditorAction
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

#ifndef MAPEDITORACTION_H
#define MAPEDITORACTION_H

#include <QObject>
#include "editorundostack.h"
#include "editoraction.h"
#include "gamemapeditor.h"
#include "gamemapreaderiface.h"

class MapEditorAction : public EditorAction
{
	Q_OBJECT

	Q_PROPERTY(MapEditorActionType type READ type WRITE setType NOTIFY typeChanged)
	Q_PROPERTY(QVariant contextId READ contextId WRITE setContextId NOTIFY contextIdChanged)

public:
	enum MapEditorActionType {
		ActionTypeInvalid		= 0,
		ActionTypeChapterList	= 0x01,
		ActionTypeChapter		= 0x02,
		ActionTypeMissionList	= 0x04,
		ActionTypeMission		= 0x08,
		ActionTypeMissionLevel	= 0x10,
		ActionTypeInventory		= 0x20,
		ActionTypeImageList		= 0x40,
		ActionTypeImage			= 0x80
	};

	Q_ENUM(MapEditorActionType);
	Q_DECLARE_FLAGS(MapEditorActionTypes, MapEditorActionType)
	Q_FLAG(MapEditorActionTypes)

	explicit MapEditorAction(GameMapEditor *editor, const MapEditorActionType &type, const QVariant &contextId = QVariant::Invalid);

	const MapEditorActionType &type() const;
	void setType(const MapEditorActionType &newType);

	const QVariant &contextId() const;
	void setContextId(const QVariant &newContextId);

signals:
	void typeChanged();
	void contextIdChanged();

protected:
	void chapterAdd(GameMapEditorChapter *chapter);
	void chapterRemove(GameMapEditorChapter *chapter);

	void objectiveAdd(GameMapEditorChapter *chapter, GameMapEditorObjective *objective);
	void objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective);

	MapEditorActionType m_type;
	QVariant m_contextId;
	GameMapEditor *m_editor;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MapEditorAction::MapEditorActionTypes)



/**
 * @brief The MapEditorActionObjectiveNew class
 */


class MapEditorActionObjectiveNew : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionObjectiveNew(GameMapEditor *editor, GameMapEditorChapter *parentChapter,
										 const QString &uuid,
										 const QString &module, const qint32 &storageId,
										 const qint32 &storageCount, const QVariantMap &data);
	virtual ~MapEditorActionObjectiveNew();

private:
	GameMapEditorObjective *m_objective;
	GameMapEditorChapter *m_parentChapter;

};



/**
 * @brief The MapEditorActionChapterNew class
 */


class MapEditorActionChapterNew : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionChapterNew(GameMapEditor *editor, const qint32 &id, const QString &name);
	virtual ~MapEditorActionChapterNew();

private:
	GameMapEditorChapter *m_chapter;

};


/**
 * @brief The MapEditorActionChapterRemove class
 */

class MapEditorActionChapterRemove : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionChapterRemove(GameMapEditor *editor, GameMapEditorChapter *chapter);
	virtual ~MapEditorActionChapterRemove();

private:
	GameMapEditorChapter *m_chapter;
	QList<QPointer<GameMapEditorMissionLevel>> m_levels;

};

#endif // MAPEDITORACTION_H
