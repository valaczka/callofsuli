/*
 * ---- Call of Suli ----
 *
 * teachermaphandler.h
 *
 * Created on: 2023. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMapHandler
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

#ifndef TEACHERMAPHANDLER_H
#define TEACHERMAPHANDLER_H

#include "basemaphandler.h"
#include "teachermap.h"
#include <QObject>

class TeacherMapHandler : public BaseMapHandler
{
	Q_OBJECT

	Q_PROPERTY(TeacherMapList *mapList READ mapList CONSTANT)

public:
	explicit TeacherMapHandler(QObject *parent = nullptr);
	virtual ~TeacherMapHandler();

	Q_INVOKABLE void mapImport(const QUrl &file);
	Q_INVOKABLE void mapDownload(TeacherMap *map);
	Q_INVOKABLE void checkDownloads();

	TeacherMapList *mapList() const;

signals:

protected:
	void reloadList() override;

private:
	TeacherMapList *const m_mapList;

};

#endif // TEACHERMAPHANDLER_H
