/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupsave.h
 *
 * Created on: 2024. 05. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupSave
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

#ifndef RPGCONTROLGROUPSAVE_H
#define RPGCONTROLGROUPSAVE_H

#include "qtimer.h"
#include "rpgcontrolgroup.h"
#include "rpgplayer.h"
#include <libtiledquick/tilelayeritem.h>


class RpgControlGroupSave;

/**
 * @brief The RpgControlGroupSaveBody class
 */

class RpgControlGroupSaveBody : public TiledObjectBody
{
public:
	explicit RpgControlGroupSaveBody(const QPointF &center, const qreal &radius,
									 TiledGame *game, Tiled::MapRenderer *renderer)
		: TiledObjectBody(center, radius, game, renderer, CP_BODY_TYPE_STATIC)
	{ }


	virtual void onShapeContactBegin(cpShape *, cpShape *shape) override;

	RpgControlGroupSave *m_control = nullptr;

};



/**
 * @brief The RpgControlGroupSave class
 */

class RpgControlGroupSave : public RpgControlGroup
{
public:
	RpgControlGroupSave(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);

	int count() const;
	void setCount(int newCount);

	QPointF position() const { return m_body ? m_body->bodyPosition() : QPointF{}; };
	const bool &isActive() const { return m_active; }

	void sensorBegin(cpShape *shape);

private:
	void updateLayers();
	void hide(RpgPlayer *player);
	void show();
	void deactivate();


	QVector<QPointer<TiledQuick::TileLayerItem>> m_tileLayers;
	RpgControlGroupSaveBody *m_body = nullptr;

	int m_count = -1;
	bool m_active = true;

	QTimer m_timer;
};



/**
 * @brief RpgControlGroupSaveBody::onShapeContactBegin
 * @param shape
 */

inline void RpgControlGroupSaveBody::onShapeContactBegin(cpShape *, cpShape *shape) {
	if (m_control)
		m_control->sensorBegin(shape);
}



#endif // RPGCONTROLGROUPSAVE_H
