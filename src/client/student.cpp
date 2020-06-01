/*
 * ---- Call of Suli ----
 *
 * student.cpp
 *
 * Created on: 2020. 05. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Student
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

#include "student.h"

Student::Student(QObject *parent)
	: AbstractActivity(parent)
{
	m_mapRepository = new MapRepository("studentMapDb", this);
	m_repositoryReady = false;
}




/**
 * @brief Student::~Student
 */

Student::~Student()
{
	if (m_mapRepository)
		delete m_mapRepository;
}


/**
 * @brief Student::clientSetup
 */


void Student::clientSetup()
{
	connect(m_client, &Client::jsonStudentReceived, this, &Student::onJsonReceived);
	m_mapRepository->setDatabaseFile(QDir::toNativeSeparators(m_client->serverDataDir()+"/maps.db"));
}




/**
 * @brief Student::mapRepositoryOpen
 */

void Student::mapRepositoryOpen()
{
	busyStackAdd("repositoryOpen", 0);
	if (!m_mapRepository->databaseOpen()) {
		busyStackRemove("repositoryOpen", 0);
		emit mapRepositoryOpenError(m_mapRepository->databaseFile());
		return;
	}

	setRepositoryReady(true);

	emit mapRepositoryOpened(m_mapRepository->databaseFile());
	busyStackRemove("repositoryOpen", 0);
}

void Student::setRepositoryReady(bool repositoryReady)
{
	if (m_repositoryReady == repositoryReady)
		return;

	m_repositoryReady = repositoryReady;
	emit repositoryReadyChanged(m_repositoryReady);
}


/**
 * @brief Student::mapDownload
 * @param uuid
 */

void Student::mapDownload(const QString &uuid)
{
	if (uuid.isEmpty())
		return;

	QJsonObject d;
	d["class"] = "student";
	d["func"] = "getMapData";
	d["uuid"] = uuid;

	this->send(d);
}



/**
 * @brief Student::onJsonReceived
 * @param object
 * @param binaryData
 * @param clientMsgId
 */

void Student::onJsonReceived(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId)
{
	QString func = object.value("func").toString();
	QJsonObject data = object.value("data").toObject();

	if (!func.isEmpty())
		busyStackRemove(func, clientMsgId);

	if (func == "getMaps")
		emit mapListLoaded(data.value("list").toArray());
	else if (func == "getMapData")
		emit mapDataReceived(data, binaryData);
	else if (func == "getMapResult")
		emit mapResultListLoaded(data.value("list").toArray());
}
