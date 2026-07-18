/*
 * ---- Call of Suli ----
 *
 * rpgdefender.h
 *
 * Created on: 2026. 05. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgDefender
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

#ifndef RPGDEFENDER_H
#define RPGDEFENDER_H

#include "rpgentity.h"
#include "rpgtower.h"
#include <QQmlEngine>




/**
 * @brief The RpgDefender class
 */

class RpgDefender : public RpgEntity
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(RpgTower *tower READ tower CONSTANT FINAL)
	Q_PROPERTY(bool visibleToAll READ visibleToAll WRITE setVisibleToAll NOTIFY visibleToAllChanged FINAL)
	Q_PROPERTY(bool hasTarget READ hasTarget WRITE setHasTarget NOTIFY hasTargetChanged FINAL)

public:
	RpgDefender(RpgGameItem *gameItem, const Rpg::DefenderObject &config);
	virtual ~RpgDefender();

	static RpgDefender* createDefender(const Rpg::DefenderObject &defender, RpgGameItem *gameItem, TiledScene *scene);

	enum State {
		StateNormal,
		StateActive,
		StateDestroyed,
		StateHidden
	};

	Q_ENUM(State);


	//virtual void initialize() override;

	virtual void updateVisibility();

	RpgDefenderPoint *defenderPoint() const;
	void setDefenderPoint(RpgDefenderPoint *newDefenderPoint);

	RpgTower *tower() const;
	void setTower(RpgTower *newTower);

	const Rpg::DefenderObject &config() const;

	bool visibleToAll() const;
	void setVisibleToAll(bool newVisibleToAll);

	bool hasTarget() const;
	void setHasTarget(bool newHasTarget);

	virtual void setMarked(const bool &marked = true) override;

signals:
	void visibleToAllChanged();
	void hasTargetChanged();

protected:
	virtual void onAlive() override;
	virtual void onDead() override;
	virtual void updateColor() override;

	bool loadFromCommonMap(const QString &name, const QHash<State, QString> &baseImageHash);
	void addMarkerItem();

protected:
	RpgDefenderPoint* m_defenderPoint = nullptr;
	QPointer<RpgTower> m_tower;
	const Rpg::DefenderObject m_config;

	bool m_visibleToAll = false;
	bool m_hasTarget = false;
	bool m_marked = false;

	RpgVisualState<State> m_visual;
	TiledScene *m_scene = nullptr;
	QList<TiledQuick::TileLayerItem *> m_layerItems;

	QQuickItem *m_markerItem = nullptr;

	friend class RpgDefenderMotor;
};





/**
 * @brief The RpgDefenderCommon class
 */

class RpgDefenderCommon : public RpgDefender
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgDefenderCommon(const QString &name, RpgGameItem *gameItem, const Rpg::DefenderObject &config,
					  const QHash<State, QString> &baseImageHash = {});

	virtual void initialize() override;

private:
	const QString m_name;
	const QHash<State, QString> m_baseImageHash;
};






/**
 * @brief The RpgDefenderMotor class
 */

class RpgDefenderMotor : public AbstractRpgMotor
{
public:
	RpgDefenderMotor(RpgDefender *object)
		: AbstractRpgMotor(object)
		, m_defender(object)
	{}

	virtual void updateBody(TiledObject *) override {};
	virtual bool beforeWorldStep(const qint64 &tick, entt::entity &entity) override;

protected:
	QPointer<RpgDefender> m_defender;
};



#endif // RPGDEFENDER_H
