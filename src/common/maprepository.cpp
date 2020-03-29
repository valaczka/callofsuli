/*
 * ---- Call of Suli ----
 *
 * maprepository.cpp
 *
 * Created on: 2020. 03. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapRepository
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

#include "maprepository.h"

#define DATETIME_JSON_FORMAT QString("yyyy-MM-dd hh:mm:ss")

MapRepository::MapRepository(const QString &connectionName, QObject *parent)
	: COSdb(connectionName, parent)
{

}




/**
 * @brief MapRepository::repositoryListReload
 */

void MapRepository::listReload()
{
	if (!databaseOpen())
		return;

	QVariantList list;
	if (m_db->execSelectQuery("SELECT id, refid, uuid, md5 FROM mapdata ORDER BY name", QVariantList(), &list))
		emit listLoaded(list);
}


/**
 * @brief MapRepository::repostoryGetInfo
 * @param id
 * @return
 */

QVariantMap MapRepository::getInfo(const int &id)
{
	if (!databaseOpen())
		return QVariantMap();

	QVariantMap r;
	QVariantList l;
	l << id;
	m_db->execSelectQueryOneRow("SELECT id, refid, uuid, md5 FROM mapdata WHERE id=?", l, &r);

	return r;
}


/**
 * @brief MapRepository::getInfoByRefId
 * @param id
 * @return
 */

QVariantMap MapRepository::getInfoByRefId(const int &refid)
{
	if (!databaseOpen())
		return QVariantMap();

	QVariantMap r;
	QVariantList l;
	l << refid;
	m_db->execSelectQueryOneRow("SELECT id, refid, uuid, md5 FROM mapdata WHERE refid=?", l, &r);

	return r;
}


/**
 * @brief MapRepository::repositoryGet
 * @param id
 */

QByteArray MapRepository::getData(const int &id)
{
	if (!databaseOpen())
		return QByteArray();

	QVariantList l;
	l << id;
	QSqlQuery q = m_db->simpleQuery("SELECT data FROM mapdata WHERE id=?", l);
	return getDataReal(q);
}



/**
 * @brief MapRepository::repositoryGetData
 * @param uuid
 */

QByteArray MapRepository::getData(const QString &uuid)
{
	if (!databaseOpen())
		return QByteArray();

	QVariantList l;
	l << uuid;
	QSqlQuery q = m_db->simpleQuery("SELECT data FROM mapdata WHERE uuid=?", l);
	return getDataReal(q);
}


/**
 * @brief MapRepository::repositoryGetId
 * @param uuid
 */

int MapRepository::getId(const QString &uuid)
{
	if (!databaseOpen())
		return -1;

	QVariantMap r;
	QVariantList l;
	l << uuid;
	if (!m_db->execSelectQueryOneRow("SELECT id FROM mapdata WHERE uuid=?", l, &r))
		return -1;

	return r.value("id", -1).toInt();
}


/**
 * @brief MapRepository::getRefId
 * @param uuid
 * @return
 */

int MapRepository::getRefId(const QString &uuid)
{
	if (!databaseOpen())
		return -1;

	QVariantMap r;
	QVariantList l;
	l << uuid;
	if (!m_db->execSelectQueryOneRow("SELECT refid FROM mapdata WHERE uuid=?", l, &r))
		return -1;

	return r.value("id", -1).toInt();
}


/**
 * @brief MapRepository::repositoryCreate
 * @param name
 * @return
 */

QVariantMap MapRepository::create(const int &refid)
{
	if (!databaseOpen())
		return QVariantMap();

	QJsonObject root;

	QJsonObject fileinfo;
	fileinfo["uuid"] = QUuid::createUuid().toString();
	fileinfo["timeCreated"] = QDateTime::currentDateTime().toString(DATETIME_JSON_FORMAT);
	fileinfo["timeModified"] = QDateTime::currentDateTime().toString(DATETIME_JSON_FORMAT);

	root["callofsuli"] = fileinfo;

	QJsonDocument d(root);

	QByteArray mapdata = d.toBinaryData();

	QString md5 = QCryptographicHash::hash(mapdata, QCryptographicHash::Md5).toHex();

	QVariantMap l;
	l["refid"] = refid;
	l["uuid"] = fileinfo["uuid"].toString();
	l["md5"] = md5;
	l["data"] = mapdata;
	int id = m_db->execInsertQuery("INSERT INTO mapdata (?k?) VALUES (?)", l);

	fileinfo["id"] = id;

	return fileinfo.toVariantMap();
}


/**
 * @brief MapRepository::updateData
 * @param id
 * @param data
 * @param uuidOverwrite
 * @return
 */

bool MapRepository::updateData(const int &id, const QByteArray &data, const bool &uuidOverwrite)
{
	QVariantMap orig = getInfo(id);

	if (orig.isEmpty()) {
		qWarning() << tr("Ismeretlen azonosító") << id;
		return false;
	}

	QJsonDocument doc = QJsonDocument::fromBinaryData(data);
	if (doc.isNull()) {
		qWarning() << tr("JSON parse error");
		return false;
	}

	QJsonObject root = doc.object();

	QString uuid = root["callofsuli"].toObject()["uuid"].toString();

	QVariantMap params;

	if (uuid != orig["uuid"].toString()) {
		emit uuidComapareError(id);

		if (!uuidOverwrite) {
			qWarning() << tr("Update map UUID error");
			return false;
		}

		params["uuid"] = uuid;
	}

	QString md5 = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

	params["md5"] = md5;
	params["data"] = data;

	QVariantMap binds;
	binds[":id"] = id;

	return m_db->execUpdateQuery("UPDATE mapdata set ? where id=:id", params, binds);
}




/**
 * @brief MapRepository::databaseInit
 * @return
 */

bool MapRepository::databaseInit()
{
	if (!m_db->batchQueryFromFile(":/sql/maprepository.sql")) {
		emit databaseError(tr("Nem sikerült előkészíteni az adatbázist: ")+m_databaseFile);
		return false;
	}

	return true;
}



/**
 * @brief MapRepository::repositoryGetReal
 * @param q
 * @return
 */

QByteArray MapRepository::getDataReal(QSqlQuery q)
{
	QByteArray ret;

	QVariantMap r = m_db->runQuery(q);
	if (r["error"].toBool()) {
		emit databaseError(r["errorString"].toString());
		return ret;
	}

	QVariantList rl = r["records"].toList();

	if (!rl.count())
		return ret;

	QVariantMap m = rl.value(0).toMap();

	return m["data"].toByteArray();
}



