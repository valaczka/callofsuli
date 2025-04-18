/*
 * ---- Call of Suli ----
 *
 * rpglightning.h
 *
 * Created on: 2024. 07. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLightning
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

#ifndef RPGLIGHTNING_H
#define RPGLIGHTNING_H

#include "rpgarmory.h"
#include "tiledgamesfx.h"


/**
 * @brief The RpgLightning class
 */

class RpgLightning : public RpgBullet
{
	Q_OBJECT

public:
	RpgLightning(TiledGame *game = nullptr);
	virtual ~RpgLightning() {}

protected:
	void load() override final;
	virtual void impactEvent(TiledObjectBody *base, cpShape *shape) override final;
};





/**
 * @brief The RpgLightningWeapon class
 */

class RpgLightningWeapon : public RpgWeapon
{
	Q_OBJECT
public:
	explicit RpgLightningWeapon(QObject *parent = nullptr);

protected:
	void eventAttack(TiledObject *target) override final;

private:
	std::unique_ptr<TiledGameSfx> m_sfx;
};


#endif // RPGLIGHTNING_H
