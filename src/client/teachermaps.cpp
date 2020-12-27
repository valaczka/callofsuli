/*
 * ---- Call of Suli ----
 *
 * teachermaps.cpp
 *
 * Created on: 2020. 12. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMaps
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

#include "teachermaps.h"


TeacherMaps::TeacherMaps(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassTeacherMap, parent)
	, m_editUuid()
	, m_mapEditor(nullptr)
{
	connect(this, &TeacherMaps::mapGet, this, &TeacherMaps::onMapGet);
	connect(this, &TeacherMaps::mapUpdate, this, &TeacherMaps::onMapUpdated);
}


/**
 * @brief TeacherMaps::~TeacherMaps
 */

TeacherMaps::~TeacherMaps()
{

}



/**
 * @brief TeacherMaps::mapEditorLoadRequest
 * @param mapEditor
 */

void TeacherMaps::mapEditorLoadRequest(MapEditor *mapEditor)
{
	m_mapEditor = mapEditor;

	if (m_editUuid.isEmpty()) {
		m_client->sendMessageWarning(tr("Belső hiba"), tr("Pályaazonosító nincs megadva!"));
		return;
	}

	QJsonObject m;
	m["uuid"] = m_editUuid;
	send("mapGet", m);
}


/**
 * @brief TeacherMaps::mapEditorCloseRequest
 * @param mapEditor
 */

void TeacherMaps::mapEditorCloseRequest(MapEditor *)
{
	m_mapEditor = nullptr;
	if (!m_editUuid.isEmpty()) {
		QJsonObject m;
		m["uuid"] = m_editUuid;
		send("mapEditUnlock", m);
		setEditUuid("");
	}
}


/**
 * @brief TeacherMaps::mapEditorSaveRequest
 * @param mapEditor
 * @param binaryData
 */

void TeacherMaps::mapEditorSaveRequest(MapEditor *, const QByteArray &binaryData)
{
	if (m_editUuid.isEmpty()) {
		m_client->sendMessageWarning(tr("Belső hiba"), tr("Pályaazonosító nincs megadva!"));
		return;
	}

	QJsonObject m;
	m["uuid"] = m_editUuid;
	send("mapUpdate", m, binaryData);
}


/**
 * @brief TeacherMaps::setEditUuid
 * @param editUuid
 */

void TeacherMaps::setEditUuid(QString editUuid)
{
	if (m_editUuid == editUuid)
		return;

	m_editUuid = editUuid;
	emit editUuidChanged(m_editUuid);
}



/**
 * @brief TeacherMaps::onMapGet
 * @param jsonData
 * @param binaryData
 */

void TeacherMaps::onMapGet(QJsonObject jsonData, QByteArray binaryData)
{
	if (!m_mapEditor || m_editUuid.isEmpty()) {
		m_client->sendMessageWarning(tr("Belső hiba"), tr("Pályaazonosító nincs megadva!"));
		return;
	}

	if (jsonData.value("uuid").toString() != m_editUuid) {
		m_client->sendMessageWarning(tr("Belső hiba"), tr("Érvénytelen adat érkezett!"));
		return;
	}

	QVariantMap d;
	d["data"] = binaryData;

	m_mapEditor->setMapName(jsonData.value("name").toString());
	m_mapEditor->loadFromActivity(d);
}


/**
 * @brief TeacherMaps::onMapUpdated
 * @param jsonData
 * @param binaryData
 */

void TeacherMaps::onMapUpdated(QJsonObject jsonData, QByteArray)
{
	if (!m_mapEditor || m_editUuid.isEmpty()) {
		m_client->sendMessageWarning(tr("Belső hiba"), tr("Pályaazonosító nincs megadva!"));
		return;
	}

	if (jsonData.value("uuid").toString() != m_editUuid) {
		m_client->sendMessageWarning(tr("Belső hiba"), tr("Érvénytelen adat érkezett!"));
		return;
	}

	if (jsonData.value("updated").toBool()) {
		QMetaObject::invokeMethod(m_mapEditor, "saveFinished", Qt::AutoConnection);
	} else {
		QMetaObject::invokeMethod(m_mapEditor, "saveFailed", Qt::AutoConnection);
	}
}

