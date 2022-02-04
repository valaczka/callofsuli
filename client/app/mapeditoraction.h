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

#include "editoraction.h"
#include "gamemapeditor.h"
#include "gamemapreaderiface.h"

class MapEditorAction : public EditorAction
{

public:
	enum MapEditorActionType {
		ActionTypeInvalid,
		ActionTypeChapterList,
		ActionTypeChapter,
		ActionTypeMissionList,
		ActionTypeMission,
		ActionTypeMissionLevel,
		ActionTypeInventory,
		ActionTypeImageList,
		ActionTypeImage
	};

	explicit MapEditorAction(GameMapEditor *editor, const MapEditorActionType &type, void *data = nullptr);

	MapEditorActionType type() const;
	void setType(MapEditorActionType newType);

protected:
	void chapterAdd(GameMapEditorChapter *chapter);
	void chapterRemove(GameMapEditorChapter *chapter);

	void objectiveAdd(GameMapEditorChapter *chapter, GameMapEditorObjective *objective);
	void objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective);

	MapEditorActionType m_type;
	GameMapEditor *m_editor;
};




/**
 * @brief The MapEditorActionObjectiveNew class
 */


class MapEditorActionObjectiveNew : public MapEditorAction
{
public:
	explicit MapEditorActionObjectiveNew(GameMapEditor *editor,
										 const QString &module, const qint32 &storageId,
										 const qint32 &storageCount, const QVariantMap &data);

private:
	QString m_uuid;
	QString m_module;
	qint32 m_storageId;
	qint32 m_storageCount;
	QVariantMap m_data;
	GameMapEditorObjective *m_objective;

};



/**
 * @brief The MapEditorActionChapterNew class
 */


class MapEditorActionChapterNew : public MapEditorAction
{
public:
	explicit MapEditorActionChapterNew(GameMapEditor *editor, const QString &name);

private:
	QString m_name;
	GameMapEditorChapter *m_chapter;

};


/**
 * @brief The MapEditorActionChapterRemove class
 */

class MapEditorActionChapterRemove : public MapEditorAction
{
public:
	explicit MapEditorActionChapterRemove(GameMapEditor *editor, GameMapEditorChapter *chapter);

private:
	QString m_name;
	GameMapEditorChapter *m_chapter;

};

#endif // MAPEDITORACTION_H
