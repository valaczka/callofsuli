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
#include "tiledweapon.h"
#include <QString>
#include <QList>
#include <QHash>

class RpgEnemyIface
{
public:
	enum RpgEnemyType {
		EnemyInvalid = 0,
		EnemyWerebear,
		EnemySoldier,
		EnemyArcher,
		EnemySoldierFix,
		EnemyArcherFix,
		EnemySkeleton
	};

	RpgEnemyIface(const RpgEnemyType &type)
		: m_enemyType(type)
	{}
	RpgEnemyIface() {}

	static QStringList availableTypes() { return m_typeHash.keys(); }
	static RpgEnemyType typeFromString(const QString &type) { return m_typeHash.value(type, EnemyInvalid); }

	const RpgEnemyType &enemyType() const { return m_enemyType; }

	virtual bool protectWeapon(const TiledWeapon::WeaponType &weaponType) = 0;

	RpgArmory *armory() const { return m_armory.get(); }

protected:
	virtual QPointF getPickablePosition(const int &num) const = 0;

	RpgEnemyType m_enemyType = EnemyInvalid;
	std::unique_ptr<RpgArmory> m_armory;

private:
	static const QHash<QString, RpgEnemyType> m_typeHash;

	friend class RpgGame;
};



#endif // RPGENEMYIFACE_H
