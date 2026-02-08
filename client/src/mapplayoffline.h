/*
 * ---- Call of Suli ----
 *
 * mapplayoffline.h
 *
 * Created on: 2026. 02. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlayOffline
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

#ifndef MAPPLAYOFFLINE_H
#define MAPPLAYOFFLINE_H

#include "mapplay.h"
#include "studentmaphandler.h"
#include <QObject>

class MapPlayOffline : public MapPlay
{
	Q_OBJECT

	Q_PROPERTY(qreal extraTimeFactor READ extraTimeFactor WRITE setExtraTimeFactor NOTIFY extraTimeFactorChanged)

public:
	explicit MapPlayOffline(StudentMapHandler *handler, OfflineClientEngine *engine, QObject *parent = nullptr);
	virtual ~MapPlayOffline();

	bool load(Campaign *campaign, StudentMap *map);

	Q_INVOKABLE virtual void updateSolver() override;
	Q_INVOKABLE virtual int getShortTimeHelper(MapPlayMissionLevel *missionLevel) const override;

	qreal extraTimeFactor() const;
	void setExtraTimeFactor(qreal newExtraTimeFactor);

	OfflineClientEngine* engine() const;

signals:
	void extraTimeFactorChanged();

protected:
	virtual void onCurrentGamePrepared() override;
	virtual void onCurrentGameFinished() override;

private:
	QPointer<StudentMapHandler> m_handler;
	QPointer<OfflineClientEngine> m_engine;
	QPointer<Campaign> m_campaign;
	QHash<GameMapMissionLevel *, int> m_shortTimeHelper;
	qreal m_extraTimeFactor = 0.;
};



#endif // MAPPLAYOFFLINE_H
