/*
 * ---- Call of Suli ----
 *
 * teacherpass.h
 *
 * Created on: 2025. 08. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherPass
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

#ifndef TEACHERPASS_H
#define TEACHERPASS_H

#include "teachergroup.h"
#include <QObject>

class TeacherPass : public QObject
{
	Q_OBJECT

	Q_PROPERTY(TeacherGroup *teacherGroup READ teacherGroup WRITE setTeacherGroup NOTIFY teacherGroupChanged FINAL)

public:
	explicit TeacherPass(QObject *parent = nullptr);
	virtual ~TeacherPass();

	TeacherGroup *teacherGroup() const;
	void setTeacherGroup(TeacherGroup *newTeacherGroup);

signals:

	void teacherGroupChanged();
private:
	QPointer<TeacherGroup> m_teacherGroup;
};

#endif // TEACHERPASS_H
