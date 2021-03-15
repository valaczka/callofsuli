/*
 * ---- Call of Suli ----
 *
 * teachergroups.cpp
 *
 * Created on: 2021. 02. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherGroups
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

#include "teachergroups.h"

TeacherGroups::TeacherGroups(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassTeacherMap, parent)
	, m_modelGroupList(nullptr)
	, m_modelClassList(nullptr)
	, m_modelUserList(nullptr)
	, m_modelMapList(nullptr)
	, m_selectedGroupId(-1)
{
	m_modelGroupList = new VariantMapModel({
											   "id",
											   "name",
											   "readableClassList"
										   },
										   this);

	m_modelClassList = new VariantMapModel({
											   "classid",
											   "name"
										   },
										   this);

	m_modelUserList = new VariantMapModel({
											  "username",
											  "firstname",
											  "lastname",
											  "classid",
											  "classname",
											  "active"
										  },
										  this);

	m_modelMapList = new VariantMapModel({
											 "uuid",
											 "name"
										 },
										 this);

	connect(this, &TeacherGroups::groupListGet, this, &TeacherGroups::onGroupListGet);
	connect(this, &TeacherGroups::groupGet, this, &TeacherGroups::onGroupGet);
	connect(this, &TeacherGroups::selectedGroupIdChanged, this, &TeacherGroups::groupSelect);
}

/**
 * @brief TeacherGroups::~TeacherGroups
 */

TeacherGroups::~TeacherGroups()
{
	delete m_modelGroupList;
	delete m_modelUserList;
	delete m_modelClassList;
	delete m_modelMapList;
}


/**
 * @brief TeacherGroups::groupSelect
 * @param groupId
 */

void TeacherGroups::groupSelect(const int &groupId)
{
	if (groupId == -1)
		return;

	QJsonObject o;
	o["id"]	= groupId;
	send("groupGet", o);
}


/**
 * @brief TeacherGroups::setModelGroupList
 * @param modelGroupList
 */

void TeacherGroups::setModelGroupList(VariantMapModel *modelGroupList)
{
	if (m_modelGroupList == modelGroupList)
		return;

	m_modelGroupList = modelGroupList;
	emit modelGroupListChanged(m_modelGroupList);
}

void TeacherGroups::setModelClassList(VariantMapModel *modelClassList)
{
	if (m_modelClassList == modelClassList)
		return;

	m_modelClassList = modelClassList;
	emit modelClassListChanged(m_modelClassList);
}

void TeacherGroups::setModelUserList(VariantMapModel *modelUserList)
{
	if (m_modelUserList == modelUserList)
		return;

	m_modelUserList = modelUserList;
	emit modelUserListChanged(m_modelUserList);
}

void TeacherGroups::setModelMapList(VariantMapModel *modelMapList)
{
	if (m_modelMapList == modelMapList)
		return;

	m_modelMapList = modelMapList;
	emit modelMapListChanged(m_modelMapList);
}

void TeacherGroups::setSelectedGroupId(int selectedGroupId)
{
	if (m_selectedGroupId == selectedGroupId)
		return;

	m_selectedGroupId = selectedGroupId;
	emit selectedGroupIdChanged(m_selectedGroupId);
}


/**
 * @brief TeacherGroups::onGroupListGet
 * @param jsonData
 */

void TeacherGroups::onGroupListGet(QJsonObject jsonData, QByteArray)
{
	m_modelGroupList->unselectAll();

	QJsonArray list = jsonData.value("list").toArray();

	m_modelGroupList->setJsonArray(list, "id");
}



/**
 * @brief TeacherGroups::onGroupGet
 * @param jsonData
 */

void TeacherGroups::onGroupGet(QJsonObject jsonData, QByteArray)
{
	m_modelClassList->unselectAll();
	m_modelUserList->unselectAll();
	m_modelMapList->unselectAll();

	setSelectedGroupId(jsonData.value("id").toInt(-1));

	m_modelClassList->setJsonArray(jsonData.value("classList").toArray(), "classid");
	m_modelUserList->setJsonArray(jsonData.value("userList").toArray(), "username");
	m_modelMapList->setJsonArray(jsonData.value("mapList").toArray(), "uuid");
}
