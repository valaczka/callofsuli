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

#include "mapeditor.h"
#include "editoraction.h"
#include "mapeditoraction.h"
#include "mapimage.h"
#include "studentmaps.h"
#include "gameenemydata.h"
#include "question.h"

MapEditor::MapEditor(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_editor(nullptr)
	, m_undoStack(new EditorUndoStack(this))
	, m_url()
	, m_availableObjectives()
	, m_availableTerrains()
	, m_availableInventories()
	, m_missionLevelModel(new ObjectGenericListModel<MapEditorMissionLevelObject>(this))
{
	// Load available objectives

	QHash<QString, ModuleInterface *> omlist =  Client::moduleObjectiveList();
	QHashIterator<QString, ModuleInterface *> omit(omlist);

	while (omit.hasNext()) {
		omit.next();
		m_availableObjectives.append(QVariantMap({
													 { "module", omit.key() },
													 { "name", omit.value()->readableName() },
													 { "icon", omit.value()->icon() },
													 { "storageModules", omit.value()->storageModules() }
												 }));
	}


	QVariantMap tmap = Client::terrainMap();
	QMapIterator<QString, QVariant> it(tmap);

	while (it.hasNext()) {
		it.next();
		QVariantMap m = it.value().toMap();
		m.insert("terrain", it.key());
		m_availableTerrains.append(m);
	}


	QHash<QString, GameEnemyData::InventoryType> ilist =  GameEnemyData::inventoryTypes();
	QHashIterator<QString, GameEnemyData::InventoryType> iit(ilist);

	while (iit.hasNext()) {
		iit.next();
		m_availableInventories.append(QVariantMap({
													  { "module", iit.key() },
													  { "name", iit.value().name },
													  { "icon", iit.value().icon }
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

	QQmlEngine *engine = Client::clientInstance()->rootEngine();

	if (!engine) {
		qWarning() << "Invalid engine" << this;
	} else if (engine->imageProvider("mapimage")) {
		qDebug() << "Remove image provider mapimage";
		engine->removeImageProvider("mapimage");
	}
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
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Pálya megnyitás"), tr("Már meg van nyitva egy pálya!"), m_url.toLocalFile());
		return;
	}

	GameMapEditor *e = GameMapEditor::fromBinaryData(Client::fileContent(url.toLocalFile()), this);

	if (!e) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/folder-alert.svg", tr("Hibás pálya"), tr("Nem lehet megnyitni a fájlt!"), url.toLocalFile());
		return;
	}

	setEditor(e);
	setUrl(url);
	m_undoStack->clear();
	setDisplayName(url.toLocalFile());

	qDebug() << "Add mapimage provider";
	MapImage *mapImage = new MapImage(e);
	Client::clientInstance()->rootEngine()->addImageProvider("mapimage", mapImage);

}


/**
 * @brief MapEditor::create
 */

void MapEditor::create()
{
	if (m_editor) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Új pálya"), tr("Már meg van nyitva egy pálya!"), m_url.toLocalFile());
		return;
	}

	GameMapEditor *e = new GameMapEditor(this);
	e->setUuid(QUuid::createUuid().toString());
	setEditor(e);
	setUrl(QUrl());
	m_undoStack->clear();
	setDisplayName(tr("-- Új pálya --"));

	qDebug() << "Add mapimage provider";
	MapImage *mapImage = new MapImage(e);
	Client::clientInstance()->rootEngine()->addImageProvider("mapimage", mapImage);
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

		QQmlEngine *engine = Client::clientInstance()->rootEngine();

		if (!engine) {
			qWarning() << "Invalid engine" << this;
		} else if (engine->imageProvider("mapimage")) {
			qDebug() << "Remove image provider mapimage";
			engine->removeImageProvider("mapimage");
		}
	}
}






/**
 * @brief MapEditor::save
 * @param newUrl
 */

void MapEditor::save(const QUrl &newUrl)
{
	if (!m_editor)
		return;

	if (m_url.isEmpty() && newUrl.isEmpty()) {
		qWarning() << "Missing file url";
		return;
	}


	QString filename;

	if (newUrl.isEmpty())
		filename = m_url.toLocalFile();
	else
		filename = newUrl.toLocalFile();

	qInfo() << tr("Mentés ide: %1").arg(filename);

	QFile f(filename);

	if (!f.open(QIODevice::WriteOnly)) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/folder-alert.svg", tr("Mentés"), tr("Nem lehet menteni a fájlt!"), filename);
		return;
	}

	if (!newUrl.isEmpty())
		m_editor->regenerateUuids();

	m_editor->setFilterUsedImages(true);
	QByteArray b = m_editor->toBinaryData();
	m_editor->setFilterUsedImages(false);

	f.write(b);

	f.close();

	if (!newUrl.isEmpty()) {
		setUrl(newUrl);
		setDisplayName(filename);
	}

	m_undoStack->setSavedStep(m_undoStack->step());

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

void MapEditor::chapterAdd(QVariantMap data, GameMapEditorMissionLevel *missionLevel)
{
	int id = 1;

	foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects())
		id = qMax(ch->id()+1, id);

	data.insert("id", id);

	m_undoStack->call(new MapEditorActionChapterNew(m_editor, data, missionLevel));

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

	m_undoStack->call(new MapEditorActionChapterMissionLevels(m_editor, chapter, ml));
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
 * @brief MapEditor::objectiveGeneratePreview
 * @param objectiveModule
 * @param objectiveData
 * @param storageModule
 * @param storageData
 * @return
 */

QVariantMap MapEditor::objectiveGeneratePreview(const QString &objectiveModule, const QVariantMap &objectiveData,
												const QString &storageModule, const QVariantMap &storageData)
{

	if (!Client::moduleObjectiveList().contains(objectiveModule)) {
		QVariantMap m;
		m["text"] = tr("Érvénytelen modul: %1").arg(objectiveModule);
		return m;
	}

	ModuleInterface *storage = nullptr;

	if (Client::moduleStorageList().contains(storageModule)) {
		storage = Client::moduleStorageList().value(storageModule);
	}

	ModuleInterface *objective = Client::moduleObjectiveList().value(objectiveModule);

	return objective->preview(objective->generateAll(objectiveData, storage, storageData));
}


/**
 * @brief MapEditor::missionAdd
 * @param data
 */

void MapEditor::missionAdd(const QVariantMap &data, const QString &terrain)
{
	auto a = new MapEditorActionMissionNew(m_editor, data, terrain);
	m_undoStack->call(a);

	emit missionOpenRequest(a->mission());
	emit missionLevelOpenRequest(a->missionLevel());
}


/**
 * @brief MapEditor::missionRemove
 * @param mission
 */

void MapEditor::missionRemove(GameMapEditorMission *mission)
{
	if (!mission)
		return;

	m_undoStack->call(new MapEditorActionMissionRemove(m_editor, mission));
}

/**
 * @brief MapEditor::missionRemoveList
 * @param list
 */

void MapEditor::missionRemoveList(const QList<GameMapEditorMission *> &list)
{
	if (list.isEmpty())
		return;

	m_undoStack->call(new MapEditorActionMissionRemove(m_editor, list));
}


/**
 * @brief MapEditor::missionModify
 * @param mission
 * @param data
 */

void MapEditor::missionModify(GameMapEditorMission *mission, const QVariantMap &data)
{
	if (!mission)
		return;

	m_undoStack->call(new MapEditorActionMissionModify(m_editor, mission, data));
}




/**
 * @brief MapEditor::missionLockAdd
 * @param mission
 * @param level
 */

void MapEditor::missionLockAdd(GameMapEditorMission *mission, GameMapEditorMissionLevel *level)
{
	if (!mission || !level)
		return;

	m_undoStack->call(new MapEditorActionMissionLockNew(m_editor, mission, level));
}



/**
 * @brief MapEditor::missionLockRemove
 * @param mission
 * @param level
 */

void MapEditor::missionLockRemove(GameMapEditorMission *mission, GameMapEditorMissionLevel *level)
{
	if (!mission || !level)
		return;

	m_undoStack->call(new MapEditorActionMissionLockRemove(m_editor, mission, level));
}


/**
 * @brief MapEditor::missionLockReplace
 * @param mission
 * @param levelOld
 * @param levelNew
 */

void MapEditor::missionLockReplace(GameMapEditorMission *mission, GameMapEditorMissionLevel *levelOld, GameMapEditorMissionLevel *levelNew)
{
	if (!mission || !levelOld || !levelNew)
		return;

	m_undoStack->call(new MapEditorActionMissionLockReplace(m_editor, mission, levelOld, levelNew));
}









/**
 * @brief MapEditor::missionLevelAdd
 * @param mission
 * @param data
 */

void MapEditor::missionLevelAdd(GameMapEditorMission *mission, QVariantMap data)
{
	if (!mission)
		return;

	GameMapEditorMissionLevel *lastLevel = mission->lastLevel();

	if (!data.contains("level") && lastLevel)
		data.insert("level", lastLevel->level()+1);

	if (!data.contains("terrain") && lastLevel) {
		QString terrain = lastLevel->terrain().section("/", 0, -2);
		int terrainLevel = lastLevel->terrain().section("/", -1, -1).toInt()+1;

		QVariantMap terrainMap = Client::terrainMap();

		for (int i=terrainLevel; i>=1; --i) {
			QString t = QString("%1/%2").arg(terrain).arg(i);
			if (terrainMap.contains(t)) {
				data.insert("terrain", t);
				break;
			}
		}
	}

	auto a = new MapEditorActionMissionLevelNew(m_editor, mission, data);

	m_undoStack->call(a);

	emit missionLevelOpenRequest(a->missionLevel());
}




/**
 * @brief MapEditor::missionLevelRemove
 * @param mission
 * @param missionLevel
 */

void MapEditor::missionLevelRemove(GameMapEditorMissionLevel *missionLevel)
{
	if (!missionLevel || !missionLevel->editorMission())
		return;

	m_undoStack->call(new MapEditorActionMissionLevelRemove(m_editor, missionLevel->editorMission(), missionLevel));

	emit missionLevelRemoved(missionLevel);
}







/**
 * @brief MapEditor::missionLevelModify
 * @param missionLevel
 * @param data
 */

void MapEditor::missionLevelModify(GameMapEditorMissionLevel *missionLevel, const QVariantMap &data)
{
	if (!missionLevel)
		return;

	m_undoStack->call(new MapEditorActionMissionLevelModify(m_editor, missionLevel, data));
}


/**
 * @brief MapEditor::missionLevelPlay
 * @param missionLevel
 */

void MapEditor::missionLevelPlay(GameMapEditorMissionLevel *missionLevel, const GameMatch::GameMode &mode)
{
	GameMap *map = GameMap::fromBinaryData(m_editor->toBinaryData());
	GameMatch *m_gameMatch = new GameMatch(missionLevel, map, this);
	m_gameMatch->setDeleteGameMap(true);

	QString err;
	if (!m_gameMatch->check(&err)) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), err);

		delete m_gameMatch;
		return;
	}

	m_gameMatch->setImageDbName("mapimage");
	m_gameMatch->setDeathmatch(false);
	m_gameMatch->setMode(mode);

	if (!Client::clientInstance()->userPlayerCharacter().isEmpty())
		m_gameMatch->setPlayerCharacter(Client::clientInstance()->userPlayerCharacter());
	else
		m_gameMatch->setPlayerCharacter("default");

/*
	connect(m_gameMatch, &GameMatch::gameWin, this, [=]() {
		QQmlEngine *engine = Client::clientInstance()->rootEngine();

		if (!engine) {
			qWarning() << "Invalid engine" << this;
		} else if (engine->imageProvider("mapimage")) {
			qDebug() << "Remove image provider mapimage";
			engine->removeImageProvider("mapimage");
		}
	});

	connect(m_gameMatch, &GameMatch::gameLose, this, [=]() {
		QQmlEngine *engine = Client::clientInstance()->rootEngine();

		if (!engine) {
			qWarning() << "Invalid engine" << this;
		} else if (engine->imageProvider("mapimage")) {
			qDebug() << "Remove image provider mapimage";
			engine->removeImageProvider("mapimage");
		}
	});
*/
	emit gamePlayReady(m_gameMatch);
}


/**
 * @brief MapEditor::missionLevelRemoveChapter
 * @param missionLevel
 * @param chapter
 */

void MapEditor::missionLevelRemoveChapter(GameMapEditorMissionLevel *missionLevel, GameMapEditorChapter *chapter)
{
	QList<GameMapEditorChapter *> list;

	foreach (GameMapEditorChapter *ch, missionLevel->chapters()->objects()) {
		if (ch != chapter)
			list.append(ch);
	}
	m_undoStack->call(new MapEditorActionMissionLevelChapters(m_editor, missionLevel, list));
}


/**
 * @brief MapEditor::missionLevelRemoveChapterList
 * @param missionLevel
 * @param list
 */

void MapEditor::missionLevelRemoveChapterList(GameMapEditorMissionLevel *missionLevel, const QList<GameMapEditorChapter *> &chapterList)
{
	QList<GameMapEditorChapter *> list;

	foreach (GameMapEditorChapter *ch, missionLevel->chapters()->objects()) {
		if (!chapterList.contains(ch))
			list.append(ch);
	}
	m_undoStack->call(new MapEditorActionMissionLevelChapters(m_editor, missionLevel, list));
}



/**
 * @brief MapEditor::missionLevelModifyChapters
 * @param missionLevel
 * @param list
 */

void MapEditor::missionLevelModifyChapters(GameMapEditorMissionLevel *missionLevel, const QList<GameMapEditorChapter *> &list)
{
	m_undoStack->call(new MapEditorActionMissionLevelChapters(m_editor, missionLevel, list));
}


/**
 * @brief MapEditor::inventoryAdd
 * @param missionLevel
 * @param data
 */

void MapEditor::inventoryAdd(GameMapEditorMissionLevel *missionLevel, const QVariantMap &data)
{
	if (!missionLevel)
		return;

	m_undoStack->call(new MapEditorActionInventoryNew(m_editor, missionLevel, data));
}


/**
 * @brief MapEditor::inventoryRemove
 * @param missionLevel
 * @param inventory
 */

void MapEditor::inventoryRemove(GameMapEditorMissionLevel *missionLevel, GameMapEditorInventory *inventory)
{
	if (!missionLevel || !inventory)
		return;

	m_undoStack->call(new MapEditorActionInventoryRemove(m_editor, missionLevel, inventory));
}


/**
 * @brief MapEditor::inventoryModify
 * @param inventory
 * @param data
 */

void MapEditor::inventoryModify(GameMapEditorInventory *inventory, const QVariantMap &data)
{
	if (!inventory)
		return;

	m_undoStack->call(new MapEditorActionInventoryModify(m_editor, inventory, data));
}


/**
 * @brief MapEditor::storageRemove
 * @param storage
 */

void MapEditor::storageRemove(GameMapEditorStorage *storage)
{
	if (!storage)
		return;

	m_undoStack->call(new MapEditorActionStorageRemove(m_editor, storage));
}



/**
 * @brief MapEditor::imageAdd
 * @param url
 */

int MapEditor::imageAdd(const QUrl &url)
{
	QFile f(url.toLocalFile());

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Fájl megnyitási hiba"), tr("Nem sikerült megnyitni a fájlt:\n%1").arg(f.fileName()));
		return -1;
	}

	const QByteArray content = f.readAll();

	f.close();

	int id = 1;

	foreach (GameMapEditorImage *s, m_editor->images()->objects())
		id = qMax(s->id()+1, id);

	m_editor->addImage(id, content);

	return id;
}




/**
 * @brief MapEditor::onStepChanged
 */


void MapEditor::onUndoRedoCompleted(const int &lastStep)
{
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

void MapEditor::updateMissionLevelModelChapter(GameMapEditorChapter *chapter)
{
	QList<MapEditorMissionLevelObject*> list;

	if (!m_editor) {
		m_missionLevelModel->resetModel(list);
		return;
	}


	foreach (GameMapEditorMission *m, m_editor->missions()->objects()) {
		foreach (GameMapEditorMissionLevel *l, m->levels()->objects()) {
			MapEditorMissionLevelObject *obj = new MapEditorMissionLevelObject(m->uuid(), m->name(), l, this);
			if (chapter && l->chapters()->objects().contains(chapter))
				obj->setSelected(true);
			list.append(obj);
		}
	}

	m_missionLevelModel->resetModel(list);

}


/**
 * @brief MapEditor::updateMissionLevelModel
 * @param mission
 */

void MapEditor::updateMissionLevelModelMission(GameMapEditorMission *mission)
{
	QList<MapEditorMissionLevelObject*> list;

	if (!m_editor || !mission) {
		m_missionLevelModel->resetModel(list);
		return;
	}


	foreach (GameMapEditorMission *m, m_editor->missions()->objects()) {
		if (m == mission)
			continue;

		bool hasFound = false;

		foreach (GameMapEditorMissionLevel *l, mission->locks()->objects()) {
			if (l->mission() == m) {
				hasFound = true;
				break;
			}
		}

		if (hasFound)
			continue;

		foreach (GameMapEditorMissionLevel *l, m->levels()->objects()) {
			MapEditorMissionLevelObject *obj = new MapEditorMissionLevelObject(m->uuid(), m->name(), l, this);
			list.append(obj);
		}
	}

	m_missionLevelModel->resetModel(list);
}


/**
 * @brief MapEditor::updateMissionLevelModel
 * @param mission
 */

void MapEditor::updateMissionLevelModelLock(GameMapEditorMissionLevel *lock)
{
	QList<MapEditorMissionLevelObject*> list;

	if (!m_editor || !lock || !lock->editorMission()) {
		m_missionLevelModel->resetModel(list);
		return;
	}

	GameMapEditorMission *m = lock->editorMission();

	foreach (GameMapEditorMissionLevel *l, m->levels()->objects()) {
		if (l == lock)
			continue;

		MapEditorMissionLevelObject *obj = new MapEditorMissionLevelObject(m->uuid(), m->name(), l, this);
		list.append(obj);
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
	QHashIterator<QString, ModuleInterface *> it(mlist);

	int id = -1;

	while (it.hasNext()) {
		it.next();
		ModuleInterface *iif = it.value();
		list.append(QVariantMap ({
									 { "id", id-- },
									 { "module", it.key() },
									 { "name", "" },
									 { "title", tr("Új %1").arg(iif->readableName()) },
									 { "icon", iif->icon() },
									 { "details", "" },
									 { "image", "" },
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
 * @brief MapEditor::inventoryInfo
 * @param module
 * @return
 */

QVariantMap MapEditor::inventoryInfo(const QString &module) const
{
	return GameEnemyData::inventoryInfo(module);
}


/**
 * @brief MapEditor::storageInfo
 * @param module
 * @return
 */

QVariantMap MapEditor::storageInfo(GameMapEditorStorage *storage) const
{
	if (storage)
		return Question::storageInfo(storage->module(), storage->data());
	else
		return Question::storageInfo("", {{}});
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
	, m_missionLevel(nullptr)
{

}

MapEditorMissionLevelObject::MapEditorMissionLevelObject(const QString &name, QObject *parent)
	: ObjectListModelObject(parent)
	, m_uuid()
	, m_name(name)
	, m_level(1)
	, m_missionLevel(nullptr)
{

}

MapEditorMissionLevelObject::MapEditorMissionLevelObject(const QString &uuid, const QString &name, QObject *parent)
	: ObjectListModelObject(parent)
	, m_uuid(uuid)
	, m_name(name)
	, m_level(1)
	, m_missionLevel(nullptr)
{

}

MapEditorMissionLevelObject::MapEditorMissionLevelObject(const QString &uuid, const QString &name, GameMapEditorMissionLevel *missionLevel, QObject *parent)
	: ObjectListModelObject(parent)
	, m_uuid(uuid)
	, m_name(name)
	, m_level(missionLevel ? missionLevel->level() : 1)
	, m_missionLevel(missionLevel)
{

}

MapEditorMissionLevelObject::~MapEditorMissionLevelObject()
{

}

GameMapEditorMissionLevel *MapEditorMissionLevelObject::missionLevel() const
{
	return m_missionLevel;
}


const QVariantList &MapEditor::availableTerrains() const
{
	return m_availableTerrains;
}


/**
 * @brief MapEditor::updateChapterModelMissionLevel
 * @param lock
 */

void MapEditor::updateChapterModelMissionLevel(GameMapEditorMissionLevel *missionLevel)
{
	if (!m_editor)
		return;

	foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects())
		ch->setSelected(missionLevel && missionLevel->chapters()->objects().contains(ch));
}



/**
 * @brief MapEditor::checkMap
 * @return
 */

QString MapEditor::checkMap() const
{
	QString errorString;

	GameMap *map = GameMap::fromBinaryData(m_editor->toBinaryData());

	if (!map) {
		errorString = tr("Hibás adatfájl\n");
		return errorString;
	}

	// Check locks

	QList<GameMapMission*> lockList;

	foreach(GameMapMission *m, map->missions()) {
		QVector<GameMapMissionLevelIface*> list;
		if (!m->getLockTree(&list, m))
			lockList.append(m);
	}


	if (!lockList.isEmpty()) {
		foreach (GameMapMission *m, lockList)
			errorString += tr("Körkörös zárolás: %1\n").arg(m->name());
	}


	// Check terrains

	QList<GameMapMissionLevel*> levelList;

	if (!StudentMaps::checkTerrains(map, &levelList)) {
		foreach (GameMapMissionLevel *ml, levelList)
			errorString += tr("Érvénytelen harcmező: %1 (%2 level %3)\n")
						   .arg(ml->terrain())
						   .arg(ml->mission()->name())
						   .arg(ml->level());
	}


	// Modules

	foreach(GameMapChapter *chapter, map->chapters()) {
		foreach(GameMapObjective *objective, chapter->objectives()) {
			QString om = objective->module();

			if (!Client::moduleObjectiveList().contains(om))
				errorString += tr("Érvénytelen modul: %1 (%2 szakasz)\n")
							   .arg(om)
							   .arg(chapter->name());

		}
	}

	foreach(GameMapStorage *storage, map->storages()) {
		QString sm = storage->module();

		if (!Client::moduleStorageList().contains(sm))
			errorString += tr("Érvénytelen adatbank modul: %1\n")
						   .arg(sm);

	}

	return errorString;
}


/**
 * @brief MapEditor::availableInventories
 * @return
 */

const QVariantList &MapEditor::availableInventories() const
{
	return m_availableInventories;
}
