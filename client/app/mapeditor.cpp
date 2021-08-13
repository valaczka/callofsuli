/*
 * ---- Call of Suli ----
 *
 * mapeditor.cpp
 *
 * Created on: 2021. 05. 24.
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
#include "sqlimage.h"
#include "gameenemydata.h"
#include "chapterimporter.h"


#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QtAndroid>
#endif


MapEditor::MapEditor(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_gameData()
	, m_loadProgress(0.0)
	, m_loadProgressFraction(qMakePair<qreal, qreal>(0.0, 1.0))
	, m_loadAbortRequest(false)
	, m_filename()
	, m_modified(false)
	, m_loaded(false)
	, m_currentMission()
	, m_modelMissionList(nullptr)
	, m_modelTerrainList(nullptr)
	, m_modelLevelChapterList(nullptr)
	, m_modelObjectiveList(nullptr)
	, m_modelInventoryList(nullptr)
	, m_modelInventoryModules(nullptr)
	, m_modelLockList(nullptr)
	, m_modelDialogMissionList(nullptr)
	, m_modelDialogChapterList(nullptr)
	, m_modelDialogChapterMissionList(nullptr)
	, m_modelChapterList(nullptr)
{
	/*m_map["getMissionList"] = &MapEditor::getMissionList;
	m_map["getCurrentMissionData"] = &MapEditor::getCurrentMissionData;
	m_map["missionModify"] = &MapEditor::missionModify;
	m_map["missionRemove"] = &MapEditor::missionRemove;
	m_map["missionLevelModify"] = &MapEditor::missionLevelModify;*/

	QVariant v = Client::readJsonFile(QString("qrc:/internal/game/parameters.json"));

	if (v.isValid())
		m_gameData = v.toMap();
	else
		qWarning() << "Invalid json data";




	CosDb *db = new CosDb("editorDb", this);
	db->setDatabaseName(Client::standardPath("tmpmapeditor.db"));
	addDb(db, true);


	m_modelMissionList = new VariantMapModel({
												 "uuid",
												 "name",
												 "description",
												 "medalImage",
												 "campaign",
												 "mandatory"
											 },
											 this);


	m_modelTerrainList = new VariantMapModel({
												 "details",
												 "name",
												 "thumbnail",
												 "readableName"
											 },
											 this);

	m_modelTerrainList->setVariantList(Client::mapToList(Client::terrainMap(), "name"), "name");


	m_modelLevelChapterList = new VariantMapModel({
													  "rid",
													  "mission",
													  "level",
													  "chapter",
													  "name",
													  "missionCount",
													  "objectiveCount"
												  },
												  this);


	m_modelObjectiveList = new VariantMapModel({
												   "rid",
												   "uuid",
												   "sortid",
												   "chapter",
												   "objectiveModule",
												   "objectiveData",
												   "storage",
												   "storageCount",
												   "storageModule",
												   "storageData"
											   },
											   this);

	m_modelInventoryList = new VariantMapModel({
												   "rid",
												   "mission",
												   "level",
												   "module",
												   "block",
												   "icount"
											   },
											   this);


	m_modelInventoryModules = new VariantMapModel({
													  "module",
													  "name",
													  "icon"
												  },
												  this);


	QHash<QByteArray, GameEnemyData::InventoryType> ilist =  GameEnemyData::inventoryTypes();
	QHash<QByteArray, GameEnemyData::InventoryType>::const_iterator it;
	QVariantMap im;

	for (it = ilist.constBegin(); it != ilist.constEnd(); ++it) {
		im[it.key()] = QVariantMap({
									   { "name", it.value().name },
									   { "icon", it.value().icon }
								   });

	}

	m_modelInventoryModules->setVariantList(Client::mapToList(im, "module"), "module");



	m_modelLockList = new VariantMapModel({
											  "lock",
											  "level",
											  "name"
										  },
										  this);

	m_modelDialogMissionList = new VariantMapModel({
													   "uuid",
													   "name",
													   "level"
												   },
												   this);


	m_modelDialogChapterList = new VariantMapModel({
													   "id",
													   "name"
												   },
												   this);


	m_modelDialogChapterMissionList = new VariantMapModel({
															  "uuid",
															  "level",
															  "name",
															  "used"
														  },
														  this);


	m_modelChapterList = new VariantMapModel({
												 "chapter",
												 "name",
												 "missionCount",
												 "objectiveCount"
											 },
											 this);



	connect(this, &MapEditor::currentMissionChanged, this, &MapEditor::getCurrentMissionData);

	connect(db, &CosDb::undone, this, [=]() {
		setModified(true);
		getMissionList();
		getChapterList();
		getObjectiveList();
		getCurrentMissionData();
		missionLockGraphUpdate();
	});
}



/**
 * @brief MapEditor::~MapEditor
 */

MapEditor::~MapEditor()
{
	if (m_loaded) {
		qDebug() << "Remove image provider";
		QQmlEngine *engine = qmlEngine(this);
		if (engine)
			engine->removeImageProvider("mapdb");
	}


	delete m_modelMissionList;
	delete m_modelTerrainList;
	delete m_modelLevelChapterList;
	delete m_modelObjectiveList;
	delete m_modelInventoryList;
	delete m_modelInventoryModules;
	delete m_modelLockList;
	delete m_modelDialogMissionList;
	delete m_modelDialogChapterList;
	delete m_modelDialogChapterMissionList;
	delete m_modelChapterList;
}


/**
 * @brief MapEditor::isWithGraphviz
 * @return
 */

bool MapEditor::isWithGraphviz() const
{
#ifdef WITH_CGRAPH
	return true;
#endif

	return false;
}



/**
 * @brief MapEditor::defaultTerrain
 * @return
 */

TerrainData MapEditor::defaultTerrain() const
{
	if (!m_client)
		return TerrainData();

	return m_client->terrain(m_client->getSetting("defaultTerrain", "Canberra_Retreat").toString());
}


/**
 * @brief MapEditor::readableFilename
 * @return
 */

QString MapEditor::readableFilename() const
{
	if (m_filename.isEmpty())
		return tr("-- névtelen --");

	QFileInfo f(m_filename);
	return f.baseName()+" ["+f.absoluteFilePath()+"]";
}


/**
 * @brief MapEditor::open
 * @param filename
 */

void MapEditor::createTargets(const QString &filename)
{
	if (m_loaded) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Az adatbázis már meg van nyitva"));
		return;
	}

	if (filename.isEmpty() || !QFile::exists(filename)) {
		m_client->sendMessageWarning(tr("A fájl nem található"), filename);
		emit loadFailed();
		return;
	}

	if (!db()->isOpen()) {
		if (!db()->open()) {
			m_client->sendMessageError(tr("Belső hiba"), tr("Nem lehet előkészíteni az adatbázist!"));
			emit loadFailed();
			return;
		}
	}

	emit loadStarted(filename);

	AbstractActivity::run(&MapEditor::openPrivate, {{"filename", filename}});
}


/**
 * @brief MapEditor::create
 * @param filename
 */

void MapEditor::create(const QString &filename)
{
	if (m_loaded) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Az adatbázis már meg van nyitva"));
		return;
	}

	if (!filename.isEmpty() && QFile::exists(filename)) {
		m_client->sendMessageWarning(tr("A fájl már létezik"), filename);
		return;
	}

	if (!db()->isOpen()) {
		if (!db()->open()) {
			m_client->sendMessageError(tr("Belső hiba"), tr("Nem lehet előkészíteni az adatbázist!"));
			emit loadFailed();
			return;
		}
	}

	emit loadStarted(filename);

	AbstractActivity::run(&MapEditor::createPrivate, {{"filename", filename}});
}


/**
 * @brief MapEditor::save
 * @param filename
 */

void MapEditor::save(const QString &filename)
{
	if (!m_loaded || !db()->isOpen()) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Az adatbázis nincs megnyitva!"));
		emit saveFailed();
		return;
	}

	if (m_filename.isEmpty() && filename.isEmpty()) {
		emit saveDialogRequest(true);
		return;
	}

	AbstractActivity::run(&MapEditor::savePrivate, {{"filename", filename}});
}


/**
 * @brief MapEditor::saveCopy
 * @param filename
 */

void MapEditor::saveCopy(const QString &filename)
{
	if (!m_loaded || !db()->isOpen()) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Az adatbázis nincs megnyitva!"));
		emit saveFailed();
		return;
	}

	if (filename.isEmpty()) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Nincs fájlnév megadva!"));
		emit saveFailed();
		return;
	}

	AbstractActivity::run(&MapEditor::savePrivate, {{"filename", filename}, {"copy", true}});
}


/**
 * @brief MapEditor::loadAbort2
 */

void MapEditor::loadAbort()
{
	m_loadAbortRequest = false;
}



/**
 * @brief MapEditor::setLoadProgress
 * @param loadProgress
 */

bool MapEditor::setLoadProgress(qreal loadProgress)
{
	m_loadProgress = m_loadProgressFraction.first+(m_loadProgressFraction.second-m_loadProgressFraction.first)*loadProgress;
	emit loadProgressChanged(m_loadProgress);

	return m_loadAbortRequest;
}



/**
 * @brief MapEditor::setLoadProgressFraction
 * @param loadProgressFraction
 */

void MapEditor::setLoadProgressFraction(QPair<qreal, qreal> loadProgressFraction)
{
	if (m_loadProgressFraction == loadProgressFraction)
		return;

	m_loadProgressFraction = loadProgressFraction;
	emit loadProgressFractionChanged(m_loadProgressFraction);
}


void MapEditor::setFilename(QString filename)
{
	if (m_filename == filename)
		return;

	m_filename = filename;
	emit filenameChanged(m_filename);
}

void MapEditor::setModified(bool modified)
{
	if (m_modified == modified)
		return;

	m_modified = modified;
	emit modifiedChanged(m_modified);
}

void MapEditor::setLoaded(bool loaded)
{
	if (m_loaded == loaded)
		return;

	m_loaded = loaded;
	emit loadedChanged(m_loaded);

	if (m_loaded)
		emit loadSucceed();
}




/**
 * @brief MapEditor::checkPermissions
 */

void MapEditor::checkPermissions()
{

#ifdef Q_OS_ANDROID
	QtAndroid::PermissionResult result1 = QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE");
	QtAndroid::PermissionResult result2 = QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");

	QStringList permissions;

	if (result1 == QtAndroid::PermissionResult::Denied)
		permissions.append("android.permission.READ_EXTERNAL_STORAGE");

	if (result2 == QtAndroid::PermissionResult::Denied)
		permissions.append("android.permission.WRITE_EXTERNAL_STORAGE");

	if (!permissions.isEmpty()) {
		QtAndroid::PermissionResultMap resultHash = QtAndroid::requestPermissionsSync(permissions, 30000);

		QList<QtAndroid::PermissionResult> results = resultHash.values();
		if (results.isEmpty() || results.contains(QtAndroid::PermissionResult::Denied)) {
			emit storagePermissionsDenied();
			return;
		}
	}
#endif

	emit storagePermissionsGranted();
}


/**
 * @brief MapEditor::getMissionList
 */

void MapEditor::getMissionList()
{
	if (!m_loaded) {
		m_modelMissionList->clear();
		return;
	}

	QVariantList list = db()->execSelectQuery("SELECT uuid, campaign, mandatory, name, description, medalImage FROM missions");
	m_modelMissionList->setVariantList(list, "uuid");
}




/**
 * @brief MapEditor::getCurrentMissionData
 */

void MapEditor::getCurrentMissionData()
{
	if (!m_loaded || m_currentMission.isEmpty()) {
		m_modelLevelChapterList->clear();
		m_modelInventoryList->clear();
		emit currentMissionDataChanged({{}});
		return;
	}


	QVariantMap data = db()->execSelectQueryOneRow("SELECT uuid, campaign, mandatory, name, description, medalImage FROM missions WHERE uuid=?",
												   {m_currentMission});


	data["levels"] = db()->execSelectQuery("SELECT level, terrain, startHP, duration, startBlock, deathmatch, questions, imageFolder, imageFile FROM missionLevels "
												"WHERE mission=?", {m_currentMission});





	QVariantList levelChapters = db()->execSelectQuery("SELECT r.rid, r.mission, r.level, chapters.name, r.chapter, "
"(SELECT COUNT(*) FROM "
"(SELECT DISTINCT mission, level FROM chapters INNER JOIN blockChapterMapChapters ON (blockChapterMapChapters.chapter=chapters.id) "
"INNER JOIN blockChapterMaps ON (blockChapterMaps.id=blockChapterMapChapters.blockid) WHERE chapters.id=r.chapter)) AS missionCount, "
"(SELECT SUM(CASE WHEN storage IS NULL THEN 1 ELSE storageCount END) FROM objectives WHERE objectives.chapter=r.chapter) as objectiveCount FROM "
"(SELECT blockChapterMaps.rowid||'-'||blockChapterMapChapters.rowid as rid,mission,level,chapter from blockChapterMaps "
"INNER JOIN blockChapterMapChapters ON (blockChapterMapChapters.blockid=blockChapterMaps.id)) r "
"LEFT JOIN chapters ON (chapters.id=r.chapter) WHERE r.mission=?",
													   {m_currentMission}
													   );

	m_modelLevelChapterList->setVariantList(levelChapters, "rid");


	QVariantList list = db()->execSelectQuery("SELECT rowid as rid, mission, level, block, module, count as icount FROM inventories "
"WHERE mission=?", {m_currentMission});

	m_modelInventoryList->setVariantList(list, "rid");




	QVariantList missionLocks = db()->execSelectQuery("SELECT lock, level, name "
"FROM missionLocks LEFT JOIN missions ON (missions.uuid=missionLocks.lock) "
"WHERE mission=?", {m_currentMission});

	m_modelLockList->setVariantList(missionLocks, "lock");

	emit currentMissionDataChanged(data);
}


/**
 * @brief MapEditor::getFirstMission
 */

void MapEditor::getFirstMission()
{
	QString firstUuid = db()->execSelectQueryOneRow("SELECT uuid FROM missions ORDER BY name LIMIT 1").value("uuid").toString();
	if (!firstUuid.isEmpty())
		setCurrentMission(firstUuid);
}


/**
 * @brief MapEditor::getObjectiveList
 */

void MapEditor::getObjectiveList()
{
	if (!m_loaded) {
		m_modelObjectiveList->clear();
		return;
	}

	QVariantList objectives = db()->execSelectQuery("SELECT chapter||'-'||uuid as rid, "
"uuid, objectives.rowid as sortid, chapter, objectives.module as objectiveModule, storage, storageCount, objectives.data as objectiveData,"
"storages.module as storageModule, storages.data as storageData "
"FROM chapters LEFT JOIN objectives ON (objectives.chapter=chapters.id) "
"LEFT JOIN storages ON (storages.id=objectives.storage)");

	m_modelObjectiveList->setVariantList(objectives, "rid");
}


/**
 * @brief MapEditor::getChapterList
 */

void MapEditor::getChapterList()
{
	if (!m_loaded) {
		m_modelChapterList->clear();
		return;
	}

	QVariantList list = db()->execSelectQuery("SELECT id as chapter, name, "
"(SELECT COUNT(*) FROM "
"(SELECT DISTINCT mission, level FROM chapters INNER JOIN blockChapterMapChapters ON (blockChapterMapChapters.chapter=chapters.id) "
"INNER JOIN blockChapterMaps ON (blockChapterMaps.id=blockChapterMapChapters.blockid) WHERE chapters.id=c.id)) AS missionCount, "
"(SELECT SUM(CASE WHEN storage IS NULL THEN 1 ELSE storageCount END) FROM objectives WHERE objectives.chapter=c.id) as objectiveCount FROM chapters c"
													   );

	m_modelChapterList->setVariantList(list, "chapter");
}


/**
 * @brief MapEditor::play
 * @param data
 */

void MapEditor::play(QVariantMap data)
{
	int level = data.value("level", -1).toInt();

	if (m_currentMission.isEmpty() || level < 1)
		return;



	QVariantMap gamedata = db()->execSelectQueryOneRow("SELECT mission, name, level, terrain, startHP, duration, startBlock,"
													"imageFolder, imageFile FROM missionLevels "
													"LEFT JOIN missions ON (missions.uuid=missionLevels.mission) "
													"WHERE mission=? AND level=?",
													   {m_currentMission, level});
	if (gamedata.isEmpty()) {
		emit playFailed(tr("Adatbázis hiba"));
		return;
	}

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.0, 1.0));
	setLoadProgress(0.0);

	m_loadAbortRequest = false;
	GameMap *game = GameMap::fromDb(db(), this, "setLoadProgress", false);

	if (!game) {
		emit playFailed(tr("Adatbázis hiba"));
		return;
	}



	GameMatch *m_gameMatch = new GameMatch(game, this);
	m_gameMatch->setImageDbName("editorDb");
	m_gameMatch->setMissionUuid(gamedata.value("mission").toByteArray());
	m_gameMatch->setName(gamedata.value("name").toString());
	m_gameMatch->setLevel(gamedata.value("level").toInt());
	m_gameMatch->setTerrain(gamedata.value("terrain").toString());
	m_gameMatch->setStartHp(gamedata.value("startHP").toInt());
	m_gameMatch->setDuration(gamedata.value("duration").toInt());
	m_gameMatch->setStartBlock(gamedata.value("startBlock").toInt());


	QString err;
	if (!m_gameMatch->check(&err)) {
		emit playFailed(err);
		delete game;
		delete m_gameMatch;
		return;
	}



	m_gameMatch->setDeleteGameMap(true);

	QString imageFolder = gamedata.value("imageFolder").toString();
	QString imageFile = gamedata.value("imageFile").toString();

	if (!imageFolder.isEmpty() && !imageFile.isEmpty())
		m_gameMatch->setBgImage(imageFolder+"/"+imageFile);


	emit playReady(m_gameMatch);
}


/**
 * @brief MapEditor::missionAdd
 * @param data
 */

void MapEditor::missionAdd(QVariantMap data)
{
	if (!data.contains("uuid"))	data["uuid"] = QUuid::createUuid().toString();
	if (!data.contains("mandatory"))	data["mandatory"] = false;
	if (!data.contains("medalImage"))	data["medalImage"] = Client::medalIcons().at(QRandomGenerator::global()->bounded(Client::medalIcons().size()));

	QString uuid = data.value("uuid").toString();

	db()->undoLogBegin(tr("Új küldetés hozzáadása"));

	int ret = db()->execInsertQuery("INSERT INTO missions(?k?) values (?)", data);

	missionLevelAddPrivate(uuid, 1);

	db()->undoLogEnd();

	if (ret != -1) {
		setCurrentMission(uuid);
		setModified(true);
		missionLockGraphUpdate();
	}
}



/**
 * @brief MapEditor::getAvailableChapterList
 * @param level
 */

void MapEditor::missionLevelGetChapterList(int level)
{
	if (!m_loaded || m_currentMission.isEmpty()) {
		m_modelDialogChapterList->clear();
		return;
	}

	QVariantList list = db()->execSelectQuery("SELECT id, name FROM chapters WHERE id NOT IN "
"(SELECT chapter FROM blockChapterMaps INNER JOIN blockChapterMapChapters ON (blockChapterMapChapters.blockid = blockChapterMaps.id) "
"WHERE mission=? AND level=?)",
											  {m_currentMission, level});

	QVariantMap response;
	response["mission"] = m_currentMission;
	response["level"] = level;
	response["chapters"] = list;

	emit missionChapterListReady(response);

}


/**
 * @brief MapEditor::missionLevelAddChapter
 * @param data
 */

void MapEditor::missionLevelChapterAdd(QVariantMap data)
{
	int level = data.value("level", -1).toInt();

	if (m_currentMission.isEmpty() || level < 1)
		return;

	QVariantList list;

	if (data.contains("chapter"))
		list.append(data.value("chapter", 0).toInt());
	else if (data.contains("list"))
		list = data.value("list").toList();


	if (data.contains("name")) {
		QString chname = data.value("name").toString();
		int chid = db()->execInsertQuery("INSERT INTO chapters (?k?) VALUES (?)", {
											 {"name", chname}
										 });
		if (chid != -1)
			list.append(chid);
	}




	if (list.isEmpty())
		return;

	db()->undoLogBegin(tr("Szakasz hozzáadása"));

	int ret = missionLevelChapterAddPrivate(m_currentMission, level, list);

	db()->undoLogEnd();

	if (ret != -1) {
		getCurrentMissionData();
		getChapterList();
		setModified(true);
	}

}




/**
 * @brief MapEditor::missionLevelChapterRemove
 * @param data
 */

void MapEditor::missionLevelChapterRemove(QVariantMap data)
{
	int level = data.value("level", -1).toInt();
	int chapter = data.value("chapter", -1).toInt();


	db()->undoLogBegin(tr("Szakasz eltávolítása"));

	bool ret = db()->execSimpleQuery("DELETE FROM blockChapterMapChapters WHERE chapter=? "
"AND blockid = (SELECT id FROM blockChapterMaps WHERE mission=? AND level=? ORDER BY id LIMIT 1)",
									 {chapter, m_currentMission, level});

	db()->undoLogEnd();

	if (ret) {
		getCurrentMissionData();
		getChapterList();
		setModified(true);
	}

}



/**
 * @brief MapEditor::levelAdd
 * @param data
 */

void MapEditor::missionLevelAdd(QVariantMap data)
{
	if (m_currentMission.isEmpty())
		return;

	int level = data.value("level", 1).toInt();

	if (!db()->execSelectQuery("SELECT * FROM missionLevels WHERE mission=? AND level=?",
	{m_currentMission, level}).isEmpty()) {
		m_client->sendMessageWarning(tr("Szint hozzáadása"), tr("A %1. szint már létezik az aktuális küldetésben!").arg(level));
		return;
	}



	db()->undoLogBegin(tr("Szint hozzáadása"));

	int ret = missionLevelAddPrivate(m_currentMission, level);

	db()->undoLogEnd();

	if (ret != -1) {
		getCurrentMissionData();
		setModified(true);
		missionLockGraphUpdate();
	}

}



/**
 * @brief MapEditor::levelRemove
 * @param data
 */

void MapEditor::missionLevelRemove(QVariantMap data)
{
	int level = data.value("level", 0).toInt();

	if (m_currentMission.isEmpty() || level < 1)
		return;

	db()->undoLogBegin(tr("Szint törlése"));

	bool ret = db()->execSimpleQuery("DELETE FROM missionLevels WHERE mission=? AND level=?", {m_currentMission, level});

	db()->undoLogEnd();

	if (ret) {
		getCurrentMissionData();
		setModified(true);
		missionLockGraphUpdate();
	}
}





/**
 * @brief MapEditor::setCurrentMission
 * @param currentMission
 */

void MapEditor::setCurrentMission(QString currentMission)
{
	if (m_currentMission == currentMission)
		return;

	m_currentMission = currentMission;
	emit currentMissionChanged(m_currentMission);
}

void MapEditor::setModelMissionList(VariantMapModel *modelMissionList)
{
	if (m_modelMissionList == modelMissionList)
		return;

	m_modelMissionList = modelMissionList;
	emit modelMissionListChanged(m_modelMissionList);
}


/**
 * @brief MapEditor::missionModify
 * @param data
 */

void MapEditor::missionModify(QVariantMap data)
{
	if (m_currentMission.isEmpty())
		return;

	db()->undoLogBegin(tr("Küldetés módosítása"));

	bool ret = db()->execUpdateQuery("UPDATE missions SET ? WHERE uuid=:id", data, {{":id", m_currentMission}});

	db()->undoLogEnd();

	if (ret) {
		getMissionList();
		getCurrentMissionData();
		setModified(true);
		missionLockGraphUpdate();
	}
}


/**
 * @brief MapEditor::missionRemove
 * @param data
 */

void MapEditor::missionRemove()
{
	if (m_currentMission.isEmpty())
		return;

	db()->undoLogBegin(tr("Küldetés törlése"));

	bool ret = db()->execSimpleQuery("DELETE FROM missions WHERE uuid=?", {m_currentMission});

	db()->undoLogEnd();

	if (ret) {
		setCurrentMission("");
		getMissionList();
		getChapterList();
		getCurrentMissionData();
		setModified(true);
		missionLockGraphUpdate();
	}
}


/**
 * @brief MapEditor::missionLevelModify
 * @param data
 */

void MapEditor::missionLevelModify(QVariantMap data)
{
	if (m_currentMission.isEmpty())
		return;

	int level = data.value("level", -1).toInt();
	data.remove("level");

	db()->undoLogBegin(tr("Küldetés %1. szint módosítása").arg(level));

	bool ret = db()->execUpdateQuery("UPDATE missionLevels SET ? WHERE mission=:id AND level=:level", data,
									 {{":id", m_currentMission},
									  {":level", level}}
									 );

	db()->undoLogEnd();

	if (ret) {
		getCurrentMissionData();
		setModified(true);
	}
}


/**
 * @brief MapEditor::missionLockGetList
 */

void MapEditor::missionLockGetList(QVariantMap data)
{
	if (m_currentMission.isEmpty())
		return;

	QString lock = data.value("lock", "").toString();

	QVariantList list;

	if (lock.isEmpty()) {
		list = db()->execSelectQuery("SELECT uuid, name, level FROM missions "
"LEFT JOIN missionLevels ON (missionLevels.mission=missions.uuid) "
"WHERE uuid<>? AND uuid NOT IN (SELECT lock FROM missionLocks WHERE mission=?)",
									 {m_currentMission, m_currentMission});
	} else {
		list = db()->execSelectQuery("SELECT uuid, name, level FROM missions "
"LEFT JOIN missionLevels ON (missionLevels.mission=missions.uuid) "
"WHERE uuid=?",
									 {lock});
	}

	QVariantMap response;
	response["mission"] = m_currentMission;
	response["lock"] = lock;
	response["levels"] = list;

	emit missionLockListReady(response);
}


/**
 * @brief MapEditor::missionLockAdd
 * @param data
 */

void MapEditor::missionLockAdd(QVariantMap data)
{
	if (m_currentMission.isEmpty())
		return;

	if (!data.contains("lock")) {
		qWarning() << "Missing lock uuid";
		return;
	}

	data["mission"] = m_currentMission;

	db()->undoLogBegin(tr("Zárolás hozzáadása"));

	int ret = db()->execInsertQuery("INSERT INTO missionLocks (?k?) VALUES (?)", data);

	db()->undoLogEnd();

	if (ret != -1) {
		getCurrentMissionData();
		setModified(true);
		missionLockGraphUpdate();
	}
}



/**
 * @brief MapEditor::missionLockModify
 * @param data
 */

void MapEditor::missionLockModify(QVariantMap data)
{
	QString lock = data.value("lock", "").toString();
	int level = data.value("level", -1).toInt();

	if (m_currentMission.isEmpty() || lock.isEmpty())
		return;

	QVariantMap d;
	if (level > 0)
		d["level"] = level;
	else
		d["level"] = QVariant::Invalid;

	db()->undoLogBegin(tr("Zárolás módosítása"));

	bool ret = db()->execUpdateQuery("UPDATE missionLocks SET ? WHERE mission=:id AND lock=:lock", d,
									 {{":id", m_currentMission},
									  {":lock", lock}}
									 );

	db()->undoLogEnd();

	if (ret) {
		getCurrentMissionData();
		setModified(true);
		missionLockGraphUpdate();
	}
}


/**
 * @brief MapEditor::missionLockRemove
 * @param data
 */

void MapEditor::missionLockRemove(QVariantMap data)
{
	QString lock = data.value("lock", "").toString();

	if (m_currentMission.isEmpty() || lock.isEmpty())
		return;

	db()->undoLogBegin(tr("Zárolás törlése"));

	bool ret = db()->execSimpleQuery("DELETE FROM missionLocks WHERE mission=? AND lock=?", {m_currentMission, lock});

	db()->undoLogEnd();

	if (ret) {
		getCurrentMissionData();
		setModified(true);
		missionLockGraphUpdate();
	}
}



/**
 * @brief MapEditor::missionLockGraphUpdate
 */

void MapEditor::missionLockGraphUpdate()
{
	QByteArray img;

#ifdef WITH_CGRAPH
	QString dot;
	dot = "digraph missionlock {\n"
		  "node [shape=record,fontname=\"Arial\"];\n";


	QVariantList l = db()->execSelectQuery("SELECT uuid, name, level FROM missions "
"LEFT JOIN missionLevels ON (missionLevels.mission=missions.uuid) ");

	QHash<QString, QVariantMap> missions;
	int num = 0;

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();
		QString uuid = m.value("uuid").toString();
		int level = m.value("level", -1).toInt();

		if (missions.contains(uuid)) {
			QVariantList ll = missions.value(uuid).value("levels").toList();
			ll.append(level);
			missions[uuid]["levels"] = ll;
		} else {
			QVariantList ll;
			ll.append(level);
			missions[uuid] = QVariantMap({{"name", m.value("name").toString()},
										  {"levels", ll},
										  {"code", num++}
										 });
		}
	}



	QHash<QString, QVariantMap>::const_iterator it;
	for (it=missions.constBegin(); it != missions.constEnd(); ++it) {
		QVariantMap m = it.value();
		dot += QString("m%1 [label=\"{ %2 | {")
			   .arg(m.value("code").toInt())
			   .arg(m.value("name").toString());

		bool isFirst = true;

		foreach (QVariant v, m.value("levels").toList()) {
			if (isFirst)
				isFirst = false;
			else
				dot += " |";

			dot += QString(" <l%1> %2")
				   .arg(v.toInt())
				   .arg(v.toInt());
		}

		dot += " } } \"];\n";
	}



	QVariantList locks = db()->execSelectQuery("SELECT mission, lock, level FROM missionLocks");

	foreach (QVariant v, locks) {
		QVariantMap m = v.toMap();
		QString mission = m.value("mission").toString();
		QString lock = m.value("lock").toString();
		int level = m.value("level", -1).toInt();

		if (level < 1)
			level = 1;

		QString	fromCode = QString("m%1:l%2")
						   .arg(missions.value(lock).value("code").toInt())
						   .arg(level);


		QString toCode = QString("m%1")
						 .arg(missions.value(mission).value("code").toInt());

		dot += fromCode+" -> "+toCode+";\n";
	}

	dot += "}";

	img = m_client->graphvizImage(dot, "png");

#endif

	QFile f("/tmp/g.png");
	f.open(QIODevice::WriteOnly);
	f.write(img);
	f.close();
}




/**
 * @brief MapEditor::inventoryAdd
 * @param data
 */

void MapEditor::inventoryAdd(QVariantMap data)
{
	if (m_currentMission.isEmpty())
		return;

	if (!data.contains("level")) {
		qWarning() << "Missing level";
		return;
	}

	data["mission"] = m_currentMission;

	db()->undoLogBegin(tr("Új felszerelés"));

	int ret = db()->execInsertQuery("INSERT INTO inventories (?k?) VALUES (?)", data);
	db()->undoLogEnd();

	if (ret != -1) {
		getCurrentMissionData();
		setModified(true);
	}
}


/**
 * @brief MapEditor::inventoryModify
 * @param data
 */

void MapEditor::inventoryModify(QVariantMap data)
{
	if (m_currentMission.isEmpty())
		return;

	int level = data.value("level", -1).toInt();
	data.remove("level");

	int rid = data.value("rid", -1).toInt();
	data.remove("rid");

	db()->undoLogBegin(tr("Felszerelés módosítása"));

	bool ret = db()->execUpdateQuery("UPDATE inventories SET ? WHERE mission=:id AND level=:level AND rowid=:rid", data,
									 {
										 {":id", m_currentMission},
										 {":level", level},
										 {":rid", rid}
									 });

	db()->undoLogEnd();

	if (ret) {
		getCurrentMissionData();
		setModified(true);
	}
}


/**
 * @brief MapEditor::inventoryRemove
 * @param data
 */

void MapEditor::inventoryRemove(QVariantMap data)
{
	if (m_currentMission.isEmpty())
		return;

	int level = data.value("level", -1).toInt();
	data.remove("level");

	int rid = data.value("rid", -1).toInt();
	data.remove("rid");

	db()->undoLogBegin(tr("Felszerelés törlése"));

	bool ret = db()->execUpdateQuery("DELETE FROM inventories WHERE mission=:id AND level=:level AND rowid=:rid", data,
									 {
										 {":id", m_currentMission},
										 {":level", level},
										 {":rid", rid}
									 });

	db()->undoLogEnd();

	if (ret) {
		getCurrentMissionData();
		setModified(true);
	}
}



/**
 * @brief MapEditor::chapterAdd
 * @param data
 */

void MapEditor::chapterAdd(QVariantMap data)
{
	QString name = data.value("name", tr("Új szakasz")).toString();

	db()->undoLogBegin(tr("Új szakasz"));

	int chid = db()->execInsertQuery("INSERT INTO chapters (?k?) VALUES (?)", {
										 {"name", name}
									 });

	db()->undoLogEnd();

	if (chid != -1) {
		getChapterList();
		setModified(true);
	}
}




/**
	 * @brief MapEditor::missionLevelChapterModify
	 * @param data
	 */

void MapEditor::chapterModify(QVariantMap data)
{
	int chapter = data.value("chapter", -1).toInt();
	data.remove("chapter");

	db()->undoLogBegin(tr("Szakasz módosítása"));

	bool ret = db()->execUpdateQuery("UPDATE chapters SET ? WHERE id=:id", data,
									 {{":id", chapter}}
									 );

	db()->undoLogEnd();

	if (ret) {
		getCurrentMissionData();
		getChapterList();
		setModified(true);
	}
}



/**
 * @brief MapEditor::chapterRemove
 * @param data
 */

void MapEditor::chapterRemove(QVariantMap data)
{
	int chapter = data.value("chapter", -1).toInt();

	db()->undoLogBegin(tr("Szakasz törlése"));

	bool ret = db()->execSimpleQuery("DELETE FROM chapters WHERE id=?", {chapter});

	db()->undoLogEnd();

	if (ret) {
		getCurrentMissionData();
		getChapterList();
		setModified(true);
	}
}


/**
 * @brief MapEditor::chapterGetMissionList
 * @param data
 */

void MapEditor::chapterGetMissionList(QVariantMap data)
{
	int chapter = data.value("chapter", -1).toInt();

	if (chapter == -1) {
		qWarning() << "Invalid chapter";
		return;
	}


	QVariantList list = db()->execSelectQuery("SELECT uuid, name, level, "
"EXISTS(SELECT mission, level FROM blockChapterMapChapters LEFT JOIN blockChapterMaps ON (blockChapterMapChapters.blockid = blockChapterMaps.id) "
"WHERE chapter=? AND mission=missions.uuid AND level=missionLevels.level) AS used "
"FROM missions "
"LEFT JOIN missionLevels ON (missionLevels.mission=missions.uuid)",
											  {chapter});


	QVariantMap response;
	response["chapter"] = chapter;
	response["chapterName"] = data.value("name", "").toString();
	response["missions"] = list;

	emit chapterMissionListReady(response);
}







/**
 * @brief MapEditor::chapterMissionListModify
 * @param data
 */

void MapEditor::chapterMissionListModify(const int &chapter, VariantMapModel *model, const QString &selectField)
{
	if (!model)
		return;

	db()->undoLogBegin(tr("Szakasz küldetéseinek módosítása"));

	VariantMapData *data = model->variantMapData();

	for (int i=0; i<data->size(); i++) {
		int key = data->at(i).first;
		QVariantMap content = data->at(i).second;

		bool origSelected = content.value(selectField, false).toBool();
		bool currentSelected = model->getSelected().contains(key);


		if (origSelected && !currentSelected) {
			db()->execSimpleQuery("DELETE FROM blockChapterMapChapters WHERE chapter=? "
			"AND blockid = (SELECT id FROM blockChapterMaps WHERE mission=? AND level=? ORDER BY id LIMIT 1)",
								  {
									  chapter,
									  content.value("uuid", "").toString(),
									  content.value("level", 0).toInt(),
								  }
								  );


		} else if (!origSelected && currentSelected) {
			missionLevelChapterAddPrivate(content.value("uuid", "").toString(),
										  content.value("level", 0).toInt(),
										  {chapter});
		}

	}

	db()->undoLogEnd();

	getCurrentMissionData();
	getChapterList();
	setModified(true);

}


/**
 * @brief MapEditor::chapterImport
 * @param data
 */

void MapEditor::chapterImport(QVariantMap data)
{
	QUrl filename = data.value("filename").toUrl();
	int chapter = data.value("chapter", -1).toInt();

	if (filename.isEmpty()) {
		qWarning() << "Missing filename";
		return;
	}

	if (chapter == -1) {
		db()->undoLogBegin(tr("Új szakasz"));

		chapter = db()->execInsertQuery("INSERT INTO chapters (?k?) VALUES (?)", {
											 {"name", tr("Importált szakasz")}
										 });

		db()->undoLogEnd();

		getChapterList();
	}


	if (chapter == -1) {
		m_client->sendMessageError(tr("Importálás"), tr("Érvénytelen szakasz!"));
		return;
	}

	ChapterImporter importer(filename.toLocalFile());

	importer.setChapterId(chapter);

	if (!importer.import()) {
		m_client->sendMessageError(tr("Importálás"), tr("Hibás adatok"));
		return;
	}

	QVariantList l = importer.records();
	emit chapterImportReady(l);
}




/**
 * @brief MapEditor::objectiveAdd
 * @param data
 */

void MapEditor::objectiveAdd(QVariantMap data)
{
	if (!data.contains("chapter")) {
		qWarning() << "Missing chapter";
		return;
	}

	if (!data.contains("module")) {
		qWarning() << "Missing module";
		return;
	}

	QVariant storage = QVariant::Invalid;

	if (data.contains("storage"))
		storage = data.value("storage").toInt();

	db()->undoLogBegin(tr("Új feladat"));

	int chid = db()->execInsertQuery("INSERT INTO objectives (?k?) VALUES (?)", {
										 { "chapter", data.value("chapter").toInt() },
										 { "module", data.value("module").toString() },
										 { "storage", storage },
										 { "storageCount", data.value("storageCount", 0).toInt() },
										 { "data", data.value("data").toString() },
									 });

	db()->undoLogEnd();

	if (chid != -1) {
		getChapterList();
		getObjectiveList();
		getCurrentMissionData();
		setModified(true);
	}
}


/**
 * @brief MapEditor::objectiveModify
 * @param data
 */

void MapEditor::objectiveModify(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	data.remove("uuid");

	db()->undoLogBegin(tr("Feladat módosítása"));

	bool ret = db()->execUpdateQuery("UPDATE objectives SET ? WHERE uuid=:id", data,
									 {{":id", uuid}}
									 );

	db()->undoLogEnd();

	if (ret) {
		getChapterList();
		getObjectiveList();
		getCurrentMissionData();
		setModified(true);
	}
}


/**
 * @brief MapEditor::objectiveRemove
 * @param data
 */

void MapEditor::objectiveRemove(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();

	db()->undoLogBegin(tr("Feladat törlése"));

	bool ret = db()->execSimpleQuery("DELETE FROM objectives WHERE uuid=?", {uuid});

	db()->undoLogEnd();

	if (ret) {
		getChapterList();
		getObjectiveList();
		getCurrentMissionData();
		setModified(true);
	}
}




/**
 * @brief MapEditor::objectiveCopy
 * @param data
 */

void MapEditor::objectiveCopy(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	int chapter = data.value("chapter", -1).toInt();

	db()->undoLogBegin(tr("Feladat kettőzése"));

	bool ret = db()->execSimpleQuery("INSERT INTO objectives(uuid, chapter, module, storage, storageCount, data) "
"SELECT ?, ?, module, storage, storageCount, data FROM objectives WHERE uuid=?",
									 {QUuid::createUuid().toString(), chapter, uuid});

	db()->undoLogEnd();

	if (ret) {
		getChapterList();
		getObjectiveList();
		getCurrentMissionData();
		setModified(true);
	}

}



/**
 * @brief MapEditor::objectiveImport
 * @param data
 */

void MapEditor::objectiveImport(QVariantMap data)
{
	QVariantList list = data.value("list").toList();

	if (list.isEmpty()) {
		qWarning() << "List empty";
		return;
	}

	db()->undoLogBegin(tr("%1 célpont importálása").arg(list.size()));

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();

		m["uuid"] = QUuid::createUuid().toString();

		int ret = db()->execInsertQuery("INSERT INTO objectives(?k?) values (?)", m);

		if (ret == -1) {
			qWarning() << "Error" << m;
		}
	}

	db()->undoLogEnd();

	getChapterList();
	getObjectiveList();
	getCurrentMissionData();
	setModified(true);
}















void MapEditor::setModelChapterList(VariantMapModel *modelChapterList)
{
	if (m_modelChapterList == modelChapterList)
		return;

	m_modelChapterList = modelChapterList;
	emit modelChapterListChanged(m_modelChapterList);
}





void MapEditor::setModelDialogChapterMissionList(VariantMapModel *modelDialogChapterMissionList)
{
	if (m_modelDialogChapterMissionList == modelDialogChapterMissionList)
		return;

	m_modelDialogChapterMissionList = modelDialogChapterMissionList;
	emit modelDialogChapterMissionListChanged(m_modelDialogChapterMissionList);
}








void MapEditor::setModelDialogChapterList(VariantMapModel *modelDialogChapterList)
{
	if (m_modelDialogChapterList == modelDialogChapterList)
		return;

	m_modelDialogChapterList = modelDialogChapterList;
	emit modelDialogChapterListChanged(m_modelDialogChapterList);
}

void MapEditor::setModelDialogMissionList(VariantMapModel *modelDialogMissionList)
{
	if (m_modelDialogMissionList == modelDialogMissionList)
		return;

	m_modelDialogMissionList = modelDialogMissionList;
	emit modelDialogMissionListChanged(m_modelDialogMissionList);
}

void MapEditor::setModelLockList(VariantMapModel *modelLockList)
{
	if (m_modelLockList == modelLockList)
		return;

	m_modelLockList = modelLockList;
	emit modelLockListChanged(m_modelLockList);
}





void MapEditor::setModelInventoryModules(VariantMapModel *modelInventoryModules)
{
	if (m_modelInventoryModules == modelInventoryModules)
		return;

	m_modelInventoryModules = modelInventoryModules;
	emit modelInventoryModulesChanged(m_modelInventoryModules);
}





void MapEditor::setModelInventoryList(VariantMapModel *modelInventoryList)
{
	if (m_modelInventoryList == modelInventoryList)
		return;

	m_modelInventoryList = modelInventoryList;
	emit modelInventoryListChanged(m_modelInventoryList);
}

void MapEditor::setModelObjectiveList(VariantMapModel *modelObjectiveList)
{
	if (m_modelObjectiveList == modelObjectiveList)
		return;

	m_modelObjectiveList = modelObjectiveList;
	emit modelObjectiveListChanged(m_modelObjectiveList);
}

void MapEditor::setModelLevelChapterList(VariantMapModel *modelLevelChapterList)
{
	if (m_modelLevelChapterList == modelLevelChapterList)
		return;

	m_modelLevelChapterList = modelLevelChapterList;
	emit modelLevelChapterListChanged(m_modelLevelChapterList);
}


/**
 * @brief MapEditor::setModelTerrainList
 * @param modelTerrainList
 */

void MapEditor::setModelTerrainList(VariantMapModel *modelTerrainList)
{
	if (m_modelTerrainList == modelTerrainList)
		return;

	m_modelTerrainList = modelTerrainList;
	emit modelTerrainListChanged(m_modelTerrainList);
}


/**
 * @brief MapEditor::openPrivate
 * @param data
 */

void MapEditor::openPrivate(QVariantMap data)
{
	QString filename = data.value("filename").toString();

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.0, 0.2));
	setLoadProgress(0.0);

	QByteArray d = Client::fileContent(filename);

	GameMap *game = GameMap::fromBinaryData(d, this, "setLoadProgress");

	if (!game) {
		m_client->sendMessageError(tr("Hibás fájl"), filename);
		db()->close();
		emit loadFailed();
		return;
	}

	if (!loadDatabasePrivate(game, filename))
		emit loadFailed();

	delete game;
}



/**
 * @brief MapEditor::createPrivate
 * @param data
 */

void MapEditor::createPrivate(QVariantMap data)
{
	QString filename = data.value("filename").toString();

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.0, 0.2));
	setLoadProgress(0.0);

	QUuid uuid = QUuid::createUuid();
	GameMap map(uuid.toByteArray());

	if (loadDatabasePrivate(&map, filename)) {
		setModified(true);
	} else {
		emit loadFailed();
	}
}



/**
 * @brief MapEditor::savePrivate
 * @param data
 */

void MapEditor::savePrivate(QVariantMap data)
{
	QString filename = data.value("filename").toString();
	bool isCopy = data.value("copy", false).toBool();

	if (filename.isEmpty() && !isCopy)
		filename = m_filename;

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.0, 0.9));
	setLoadProgress(0.0);

	m_loadAbortRequest = false;
	GameMap *game = GameMap::fromDb(db(), this, "setLoadProgress");

	if (!game) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Adatbázis hiba"));
		emit saveFailed();
		return;
	}

	if (isCopy)
		game->regenerateUuids();


	QFile f(filename);
	if (!f.open(QIODevice::WriteOnly)) {
		m_client->sendMessageError(tr("Írási hiba"), filename);
		emit saveFailed();
		return;
	}

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.9, 1.0));
	QByteArray d = game->toBinaryData();

	f.write(d);
	f.close();

	if (!isCopy)
		setModified(false);

	emit saveSucceed(filename, isCopy);

	if (m_filename != filename && !isCopy) {
		setFilename(filename);
	}

	delete game;
}




/**
 * @brief MapEditor::clientSetup
 */

void MapEditor::clientSetup()
{
	if (!m_client)
		return;
}


/**
 * @brief MapEditor::missionLevelDefaults
 * @param level
 * @return
 */

QVariantMap MapEditor::missionLevelDefaults(const int &level)
{
	return m_gameData
			.value("level").toMap()
			.value(QString("%1").arg(level)).toMap()
			.value("defaults").toMap();
}


/**
 * @brief MapEditor::missionLevelAddPrivate
 * @param level
 * @return
 */


int MapEditor::missionLevelAddPrivate(const QString &mission, const int &level)
{
	QVariantMap defaults = missionLevelDefaults(level);

	int ret = db()->execInsertQuery("INSERT INTO missionLevels (?k?) VALUES (?)", {
										{"mission", mission},
										{"level", level},
										{"terrain", defaultTerrain().name},
										{"duration", defaults.value("duration", 120).toInt()},
										{"startHP", defaults.value("startHP", 3).toInt()},
										{"questions", defaults.value("questions", 0.5).toReal()},
										{"deathmatch", defaults.value("deathmatch", false).toBool()}
									});

	if (ret != -1) {
		QVariantList l = defaults.value("inventory").toList();

		foreach (QVariant v, l) {
			QVariantMap m = v.toMap();
			m["mission"] = mission;
			m["level"] = level;

			db()->execInsertQuery("INSERT INTO inventories (?k?) VALUES (?)", m);
		}
	}

	return ret;
}





/**
 * @brief MapEditor::missionLevelChapterAddPrivate
 * @param mission
 * @param level
 * @param list
 * @return
 */

int MapEditor::missionLevelChapterAddPrivate(const QString &mission, const int &level, const QVariantList &list)
{
	int blockid = db()->execSelectQueryOneRow("SELECT id FROM blockChapterMaps WHERE mission=? AND level=? ORDER BY id LIMIT 1",
											  {mission, level}).value("id", -1).toInt();

	if (blockid == -1) {
		blockid = db()->execInsertQuery("INSERT INTO blockChapterMaps (?k?) VALUES (?)", {
											{"mission", mission},
											{"level", level}
										});
	}


	int ret = -1;

	foreach (QVariant v, list) {
		ret = db()->execInsertQuery("INSERT INTO blockChapterMapChapters (?k?) VALUES (?)", {
										{"blockid", blockid},
										{"chapter", v.toInt()}
									});

		if (ret == -1)
			break;
	}

	return ret;
}





/**
 * @brief MapEditor::loadDatabasePrivate
 * @param game
 * @return
 */


bool MapEditor::loadDatabasePrivate(GameMap *game, const QString &filename)
{
	setLoadProgressFraction(qMakePair<qreal, qreal>(0.2, 0.6));
	game->setProgressFunc(this, "setLoadProgress");


	// Generate auto mission medals

	foreach(GameMap::Campaign *c, game->campaigns()) {
		foreach(GameMap::Mission *m, c->missions()) {
			if (m->medalImage().isEmpty())
				m->setMedalImage(Client::medalIcons().at(QRandomGenerator::global()->bounded(Client::medalIcons().size())));
		}
	}

	foreach(GameMap::Mission *m, game->orphanMissions()) {
		if (m->medalImage().isEmpty())
			m->setMedalImage(Client::medalIcons().at(QRandomGenerator::global()->bounded(Client::medalIcons().size())));
	}


	if (!game->toDb(db())) {
		m_client->sendMessageError(tr("Adatfájl hiba"), db()->databaseName());
		db()->close();
		return false;
	}

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.6, 1.0));
	if (!createTriggersPrivate())
		return false;

	qDebug() << "Add sqlimage provider";
	QQmlEngine *engine = qmlEngine(this);
	SqlImage *sqlImage = new SqlImage(m_client, db(), "images");
	engine->addImageProvider("mapdb", sqlImage);
	setFilename(filename);
	setLoaded(true);

	return true;
}






/**
 * @brief MapEditor::createTriggersPrivate
 * @return
 */

bool MapEditor::createTriggersPrivate()
{
	QStringList tableList;
	tableList << "map";
	tableList << "chapters";
	tableList << "storages";
	tableList << "objectives";
	tableList << "campaigns";
	tableList << "campaignLocks";
	tableList << "missions";
	tableList << "missionLocks";
	tableList << "missionLevels";
	tableList << "blockChapterMaps";
	tableList << "blockChapterMapBlocks";
	tableList << "blockChapterMapChapters";
	tableList << "blockChapterMapFavorites";
	tableList << "inventories";
	tableList << "images";


	qreal step = 0.0;
	qreal maxStep = (qreal) tableList.count()+1;

	setLoadProgress(step/maxStep);

	if (!db()->createUndoTables())	return false;

	setLoadProgress(++step/maxStep);


	foreach (QString k, tableList) {
		db()->createTrigger(k);
		setLoadProgress(++step/maxStep);
	}

	return true;
}
