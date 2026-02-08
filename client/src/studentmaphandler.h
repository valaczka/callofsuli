/*
 * ---- Call of Suli ----
 *
 * studentmaphandler.h
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentMapHandler
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

#ifndef STUDENTMAPHANDLER_H
#define STUDENTMAPHANDLER_H

#include "basemaphandler.h"
#include "studentmap.h"
#include "teachergroup.h"

class Campaign;
class OfflineClientEngine;

#ifndef OPAQUE_PTR_OfflineClientEngine
#define OPAQUE_PTR_OfflineClientEngine
Q_DECLARE_OPAQUE_POINTER(OfflineClientEngine*)
#endif


/**
 * @brief The StudentMapHandler class
 */

class StudentMapHandler : public BaseMapHandler
{
	Q_OBJECT

	Q_PROPERTY(StudentMapList *mapList READ mapList CONSTANT)
	Q_PROPERTY(OfflineClientEngine *offlineEngine READ offlineEngine WRITE setOfflineEngine NOTIFY offlineEngineChanged FINAL)

public:
	explicit StudentMapHandler(QObject *parent = nullptr);
	virtual ~StudentMapHandler();

	Q_INVOKABLE void mapDownload(StudentMap *map);
	Q_INVOKABLE void checkDownloads();

	Q_INVOKABLE void getUserCampaign(Campaign *campaign);
	Q_INVOKABLE void playCampaignMap(Campaign *campaign, StudentMap *map, const QString &missionUuid);

	StudentMapList *mapList() const;

	Q_INVOKABLE void reloadFreePlayMapList(TeacherGroupFreeMapList *list);

	OfflineClientEngine *offlineEngine() const;
	void setOfflineEngine(OfflineClientEngine *newOfflineEngine);

signals:
	void offlineEngineChanged();

protected:
	void reloadList() override;

private:
	std::unique_ptr<StudentMapList> m_mapList;
	QPointer<OfflineClientEngine> m_offlineEngine;

	friend class OfflineClientEngine;
};



#endif // STUDENTMAPHANDLER_H
