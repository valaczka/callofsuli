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
#include "mapimage.h"

MapEditor::MapEditor(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_editor(nullptr)
	, m_undoStack(new EditorUndoStack(this))
	, m_url()
	, m_availableObjectives()
	, m_availableTerrains()
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
	e->setUuid(QUuid::createUuid().toString());
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
		Client::clientInstance()->sendMessageWarning(tr("Mentés"), tr("Nem lehet menteni a fájlt!"), filename);
		return;
	}

	if (!newUrl.isEmpty())
		m_editor->regenerateUuids();

	QByteArray b = m_editor->toBinaryData();

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

void MapEditor::missionLevelPlay(GameMapEditorMissionLevel *missionLevel)
{
	GameMap *map = GameMap::fromBinaryData(m_editor->toBinaryData());
	GameMatch *m_gameMatch = new GameMatch(missionLevel, map, this);
	m_gameMatch->setDeleteGameMap(true);

	QString err;
	if (!m_gameMatch->check(&err)) {
		Client::clientInstance()->sendMessageError(tr("Belső hiba"), err);

		delete m_gameMatch;
		return;
	}

	qDebug() << "Add mapimage provider";
	MapImage *mapImage = new MapImage(map);
	Client::clientInstance()->rootEngine()->addImageProvider("mapimage", mapImage);


	m_gameMatch->setImageDbName("mapimage");
	m_gameMatch->setDeathmatch(false);

	if (!Client::clientInstance()->userPlayerCharacter().isEmpty())
		m_gameMatch->setPlayerCharacter(Client::clientInstance()->userPlayerCharacter());
	else
		m_gameMatch->setPlayerCharacter("default");


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

	emit gamePlayReady(m_gameMatch);
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
			MapEditorMissionLevelObject *obj = new MapEditorMissionLevelObject(m->uuid(), m->name(), l->level(), this);
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
			MapEditorMissionLevelObject *obj = new MapEditorMissionLevelObject(m->uuid(), m->name(), l->level(), this);
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

		MapEditorMissionLevelObject *obj = new MapEditorMissionLevelObject(m->uuid(), m->name(), l->level(), this);
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


const QVariantList &MapEditor::availableTerrains() const
{
	return m_availableTerrains;
}
