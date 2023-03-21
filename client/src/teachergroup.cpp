/*
 * ---- Call of Suli ----
 *
 * teachergroup.cpp
 *
 * Created on: 2023. 03. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherGroup
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

#include "teachergroup.h"
#include "Logger.h"
#include "clientcache.h"
#include "application.h"


TeacherGroup::TeacherGroup(QObject *parent)
	: QObject{parent}
	, m_userList(new UserList(this))
	, m_memberList(new UserList(this))
	, m_classList(new ClassList(this))
{
	LOG_CTRACE("client") << "TeacherGroup created" << this;

}


/**
 * @brief TeacherGroup::~TeacherGroup
 */

TeacherGroup::~TeacherGroup()
{
	delete m_classList;
	delete m_userList;
	delete m_memberList;

	LOG_CTRACE("client") << "TeacherGroup destroyed" << this;
}


/**
 * @brief TeacherGroup::loadFromJson
 * @param object
 * @param allField
 */

void TeacherGroup::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setGroupid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("name")) || allField)
		setName(object.value(QStringLiteral("name")).toString());

	if (object.contains(QStringLiteral("active")) || allField)
		setActive(object.value(QStringLiteral("active")).toInt());

	if (object.contains(QStringLiteral("classList")) || allField)
		OlmLoader::loadFromJsonArray<ClassObject>(m_classList, object.value(QStringLiteral("classList")).toArray(), "classid", "classid", true);

	if (object.contains(QStringLiteral("userList")) || allField)
		OlmLoader::loadFromJsonArray<User>(m_userList, object.value(QStringLiteral("userList")).toArray(), "username", "username", true);

	if (object.contains(QStringLiteral("memberList")) || allField)
		OlmLoader::loadFromJsonArray<User>(m_memberList, object.value(QStringLiteral("memberList")).toArray(), "username", "username", true);
}


/**
 * @brief TeacherGroup::reload
 */

void TeacherGroup::reload()
{
	if (m_groupid <= 0)
		return;

	Application::instance()->client()->send(WebSocket::ApiTeacher, QStringLiteral("group/%1").arg(m_groupid))
			->fail([](const QString &err) {
		Application::instance()->client()->messageWarning(err, tr("Letöltés sikertelen"));
	})
			->done(std::bind(&TeacherGroup::loadFromJson, this, std::placeholders::_1, true));
}



/**
 * @brief TeacherGroup::groupid
 * @return
 */

int TeacherGroup::groupid() const
{
	return m_groupid;
}

void TeacherGroup::setGroupid(int newGroupid)
{
	if (m_groupid == newGroupid)
		return;
	m_groupid = newGroupid;
	emit groupidChanged();
}

const QString &TeacherGroup::name() const
{
	return m_name;
}

void TeacherGroup::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

bool TeacherGroup::active() const
{
	return m_active;
}

void TeacherGroup::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}

UserList *TeacherGroup::userList() const
{
	return m_userList;
}

UserList *TeacherGroup::memberList() const
{
	return m_memberList;
}

ClassList *TeacherGroup::classList() const
{
	return m_classList;
}

