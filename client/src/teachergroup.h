/*
 * ---- Call of Suli ----
 *
 * teachergroup.h
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

#ifndef TEACHERGROUP_H
#define TEACHERGROUP_H

#include <QObject>
#include "QOlm/QOlm.hpp"
#include "user.h"
#include "classobject.h"

class TeacherGroup;
using TeacherGroupList = qolm::QOlm<TeacherGroup>;
Q_DECLARE_METATYPE(TeacherGroupList*)

class TeacherGroup : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int groupid READ groupid WRITE setGroupid NOTIFY groupidChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(UserList* userList READ userList CONSTANT)
	Q_PROPERTY(UserList* memberList READ memberList CONSTANT)
	Q_PROPERTY(ClassList* classList READ classList CONSTANT)

public:
	explicit TeacherGroup(QObject *parent = nullptr);
	virtual ~TeacherGroup();

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	Q_INVOKABLE void reload();

	int groupid() const;
	void setGroupid(int newGroupid);

	const QString &name() const;
	void setName(const QString &newName);

	bool active() const;
	void setActive(bool newActive);

	UserList *userList() const;
	UserList *memberList() const;
	ClassList *classList() const;

signals:
	void groupidChanged();
	void nameChanged();
	void activeChanged();

private:
	int m_groupid = -1;
	QString m_name;
	bool m_active = false;
	UserList *const m_userList;
	UserList *const m_memberList;
	ClassList *const m_classList;
};

#endif // TEACHERGROUP_H
