/*
 * ---- Call of Suli ----
 *
 * rpgtower.h
 *
 * Created on: 2026. 05. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgTower
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

#ifndef RPGTOWER_H
#define RPGTOWER_H

#include "rpggameitem.h"
#include "tiledobject.h"
#include "rpgstream.h"


/**
 * @brief The RpgTower class
 */

class RpgTower : public TiledObject
{
	Q_OBJECT

	Q_PROPERTY(int load READ load WRITE setLoad NOTIFY loadChanged FINAL)
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
	Q_PROPERTY(RpgGameItem *gameItem READ gameItem CONSTANT FINAL)

public:
	RpgTower(RpgGameItem *gameItem, Tiled::MapObject *object, Tiled::MapRenderer *renderer);

	virtual void initialize() override;
	void worldStep() override;

	void addLayers(const QMultiMap<RpgStream::Team, TiledQuick::TileLayerItem *> &layers);
	void setVisualItem(TiledVisualItem *item);

	int load() const;
	void setLoad(int newLoad);

	QColor color() const;
	void setColor(const QColor &newColor);

	const RpgStream::TowerState &state() const;
	void setState(const RpgStream::TowerState &newState);

	RpgGameItem *gameItem() const;

signals:
	void loadChanged();
	void colorChanged();

protected:
	void synchronize() override;

private:
	RpgGameItem *m_gameItem = nullptr;

	RpgStream::TowerState m_state;

	RpgVisualState<RpgStream::Team> m_visual;
	QQuickItem *m_markerItem = nullptr;

	int m_load = 0;
	QColor m_color = QColorConstants::Svg::white;
};

#endif // RPGTOWER_H
