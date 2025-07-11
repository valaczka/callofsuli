/*
 * ---- Call of Suli ----
 *
 * rpgenemy.h
 *
 * Created on: 2025. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEnemy
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

#ifndef RPGENEMY_H
#define RPGENEMY_H

#include "isometricenemy.h"
#include "rpgenemyiface.h"
#include "rpgplayer.h"
#include <QObject>
#include <QQmlEngine>




/**
 * @brief The RpgEnemyConfig class
 */

class RpgEnemyConfig : public QSerializer
{
	Q_GADGET

public:
	RpgEnemyConfig()
		: QSerializer()
		, weapon(RpgGameData::Weapon::WeaponInvalid)
		, playerFeatures(RpgGameData::Player::FeatureInvalid)
	{}

	QS_SERIALIZABLE

	QS_FIELD(RpgGameData::Weapon::WeaponType, weapon)
	QS_FIELD(RpgGameData::Player::Features, playerFeatures)			// player feature override
};




/**
 * @brief The RpgEnemy class
 */

class RpgEnemy : public IsometricEnemy, public RpgEnemyIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledWeapon* defaultWeapon READ defaultWeapon CONSTANT FINAL)

public:
	RpgEnemy(const RpgGameData::EnemyBaseData::EnemyType &type, RpgGame *game, const qreal &radius = 10.);

	virtual ObjectId objectId() const override final { return ifaceObjectId(); }
	virtual void setObjectId(const ObjectId &newObjectId) override final { ifaceSetObjectId(newObjectId); }
	virtual void setObjectId(const int &ownerId, const int &sceneId, const int &id) override final { ifaceSetObjectId(ownerId, sceneId, id); }

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Enemy> &snapshot) override;
	virtual void updateFromSnapshot(const RpgGameData::Enemy &snap) override;
	virtual bool isLastSnapshotValid(const RpgGameData::Enemy &snap, const RpgGameData::Enemy &lastSnap) const override;

	const RpgEnemyConfig &config() const;
	void setConfig(const RpgEnemyConfig &newConfig);

	bool isWatchingPlayer(RpgPlayer *player) const;

protected:
	virtual bool enemyWorldStep() override;
	virtual bool enemyWorldStepNotReachedPlayer() override;
	virtual void attackPlayer(RpgPlayer *player, RpgWeapon *weapon) override;
	virtual void onAlive() override;
	virtual void onDead() override;

	virtual bool featureOverride(const PlayerFeature &feature, IsometricPlayer *player) const override final;
	virtual bool featureOverride(const PlayerFeature &feature, RpgPlayer *player) const;

	bool checkFeature(const RpgGameData::Player::Feature &feature, RpgPlayer *player) const;

	RpgGameData::Enemy serializeEnemy() const;

	RpgEnemyConfig m_config;
	QHash<RpgGameData::Enemy::EnemyState, qint64> m_stateLastRenderedTicks;

	friend class ActionRpgMultiplayerGame;
};

#endif // RPGENEMY_H
