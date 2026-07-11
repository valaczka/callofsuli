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

#include "qtypes.h"
#include "rpgstream.h"

/// Config ----------------------------------------------

#define	CFG_GAME_DURATION				3*60*60					// duration in tick
#define CFG_GAME_STAGE_MAIN				10*60 //60*60					// start stage main in tick
#define CFG_GAME_STAGE_LAST				CFG_GAME_DURATION-30*60	// start last stage in tick
#define CFG_GAME_STAGE_SELECT			1250					// stage select max. length in msec

#define	CFG_EMITTER_CAPACITY_STAGE_WU	0.75					// mp emitter capacity ratio in stage Warming Up
#define	CFG_EMITTER_DELAY_STAGE_WU		5*60					// mp emitter delay in tick in stage Warming Up

#define	CFG_EMITTER_CAPACITY_STAGE_M	1.0						// mp emitter capacity ratio in stage Main
#define	CFG_EMITTER_DELAY_STAGE_M		1*60					// mp emitter delay in tick in stage Main

#define	CFG_EMITTER_CAPACITY_STAGE_L	1.5						// mp emitter capacity ratio in stage Last
#define	CFG_EMITTER_DELAY_STAGE_L		1						// mp emitter delay in tick in stage Last

#define CFG_PLAYER_RESPAWN				300						// player respawn in tick

#define CFG_PENALTY_TOWER				300						// player question penalty in tick after failed tower attack
#define CFG_PENALTY_AUTO_UNLOCK			300						// player penalty after auto unlock (no answer)

#define CFG_TOWER_LOCK					60*30					// tower lock after activation in tick
#define CFG_TOWER_INACTIVE				80						// tower inactivate below percent

#define CFG_DEFENDER_DESTROY			300						// tower defender destroy after inactivation in tick

#define CFG_MAX_KNOCKBACK				900						// max. knockback velocity limit
#define CFG_KNOCKBACK_DECAY_PER_SEC		5.0f					// knockback decay

#define CFG_POINT						3						// team points / tower / sec
#define CFG_POINT_STAGE_L				6						// team points / tower / sec in stage Last

#define CFG_MP_CHANGE_BULLET			1//8					// mp cost of bullet change

#define CFG_QUESTION_MAX_DURATION		10*60	// 30*60		// question's max. duration in tick

/// -----------------------------------------------------


/// DEFENDER

struct CfgDefenderBase
{
	const quint32 maxHp = 3;

	const quint32 radius = 350;					// Ekkora körben hat

	const quint32 repeaterDelay = 30;			// Ennyi tick kell két akció között
	const quint32 actionsToHpLoss = 3;			// Ennyi akció után veszít 1 hp-t

	const bool alwaysVisible = false;			// Már rögtön látható-e?
};




struct CfgDefenderPulse
{
	const CfgDefenderBase base = {
		.maxHp = 3,
		.radius = 150,
		.repeaterDelay = 20,
		.actionsToHpLoss = 12,
	};

	const quint32 push = 400;
	const quint32 pushDist = base.radius * 1.75;

	RpgStream::EntityConfig toEntityConfig() const {
		RpgStream::EntityConfig cfg;
		cfg.setPush(push);
		cfg.setPushDist(pushDist);
		cfg.setMaxHp(base.maxHp);
		return cfg;
	}
};



static inline const CfgDefenderPulse cfgDefenderPulse = {};
static inline const CfgDefenderBase cfgDefenderFog = { .radius = 250 };
static inline const CfgDefenderBase cfgDefenderMultiplier = { .maxHp = 4 };





























#endif // RPGCONFIG_H
