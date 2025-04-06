/*
 * ---- Call of Suli ----
 *
 * isometricwerebear.h
 *
 * Created on: 2024. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricWerebear
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

#ifndef RPGWEREBEAR_H
#define RPGWEREBEAR_H

#include "isometricenemy.h"
#include "rpgenemy.h"
#include "rpgenemyiface.h"
#include "tiledeffect.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>



/**
 * @brief The IsometricWerebearWeaponHand class
 */

class RpgWerebearWeaponHand : public RpgWeapon
{
	Q_OBJECT

public:
	RpgWerebearWeaponHand(QObject *parent = nullptr);

protected:
	void eventAttack(TiledObject *target) override final;

};


/**
 * @brief The RpgWerebear class
 */

class RpgWerebear : public RpgEnemy
{
	Q_OBJECT
	QML_ELEMENT

public:
	explicit RpgWerebear(TiledScene *scene = nullptr);
	virtual ~RpgWerebear();

	virtual TiledObjectBody::ObjectId objectId() const override { return RpgEnemy::objectId(); }

	RpgWeapon *defaultWeapon() const override;

protected:
	//bool enemyWorldStep() override final;
	void updateSprite() override final;

	RpgGameData::Enemy serializeThis() const override;

	void load() override final;
	void eventPlayerReached(IsometricPlayer */*player*/) override final;
	void eventPlayerLeft(IsometricPlayer */*player*/) override final;

	void attackedByPlayer(RpgPlayer *player, const RpgGameData::Weapon::WeaponType &weaponType) override final;

	void playAttackEffect(RpgWeapon *weapon) override final;
	void playDeadEffect();
	void playSeeEffect();

	QPointF getPickablePosition(const int &num) const override final;

	virtual int enemyType() const override { return RpgEnemyIface::enemyType(); }

private:
	void onCurrentSpriteChanged();
	bool isStanding() const;
	void toDeathSprite();

	TiledGameSfx m_sfxFootStep;
	TiledGameSfx m_sfxPain;
	TiledGameSfx m_sfxRoar;
	TiledEffectHealed m_effectHealed;

	std::unique_ptr<RpgWerebearWeaponHand> m_weaponHand;

	int m_nextHit = 1;
};





#endif // RPGWEREBEAR_H
