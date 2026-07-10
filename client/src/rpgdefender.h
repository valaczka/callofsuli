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

	RpgStream::Team team() const;
	void setTeam(RpgStream::Team newTeam);

	const Rpg::DefenderObject &config() const;

	bool visibleToAll() const;
	void setVisibleToAll(bool newVisibleToAll);

	bool hasTarget() const;
	void setHasTarget(bool newHasTarget);

signals:
	void visibleToAllChanged();
	void hasTargetChanged();

protected:
	virtual void onAlive() override;
	virtual void onDead() override;

	bool loadFromCommonMap(const QString &name);

protected:
	RpgDefenderPoint* m_defenderPoint = nullptr;
	QPointer<RpgTower> m_tower;
	RpgStream::Team m_team = RpgStream::TeamNone;
	const Rpg::DefenderObject m_config;

	bool m_visibleToAll = false;
	bool m_hasTarget = false;

	RpgVisualState<State> m_visual;
	TiledScene *m_scene = nullptr;
	QList<TiledQuick::TileLayerItem *> m_layerItems;

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
	RpgDefenderCommon(const QString &name, RpgGameItem *gameItem, const Rpg::DefenderObject &config)
		: RpgDefender(gameItem, config)
		, m_name(name)
	{}

	virtual void initialize() override {
		if (!loadFromCommonMap(m_name)) {
			LOG_CERROR("game") << "Common defender load failed" << m_name;
		}
	}

private:
	const QString m_name;
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
