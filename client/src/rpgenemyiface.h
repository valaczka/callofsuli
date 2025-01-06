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
#include "tiledweapon.h"
#include <QString>
#include <QList>
#include <QHash>

class RpgEnemyIface : public RpgGameDataInterface
{
public:
	RpgEnemyIface(const RpgGameData::Enemy::EnemyType &type)
		: m_enemyType(type)
	{}
	RpgEnemyIface() {}

	static QStringList availableTypes() { return m_typeHash.keys(); }
	static RpgGameData::Enemy::EnemyType typeFromString(const QString &type) { return m_typeHash.value(type, RpgGameData::Enemy::EnemyInvalid); }
	static QString directoryBaseName(const RpgGameData::Enemy::EnemyType type, const QString &subType = {});

	const RpgGameData::Enemy::EnemyType &enemyType() const { return m_enemyType; }

	virtual bool protectWeapon(const TiledWeapon::WeaponType &weaponType) = 0;

	RpgArmory *armory() const { return m_armory.get(); }


protected:
	virtual QPointF getPickablePosition(const int &num) const = 0;

	RpgGameData::Enemy::EnemyType m_enemyType = RpgGameData::Enemy::EnemyInvalid;
	std::unique_ptr<RpgArmory> m_armory;

private:
	static const QHash<QString, RpgGameData::Enemy::EnemyType> m_typeHash;

	friend class RpgGame;
};




/**
 * @brief RpgEnemyIface::directory
 * @param type
 * @param subType
 * @return
 */

inline QString RpgEnemyIface::directoryBaseName(const RpgGameData::Enemy::EnemyType type, const QString &subType)
{
	if (type == RpgGameData::Enemy::EnemyWerebear)
		return QStringLiteral("werebear");
	else
		return subType;
}



#endif // RPGENEMYIFACE_H
