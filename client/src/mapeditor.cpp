/*
 * ---- Call of Suli ----
 *
 * mapeditor.cpp
 *
 * Created on: 2023. 05. 06.
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
#include "application.h"
#include "mapimage.h"
#include "gamepickable.h"
#include "question.h"
#include "abstractlevelgame.h"

const QString MapEditor::m_backupSuffix = QStringLiteral(".autosave");

/**
 * @brief MapEditor::MapEditor
 * @param parent
 */

MapEditor::MapEditor(QObject *parent)
	: QObject{parent}
	, m_client(Application::instance()->client())
	, m_undoStack(new EditorUndoStack(this))
	, m_mapPlay(new MapPlayEditor(m_client, this))
{
	LOG_CTRACE("client") << "MapEditor created" << this;

	m_saveTimer.setSingleShot(true);
	m_saveTimer.setInterval(5000);
	connect(&m_saveTimer, &QTimer::timeout, this, &MapEditor::saveAuto);
	connect(m_undoStack, &EditorUndoStack::callCompleted, this, &MapEditor::onModified);
	connect(m_undoStack, &EditorUndoStack::undoCompleted, this, &MapEditor::onModified);
	connect(m_undoStack, &EditorUndoStack::redoCompleted, this, &MapEditor::onModified);

	connect(this, &MapEditor::saveRequest, this, &MapEditor::onSaveRequestFile);
	connect(this, &MapEditor::autoSaveRequest, this, &MapEditor::onAutoSaveRequestFile);
}


/**
 * @brief MapEditor::~MapEditor
 */

MapEditor::~MapEditor()
{
	delete m_mapPlay;
	delete m_undoStack;

	if (m_tmpObjective) {
		delete m_tmpObjective;
		m_tmpObjective = nullptr;
	}

	if (m_tmpStorage) {
		delete m_tmpStorage;
		m_tmpStorage = nullptr;
	}

	LOG_CTRACE("client") << "MapEditor destroyed" << this;
}


/**
 * @brief MapEditor::map
 * @return
 */

MapEditorMap *MapEditor::map() const
{
	return m_map;
}

void MapEditor::setMap(MapEditorMap *newMap)
{
	if (m_map == newMap)
		return;
	m_map = newMap;
	emit mapChanged();
}


/**
 * @brief MapEditor::createFile
 */

void MapEditor::createFile()
{
	LOG_CDEBUG("client") << "Create map";

	MapEditorMap *map = new MapEditorMap(this);
	map->setUuid(QUuid::createUuid().toString(QUuid::WithoutBraces));

	unloadMap();
	setMap(map);
	loadMap();

	m_currentFileName = QLatin1String("");
	m_currentBackupName = QLatin1String("");

	setDisplayName(tr("--- új pálya ---"));
	setModified(true);
}



/**
 * @brief MapEditor::saveAs
 * @param file
 */

void MapEditor::saveAs(const QUrl &file)
{
	const QString &fname = file.toLocalFile();

	if (fname.isEmpty())
		return;

	if (!m_map)
		return onSaved(false);

	const QByteArray &data = m_map->toBinaryData();

	QFile f(fname);

	if (!f.open(QIODevice::WriteOnly)) {
		LOG_CWARNING("client") << "Can't write file:";
		return onSaved(false);
	}

	f.write(data);

	f.close();

	if (!m_currentBackupName.isEmpty())
		QFile::remove(m_currentBackupName);

	m_currentFileName = fname;
	m_currentBackupName = file.toLocalFile().append(m_backupSuffix);

	setFileDisplayName();

	onSaved(true);
}



/**
 * @brief MapEditor::openFile
 * @param file
 * @return
 */

void MapEditor::openFile(const QUrl &file, const bool &fromBackup)
{
	QString fname = file.toLocalFile();

	if (fromBackup)
		fname.append(m_backupSuffix);

	bool err = false;
	const QByteArray &b = Utils::fileContent(fname, &err);

	if (err)
		return m_client->messageError(tr("Nem lehet megnyitni a fájlt!"), tr("Pályaszerkesztő"));

	if (!loadFromBinaryData(b))
		return m_client->messageError(tr("Érvénytelen fájl!"), tr("Pályaszerkesztő"));

	m_currentFileName = file.toLocalFile();
	m_currentBackupName = file.toLocalFile().append(m_backupSuffix);

	if (QFile::exists(m_currentBackupName) && !fromBackup)
		QFile::remove(m_currentBackupName);

	setFileDisplayName();

	if (fromBackup)
		setModified(true);
}



/**
 * @brief MapEditor::hasBackup
 * @param file
 * @return
 */

bool MapEditor::hasBackup(const QUrl &file) const
{
	return QFile::exists(file.toLocalFile().append(m_backupSuffix));
}


/**
 * @brief MapEditor::currentFolder
 * @return
 */

QUrl MapEditor::currentFolder() const
{
	if (!m_currentFileName.isEmpty())
		return QUrl::fromLocalFile(QFileInfo(m_currentFileName).path());
	else
		return QUrl();
}



/**
 * @brief MapEditor::save
 */

void MapEditor::save()
{
	emit saveRequest();
}


/**
 * @brief MapEditor::saveAuto
 */

void MapEditor::saveAuto()
{
	emit autoSaveRequest();
}


/**
 * @brief MapEditor::onSaved
 * @param success
 */

void MapEditor::onSaved(const bool &success)
{
	if (success) {
		setAutoSaved(true);
		setModified(false);
	} else {
		m_client->snack(tr("A mentés sikertelen"));
	}
}



/**
 * @brief MapEditor::onAutoSaved
 * @param success
 */

void MapEditor::onAutoSaved(const bool &success)
{
	if (success) {
		setAutoSaved(true);
	} else {
		m_client->snack(tr("A mentés sikertelen"));
	}
}





/**
 * @brief MapEditor::loadFromBinaryData
 * @param data
 * @return
 */

bool MapEditor::loadFromBinaryData(const QByteArray &data)
{
	LOG_CDEBUG("client") << "Load map from binary data";

	MapEditorMap *map = new MapEditorMap(this);

	if (!map->loadFromBinaryData(data)) {
		LOG_CWARNING("client") << "Invalid map";
		map->deleteLater();
		return false;
	}

	unloadMap();
	setMap(map);
	loadMap();

	return true;
}




/**
 * @brief MapEditor::unloadMap
 */

void MapEditor::unloadMap()
{
	if (Application::instance() && Application::instance()->engine() && Application::instance()->engine()->imageProvider(QStringLiteral("mapimage"))) {
		LOG_CDEBUG("client") << "Remove image provider mapimage";
		Application::instance()->engine()->removeImageProvider(QStringLiteral("mapimage"));
	}

	if (m_map) {
		m_map->deleteLater();
		setMap(nullptr);
	}
}



/**
 * @brief MapEditor::loadMap
 */

void MapEditor::loadMap()
{
	if (!m_map)
		return;

	LOG_CDEBUG("client") << "Add mapimage provider for map:" << m_map->uuid();
	MapImage *mapImage = new MapImage(m_map);
	Application::instance()->engine()->addImageProvider(QStringLiteral("mapimage"), mapImage);
}


/**
 * @brief MapEditor::onModified
 */

void MapEditor::onModified()
{
	setModified(true);
	setAutoSaved(false);
	m_saveTimer.start();
}



/**
 * @brief MapEditor::onSaveRequestFile
 */

void MapEditor::onSaveRequestFile()
{
	if (m_currentFileName.isEmpty())
		return;

	if (!m_map)
		return onSaved(false);

	const QByteArray &data = m_map->toBinaryData();

	QFile f(m_currentFileName);

	if (!f.open(QIODevice::WriteOnly)) {
		LOG_CWARNING("client") << "Can't write file:";
		return onSaved(false);
	}

	f.write(data);

	f.close();

	if (!m_currentBackupName.isEmpty())
		QFile::remove(m_currentBackupName);

	onSaved(true);
}



/**
 * @brief MapEditor::onAutoSaveRequestFile
 */

void MapEditor::onAutoSaveRequestFile()
{
	if (m_currentFileName.isEmpty() || m_currentBackupName.isEmpty())
		return;

	if (!m_map)
		return onAutoSaved(false);

	const QByteArray &data = m_map->toBinaryData();

	QFile f(m_currentBackupName);

	if (!f.open(QIODevice::WriteOnly)) {
		LOG_CWARNING("client") << "Can't write file:";
		return onAutoSaved(false);
	}

	f.write(data);

	f.close();

	onAutoSaved(true);
}



bool MapEditor::autoSaved() const
{
	return m_autoSaved;
}

void MapEditor::setAutoSaved(bool newAutoSaved)
{
	if (m_autoSaved == newAutoSaved)
		return;
	m_autoSaved = newAutoSaved;
	emit autoSavedChanged();
}



bool MapEditor::modified() const
{
	return m_modified;
}

void MapEditor::setModified(bool newModified)
{
	if (m_modified == newModified)
		return;
	m_modified = newModified;
	emit modifiedChanged();
	emit displayNameChanged();
}




/**
 * @brief MapEditor::displayName
 * @return
 */

QString MapEditor::displayName() const
{
	return m_modified ? tr("* ")+m_displayName : m_displayName;
}


/**
 * @brief MapEditor::setDisplayName
 * @param newDisplayName
 */

void MapEditor::setDisplayName(const QString &newDisplayName)
{
	if (m_displayName == newDisplayName)
		return;
	m_displayName = newDisplayName;
	emit displayNameChanged();
}


/**
 * @brief MapEditor::objectiveInfo
 * @param objective
 * @return
 */

QVariantMap MapEditor::objectiveInfo(MapEditorObjective *objective) const
{
	if (!objective)
		return Question::objectiveInfo(QLatin1String(""), {});

	return Question::objectiveInfo(objective->module(), objective->data(), objective->storage() ? objective->storage()->module() : QLatin1String(""),
								   objective->storage() ? objective->storage()->data() : QVariantMap());
}


/**
 * @brief MapEditor::storageInfo
 * @param storage
 * @return
 */

QVariantMap MapEditor::storageInfo(MapEditorStorage *storage) const
{
	if (!storage)
		return Question::storageInfo(QLatin1String(""), {});

	return Question::storageInfo(storage->module(), storage->data());
}


/**
 * @brief MapEditor::objectiveQml
 * @param objective
 * @return
 */

QString MapEditor::objectiveQml(MapEditorObjective *objective) const
{
	if (objective) {
		ModuleInterface *mi = Application::instance()->objectiveModules().value(objective->module());

		if (mi)
			return mi->qmlEditor();
	}

	return QLatin1String("");
}



/**
 * @brief MapEditor::storageQml
 * @param storage
 * @return
 */

QString MapEditor::storageQml(MapEditorStorage *storage) const
{
	if (storage) {
		ModuleInterface *mi = Application::instance()->storageModules().value(storage->module());

		if (mi)
			return mi->qmlEditor();
	}

	return QLatin1String("");
}



/**
 * @brief MapEditor::inventoryInfo
 * @param inventory
 * @return
 */

QVariantMap MapEditor::inventoryInfo(MapEditorInventory *inventory) const
{
	if (!inventory)
		return QVariantMap();

	const GamePickable::GamePickableData &data = GamePickable::pickableDataHash().value(inventory->module());

	if (data.type == GamePickable::PickableInvalid)
		return QVariantMap {
			{ QStringLiteral("name"), tr("Érvénytelen modul: %1").arg(inventory->module()) },
			{ QStringLiteral("icon"), QLatin1String("") }
		};
	else
		return QVariantMap {
			{ QStringLiteral("name"), data.name },
			{ QStringLiteral("icon"), data.image }
		};
}



/**
 * @brief MapEditor::pickableList
 * @return
 */

QVariantList MapEditor::pickableListModel() const
{
	QVariantList list;

	foreach (const GamePickable::GamePickableData &data, GamePickable::pickableDataTypes())
		list.append(QVariantMap {
						{ QStringLiteral("id"), data.id },
						{ QStringLiteral("text"), data.name },
						{ QStringLiteral("icon"), data.image },
					});

	return list;
}



/**
 * @brief MapEditor::objectiveListModel
 * @return
 */

QVariantList MapEditor::objectiveListModel() const
{
	QVariantList list;

	QStringList keys = Application::instance()->objectiveModules().keys();

	keys.sort();

	foreach (const QString &k, keys) {
		const ModuleInterface *iface = Application::instance()->objectiveModules().value(k);
		if (!iface)
			continue;

		list.append(QVariantMap {
						{ QStringLiteral("module"), iface->name() },
						{ QStringLiteral("text"), iface->readableName() },
						{ QStringLiteral("icon"), iface->icon() },
					});
	}

	return list;
}



/**
 * @brief MapEditor::storageListModel
 * @param objectiveModule
 * @return
 */

QVariantList MapEditor::storageListModel(const QString &objectiveModule) const
{
	QVariantList list;

	const ModuleInterface *iface = Application::instance()->objectiveModules().value(objectiveModule);

	if (!iface)
		return list;

	foreach (const QString &s, iface->storageModules()) {
		const ModuleInterface *iface = Application::instance()->storageModules().value(s);

		if (!iface)
			continue;

		list.append(QVariantMap {
						{ QStringLiteral("module"), iface->name() },
						{ QStringLiteral("text"), iface->readableName() },
						{ QStringLiteral("icon"), iface->icon() },
					});
	}

	return list;
}



/**
 * @brief MapEditor::storageModel
 * @param storageModule
 * @return
 */

QVariantList MapEditor::storageModel(const QString &storageModule) const
{
	QVariantList list;

	if (!m_map)
		return list;

	for (MapEditorStorage *s : *m_map->storageList()) {
		if (s->module() != storageModule)
			continue;

		QVariantMap m = storageInfo(s);
		m.insert(QStringLiteral("storage"), QVariant::fromValue(s));
		list.append(m);
	}


	return list;
}



/**
 * @brief MapEditor::storageListAllModel
 * @return
 */

QVariantList MapEditor::storageListAllModel() const
{
	QVariantList list;

	QStringList keys = Application::instance()->storageModules().keys();

	keys.sort();

	foreach (const QString &k, keys) {
		const ModuleInterface *iface = Application::instance()->storageModules().value(k);
		if (!iface)
			continue;

		list.append(QVariantMap {
						{ QStringLiteral("module"), iface->name() },
						{ QStringLiteral("text"), iface->readableName() },
						{ QStringLiteral("icon"), iface->icon() },
					});
	}

	return list;
}




/**
 * @brief MapEditor::undoStack
 * @return
 */

EditorUndoStack *MapEditor::undoStack() const
{
	return m_undoStack;
}



/**
 * @brief MapEditor::missionAdd
 */

MapEditorMission* MapEditor::missionAdd(const QString &name)
{
	if (!m_map)
		return nullptr;

	const QStringList &medals = AbstractLevelGame::availableMedal();

	MapEditorMission mission;

	if (!medals.isEmpty())
		mission.setMedalImage(medals.at(QRandomGenerator::global()->bounded(medals.size())));

	const QString &uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

	mission.setUuid(uuid);
	mission.setName(name.isEmpty() ? tr("Új küldetés") : name);
	mission.setModes(GameMap::Lite|GameMap::Action);

	MapEditorMissionLevel *level = mission.createNextLevel(m_map);
	level->setLevel(1);
	level->setCanDeathmatch(true);

	mission.levelList()->append(level);

	const QVariantMap &data = mission.toVariantMap();

	mission.levelList()->remove(level);
	level->deleteLater();
	level = nullptr;

	EditorAction *action = new EditorAction(m_map, tr("Új küldetés: %1").arg(name));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = new MapEditorMission(action->map());
		m->fromVariantMap(action->data());

		action->map()->missionList()->append(m);
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (m)
			action->map()->missionList()->remove(m);
	});

	m_undoStack->call(action);

	return m_map->mission(uuid);
}



/**
 * @brief MapEditor::missionRemove
 * @param mission
 */

void MapEditor::missionRemove(MapEditorMission *mission)
{
	if (!m_map || !mission)
		return;

	QVariantMap data = mission->toVariantMap();

	QVariantList locks;

	for (MapEditorMission *mis : *m_map->missionList()) {
		for (MapEditorMissionLevel *ml : *mission->levelList()) {
			if (mis->lockList().contains(ml)) {
				locks.append(QVariantMap{
								 { QStringLiteral("target"), mis->uuid() },
								 { QStringLiteral("level"), ml->level() }
							 });
			}
		}
	}

	data.insert(QStringLiteral("lockTargets"), locks);

	EditorAction *action = new EditorAction(m_map, tr("Pálya törlése: %1").arg(mission->name()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *mis = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (!mis)
			return;

		const QVariantList &locks = action->data().value(QStringLiteral("lockTargets")).toList();
		foreach (const QVariant &v, locks) {
			const QVariantMap &lm = v.toMap();

			MapEditorMission *m = action->map()->mission(lm.value(QStringLiteral("target")).toString());
			MapEditorMissionLevel *locklevel = mis->level(lm.value(QStringLiteral("level")).toInt());

			if (m && locklevel)
				m->lockRemove(locklevel);
		}

		action->map()->missionList()->remove(mis);
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *mis = new MapEditorMission(action->map());
		mis->fromVariantMap(action->data());

		action->map()->missionList()->append(mis);

		const QVariantList &locks = action->data().value(QStringLiteral("lockTargets")).toList();
		foreach (const QVariant &v, locks) {
			const QVariantMap &lm = v.toMap();

			MapEditorMission *tm = action->map()->mission(lm.value(QStringLiteral("target")).toString());
			MapEditorMissionLevel *locklevel = mis->level(lm.value(QStringLiteral("level")).toInt());

			if (tm && locklevel)
				tm->lockAdd(locklevel);
		}

	});

	m_undoStack->call(action);
}



/**
 * @brief MapEditor::missionModify
 * @param mission
 * @param modifyFunc
 */

void MapEditor::missionModify(MapEditorMission *mission, QJSValue modifyFunc)
{
	if (!m_map || !mission)
		return;

	QVariantMap data;
	data.insert(QStringLiteral("uuid"), mission->uuid());
	data.insert(QStringLiteral("from"), mission->toVariantMap(true));

	if (modifyFunc.isCallable())
		modifyFunc.call();

	data.insert(QStringLiteral("to"), mission->toVariantMap(true));

	EditorAction *action = new EditorAction(m_map, tr("Pálya módosítása: %1").arg(mission->name()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *mis = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (!mis)
			return;

		mis->fromVariantMap(action->data().value(QStringLiteral("to")).toMap(), true);
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *mis = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (!mis)
			return;

		mis->fromVariantMap(action->data().value(QStringLiteral("from")).toMap(), true);

	});

	m_undoStack->call(action);
}


/**
 * @brief MapEditor::missionLockAdd
 * @param mission
 * @param lock
 */

void MapEditor::missionLockAdd(MapEditorMission *mission, MapEditorMissionLevel *lock)
{
	if (!m_map || !mission || !lock || !lock->editorMission())
		return;

	QVariantMap data;

	data.insert(QStringLiteral("uuid"), mission->uuid());
	data.insert(QStringLiteral("lockUuid"), lock->editorMission()->uuid());
	data.insert(QStringLiteral("lockLevel"), lock->level());

	EditorAction *action = new EditorAction(m_map, tr("Zárolás hozzáadása: %1").arg(mission->name()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (m)
			m->lockAdd(action->data().value(QStringLiteral("lockUuid")).toString(),
					   action->data().value(QStringLiteral("lockLevel")).toInt());

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (m)
			m->lockRemove(action->data().value(QStringLiteral("lockUuid")).toString(),
						  action->data().value(QStringLiteral("lockLevel")).toInt());
	});

	m_undoStack->call(action);
}



/**
 * @brief MapEditor::missionLockAdd
 * @param mission
 * @param uuid
 * @param level
 */

void MapEditor::missionLockAdd(MapEditorMission *mission, const QString &uuid, const int &level)
{
	if (!m_map)
		return;

	MapEditorMissionLevel *ml = m_map->missionLevel(uuid, level);

	if (ml)
		missionLockAdd(mission, ml);
}



/**
 * @brief MapEditor::missionLockRemove
 * @param mission
 * @param lock
 */

void MapEditor::missionLockRemove(MapEditorMission *mission, MapEditorMissionLevel *lock)
{
	if (!m_map || !mission || !lock || !lock->editorMission())
		return;

	QVariantMap data;

	data.insert(QStringLiteral("uuid"), mission->uuid());
	data.insert(QStringLiteral("lockUuid"), lock->editorMission()->uuid());
	data.insert(QStringLiteral("lockLevel"), lock->level());

	EditorAction *action = new EditorAction(m_map, tr("Zárolás törlése: %1").arg(mission->name()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (m)
			m->lockRemove(action->data().value(QStringLiteral("lockUuid")).toString(),
						  action->data().value(QStringLiteral("lockLevel")).toInt());

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (m)
			m->lockAdd(action->data().value(QStringLiteral("lockUuid")).toString(),
					   action->data().value(QStringLiteral("lockLevel")).toInt());
	});

	m_undoStack->call(action);
}



/**
 * @brief MapEditor::storageAdd
 * @param storage
 * @param modifyFunc
 */

void MapEditor::storageAdd(MapEditorStorage *storage, QJSValue modifyFunc)
{
	if (!m_map || !storage)
		return;

	if (modifyFunc.isCallable())
		modifyFunc.call();

	QVariantMap data = storage->toVariantMap();
	data[QStringLiteral("id")] = m_map->nextIndexStorage();


	EditorAction *action = new EditorAction(m_map, tr("Új adatbank"));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorStorage *s = new MapEditorStorage(action->map());
		s->fromVariantMap(action->data());
		action->map()->storageList()->append(s);

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorStorage *s = action->map()->storage(action->data().value(QStringLiteral("id")).toInt());

		if (s)
			action->map()->storageList()->remove(s);
	});

	m_undoStack->call(action);
}





/**
 * @brief MapEditor::storageModify
 * @param storage
 * @param modifyFunc
 */

void MapEditor::storageModify(MapEditorStorage *storage, QJSValue modifyFunc)
{
	if (!m_map || !storage)
		return;

	QVariantMap data;
	data.insert(QStringLiteral("id"), storage->id());
	data.insert(QStringLiteral("storageFrom"), storage->toVariantMap(true));

	if (modifyFunc.isCallable())
		modifyFunc.call();

	data.insert(QStringLiteral("storageTo"), storage->toVariantMap(true));

	EditorAction *action = new EditorAction(m_map, tr("Adatbank módosítása"));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorStorage *s = action->map()->storage(action->data().value(QStringLiteral("id")).toInt());

		if (s)
			s->fromVariantMap(action->data().value(QStringLiteral("storageTo")).toMap(), true);

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorStorage *s = action->map()->storage(action->data().value(QStringLiteral("id")).toInt());

		if (s)
			s->fromVariantMap(action->data().value(QStringLiteral("storageFrom")).toMap(), true);
	});

	m_undoStack->call(action);
}




/**
 * @brief MapEditor::storageLoadEditor
 * @param module
 */

void MapEditor::storageLoadEditor(const QString &module)
{
	if (module.isEmpty())
		return;

	if (m_tmpStorage) {
		m_tmpStorage->deleteLater();
		m_tmpStorage = nullptr;
	}

	LOG_CTRACE("client") << "Load storage editor" << module;

	m_tmpStorage = new MapEditorStorage(m_map);
	m_tmpStorage->setModule(module);

	m_client->stackPushPage(QStringLiteral("MapEditorStorageEditor.qml"), QVariantMap{
								{ QStringLiteral("storage"), QVariant::fromValue(m_tmpStorage) },
								{ QStringLiteral("modified"), true },
							});
}




/**
 * @brief MapEditor::storageRemove
 * @param storage
 */

void MapEditor::storageRemove(MapEditorStorage *storage)
{
	if (!m_map || !storage)
		return;

	const QVariantMap &data = storage->toVariantMap();

	for (MapEditorChapter *ch : *m_map->chapterList()) {
		for (MapEditorObjective *o : *ch->objectiveList()) {
			if (o->storageId() == storage->id()) {
				LOG_CERROR("client") << "Storage used in objectives:" << storage->id();
				return;
			}
		}
	}

	EditorAction *action = new EditorAction(m_map, tr("Adatbank törlése"));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorStorage *s = action->map()->storage(action->data().value(QStringLiteral("id")).toInt());

		if (s)
			action->map()->storageList()->remove(s);

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorStorage *s = new MapEditorStorage(action->map());
		s->fromVariantMap(action->data());

		action->map()->storageList()->append(s);
	});

	m_undoStack->call(action);
}




/**
 * @brief MapEditor::chapterAdd
 * @param name
 */

void MapEditor::chapterAdd(const QString &name, MapEditorMissionLevel *missionLevel)
{
	if (!m_map)
		return;

	MapEditorChapter chapter;
	chapter.setId(m_map->nextIndexChapter());
	chapter.setName(name.isEmpty() ? tr("Új feladatcsoport") : name);

	QVariantMap data = chapter.toVariantMap();

	if (missionLevel) {
		data.insert(QStringLiteral("missionUuid"), missionLevel->editorMission()->uuid());
		data.insert(QStringLiteral("missionLevel"), missionLevel->level());
	}


	EditorAction *action = new EditorAction(m_map, tr("Új feladatcsoport: %1").arg(name));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = new MapEditorChapter(action->map());
		ch->fromVariantMap(action->data());

		action->map()->chapterList()->append(ch);

		if (action->data().contains(QStringLiteral("missionUuid"))) {
			MapEditorMissionLevel *ml = action->map()->missionLevel(
						action->data().value(QStringLiteral("missionUuid")).toString(),
						action->data().value(QStringLiteral("missionLevel")).toInt()
						);

			if (ml)
				ml->chapterAdd(ch);
		}
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("id")).toInt());

		if (!ch)
			return;

		if (action->data().contains(QStringLiteral("missionUuid"))) {
			MapEditorMissionLevel *ml = action->map()->missionLevel(
						action->data().value(QStringLiteral("missionUuid")).toString(),
						action->data().value(QStringLiteral("missionLevel")).toInt()
						);

			if (ml)
				ml->chapterRemove(ch);
		}

		action->map()->chapterList()->remove(ch);
	});

	m_undoStack->call(action);
}




/**
 * @brief MapEditor::chapterRemove
 * @param chapter
 */

void MapEditor::chapterRemove(MapEditorChapter *chapter)
{
	if (!m_map || !chapter)
		return;

	QVariantMap data = chapter->toVariantMap();

	QVariantList levels;

	for (MapEditorMission *mis : *m_map->missionList()) {
		for (MapEditorMissionLevel *ml : *mis->levelList()) {
			if (ml->chapterList().contains(chapter)) {
				levels.append(QVariantMap{
								  { QStringLiteral("uuid"), mis->uuid() },
								  { QStringLiteral("level"), ml->level() }
							  });
			}
		}
	}

	data.insert(QStringLiteral("usedLevels"), levels);

	EditorAction *action = new EditorAction(m_map, tr("Feladatcsoport törlése: %1").arg(chapter->name()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("id")).toInt());

		if (ch) {
			const QVariantList &levels = action->data().value(QStringLiteral("usedLevels")).toList();
			foreach (const QVariant &v, levels) {
				const QVariantMap &lm = v.toMap();

				MapEditorMissionLevel *ml = action->map()->missionLevel(lm.value(QStringLiteral("uuid")).toString(),
																		lm.value(QStringLiteral("level")).toInt());

				if (ml)
					ml->chapterRemove(ch);
			}

			action->map()->chapterList()->remove(ch);
		}
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = new MapEditorChapter(action->map());
		ch->fromVariantMap(action->data());

		action->map()->chapterList()->append(ch);

		ch->recalculateStorageCount();

		const QVariantList &levels = action->data().value(QStringLiteral("usedLevels")).toList();

		foreach (const QVariant &v, levels) {
			const QVariantMap &lm = v.toMap();

			MapEditorMissionLevel *ml = action->map()->missionLevel(lm.value(QStringLiteral("uuid")).toString(),
																	lm.value(QStringLiteral("level")).toInt());

			if (ml)
				ml->chapterAdd(ch);
		}
	});

	m_undoStack->call(action);
}



/**
 * @brief MapEditor::chapterModify
 * @param chapter
 * @param modifyFunc
 */

void MapEditor::chapterModify(MapEditorChapter *chapter, QJSValue modifyFunc)
{
	if (!m_map || !chapter)
		return;

	QVariantMap data;
	data.insert(QStringLiteral("id"), chapter->id());
	data.insert(QStringLiteral("from"), chapter->toVariantMap(true));

	if (modifyFunc.isCallable())
		modifyFunc.call();

	data.insert(QStringLiteral("to"), chapter->toVariantMap(true));

	EditorAction *action = new EditorAction(m_map, tr("Feladatcsoport módosítása: %1").arg(chapter->name()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("id")).toInt());

		if (!ch)
			return;

		ch->fromVariantMap(action->data().value(QStringLiteral("to")).toMap(), true);
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("id")).toInt());

		if (!ch)
			return;

		ch->fromVariantMap(action->data().value(QStringLiteral("from")).toMap(), true);

	});

	m_undoStack->call(action);
}




/**
 * @brief MapEditor::objectiveLoadEditor
 * @param module
 * @param storageModule
 * @param storage
 */

void MapEditor::objectiveLoadEditor(MapEditorChapter *chapter, const QString &module, const QString &storageModule, MapEditorStorage *storage)
{
	if (module.isEmpty() || !chapter)
		return;

	if (m_tmpObjective) {
		m_tmpObjective->deleteLater();
		m_tmpObjective = nullptr;
	}

	if (m_tmpStorage) {
		m_tmpStorage->deleteLater();
		m_tmpStorage = nullptr;
	}

	LOG_CTRACE("client") << "Load objective editor" << module << storageModule << storage;

	m_tmpObjective = new MapEditorObjective(m_map);
	m_tmpObjective->setModule(module);

	if (!storageModule.isEmpty()) {
		if (storage)
			m_tmpObjective->setStorageId(storage->id());
		else {
			m_tmpStorage = new MapEditorStorage(m_map);
			m_tmpStorage->setModule(storageModule);
		}
	}

	m_client->stackPushPage(QStringLiteral("MapEditorObjectiveEditor.qml"), QVariantMap{
								{ QStringLiteral("chapter"), QVariant::fromValue(chapter) },
								{ QStringLiteral("objective"), QVariant::fromValue(m_tmpObjective) },
								{ QStringLiteral("storage"), QVariant::fromValue(storage ? storage : m_tmpStorage) },
								{ QStringLiteral("modified"), true },
							});



}



/**
 * @brief MapEditor::objectiveAdd
 * @param data
 */

void MapEditor::objectiveAdd(MapEditorChapter *chapter, MapEditorObjective *objective, MapEditorStorage *storage, QJSValue modifyFunc)
{
	if (!m_map || !objective || !chapter)
		return;

	QVariantMap data;
	data.insert(QStringLiteral("chapterId"), chapter->id());


	int storageId = 0;

	if (storage) {
		MapEditorStorage s(m_map);
		s.setModule(storage->module());
		s.setData(storage->data());

		if (storage->id() > 0) {
			s.setId(storage->id());

			data.insert(QStringLiteral("storageFrom"), s.toVariantMap(true));

			if (modifyFunc.isCallable())
				modifyFunc.call();

			s.setData(storage->data());

			data.insert(QStringLiteral("storageTo"), s.toVariantMap(true));

		} else {
			s.setId(m_map->nextIndexStorage());

			if (modifyFunc.isCallable())
				modifyFunc.call();

			s.setData(storage->data());

			data.insert(QStringLiteral("storageTo"), s.toVariantMap());
		}

		storageId = s.id();

	}


	MapEditorObjective o(m_map);
	o.setUuid(QUuid::createUuid().toString(QUuid::WithoutBraces));
	o.setData(objective->data());
	o.setModule(objective->module());
	o.setStorageCount(objective->storageCount());
	o.setStorageId(storageId);


	data.insert(QStringLiteral("objective"), o.toVariantMap());



	EditorAction *action = new EditorAction(m_map, tr("Új feladat: %1").arg(chapter->name()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("chapterId")).toInt());

		if (!ch)
			return;

		if (action->data().contains(QStringLiteral("storageTo"))) {
			if (action->data().contains(QStringLiteral("storageFrom"))) {
				MapEditorStorage *s = action->map()->storage(action->data().value(QStringLiteral("storageTo")).toMap().value(QStringLiteral("id")).toInt());

				if (s)
					s->fromVariantMap(action->data().value(QStringLiteral("storageTo")).toMap(), true);
			} else {
				MapEditorStorage *s = new MapEditorStorage(action->map());
				s->fromVariantMap(action->data().value(QStringLiteral("storageTo")).toMap());
				action->map()->storageList()->append(s);
			}
		}

		MapEditorObjective *o = new MapEditorObjective(action->map());
		o->fromVariantMap(action->data().value(QStringLiteral("objective")).toMap());

		ch->objectiveList()->append(o);

		ch->recalculateObjectiveCount();
		ch->recalculateStorageCount();
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("chapterId")).toInt());

		if (!ch)
			return;

		MapEditorObjective *o = ch->objective(action->data().value(QStringLiteral("objective")).toMap().value(QStringLiteral("uuid")).toString());

		if (o)
			ch->objectiveList()->remove(o);

		if (action->data().contains(QStringLiteral("storageTo"))) {
			MapEditorStorage *s = action->map()->storage(action->data().value(QStringLiteral("storageTo")).toMap().value(QStringLiteral("id")).toInt());
			if (s && action->data().contains(QStringLiteral("storageFrom"))) {
				s->fromVariantMap(action->data().value(QStringLiteral("storageFrom")).toMap(), true);
			} else if (s) {
				action->map()->storageList()->remove(s);
			}
		}

		ch->recalculateObjectiveCount();
		ch->recalculateStorageCount();

	});

	m_undoStack->call(action);
}



/**
 * @brief MapEditor::objectiveModify
 * @param objective
 * @param storage
 * @param modifyFunc
 */

void MapEditor::objectiveModify(MapEditorObjective *objective, MapEditorStorage *storage, QJSValue modifyFunc)
{
	if (!m_map || !objective)
		return;

	QVariantMap data;
	data.insert(QStringLiteral("uuid"), objective->uuid());
	data.insert(QStringLiteral("objectiveFrom"), objective->toVariantMap(true));

	if (storage) {
		data.insert(QStringLiteral("storageId"), storage->id());
		data.insert(QStringLiteral("storageFrom"), storage->toVariantMap(true));
	}

	if (modifyFunc.isCallable())
		modifyFunc.call();

	data.insert(QStringLiteral("objectiveTo"), objective->toVariantMap(true));

	if (storage)
		data.insert(QStringLiteral("storageTo"), storage->toVariantMap(true));


	EditorAction *action = new EditorAction(m_map, tr("Feladat módosítása"));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		if (action->data().contains(QStringLiteral("storageId"))) {
			MapEditorStorage *s = action->map()->storage(action->data().value(QStringLiteral("storageId")).toInt());

			if (s)
				s->fromVariantMap(action->data().value(QStringLiteral("storageTo")).toMap(), true);
		}

		MapEditorObjective *o = action->map()->objective(action->data().value(QStringLiteral("uuid")).toString());

		if (o)
			o->fromVariantMap(action->data().value(QStringLiteral("objectiveTo")).toMap(), true);
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		if (action->data().contains(QStringLiteral("storageId"))) {
			MapEditorStorage *s = action->map()->storage(action->data().value(QStringLiteral("storageId")).toInt());

			if (s)
				s->fromVariantMap(action->data().value(QStringLiteral("storageFrom")).toMap(), true);
		}

		MapEditorObjective *o = action->map()->objective(action->data().value(QStringLiteral("uuid")).toString());

		if (o)
			o->fromVariantMap(action->data().value(QStringLiteral("objectiveFrom")).toMap(), true);

	});

	m_undoStack->call(action);
}



/**
 * @brief MapEditor::objectiveRemove
 * @param objectiveList
 */

void MapEditor::objectiveRemove(MapEditorChapter *chapter, const QList<MapEditorObjective *> &objectiveList)
{
	if (!m_map || !chapter || objectiveList.isEmpty())
		return;

	QVariantMap data;

	data.insert(QStringLiteral("chapterId"), chapter->id());

	QVariantList list;

	foreach (MapEditorObjective *o, objectiveList)
		list.append(o->toVariantMap());

	data.insert(QStringLiteral("list"), list);

	EditorAction *action = new EditorAction(m_map, tr("%1 feladat törlése").arg(objectiveList.size()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("chapterId")).toInt());

		if (!ch)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const QVariantMap &data = v.toMap();
			MapEditorObjective *o = ch->objective(data.value(QStringLiteral("uuid")).toString());

			if (o)
				ch->objectiveList()->remove(o);
		}

		ch->recalculateObjectiveCount();
		ch->recalculateStorageCount();
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("chapterId")).toInt());

		if (!ch)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const QVariantMap &data = v.toMap();
			MapEditorObjective *o = new MapEditorObjective(action->map());
			o->fromVariantMap(data);
			ch->objectiveList()->append(o);
		}

		ch->recalculateObjectiveCount();
		ch->recalculateStorageCount();
	});

	m_undoStack->call(action);
}



/**
 * @brief MapEditor::objectiveDuplicate
 * @param chapter
 * @param objective
 */

void MapEditor::objectiveDuplicate(MapEditorChapter *chapter, const QList<MapEditorObjective *> &objectiveList)
{
	if (!m_map || objectiveList.isEmpty() || !chapter)
		return;

	QVariantMap data;
	data.insert(QStringLiteral("chapterId"), chapter->id());

	QVariantList list;

	foreach (MapEditorObjective *objective, objectiveList) {
		MapEditorObjective o(m_map);
		o.setUuid(QUuid::createUuid().toString(QUuid::WithoutBraces));
		o.setData(objective->data());
		o.setModule(objective->module());
		o.setStorageCount(objective->storageCount());
		o.setStorageId(objective->storageId());
		list.append(o.toVariantMap());
	}

	data.insert(QStringLiteral("list"), list);



	EditorAction *action = new EditorAction(m_map, tr("%1 feladat kettőzése").arg(objectiveList.size()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("chapterId")).toInt());

		if (!ch)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const QVariantMap &data = v.toMap();
			MapEditorObjective *o = new MapEditorObjective(action->map());
			o->fromVariantMap(data);
			ch->objectiveList()->append(o);
		}

		ch->recalculateObjectiveCount();
		ch->recalculateStorageCount();

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *ch = action->map()->chapter(action->data().value(QStringLiteral("chapterId")).toInt());

		if (!ch)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const QVariantMap &data = v.toMap();
			MapEditorObjective *o = ch->objective(data.value(QStringLiteral("uuid")).toString());

			if (o)
				ch->objectiveList()->remove(o);

		}

		ch->recalculateObjectiveCount();
		ch->recalculateStorageCount();

	});

	m_undoStack->call(action);
}


/**
 * @brief MapEditor::objectiveCopyOrMove
 * @param chapter
 * @param objective
 * @param toChapterId
 * @param isCopy
 */

void MapEditor::objectiveCopyOrMove(MapEditorChapter *chapter, const QList<MapEditorObjective *> &objectiveList,
									const int &toChapterId, const bool &isCopy, const QString &chapterName)
{
	if (!m_map || objectiveList.isEmpty() || !chapter)
		return;

	QVariantMap data;
	data.insert(QStringLiteral("chapterId"), chapter->id());
	data.insert(QStringLiteral("copy"), isCopy);

	if (toChapterId < 0) {
		MapEditorChapter chapter;
		chapter.setId(m_map->nextIndexChapter());
		chapter.setName(chapterName.isEmpty() ? tr("Új feladatcsoport") : chapterName);

		data.insert(QStringLiteral("newChapter"), chapter.toVariantMap());
		data.insert(QStringLiteral("chapterToId"), chapter.id());
	} else {
		MapEditorChapter *chapterTo = m_map->chapter(toChapterId);

		if (!chapterTo)
			return;

		data.insert(QStringLiteral("chapterToId"), chapterTo->id());
	}



	QVariantList list;

	foreach (MapEditorObjective *objective, objectiveList) {
		QVariantMap d = objective->toVariantMap();
		if (isCopy)
			d.insert(QStringLiteral("newUuid"), QUuid::createUuid().toString(QUuid::WithoutBraces));
		list.append(d);
	}

	data.insert(QStringLiteral("list"), list);


	EditorAction *action = new EditorAction(m_map, isCopy ? tr("%1 feladat másolása").arg(objectiveList.size()) :
															tr("%1 feladat áthelyezése").arg(objectiveList.size()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *chFrom = action->map()->chapter(action->data().value(QStringLiteral("chapterId")).toInt());
		MapEditorChapter *chTo = nullptr;

		if (action->data().contains(QStringLiteral("newChapter"))) {
			chTo = new MapEditorChapter(action->map());
			chTo->fromVariantMap(action->data().value(QStringLiteral("newChapter")).toMap());
			action->map()->chapterList()->append(chTo);
		} else {
			chTo = action->map()->chapter(action->data().value(QStringLiteral("chapterToId")).toInt());
		}

		if (!chFrom || !chTo)
			return;

		const bool &isCopy = action->data().value(QStringLiteral("copy")).toBool();

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const QVariantMap &data = v.toMap();
			MapEditorObjective *oFrom = chFrom->objective(data.value(QStringLiteral("uuid")).toString());

			if (!oFrom)
				continue;

			MapEditorObjective *oTo = new MapEditorObjective(action->map());
			oTo->fromVariantMap(data);
			if (isCopy)
				oTo->setUuid(data.value(QStringLiteral("newUuid")).toString());

			chTo->objectiveList()->append(oTo);

			if (!isCopy)
				chFrom->objectiveList()->remove(oFrom);
		}

		chFrom->recalculateObjectiveCount();
		chFrom->recalculateStorageCount();

		chTo->recalculateObjectiveCount();
		chTo->recalculateStorageCount();

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorChapter *chFrom = action->map()->chapter(action->data().value(QStringLiteral("chapterId")).toInt());
		MapEditorChapter *chTo = action->map()->chapter(action->data().value(QStringLiteral("chapterToId")).toInt());

		if (!chFrom || !chTo)
			return;

		const bool &isCopy = action->data().value(QStringLiteral("copy")).toBool();

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const QVariantMap &data = v.toMap();
			const QString &uuid = isCopy ?
						data.value(QStringLiteral("newUuid")).toString() :
						data.value(QStringLiteral("uuid")).toString();

			MapEditorObjective *oTo = chTo->objective(uuid);

			if (!oTo)
				continue;

			if (!isCopy) {
				MapEditorObjective *oFrom = new MapEditorObjective(action->map());
				oFrom->fromVariantMap(data);
				chFrom->objectiveList()->append(oFrom);
			}

			chTo->objectiveList()->remove(oTo);
		}

		chFrom->recalculateObjectiveCount();
		chFrom->recalculateStorageCount();


		if (action->data().contains(QStringLiteral("newChapter"))) {
			action->map()->chapterList()->remove(chTo);
		} else {
			chTo->recalculateObjectiveCount();
			chTo->recalculateStorageCount();
		}

	});

	m_undoStack->call(action);
}





/**
 * @brief MapEditor::missionLevelAdd
 * @param mission
 * @return
 */

MapEditorMissionLevel *MapEditor::missionLevelAdd(MapEditorMission *mission)
{
	if (!m_map || !mission)
		return nullptr;

	MapEditorMissionLevel *level = mission->createNextLevel();

	if (!level)
		return nullptr;

	QVariantMap data = level->toVariantMap();
	data.insert(QStringLiteral("missionUuid"), mission->uuid());

	delete level;
	level = nullptr;

	EditorAction *action = new EditorAction(m_map, tr("Új szint: %1").arg(mission->name()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = action->map()->mission(action->data().value(QStringLiteral("missionUuid")).toString());

		if (!m)
			return;

		MapEditorMissionLevel *ml = m->createNextLevel();

		if (!ml)
			return;

		ml->fromVariantMap(action->data());

		m->levelList()->append(ml);
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = action->map()->mission(action->data().value(QStringLiteral("missionUuid")).toString());

		if (m) {
			MapEditorMissionLevel *ml = m->level(action->data().value(QStringLiteral("level")).toInt());
			if (ml)
				m->levelList()->remove(ml);
		}
	});

	m_undoStack->call(action);

	return m_map->missionLevel(mission->uuid(), data.value(QStringLiteral("level")).toInt());
}






/**
 * @brief MapEditor::missionLevelRemove
 * @param missionLevel
 */

void MapEditor::missionLevelRemove(MapEditorMissionLevel *missionLevel)
{
	if (!m_map || !missionLevel || !missionLevel->editorMission())
		return;

	if (!missionLevel->canDelete())
		return;

	QVariantMap data = missionLevel->toVariantMap();
	data.insert(QStringLiteral("missionUuid"), missionLevel->editorMission()->uuid());

	EditorAction *action = new EditorAction(m_map, tr("Szint törlése: %1 (%2)").arg(missionLevel->editorMission()->name()).arg(missionLevel->level()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = action->map()->mission(action->data().value(QStringLiteral("missionUuid")).toString());

		if (!m)
			return;

		MapEditorMissionLevel *ml = m->level(action->data().value(QStringLiteral("level")).toInt());

		if (!ml)
			return;

		m->levelRemove(ml);
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *m = action->map()->mission(action->data().value(QStringLiteral("missionUuid")).toString());

		if (m) {
			const int l = action->data().value(QStringLiteral("level")).toInt();

			MapEditorMissionLevel *ml = m->level(l);

			if (ml) {
				LOG_CERROR("client") << "Level" << l << "already exists in mission:" << qPrintable(m->uuid());
				ml->fromVariantMap(action->data());
			} else {
				ml = new MapEditorMissionLevel(m);
				ml->fromVariantMap(action->data());
				m->levelAdd(ml);
			}

		}
	});

	m_undoStack->call(action);
}




/**
 * @brief MapEditor::missionLevelModify
 * @param missionLevel
 * @param modifyFunc
 */

void MapEditor::missionLevelModify(MapEditorMissionLevel *missionLevel, QJSValue modifyFunc)
{
	if (!m_map || !missionLevel || !missionLevel->editorMission())
		return;

	QVariantMap data;
	data.insert(QStringLiteral("uuid"), missionLevel->editorMission()->uuid());
	data.insert(QStringLiteral("level"), missionLevel->level());
	data.insert(QStringLiteral("from"), missionLevel->toVariantMap(true));

	if (modifyFunc.isCallable())
		modifyFunc.call();

	data.insert(QStringLiteral("to"), missionLevel->toVariantMap(true));

	EditorAction *action = new EditorAction(m_map, tr("Szint módosítása: %1 (%2)").arg(missionLevel->editorMission()->name()).arg(missionLevel->level()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *mis = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (!mis)
			return;

		MapEditorMissionLevel *level = mis->level(action->data().value(QStringLiteral("level")).toInt());

		if (!level)
			return;

		level->fromVariantMap(action->data().value(QStringLiteral("to")).toMap(), true);
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMission *mis = action->map()->mission(action->data().value(QStringLiteral("uuid")).toString());

		if (!mis)
			return;

		if (!mis)
			return;

		MapEditorMissionLevel *level = mis->level(action->data().value(QStringLiteral("level")).toInt());

		if (!level)
			return;

		level->fromVariantMap(action->data().value(QStringLiteral("from")).toMap(), true);

	});

	m_undoStack->call(action);
}



/**
 * @brief MapEditor::missionLevelChapterAdd
 * @param missionLevel
 * @param chapterList
 */

void MapEditor::missionLevelChapterAdd(MapEditorMissionLevel *missionLevel, const QList<MapEditorChapter *> &chapterList)
{
	if (!m_map || !missionLevel || !missionLevel->editorMission() || chapterList.isEmpty())
		return;

	QVariantMap data;
	data.insert(QStringLiteral("missionUuid"), missionLevel->editorMission()->uuid());
	data.insert(QStringLiteral("missionLevel"), missionLevel->level());

	QVariantList list;

	foreach (MapEditorChapter *chapter, chapterList)
		list.append(chapter->id());

	data.insert(QStringLiteral("list"), list);

	EditorAction *action = new EditorAction(m_map, tr("Feleadatcsoport hozzáadása: %1 (%2)").arg(missionLevel->editorMission()->name()).arg(missionLevel->level()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const int &chapterId = v.toInt();

			ml->chapterAdd(chapterId);
		}

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const int &chapterId = v.toInt();

			ml->chapterRemove(chapterId);
		}
	});

	m_undoStack->call(action);
}


/**
 * @brief MapEditor::missionLevelChapterRemove
 * @param missionLevel
 * @param chapterList
 */

void MapEditor::missionLevelChapterRemove(MapEditorMissionLevel *missionLevel, const QList<MapEditorChapter *> &chapterList)
{
	if (!m_map || !missionLevel || !missionLevel->editorMission() || chapterList.isEmpty())
		return;

	QVariantMap data;
	data.insert(QStringLiteral("missionUuid"), missionLevel->editorMission()->uuid());
	data.insert(QStringLiteral("missionLevel"), missionLevel->level());

	QVariantList list;

	foreach (MapEditorChapter *chapter, chapterList)
		list.append(chapter->id());

	data.insert(QStringLiteral("list"), list);

	EditorAction *action = new EditorAction(m_map, tr("Feleadatcsoport eltávolítása: %1 (%2)").arg(missionLevel->editorMission()->name()).arg(missionLevel->level()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const int &chapterId = v.toInt();

			ml->chapterRemove(chapterId);
		}

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const int &chapterId = v.toInt();

			ml->chapterAdd(chapterId);
		}
	});

	m_undoStack->call(action);
}



/**
 * @brief MapEditor::missionLevelInventoryAdd
 * @param missionLevel
 * @param type
 * @return
 */

void MapEditor::missionLevelInventoryAdd(MapEditorMissionLevel *missionLevel, const QString &type)
{
	if (!m_map || !missionLevel || !missionLevel->editorMission())
		return;

	const GamePickable::GamePickableData &pickable = GamePickable::pickableDataHash().value(type);

	if (pickable.type == GamePickable::PickableInvalid)
		return;

	MapEditorInventory inventory(m_map);
	inventory.setInventoryid(m_map->nextIndexInventory());
	inventory.setModule(pickable.id);
	inventory.setCount(1);
	inventory.setBlock(0);

	QVariantMap data = inventory.toVariantMap();
	data.insert(QStringLiteral("missionUuid"), missionLevel->editorMission()->uuid());
	data.insert(QStringLiteral("missionLevel"), missionLevel->level());


	EditorAction *action = new EditorAction(m_map, tr("Új felszerelés: %1 (%2)").arg(missionLevel->editorMission()->name()).arg(missionLevel->level()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		MapEditorInventory *inventory = new MapEditorInventory(action->map());
		inventory->fromVariantMap(action->data());
		ml->inventoryAdd(inventory);

	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		ml->inventoryRemove(action->data().value(QStringLiteral("id")).toInt());
	});

	m_undoStack->call(action);
}





/**
 * @brief MapEditor::missionLevelInventoryRemove
 * @param missionLevel
 * @param inventory
 * @return
 */

void MapEditor::missionLevelInventoryRemove(MapEditorMissionLevel *missionLevel, const QList<MapEditorInventory *> &inventoryList)
{
	if (!m_map || !missionLevel || !missionLevel->editorMission() || inventoryList.isEmpty())
		return;

	QVariantMap data;
	data.insert(QStringLiteral("missionUuid"), missionLevel->editorMission()->uuid());
	data.insert(QStringLiteral("missionLevel"), missionLevel->level());

	QVariantList list;

	foreach (MapEditorInventory *inventory, inventoryList)
		list.append(inventory->toVariantMap());

	data.insert(QStringLiteral("list"), list);


	EditorAction *action = new EditorAction(m_map, tr("Felszerelés törlése: %1 (%2)").arg(missionLevel->editorMission()->name()).arg(missionLevel->level()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const QVariantMap &data = v.toMap();
			ml->inventoryRemove(data.value(QStringLiteral("id")).toInt());
		}
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		foreach (const QVariant &v, action->data().value(QStringLiteral("list")).toList()) {
			const QVariantMap &data = v.toMap();

			MapEditorInventory *inventory = new MapEditorInventory(action->map());
			inventory->fromVariantMap(data);
			ml->inventoryAdd(inventory);
		}

	});

	m_undoStack->call(action);
}





/**
 * @brief MapEditor::missionLevelInventoryModify
 * @param missionLevel
 * @param inventory
 * @param modifyFunc
 */

void MapEditor::missionLevelInventoryModify(MapEditorMissionLevel *missionLevel, MapEditorInventory *inventory, QJSValue modifyFunc)
{
	if (!m_map || !missionLevel || !missionLevel->editorMission() || !inventory)
		return;

	QVariantMap data = inventory->toVariantMap();
	data.insert(QStringLiteral("missionUuid"), missionLevel->editorMission()->uuid());
	data.insert(QStringLiteral("missionLevel"), missionLevel->level());


	data.insert(QStringLiteral("from"), inventory->toVariantMap(true));

	if (modifyFunc.isCallable())
		modifyFunc.call();

	data.insert(QStringLiteral("to"), inventory->toVariantMap(true));

	EditorAction *action = new EditorAction(m_map, tr("Felszerelés módosítása: %1 (%2)").arg(missionLevel->editorMission()->name()).arg(missionLevel->level()));
	action->setData(data);
	action->setRedoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		MapEditorInventory *inventory = ml->inventory(action->data().value(QStringLiteral("id")).toInt());

		if (!inventory)
			return;

		inventory->fromVariantMap(action->data().value(QStringLiteral("to")).toMap(), true);
	});
	action->setUndoFunc([action]{
		if (!action->map())
			return;

		MapEditorMissionLevel *ml = action->map()->missionLevel(
					action->data().value(QStringLiteral("missionUuid")).toString(),
					action->data().value(QStringLiteral("missionLevel")).toInt()
					);

		if (!ml)
			return;

		MapEditorInventory *inventory = ml->inventory(action->data().value(QStringLiteral("id")).toInt());

		if (!inventory)
			return;

		inventory->fromVariantMap(action->data().value(QStringLiteral("from")).toMap(), true);

	});

	m_undoStack->call(action);
}






/**
 * @brief MapEditor::missionLevelPlay
 * @param missionLevel
 * @param mode
 */

void MapEditor::missionLevelPlay(MapEditorMissionLevel *missionLevel, int mode)
{
	LOG_CTRACE("client") << "PLAY" << missionLevel << mode;

	if (!missionLevel)
		return;

	if (m_mapPlay->reloadMap(m_map))
		m_mapPlay->play(missionLevel, (GameMap::GameMode) mode);
}





/**
 * @brief MapPlayEditor::reloadMap
 * @param map
 */

bool MapPlayEditor::reloadMap(MapEditorMap *map)
{
	if (!map)
		return false;

	LOG_CTRACE("client") << "Reload MapPlayEditorMap";

	GameMap *gmap = GameMap::fromBinaryData(map->toBinaryData());

	if (!gmap) {
		m_client->messageError(tr("Nem lehet létrehozni a pályát!"), tr("Belső hiba"));
		return false;
	}

	loadGameMap(gmap);

	return true;
}


/**
 * @brief MapPlayEditor::play
 * @param missionLevel
 * @param mode
 * @return
 */

bool MapPlayEditor::play(MapEditorMissionLevel *missionLevel, const GameMap::GameMode &mode)
{
	if (!missionLevel || !missionLevel->editorMission())
		return false;

	MapPlayMissionLevel *playLevel = nullptr;

	for (MapPlayMission *mis : *m_missionList) {
		if (mis->uuid() == missionLevel->editorMission()->uuid()) {
			for (MapPlayMissionLevel *ml : *mis->missionLevelList()) {
				if (ml->level() == missionLevel->level()) {
					playLevel = ml;
					break;
				}
			}
		}
		if (playLevel)
			break;
	}


	if (!playLevel) {
		m_client->messageError(tr("Nem lehet elindítani a játékot!"), tr("Belső hiba"));
		return false;
	}

	return MapPlay::play(playLevel, mode);
}



/**
 * @brief MapPlayEditor::onCurrentGamePrepared
 */

void MapPlayEditor::onCurrentGamePrepared()
{
	if (!m_currentGame)
		return;

	m_currentGame->load();

	setGameState(StatePlay);
}


/**
 * @brief MapPlayEditor::onCurrentGameFinished
 */

void MapPlayEditor::onCurrentGameFinished()
{
	if (!m_currentGame || !m_client)
		return;

	AbstractLevelGame *g = m_currentGame;

	setCurrentGame(nullptr);
	m_client->setCurrentGame(nullptr);
	g->setReadyToDestroy(true);

	setGameState(StateFinished);
}
