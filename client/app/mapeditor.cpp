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


MapEditor::MapEditor(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
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
{
	/*m_map["getMissionList"] = &MapEditor::getMissionList;
	m_map["getCurrentMissionData"] = &MapEditor::getCurrentMissionData;
	m_map["missionModify"] = &MapEditor::missionModify;
	m_map["missionRemove"] = &MapEditor::missionRemove;
	m_map["missionLevelModify"] = &MapEditor::missionLevelModify;*/

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


	connect(this, &MapEditor::currentMissionChanged, this, &MapEditor::getCurrentMissionData);

	connect(db, &CosDb::undone, this, [=]() {
		setModified(true);
		getMissionList();
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

void MapEditor::open(const QString &filename)
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


	data["levels"] = db()->execSelectQuery("SELECT level, terrain, startHP, duration, startBlock, deathmatch, imageFolder, imageFile FROM missionLevels "
												"WHERE mission=?", {m_currentMission});





	QVariantList levelChapters = db()->execSelectQuery("SELECT r.rid, r.mission, r.level, chapters.name, r.chapter, "
"(SELECT COUNT(*) FROM "
"(SELECT DISTINCT mission FROM chapters LEFT JOIN blockChapterMapChapters ON (blockChapterMapChapters.chapter=chapters.id) "
"LEFT JOIN blockChapterMaps ON (blockChapterMaps.id=blockChapterMapChapters.blockid) WHERE chapters.id=r.chapter)) AS missionCount, "
"(SELECT SUM(CASE WHEN storage IS NULL THEN 1 ELSE storageCount END) FROM objectives WHERE objectives.chapter=r.chapter) as objectiveCount FROM "
"(SELECT blockChapterMaps.rowid||'-'||blockChapterMapChapters.rowid as rid,mission,level,chapter from blockChapterMaps "
"LEFT JOIN blockChapterMapChapters ON (blockChapterMapChapters.blockid=blockChapterMaps.id)) r "
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
