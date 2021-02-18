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
{
	m_modelGroupList = new VariantMapModel({
											 "id",
											 "name"
										 },
										 this);

	connect(this, &TeacherGroups::groupListGet, this, &TeacherGroups::onGroupListGet);
}

/**
 * @brief TeacherGroups::~TeacherGroups
 */

TeacherGroups::~TeacherGroups()
{
	delete m_modelGroupList;
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
