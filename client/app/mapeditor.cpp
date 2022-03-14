/*
 * ---- Call of Suli ----
 *
 * mapeditor.cpp
 *
 * Created on: 2022. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditor
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

#include <QtJsonSerializer>

#include "mapeditor.h"
#include "editoraction.h"
#include "mapeditoraction.h"

MapEditor::MapEditor(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_editor(nullptr)
	, m_undoStack(new EditorUndoStack(this))
	, m_url()
	, m_availableObjectives()
	, m_missionLevelModel(new ObjectGenericListModel<MapEditorMissionLevelObject>(this))
{
	// Load available objectives

	QHash<QString, ModuleInterface *> omlist =  Client::moduleObjectiveList();
	QHash<QString, ModuleInterface *>::const_iterator omit;

	for (omit = omlist.constBegin(); omit != omlist.constEnd(); ++omit) {
		m_availableObjectives.append(QVariantMap({
													 { "module", omit.key() },
													 { "name", omit.value()->readableName() },
													 { "icon", omit.value()->icon() },
													 { "storageModules", omit.value()->storageModules() }
												 }));
	}


	connect(m_undoStack, &EditorUndoStack::undoCompleted, this, &MapEditor::onUndoRedoCompleted);
	connect(m_undoStack, &EditorUndoStack::redoCompleted, this, &MapEditor::onUndoRedoCompleted);
}


/**
 * @brief MapEditor::~MapEditor
 */

MapEditor::~MapEditor()
{
	delete m_missionLevelModel;
}


/**
 * @brief MapEditor::editor
 * @return
 */

GameMapEditor *MapEditor::editor() const
{
	return m_editor;
}


void MapEditor::setEditor(GameMapEditor *newEditor)
{
	if (m_editor == newEditor)
		return;
	m_editor = newEditor;
	emit editorChanged();
}


EditorUndoStack *MapEditor::undoStack() const
{
	return m_undoStack;
}

void MapEditor::setUndoStack(EditorUndoStack *newUndoStack)
{
	if (m_undoStack == newUndoStack)
		return;
	m_undoStack = newUndoStack;
	emit undoStackChanged();
}



/**
 * @brief MapEditor::open
 * @param url
 */

void MapEditor::open(const QUrl &url)
{
	if (m_editor) {
		Client::clientInstance()->sendMessageWarning(tr("Pálya megnyitás"), tr("Már meg van nyitva egy pálya!"), m_url.toLocalFile());
		return;
	}

	GameMapEditor *e = GameMapEditor::fromBinaryData(Client::fileContent(url.toLocalFile()), this);

	if (!e) {
		Client::clientInstance()->sendMessageWarning(tr("Hibás pálya"), tr("Nem lehet megnyitni a fájlt!"), url.toLocalFile());
		return;
	}

	setEditor(e);
	setUrl(url);
	m_undoStack->clear();
	setDisplayName(url.toLocalFile());
}


/**
 * @brief MapEditor::create
 */

void MapEditor::create()
{
	if (m_editor) {
		Client::clientInstance()->sendMessageWarning(tr("Új pálya"), tr("Már meg van nyitva egy pálya!"), m_url.toLocalFile());
		return;
	}

	GameMapEditor *e = new GameMapEditor(this);
	setEditor(e);
	setUrl(QUrl());
	m_undoStack->clear();
	setDisplayName(tr("-- Új pálya --"));
}



/**
 * @brief MapEditor::close
 */

void MapEditor::close()
{
	setUrl(QUrl());
	setDisplayName("");
	m_undoStack->clear();

	if (m_editor) {
		m_editor->deleteLater();
		setEditor(nullptr);
	}
}


/**
 * @brief MapEditor::url
 * @return
 */

const QUrl &MapEditor::url() const
{
	return m_url;
}

void MapEditor::setUrl(const QUrl &newUrl)
{
	if (m_url == newUrl)
		return;
	m_url = newUrl;
	emit urlChanged();
}

const QString &MapEditor::displayName() const
{
	return m_displayName;
}

void MapEditor::setDisplayName(const QString &newDisplayName)
{
	if (m_displayName == newDisplayName)
		return;
	m_displayName = newDisplayName;
	emit displayNameChanged();
}



/**
 * @brief MapEditor::chapterAdd
 * @param data
 */

void MapEditor::chapterAdd(QVariantMap data)
{
	int id = 1;

	foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects())
		id = qMax(ch->id()+1, id);

	data.insert("id", id);

	m_undoStack->call(new MapEditorActionChapterNew(m_editor, data));

}


/**
 * @brief MapEditor::chapterRemove
 * @param chapter
 */

void MapEditor::chapterRemove(GameMapEditorChapter *chapter)
{
	if (!chapter)
		return;

	m_undoStack->call(new MapEditorActionChapterRemove(m_editor, chapter));
}


/**
 * @brief MapEditor::chapterRemove
 * @param list
 */

void MapEditor::chapterRemoveList(const QList<GameMapEditorChapter *> &list)
{
	if (list.isEmpty())
		return;

	m_undoStack->call(new MapEditorActionChapterRemove(m_editor, list));
}

/**
 * @brief MapEditor::chapterModify
 * @param chapter
 * @param data
 */

void MapEditor::chapterModify(GameMapEditorChapter *chapter, const QVariantMap &data)
{
	if (!chapter)
		return;

	m_undoStack->call(new MapEditorActionChapterModify(m_editor, chapter, data));
}



/**
 * @brief MapEditor::chapterModelUnselectAll
 * @param model
 * @return
 */

bool MapEditor::chapterModelUnselectObjectives(ObjectGenericListModel<GameMapEditorChapter> *model)
{
	if (!model)
		return false;

	bool hasUnselect = false;

	foreach (GameMapEditorChapter *ch, model->objects()) {
		if (ch->objectives()->selectedCount()) {
			hasUnselect = true;
			ch->objectives()->unselectAll();
		}
	}

	return hasUnselect;
}


/**
 * @brief MapEditor::chapterModifyMissionLevels
 * @param chapter
 * @param list
 */

void MapEditor::chapterModifyMissionLevels(GameMapEditorChapter *chapter, const QList<MapEditorMissionLevelObject *> &list)
{
	QList<GameMapEditorMissionLevel*> ml = toMissionLevelList(list);

	// TODO: MapEditorActionChapterMissionLevels

}


/**
 * @brief MapEditor::objectiveAdd
 * @param chapter
 * @param data
 */

void MapEditor::objectiveAdd(GameMapEditorChapter *chapter, const QVariantMap &data, const QVariantMap &storageData)
{
	if (!chapter)
		return;

	m_undoStack->call(new MapEditorActionObjectiveNew(m_editor, chapter, data, storageData));
}


/**
 * @brief MapEditor::objectiveRemove
 * @param chapter
 * @param objective
 */

void MapEditor::objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective)
{
	if (!chapter || !objective)
		return;

	m_undoStack->call(new MapEditorActionObjectiveRemove(m_editor, chapter, objective));
}


/**
 * @brief MapEditor::objectiveRemoveList
 * @param chapter
 * @param list
 */

void MapEditor::objectiveRemoveList(GameMapEditorChapter *chapter, const QList<GameMapEditorObjective *> &list)
{
	if (!chapter || list.isEmpty())
		return;

	m_undoStack->call(new MapEditorActionObjectiveRemove(m_editor, chapter, list));
}


/**
 * @brief MapEditor::objectiveModify
 * @param objective
 * @param data
 */

void MapEditor::objectiveModify(GameMapEditorChapter *chapter, GameMapEditorObjective *objective,
								const QVariantMap &data, const QVariantMap &storageData)
{
	if (!objective)
		return;

	if (objective->storageId() != -1 && !storageData.isEmpty())
		m_undoStack->call(new MapEditorActionObjectiveModify(m_editor, chapter, objective, data,
															 m_editor->storage(objective->storageId()), storageData));
	else
		m_undoStack->call(new MapEditorActionObjectiveModify(m_editor, chapter, objective, data));
}


/**
 * @brief MapEditor::objectiveMoveCopy
 * @param chapter
 * @param isCopy
 * @param objective
 * @param targetChapterId
 * @param newChapterName
 */
void MapEditor::objectiveMoveCopy(GameMapEditorChapter *chapter, const bool &isCopy, GameMapEditorObjective *objective,
								  const int &targetChapterId, const QString &newChapterName)
{
	if (!objective)
		return;

	QVariantMap d;
	d.insert("id", targetChapterId);
	d.insert("name", newChapterName);

	m_undoStack->call(new MapEditorActionObjectiveMove(m_editor, chapter, objective, isCopy, d));
}


/**
 * @brief MapEditor::objectiveMoveCopyList
 * @param chapter
 * @param isCopy
 * @param list
 * @param targetChapterId
 * @param newChapterName
 */

void MapEditor::objectiveMoveCopyList(GameMapEditorChapter *chapter, const bool &isCopy, const QList<GameMapEditorObjective *> &list,
									  const int &targetChapterId, const QString &newChapterName)
{
	QVariantMap d;
	d.insert("id", targetChapterId);
	d.insert("name", newChapterName);

	m_undoStack->call(new MapEditorActionObjectiveMove(m_editor, chapter, list, isCopy, d));
}




/**
 * @brief MapEditor::onStepChanged
 */


void MapEditor::onUndoRedoCompleted(const int &lastStep)
{
	if (m_editor) {
		{
			qDebug() << "=========";
			foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
				qDebug() << ch->id() << ch->name();
			}
			qDebug() << "=========";
		}
	}

	if (m_undoStack->actions().isEmpty()) {
		emit actionContextUpdated(MapEditorAction::ActionTypeInvalid, QVariant::Invalid);
		return;
	}

	int s = qMin(qMax(lastStep, 0), m_undoStack->actions().size());

	EditorAction *a = m_undoStack->actions().at(s);
	MapEditorAction *ma = qobject_cast<MapEditorAction*>(a);

	if (!ma) {
		emit actionContextUpdated(MapEditorAction::ActionTypeInvalid, QVariant::Invalid);
		return;
	}

	emit actionContextUpdated(ma->type(), ma->contextId());
}


/**
 * @brief MapEditor::toMissionLevelList
 * @param list
 * @return
 */

QList<GameMapEditorMissionLevel *> MapEditor::toMissionLevelList(const QList<MapEditorMissionLevelObject *> &list)
{
	QList<GameMapEditorMissionLevel *> ret;

	if (!m_editor)
		return ret;

	ret.reserve(list.size());

	foreach (MapEditorMissionLevelObject *o, list) {
		GameMapEditorMissionLevel *ml = m_editor->missionLevel(o->uuid(), o->level());

		ret.append(ml);
	}

	return ret;
}



/**
 * @brief MapEditor::missionLevelModel
 * @return
 */

ObjectGenericListModel<MapEditorMissionLevelObject> *MapEditor::missionLevelModel() const
{
	return m_missionLevelModel;
}


/**
 * @brief MapEditor::updateMissionLevelModel
 * @param chapter
 */

void MapEditor::updateMissionLevelModel(GameMapEditorChapter *chapter)
{
	if (!m_editor)
		return;

	QList<MapEditorMissionLevelObject*> list;

	foreach (GameMapEditorMission *m, m_editor->missions()->objects()) {
		foreach (GameMapEditorMissionLevel *l, m->levels()->objects()) {
			MapEditorMissionLevelObject *obj = new MapEditorMissionLevelObject(m->uuid(), m->name(), l->level(), this);
			if (chapter && l->chapters()->objects().contains(chapter))
				obj->setSelected(true);
			list.append(obj);
		}
	}

	m_missionLevelModel->resetModel(list);

}


/**
 * @brief MapEditor::availableObjectives
 * @return
 */

const QVariantList &MapEditor::availableObjectives() const
{
	return m_availableObjectives;
}



/**
 * @brief MapEditor::getStorages
 * @return
 */

QVariantList MapEditor::getStorages() const
{
	QVariantList list;

	if (!m_editor)
		return list;

	QHash<QString, ModuleInterface *> mlist =  Client::moduleStorageList();
	QHash<QString, ModuleInterface *>::const_iterator it;

	int id = -1;

	for (it = mlist.constBegin(); it != mlist.constEnd(); ++it) {
		ModuleInterface *iif = it.value();
		list.append(QVariantMap ({
									 { "id", id-- },
									 { "module", it.key() },
									 { "name", "" },
									 { "title", tr("Új %1").arg(iif->readableName()) },
									 { "icon", iif->icon() },
									 { "details", "" },
									 { "storageData", QVariantMap() },
									 { "objectiveCount", 0 }
								 }));
	}

	QList<GameMapEditorStorage *> storages = m_editor->storages()->objects();

	foreach(GameMapEditorStorage *s, storages) {
		QVariantMap m = Question::storageInfo(s->module(), s->data());
		/*
								   { "name", "" },
							   { "icon", "image://font/Material Icons/\ue002" },
							   { "title", QObject::tr("Érvénytelen modul!") },
							   { "details", "" },
							   { "image", "" }
							   */

		m.insert("id", s->id());
		m.insert("module", s->module());
		m.insert("storageData", s->data());

		// objectiveCount

		int objectiveCount = 0;

		foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
			foreach (GameMapEditorObjective *o, ch->objectives()->objects()) {
				if (o->storageId() == s->id())
					objectiveCount++;
			}
		}

		m.insert("objectiveCount", objectiveCount);

		list.append(m);
	}

	return list;
}



/**
 * @brief MapEditor::objectiveQml
 * @param module
 * @return
 */

QString MapEditor::objectiveQml(const QString &module) const
{
	if (!Client::moduleObjectiveList().contains(module))
		return "";

	ModuleInterface *mi = Client::moduleObjectiveList().value(module);

	return mi->qmlEditor();
}




/**
 * @brief MapEditor::storageQml
 * @param module
 * @return
 */

QString MapEditor::storageQml(const QString &module) const
{
	if (!Client::moduleStorageList().contains(module))
		return "";

	ModuleInterface *mi = Client::moduleStorageList().value(module);

	return mi->qmlEditor();
}



/**
 * @brief MapEditorMissionLevelObject::MapEditorMissionLevelObject
 * @param parent
 */

MapEditorMissionLevelObject::MapEditorMissionLevelObject(QObject *parent)
	: ObjectListModelObject(parent)
	, m_uuid()
	, m_name()
	, m_level(0)
{

}

MapEditorMissionLevelObject::MapEditorMissionLevelObject(const QString &name, QObject *parent)
	: ObjectListModelObject(parent)
	, m_uuid()
	, m_name(name)
	, m_level(1)
{

}

MapEditorMissionLevelObject::MapEditorMissionLevelObject(const QString &uuid, const QString &name, QObject *parent)
	: ObjectListModelObject(parent)
	, m_uuid(uuid)
	, m_name(name)
	, m_level(1)
{

}

MapEditorMissionLevelObject::MapEditorMissionLevelObject(const QString &uuid, const QString &name, const int &level, QObject *parent)
	: ObjectListModelObject(parent)
	, m_uuid(uuid)
	, m_name(name)
	, m_level(level)
{

}

MapEditorMissionLevelObject::~MapEditorMissionLevelObject()
{

}

