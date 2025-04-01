/*
 * ---- Call of Suli ----
 *
 * rpgenemybase.h
 *
 * Created on: 2024. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEnemyBase
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

#ifndef RPGENEMYBASE_H
#define RPGENEMYBASE_H

#include "rpgenemy.h"
#include "tiledeffect.h"
#include "tiledgamesfx.h"
#include <QQmlEngine>

class RpgEnemyBase : public RpgEnemy
{
	Q_OBJECT
	QML_ELEMENT

public:
	explicit RpgEnemyBase(const RpgGameData::EnemyBaseData::EnemyType &type, TiledScene *scene = nullptr);
	explicit RpgEnemyBase(TiledScene *scene = nullptr) : RpgEnemyBase(RpgGameData::EnemyBaseData::EnemyInvalid, scene) {}
	virtual ~RpgEnemyBase();

	QString subType() const;
	void setSubType(const QString &newSubType);

	virtual int enemyType() const override { return RpgEnemyIface::enemyType(); }

	virtual bool canBulletImpact(const RpgGameData::Weapon::WeaponType &type) const override;


protected:
	void updateSprite() override final;

	void load() override final;
	RpgWeapon *defaultWeapon() const override;

	std::unique_ptr<RpgGameData::Body> serializeThis() const override;

	void eventPlayerReached(IsometricPlayer *player) override final;
	void eventPlayerLeft(IsometricPlayer */*player*/) override final {}

	void attackedByPlayer(RpgPlayer *player, const RpgGameData::Weapon::WeaponType &weaponType) override final;


	void playAttackEffect(RpgWeapon *weapon) override final;

	QPointF getPickablePosition(const int &num) const override final;

	QString m_subType;
	QString m_directory = QStringLiteral("base");

private:
	void onCurrentSpriteChanged();
	void loadType();

	TiledEffectHealed m_effectHealed;
	TiledEffectFire m_effectFire;
	TiledEffectSleep m_effectSleep;

	TiledGameSfx m_sfxRoar;
};

#endif // RPGENEMYBASE_H
