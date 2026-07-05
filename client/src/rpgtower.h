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





class RpgDefender;
class RpgTower;



/**
 * @brief The RpgDefenderPoint class
 */

class RpgDefenderPoint : public TiledObjectBody
{
public:
	explicit RpgDefenderPoint(const QPointF &center,
							  TiledGame *game,
							  Tiled::MapRenderer *renderer = nullptr,
							  const QPointF &offset = {});

	RpgDefender *defender() const;
	void setDefender(RpgDefender *newDefender);

	RpgTower *tower() const;
	void setTower(RpgTower *newTower);

private:
	const static DrawBodyStyle m_style;

	RpgDefender *m_defender = nullptr;
	RpgTower *m_tower = nullptr;
};







/**
 * @brief The RpgTower class
 */

class RpgTower : public TiledObject
{
	Q_OBJECT

	ADD_SCATTER_POINT

	Q_PROPERTY(int load READ load WRITE setLoad NOTIFY loadChanged FINAL)
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
	Q_PROPERTY(RpgGameItem *gameItem READ gameItem CONSTANT FINAL)
	Q_PROPERTY(bool canAttack READ canAttack WRITE setCanAttack NOTIFY canAttackChanged FINAL)

public:
	RpgTower(RpgGameItem *gameItem, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	virtual ~RpgTower();

	virtual void initialize() override;
	void worldStep() override;

	void addLayers(const QMultiMap<RpgStream::Team, TiledQuick::TileLayerItem *> &layers);
	void setVisualItem(TiledVisualItem *item);
	void addDefenderPoints(const QList<RpgDefenderPoint*> &list);

	void setDefenderLayersVisible(const bool visible = true);
	void reloadDefenderLayersVisibility();

	void setVisible(const bool &visible = true);

	QQuickItem *markerItem() const;

	int load() const;
	void setLoad(int newLoad);

	QColor color() const;
	void setColor(const QColor &newColor);

	const RpgStream::TowerState &state() const;
	void setState(const RpgStream::TowerState &newState);

	RpgGameItem *gameItem() const;

	const QList<RpgDefenderPoint *> &defenderPoints() const;

	bool canAttack() const;
	void setCanAttack(bool newCanAttack);


signals:
	void loadChanged();
	void colorChanged();
	void canAttackChanged();

protected:
	void synchronize() override;

private:
	RpgGameItem *m_gameItem = nullptr;
	bool m_visible = false;

	RpgStream::TowerState m_state;

	RpgVisualState<RpgStream::Team> m_visual;
	QQuickItem *m_markerItem = nullptr;

	QList<RpgDefenderPoint *> m_defenderPoints;

	int m_load = 0;
	QColor m_color = QColorConstants::Svg::white;
	bool m_canAttack = false;
	bool m_defenderLayers = false;
};

#endif // RPGTOWER_H
