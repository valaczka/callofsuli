/*
 * ---- Call of Suli ----
 *
 * rpgconfig.h
 *
 * Created on: 2026. 06. 05.
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

#ifndef RPGCONFIG_H
#define RPGCONFIG_H


/// Config ----------------------------------------------

#define CFG_PLAYER_RESPAWN				300						// player respawn in tick

#define CFG_PENALTY_TOWER				300						// player question penalty in tick after failed tower attack
#define CFG_PENALTY_AUTO_UNLOCK			300						// player penalty after auto unlock (no answer)

#define CFG_TOWER_LOCK					60*30					// tower lock after activation in tick
#define CFG_TOWER_INACTIVE				80						// tower inactivate below percent

#define CFG_DEFENDER_DESTROY			300						// tower defender destroy after inactivation in tick

#define CFG_MAX_KNOCKBACK				1200					// max. knockback velocity limit
#define CFG_KNOCKBACK_DECAY_PER_SEC		5.0f					// knockback decay


#define CFG_MP_CHANGE_BULLET			8						// mp cost of bullet change

/// -----------------------------------------------------



#endif // RPGCONFIG_H
