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
#include "rpgenemyiface.h"
#include "tiledeffect.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>



/**
 * @brief The IsometricWerebearWeaponHand class
 */

class RpgWerebearWeaponHand : public TiledWeapon
{
	Q_OBJECT

public:
	RpgWerebearWeaponHand(QObject *parent = nullptr);

	bool protect(const WeaponType &) override final { return false; }
	bool canProtect(const WeaponType &) const override final { return false; }
	bool canAttack() const override final { return true; }

protected:
	void eventAttack(TiledObject *target) override final;

};


/**
 * @brief The RpgWerebear class
 */

class RpgWerebear : public IsometricEnemy, public RpgEnemyIface
{
	Q_OBJECT
	QML_ELEMENT

public:
	explicit RpgWerebear(TiledScene *scene = nullptr);
	virtual ~RpgWerebear();

	TiledWeapon *defaultWeapon() const override;

	int getNewHpAfterAttack(const int &origHp, const TiledWeapon::WeaponType &weaponType,
									IsometricPlayer *player = nullptr) const override;

protected:
	//bool enemyWorldStep() override final;
	void updateSprite() override final;

	void load() override final;
	void eventPlayerReached(IsometricPlayer */*player*/) override final;
	void eventPlayerLeft(IsometricPlayer */*player*/) override final;

	void attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType) override final;

	void playAttackEffect(TiledWeapon *weapon) override final;
	void playDeadEffect();
	void playSeeEffect();

	QPointF getPickablePosition(const int &num) const override final;

	virtual bool protectWeapon(const TiledWeapon::WeaponType &weaponType) override final;

	virtual int enemyType() const override { return RpgEnemyIface::enemyType(); }

	virtual std::unique_ptr<RpgGameData::Body> serialize() const override;
	virtual bool deserialize(const RpgGameData::Body *from) const override;

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
