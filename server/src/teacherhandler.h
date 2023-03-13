/*
 * ---- Call of Suli ----
 *
 * teacherhandler.h
 *
 * Created on: 2023. 01. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherHandler
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

#ifndef TEACHERHANDLER_H
#define TEACHERHANDLER_H

#include "abstracthandler.h"

class TeacherHandler : public AbstractHandler
{
	Q_OBJECT
public:
	explicit TeacherHandler(Client *client);

	//QDeferred<RankList> getRankList() const;

protected:
	virtual void handleRequestResponse() {};
	virtual void handleEvent() {};

private slots:
	void groupList();
	void groupAdd();
	void groupModify();
	void groupRemove();

private:
};

#endif // TEACHERHANDLER_H
