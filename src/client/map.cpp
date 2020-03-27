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

Map::Map(QObject *parent) : AbstractActivity(parent)
{
	m_databaseFile = Client::standardPath("tmpmap.db");

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
}


/**
 * @brief Map::~Map
 */

Map::~Map()
{
	if (m_db->isOpen())
		m_db->close();

	if (QFile::exists(m_databaseFile)) {
		qDebug() << tr("Remove map file ")+m_databaseFile << QFile::remove(m_databaseFile);
	}
}



/**
 * @brief Map::create
 * @return
 */

bool Map::create()
{
	Q_ASSERT (m_client);

	QVariantMap params;

	setMapTitle(tr("-- Új pálya --"));

	params["title"] = m_mapTitle;

	QVariantMap r = m_db->runInsertQuery("INSERT INTO info(?k?) values (?)", params);

	if (r["errors"].toBool() || r["lastInsertId"] == QVariant::Invalid)
		return false;

	setMapUuid(QUuid::createUuid().toString());
	return true;

}


/**
 * @brief Map::loadFromJson
 * @param data
 * @return
 */

bool Map::loadFromJson(const QByteArray &data)
{
	Q_ASSERT (m_client);

	QJsonDocument doc = QJsonDocument::fromBinaryData(data);
	//QJsonDocument doc = QJsonDocument::fromJson(data);

	if (doc.isNull())
		return false;

	QJsonObject root = doc.object();

	/*QJsonObject fileinfo = root["callofsuli"].toObject();

	if (fileinfo.isEmpty()) {
		return false;
	}*/

	foreach (QString t, m_tableNames) {
		if (!JsonToTable(root[t].toArray(), t, false))
			return false;
	}

	if (!JsonToTable(root["objective"].toArray(), "objective", true))
		return false;

	QString uuid = root["callofsuli"].toObject()["uuid"].toString();
	setMapUuid(uuid.isEmpty() ? QUuid::createUuid().toString() : uuid);

	return true;
}


/**
 * @brief Map::loadFromFile
 * @param filename
 * @return
 */

bool Map::loadFromFile(const QString &filename)
{
	QFile f(filename);

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		m_client->sendMessageError(tr("Fájl megnyitási hiba"), tr("A fájl nem található vagy nem olvasható!"), filename);
		return false;
	}

	QByteArray b = f.readAll();

	f.close();

	return loadFromJson(b);
}



/**
 * @brief Map::saveToJson
 * @return
 */

QByteArray Map::saveToJson()
{
	QJsonObject root;

	QJsonObject fileinfo;
	fileinfo["versionMajor"] = m_client->clientVersionMajor();
	fileinfo["versionMinor"] = m_client->clientVersionMinor();
	fileinfo["uuid"] = m_mapUuid;

	root["callofsuli"] = fileinfo;

	foreach (QString t, m_tableNames) {
		root[t] = tableToJson(t);
	}

	root["objective"] = tableToJson("objective", true);

	QJsonDocument d(root);

	return d.toBinaryData();
	//return d.toJson(QJsonDocument::Indented);
}


/**
 * @brief Map::saveToFile
 * @param filename
 * @return
 */

bool Map::saveToFile(const QString &filename)
{
	QSaveFile f(filename);

	if (!f.open(QIODevice::WriteOnly)) {
		m_client->sendMessageError(tr("Mentési hiba"), tr("Nem sikerült menteni a fájlt!"), filename);
		return false;
	}

	QByteArray b = saveToJson();
	f.write(b);

	if (!f.commit()) {
		m_client->sendMessageError(tr("Mentési hiba"), tr("Nem sikerült menteni a fájlt!"), filename);
		return false;
	}

	return true;
}


/**
 * @brief Map::updateMapName
 * @param name
 * @return
 */

void Map::updateMapTitle(const QString &name)
{
	QVariantList l;
	l << name;
	QVariantMap m = m_db->runSimpleQuery("UPDATE info SET title=?", l);

	if (!m["error"].toBool()) {
		setMapTitle(name);
	}
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
		qWarning() << m["errorString"].toString();
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

		QVariantMap m = m_db->runInsertQuery("INSERT INTO "+table+" (?k?) VALUES (?)", rr);
		if (m["errors"].toBool() || m["lastInsertId"] == QVariant::Invalid)
			return false;

	}

	return true;
}





void Map::setMapTitle(QString mapName)
{
	if (m_mapTitle == mapName)
		return;

	m_mapTitle = mapName;
	emit mapTitleChanged(m_mapTitle);
}






/**
 * @brief Map::databaseInit
 * @return
 */

bool Map::databaseInit()
{
	Q_ASSERT(m_client);

	if (!m_db->batchQueryFromFile(":/sql/map.sql")) {
		m_client->sendMessageError(tr("Adatbázis"), tr("Nem sikerült előkészíteni az adatbázist!"), databaseFile());
		return false;
	}

	return true;
}
