/*
 * ---- Call of Suli ----
 *
 * rpgenemyiface.h
 *
 * Created on: 2024. 03. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef RPGENEMYIFACE_H
#define RPGENEMYIFACE_H

#include "qpoint.h"
#include "rpgarmory.h"
#include "rpggamedataiface.h"
#include "rpgconfig.h"
#include <QString>
#include <QList>
#include <QHash>
#include "isometricplayer.h"



class RpgPlayer;


class RpgEnemyIface : public RpgGameDataInterface<RpgGameData::Enemy, RpgGameData::EnemyBaseData>
{
public:
	RpgEnemyIface(const RpgGameData::EnemyBaseData::EnemyType &type)
		: RpgGameDataInterface<RpgGameData::Enemy, RpgGameData::EnemyBaseData>()
		, m_enemyType(type)
	{}

	RpgEnemyIface()
		: RpgEnemyIface(RpgGameData::EnemyBaseData::EnemyInvalid)
	{}

	static QStringList availableTypes() { return m_typeHash.keys(); }
	static RpgGameData::EnemyBaseData::EnemyType typeFromString(const QString &type) {
		return m_typeHash.value(type, RpgGameData::EnemyBaseData::EnemyInvalid);
	}
	static QString directoryBaseName(const RpgGameData::EnemyBaseData::EnemyType type, const QString &subType = {});

	const RpgGameData::EnemyBaseData::EnemyType &enemyType() const { return m_enemyType; }


	RpgArmory *armory() const { return m_armory.get(); }


	virtual RpgWeapon *defaultWeapon() const = 0;
	virtual bool canBulletImpact(const RpgGameData::Weapon::WeaponType &type) const { Q_UNUSED(type); return true; }
	virtual void attackedByPlayer(RpgPlayer *player, const RpgGameData::Weapon::WeaponType &weaponType) = 0;

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Enemy> &snapshot) = 0;
	virtual void updateFromSnapshot(const RpgGameData::Enemy &snap) = 0;

protected:
	virtual QPointF getPickablePosition(const int &num) const = 0;
	virtual void attackPlayer(RpgPlayer *player, RpgWeapon *weapon) = 0;
	virtual void playAttackEffect(RpgWeapon *weapon) { Q_UNUSED(weapon); }

	RpgGameData::EnemyBaseData::EnemyType m_enemyType = RpgGameData::EnemyBaseData::EnemyInvalid;
	std::unique_ptr<RpgArmory> m_armory;

private:
	static const QHash<QString, RpgGameData::EnemyBaseData::EnemyType> m_typeHash;

	friend class RpgGame;
	friend class ActionRpgGame;
	friend class ActionRpgMultiplayerGame;
};




/**
 * @brief RpgEnemyIface::directoryBaseName
 * @param type
 * @param subType
 * @return
 */

inline QString RpgEnemyIface::directoryBaseName(const RpgGameData::EnemyBaseData::EnemyType type, const QString &subType)
{
	if (type == RpgGameData::EnemyBaseData::EnemyWerebear)
		return QStringLiteral("werebear");
	else
		return subType;
}



#endif // RPGENEMYIFACE_H
