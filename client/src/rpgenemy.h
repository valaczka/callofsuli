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
#include <QObject>
#include <QQmlEngine>

class RpgEnemy : public IsometricEnemy, public RpgEnemyIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(TiledWeapon* defaultWeapon READ defaultWeapon CONSTANT FINAL)

public:
	RpgEnemy(const RpgGameData::EnemyBaseData::EnemyType &type, TiledScene *scene);

	virtual TiledObjectBody::ObjectId objectId() const override { return IsometricEnemy::objectId(); }

	virtual void updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Enemy> &snapshot) override;
	virtual void updateFromSnapshot(const RpgGameData::Enemy &snap) override;
	virtual void updateFromLastSnapshot(const RpgGameData::Enemy &snap) override {
		RpgGameDataInterface::updateFromLastSnapshot(snap, &m_lastSnapshot);
	}

protected:
	virtual bool enemyWorldStep() override;
	virtual bool enemyWorldStepOnVisiblePlayer() override;
	virtual void attackPlayer(RpgPlayer *player, RpgWeapon *weapon) override;

	RpgGameData::Enemy serializeEnemy() const;

	qint64 m_lastSnap = -1;
	RpgGameData::Enemy m_lastSnapshot;

	friend class ActionRpgMultiplayerGame;
};

#endif // RPGENEMY_H
