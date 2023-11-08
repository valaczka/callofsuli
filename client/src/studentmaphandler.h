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

class Campaign;

/**
 * @brief The StudentMapHandler class
 */

class StudentMapHandler : public BaseMapHandler
{
	Q_OBJECT

	Q_PROPERTY(StudentMapList *mapList READ mapList CONSTANT)

public:
	explicit StudentMapHandler(QObject *parent = nullptr);
	virtual ~StudentMapHandler();

	Q_INVOKABLE void mapDownload(StudentMap *map);
	Q_INVOKABLE void checkDownloads();

	Q_INVOKABLE void getUserCampaign(Campaign *campaign);
	Q_INVOKABLE void playCampaignMap(Campaign *campaign, StudentMap *map);

	StudentMapList *mapList() const;

protected:
	void reloadList() override;

private:
	std::unique_ptr<StudentMapList> m_mapList;
};



#endif // STUDENTMAPHANDLER_H
