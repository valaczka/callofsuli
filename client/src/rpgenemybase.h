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

#include "isometricenemy.h"
#include "rpgenemyiface.h"
#include "tiledeffect.h"
#include <QQmlEngine>

class RpgEnemyBase : public IsometricEnemy, public RpgEnemyIface
{
	Q_OBJECT
	QML_ELEMENT

public:
	explicit RpgEnemyBase(const RpgEnemyType &type, QQuickItem *parent = nullptr);
	explicit RpgEnemyBase(QQuickItem *parent = nullptr) : RpgEnemyBase(RpgEnemyIface::EnemyInvalid, parent) {}
	virtual ~RpgEnemyBase();

	TiledWeapon *defaultWeapon() const override;
	virtual bool canBulletImpact(const TiledWeapon::WeaponType &type) const override;

	int getNewHpAfterAttack(const int &origHp, const TiledWeapon::WeaponType &weaponType,
									IsometricPlayer *player = nullptr) const override;

	QString subType() const;
	void setSubType(const QString &newSubType);


protected:
	void updateSprite() override final;

	void load() override final;
	void eventPlayerReached(IsometricPlayer *player) override final;
	void eventPlayerLeft(IsometricPlayer */*player*/) override final {}

	void attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType) override final;


	void playAttackEffect(TiledWeapon *weapon) override final;

	QPointF getPickablePosition(const int &num) const override final;

	bool protectWeapon(const TiledWeapon::WeaponType &weaponType) override;

	QString m_subType;
	QString m_directory = QStringLiteral("base");

private:
	void onCurrentSpriteChanged();
	void loadType();

	TiledEffectHealed m_effectHealed;
	TiledEffectFire m_effectFire;
	TiledEffectSleep m_effectSleep;
};

#endif // RPGENEMYBASE_H
