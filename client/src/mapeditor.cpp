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
#include "abstractlevelgame.h"


/**
 * @brief MapEditor::MapEditor
 * @param parent
 */

MapEditor::MapEditor(QObject *parent)
	: QObject{parent}
	, m_client(Application::instance()->client())
	, m_undoStack(new EditorUndoStack(this))
{
	LOG_CTRACE("client") << "MapEditor created" << this;

	m_saveTimer.setSingleShot(true);
	m_saveTimer.setInterval(5000);
	connect(&m_saveTimer, &QTimer::timeout, this, &MapEditor::saveAuto);
	connect(m_undoStack, &EditorUndoStack::callCompleted, this, &MapEditor::onModified);
	connect(m_undoStack, &EditorUndoStack::undoCompleted, this, &MapEditor::onModified);
	connect(m_undoStack, &EditorUndoStack::redoCompleted, this, &MapEditor::onModified);
}


/**
 * @brief MapEditor::~MapEditor
 */

MapEditor::~MapEditor()
{
	delete m_undoStack;
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
 * @brief MapEditor::save
 */

void MapEditor::save()
{
	LOG_CWARNING("client") << "SAVE";
	setAutoSaved(true);
	setModified(false);
}


/**
 * @brief MapEditor::saveAuto
 */

void MapEditor::saveAuto()
{
	LOG_CWARNING("client") << "SAVE AUTO";
	setAutoSaved(true);
}



/**
 * @brief MapEditor::loadMapTest
 * @return
 */


bool MapEditor::loadMapTest()
{
	bool err = false;
	const QByteArray &b = Utils::fileContent("/home/valaczka/test.map", &err);

	if (err)
		return false;

	setDisplayName("test.map");

	return loadFromBinaryData(b);
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


	LOG_CDEBUG("client") << "Add mapimage provider for map:" << m_map->uuid();
	MapImage *mapImage = new MapImage(m_map);
	Application::instance()->engine()->addImageProvider(QStringLiteral("mapimage"), mapImage);

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
 * @brief MapEditor::onModified
 */

void MapEditor::onModified()
{
	setModified(true);
	setAutoSaved(false);
	m_saveTimer.start();
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
}




/**
 * @brief MapEditor::displayName
 * @return
 */

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

	MapEditorMissionLevel *level = mission.createNextLevel();
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
