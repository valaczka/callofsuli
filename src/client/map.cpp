/*
 * ---- Call of Suli ----
 *
 * map.cpp
 *
 * Created on: 2020. 03. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Map
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "map.h"

#define DATETIME_JSON_FORMAT QString("yyyy-MM-dd hh:mm:ss")

Map::Map(QObject *parent)
	: AbstractActivity("mapDB", parent)
{
	m_tableNames << "info"
				 << "intro"
				 << "campaign"
				 << "mission"
				 << "missionLevel"
				 << "summary"
				 << "summaryLevel"
				 << "bindCampaignMission"
				 << "chapter"
				 << "bindMissionChapter"
				 << "bindIntroCampaign"
				 << "bindIntroMission"
				 << "bindIntroSummary"
				 << "bindIntroChapter"
				 << "bindObjectiveChapter"
				 << "bindObjectiveSummary";

	m_mapType = MapInvalid;
}


/**
 * @brief Map::~Map
 */

Map::~Map()
{
	if (m_db->isOpen())
		m_db->close();

	if (QFile::exists(m_databaseFile)) {
		qDebug() << tr("Remove temporary map file ")+m_databaseFile << QFile::remove(m_databaseFile);
	}
}




/**
 * @brief Map::create
 * @return
 */

bool Map::create()
{
	Q_ASSERT (m_client);

	if (!databaseOpen() || !databasePrepare()) {
		return false;
	}

	setMapUuid(QUuid::createUuid().toString());
	setMapTimeCreated(QDateTime::currentDateTime().toString(DATETIME_JSON_FORMAT));
	updateMapOriginalFile("");

	emit mapLoaded();

	return true;
}




/**
 * @brief Map::save
 */

void Map::save(const bool &binaryFormat)
{
	QByteArray data = saveToJson(binaryFormat);

	if (!m_mapOriginalFile.isEmpty()) {
		qDebug() << tr("Adatbázis mentése fájlba: ")+m_mapOriginalFile;
		if (saveToFile(m_mapOriginalFile, data))
			emit mapSaved(data, m_mapUuid);
	} else {
		qDebug() << tr("Adatbázis mentése");
		emit mapSaved(data, m_mapUuid);
	}
}






/**
 * @brief Map::loadFromJson
 * @param data
 * @return
 */

bool Map::loadFromJson(const QByteArray &data, const bool &binaryFormat)
{
	Q_ASSERT (m_client);

	if (!databaseOpen() || !databasePrepare()) {
		return false;
	}

	QJsonDocument doc = binaryFormat ? QJsonDocument::fromBinaryData(data) : QJsonDocument::fromJson(data);

	if (doc.isNull())
		return false;

	QJsonObject root = doc.object();

	QJsonObject fileinfo = root["callofsuli"].toObject();

	if (fileinfo.isEmpty()) {
		return false;
	}

	foreach (QString t, m_tableNames) {
		if (!JsonToTable(root[t].toArray(), t, false))
			return false;
	}

	if (!JsonToTable(root["objective"].toArray(), "objective", true))
		return false;

	QString uuid = fileinfo["uuid"].toString();
	setMapUuid(uuid.isEmpty() ? QUuid::createUuid().toString() : uuid);
	setMapTimeCreated(fileinfo["timeCreated"].toString());

	emit mapLoaded();

	return true;
}


/**
 * @brief Map::loadFromFile
 * @param filename
 * @return
 */

bool Map::loadFromFile(const QString &filename, const bool &binaryFormat)
{
	QFile f(filename);

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		m_client->sendMessageError(tr("Fájl megnyitási hiba"), tr("A fájl nem található vagy nem olvasható!"), filename);
		return false;
	}

	QByteArray b = f.readAll();

	f.close();

	bool ret = loadFromJson(b, binaryFormat);

	return ret;
}


/**
 * @brief Map::loadFromBackup
 * @return
 */

bool Map::loadFromBackup()
{
	Q_ASSERT(m_client);

	if (!databaseOpen()) {
		return false;
	}

	QVariantMap m = m_db->runSimpleQuery("SELECT originalFile, uuid, timeCreated from mapeditor");
	if (!m["error"].toBool() && m["records"].toList().count()) {
		QVariantMap r = m["records"].toList().value(0).toMap();
		QString filename = r.value("originalFile").toString();
		QString uuid = r.value("uuid").toString();
		QString timeCreated = r.value("timeCreated").toString();

		setMapUuid(uuid.isEmpty() ? QUuid::createUuid().toString() : uuid);
		setMapTimeCreated(timeCreated);
		setMapOriginalFile(filename);

		emit mapLoadedFromBackup();
		emit mapLoaded();
		return true;
	}

	return false;
}



/**
 * @brief Map::saveToJson
 * @return
 */

QByteArray Map::saveToJson(const bool &binaryFormat)
{
	QJsonObject root;

	QJsonObject fileinfo;
	fileinfo["versionMajor"] = m_client->clientVersionMajor();
	fileinfo["versionMinor"] = m_client->clientVersionMinor();
	fileinfo["uuid"] = m_mapUuid;
	fileinfo["timeCreated"] = m_mapTimeCreated.isEmpty() ? QDateTime::currentDateTime().toString(DATETIME_JSON_FORMAT) : m_mapTimeCreated;
	fileinfo["timeModified"] = QDateTime::currentDateTime().toString(DATETIME_JSON_FORMAT);

	root["callofsuli"] = fileinfo;

	foreach (QString t, m_tableNames) {
		root[t] = tableToJson(t);
	}

	root["objective"] = tableToJson("objective", true);

	QJsonDocument d(root);


	return binaryFormat ? d.toBinaryData() : d.toJson(QJsonDocument::Indented);
}



/**
 * @brief Map::saveToFile
 * @param filename
 * @return
 */

bool Map::saveToFile(const QString &filename, const QByteArray &data)
{
	QSaveFile f(filename);

	if (!f.open(QIODevice::WriteOnly)) {
		m_client->sendMessageError(tr("Mentési hiba"), tr("Nem sikerült menteni a fájlt!"), filename);
		return false;
	}

	if (data.isNull()) {
		QByteArray b = saveToJson(true);
		f.write(b);
	} else {
		f.write(data);
	}

	if (!f.commit()) {
		m_client->sendMessageError(tr("Mentési hiba"), tr("Nem sikerült menteni a fájlt!"), filename);
		return false;
	}

	return true;
}







/**
 * @brief Map::updateMapOriginalFile
 * @param filename
 */

void Map::updateMapOriginalFile(const QString &filename)
{
	QVariantMap l;
	l["originalFile"] = filename;
	l["uuid"] = m_mapUuid;
	l["timeCreated"] = m_mapTimeCreated;
	m_db->execInsertQuery("INSERT OR REPLACE INTO mapeditor (?k?) VALUES(?)", l);
	setMapOriginalFile(filename);
}



void Map::setMapOriginalFile(QString mapOriginalFile)
{
	if (m_mapOriginalFile == mapOriginalFile)
		return;

	m_mapOriginalFile = mapOriginalFile;
	emit mapOriginalFileChanged(m_mapOriginalFile);
}




void Map::setMapTimeCreated(QString mapTimeCreated)
{
	if (m_mapTimeCreated == mapTimeCreated)
		return;

	m_mapTimeCreated = mapTimeCreated;
	emit mapTimeCreatedChanged(m_mapTimeCreated);
}



void Map::setMapUuid(QString mapUuid)
{
	if (m_mapUuid == mapUuid)
		return;

	m_mapUuid = mapUuid;
	emit mapUuidChanged(m_mapUuid);
}



/**
 * @brief Map::tableToJson
 * @param table
 * @return
 */


QJsonArray Map::tableToJson(const QString &table, const bool &convertData)
{
	QVariantMap m = m_db->runSimpleQuery("SELECT * from "+table+" ORDER BY rowid");

	if (m["error"].toBool()) {
		return QJsonArray();
	}

	QJsonArray list;

	if (convertData) {
		QVariantList r = m["records"].toList();
		for (int i=0; i<r.count(); ++i) {
			QJsonObject rrObj;
			QVariantMap rr = r.value(i).toMap();

			QStringList keys = rr.keys();

			foreach (QString k, keys) {
				if (k == "data") {
					QByteArray data = rr.value(k).toString().toUtf8();
					QJsonDocument doc = QJsonDocument::fromJson(data);
					if (doc.isNull()) {
						qWarning() << tr("Objective JSON error ") << i << data;
					} else {
						rrObj[k] = doc.object();
					}
				} else {
					rrObj[k] = rr.value(k).toJsonValue();
				}
			}

			list << rrObj;
		}

	} else {
		list = m["records"].toJsonArray();
	}

	return list;
}




/**
 * @brief Map::JsonToTable
 * @param array
 * @param convertData
 * @return
 */

bool Map::JsonToTable(const QJsonArray &array, const QString &table, const bool &convertData)
{
	for (int i=0; i<array.count(); ++i) {
		QJsonObject rec = array[i].toObject();
		QVariantMap rr;

		if (convertData) {
			QStringList keys = rec.keys();

			foreach (QString k, keys) {
				if (k == "data") {
					QJsonDocument doc(rec.value(k).toObject());
					rr[k] = QString(doc.toJson(QJsonDocument::Compact));
				} else {
					rr[k] = rec.value(k).toVariant();
				}
			}


		} else {
			rr = rec.toVariantMap();
		}

		if (!m_db->execInsertQuery("INSERT INTO "+table+" (?k?) VALUES (?)", rr))
			return false;

	}

	return true;
}








/**
 * @brief Map::databaseInit
 * @return
 */

bool Map::databasePrepare()
{
	Q_ASSERT(m_client);

	if (!m_db->batchQueryFromFile(":/sql/map.sql")) {
		m_client->sendMessageError(tr("Adatbázis"), tr("Nem sikerült előkészíteni az adatbázist!"), m_databaseFile);
		return false;
	}

	if (!m_db->batchQueryFromFile(":/sql/mapeditor.sql")) {
		m_client->sendMessageError(tr("Adatbázis"), tr("Nem sikerült előkészíteni az adatbázist!"), m_databaseFile);
		return false;
	}

	return true;
}





/**
 * @brief Map::databaseChecio
 */

bool Map::databaseCheck()
{
	Q_ASSERT(m_client);

	if (m_mapType == MapInvalid) {
		m_client->sendMessageError(tr("Internal error"), tr("Az adatbázis típusa nincs megadva!"));
		return false;
	}

	if (m_mapType == MapEditor) {
		setDatabaseFile(Client::standardPath("tmpmapeditor.db"));
	} else if (m_mapType == MapGame) {
		setDatabaseFile(Client::standardPath("tmpmapgame.db"));
	}

	if (m_mapType == MapEditor && QFile::exists(m_databaseFile)) {
		qInfo() << tr("Létező ideiglenes adatbázis: ")+m_databaseFile;
		if (!databaseOpen()) {
			m_db->close();

			qInfo() << tr("Nem sikerült megnyitni a fájlt, törlöm: ")+m_databaseFile;

			if (!QFile::remove(m_databaseFile)) {
				m_client->sendMessageError(tr("Internal error"), tr("Nem sikerült törölni az ideiglenes adatbázist!"), m_databaseFile);
				return false;
			}

			return true;
		}

		QVariantMap m = m_db->runSimpleQuery("SELECT originalFile, uuid from mapeditor");
		if (!m["error"].toBool() && m["records"].toList().count()) {
			QVariantMap r = m["records"].toList().value(0).toMap();
			QString filename = r.value("originalFile").toString();
			QString uuid = r.value("uuid").toString();

			if (!uuid.isEmpty()) {
				emit mapBackupExists(filename);
				m_db->close();
				return true;
			}
		}

		m_db->close();

		qInfo() << tr("Hibás adatbázis, törlöm: ")+m_databaseFile;

		if (!QFile::remove(m_databaseFile)) {
			m_client->sendMessageError(tr("Internal error"), tr("Nem sikerült törölni az ideiglenes adatbázist!"), m_databaseFile);
			return false;
		}

		return true;
	}

	return true;
}




void Map::setMapType(Map::MapType mapType)
{
	if (m_mapType == mapType)
		return;

	m_mapType = mapType;
	emit mapTypeChanged(m_mapType);
}

