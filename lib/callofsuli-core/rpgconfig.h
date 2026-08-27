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
#include <random>

/// Config ----------------------------------------------

#define	CFG_GAME_DURATION				5*60*60					// duration in tick
#define CFG_GAME_STAGE_MAIN				60*60					// start stage main in tick
#define CFG_GAME_STAGE_LAST				CFG_GAME_DURATION-60*60	// start last stage in tick
#define CFG_GAME_STAGE_SELECT			30000					// stage select max. length in msec

#define	CFG_EMITTER_CAPACITY_STAGE_WU	0.75					// mp emitter capacity ratio in stage Warming Up
#define	CFG_EMITTER_DELAY_STAGE_WU		5*60					// mp emitter delay in tick in stage Warming Up

#define	CFG_EMITTER_CAPACITY_STAGE_M	1.0						// mp emitter capacity ratio in stage Main
#define	CFG_EMITTER_DELAY_STAGE_M		1*60					// mp emitter delay in tick in stage Main

#define	CFG_EMITTER_CAPACITY_STAGE_L	1.5						// mp emitter capacity ratio in stage Last
#define	CFG_EMITTER_DELAY_STAGE_L		1						// mp emitter delay in tick in stage Last

#define CFG_PLAYER_RESPAWN				300						// player respawn in tick

#define	CFG_NPC_RESPAWN_STAGE_WU		45*60					// NPC respawn in tick in stage Warming Up
#define	CFG_NPC_RESPAWN_STAGE_M			30*60					// NPC respawn in tick in stage Main
#define	CFG_NPC_RESPAWN_STAGE_L			15*60					// NPC respawn in tick in stage Last

#define CFG_PENALTY_AUTO_UNLOCK			300						// player penalty after auto unlock (no answer)

#define CFG_TOWER_COUNT					4						// used towers
#define CFG_TOWER_LOCK					20*60					// tower lock after activation in tick
#define CFG_TOWER_LOCK_STAGE_L			0						// tower lock after activation in tick in stage Last
#define CFG_TOWER_INACTIVE				80						// tower inactivate below percent

#define CFG_DEFENDER_DESTROY			300						// tower defender destroy after inactivation in tick

#define CFG_MAX_KNOCKBACK				900						// max. knockback velocity limit
#define CFG_KNOCKBACK_DECAY_PER_SEC		5.0f					// knockback decay

#define CFG_POINT						3						// team points / tower / sec
#define CFG_POINT_STAGE_L				6						// team points / tower / sec in stage Last

#define CFG_MAX_POINT_FACTOR			1.0						// max. point calculation factor for multiplayer heats

#define CFG_MP_CHANGE_BULLET			10						// mp cost of bullet change

#define CFG_QUESTION_MAX_DURATION		30*60					// question's max. duration in tick

#define CFG_RESULT_WEIGHT_QUESTION		0.6						// weight of question ratio
#define CFG_RESULT_WEIGHT_PTS			0.3						// weight of points
#define CFG_RESULT_WEIGHT_STREAK		0.1						// weight of question's streak
#define CFG_RESULT_HEAT_RATIO			0.5						// heat ratio [ factor *= (1+ratio*heat) ]

#define CFG_POWER_LEVEL_COUNT			8						// Character's maximum power level
#define CFG_POWER_POINT_TOKEN			100						// x point = 1 token

/// -----------------------------------------------------


/// Character's power level details

struct CfgPowerLevel {
	int hp = 0;
	int mp = 0;
	int bullet = 0;
	int towerPlus = 0;				// lépésszámot adunk meg, de az atLevel() már töltöttséget ad vissza!
	int towerMinus = 0;				// lépésszámot adunk meg, de az atLevel() már töltöttséget ad vissza!
	float penalty = 0;				// "static"
	int push = 0;
	int pushDist = 0;
	int pushRest = 0;
	int skipLock = 0;				// "static"
	int defenderCount = 0;			// "static"
	int utilityCount = 0;			// "static"




	/**
	 * @brief powerLevelCostAt
	 * @param cost
	 * @param level
	 * @return
	 */

	static int powerLevelCostAt(const int &cost, const int &level) {
		return level > 1 ? cost * (1.f + 0.25 * (level-2)) : 0;
	}


	/**
	 * @brief fromPlayerConfig
	 * @param cfg
	 * @return
	 */

	static CfgPowerLevel fromPlayerConfig(const RpgStream::PlayerConfig &cfg)
	{
		CfgPowerLevel p;

		p.hp = cfg.entity().maxHp();
		p.mp = cfg.maxMp();
		p.bullet = cfg.maxBullet();
		p.towerPlus = cfg.towerPlus();
		p.towerMinus = cfg.towerMinus();

		p.push = cfg.entity().push();
		p.pushDist = cfg.entity().pushDist();
		p.pushRest = cfg.entity().resist();

		return p;
	}


	/**
	 * @brief atLevel
	 * @param level
	 * @return
	 */

	CfgPowerLevel atLevel(const int &level) const
	{
		CfgPowerLevel r;

		int realLevel = 1;

		if (level >= 1 && level <= CFG_POWER_LEVEL_COUNT)
			realLevel = level-1;


		// A lépésszámnak megfeleltetendő érték, amivel a tower töltöttsége változik (%)

		static const std::array<int, CFG_POWER_LEVEL_COUNT> towerLevel = {
			10, 20, 25, 25, 33, 50, 75, 100
		};


		// A kezdő lépéshez képesti lépésszám-változás

		static const std::array<int, CFG_POWER_LEVEL_COUNT> towerStepPlus = {
			0, 0, 1, 1, 1, 2, 2, 3
		};

		static const std::array<int, CFG_POWER_LEVEL_COUNT> towerStepMinus = {
			0, 0, 0, 1, 1, 1, 2, 2
		};


		r.hp = (float) this->hp * (1.0 + 0.2 * realLevel);
		r.mp = (float) this->mp * (1.0 + 0.2 * realLevel);
		r.bullet = (float) this->bullet * (1.0 + 0.25 * realLevel);
		r.push = (float) this->push * (1.0 + 0.02 * realLevel);
		r.pushDist = (float) this->pushDist * (1.0 + 0.1 * realLevel);
		r.pushRest = (float) this->pushRest * (1.0 + 0.3 * realLevel);

		const int tPlus = std::min(CFG_POWER_LEVEL_COUNT-1, this->towerPlus + towerStepPlus.at(realLevel));
		r.towerPlus = towerLevel.at(tPlus);

		const int tMinus = std::min(CFG_POWER_LEVEL_COUNT-1, this->towerMinus + towerStepMinus.at(realLevel));
		r.towerMinus = towerLevel.at(tMinus);


		// STATIC

		static const std::array<float, CFG_POWER_LEVEL_COUNT> penaltyValue = {
			5.f, 5.f, 4.5, 4.f, 3.5, 3.f, 2.5, 2.f
		};

		static const std::array<int, CFG_POWER_LEVEL_COUNT> dCountValue = {
			1, 1, 2, 2, 2, 2, 3, 3
		};

		static const std::array<int, CFG_POWER_LEVEL_COUNT> uCountValue = {
			0, 0, 0, 1, 1, 2, 2, 2
		};

		static const std::array<int, CFG_POWER_LEVEL_COUNT> sCountValue = {
			5, 5, 5, 4, 4, 4, 3, 3
		};

		r.penalty = penaltyValue.at(realLevel);
		r.defenderCount = dCountValue.at(realLevel);
		r.utilityCount = uCountValue.at(realLevel);
		r.skipLock = sCountValue.at(realLevel);

		return r;
	}
};




/// Rpg Drop


struct CfgDrop
{
	enum Tier {
		Common = 0,
		Uncommon,
		Rare,
		Epic,
		Legendary
	};

	enum Type {
		DropInvalid = 0,
		DropGame = 1,
		DropTerrain = 2,
	};

	Tier tier = Common;

	quint32 xp = 0;
	quint32 point = 0;
	quint32 token = 0;
};




/**
 * @brief The CfgDropRange class
 */

struct CfgDropRange
{
	std::vector<quint32> xp;
	std::vector<quint32> point;
	std::vector<quint32> token;
};

static inline const QHash<CfgDrop::Tier, CfgDropRange> cfgDropRange = {
	{ CfgDrop::Common, {
		  .xp = {500, 550, 600, 650, 700, 750, 800},
		  .point = {250, 300, 350, 400, 450, 500, 550, 600},
		  .token = {50, 60, 70, 80, 90}
	  }	},

	{ CfgDrop::Uncommon, {
		  .xp = {700, 750, 800, 850, 900, 950, 1000, 1050},
		  .point = {500, 550, 600, 650, 700, 750, 800, 850},
		  .token = {80, 90, 100, 110, 120, 130}
	  }	},

	{ CfgDrop::Rare, {
		  .xp = {1000, 1100, 1200, 1300, 1400},
		  .point = {800, 900, 1000, 1100, 1200},
		  .token = {120, 130, 140, 150, 160}
	  }	},

	{ CfgDrop::Epic, {
		  .xp = {1500, 1750, 2000, 2250, 2500},
		  .point = {1300, 1500, 1700, 1900},
		  .token = {180, 200, 220, 240, 260}
	  }	},

	{ CfgDrop::Legendary, {
		  .xp = {3000, 3500, 4000, 4500},
		  .point = {2000, 2500, 3000, 3500, 4000},
		  .token = {300, 350, 400, 450, 500}
	  }	},
};

static inline std::discrete_distribution<int> cfgDropDistributionNormal = { 50, 28, 15, 5, 2 };
static inline std::discrete_distribution<int> cfgDropDistributionMedium = { 0, 50, 30, 15, 5 };
static inline std::discrete_distribution<int> cfgDropDistributionHeigh =  { 0, 0, 50, 40, 10 };



/**
 * @brief The CfgDropGenerator class
 */

struct CfgDropGenerator
{
	static inline CfgDrop generate(std::mt19937 &rnd, std::discrete_distribution<int> &dist) {
		CfgDrop d;
		d.tier = static_cast<CfgDrop::Tier>(dist(rnd));

		const CfgDropRange &r = cfgDropRange.value(d.tier);

		if (!r.xp.empty()) {
			std::uniform_int_distribution<int> dist(0, r.xp.size()-1);
			d.xp = r.xp.at(dist(rnd));
		}

		if (!r.point.empty()) {
			std::uniform_int_distribution<int> dist(0, r.point.size()-1);
			d.point = r.point.at(dist(rnd));
		}

		if (!r.token.empty()) {
			std::uniform_int_distribution<int> dist(0, r.token.size()-1);
			d.token = r.token.at(dist(rnd));
		}

		return d;
	}

	static inline CfgDrop generate(std::mt19937 &rnd) {
		return generate(rnd, cfgDropDistributionNormal);
	}
};



/**
 * @brief cfgDropDay
 */

static inline const std::vector<int> cfgDropDay = { 1, 3, 5 };				// Napi győzelem
static inline const std::vector<int> cfgDropTerrain = { 2, 5, 10, 15 };		// Terep győzelem




/// Rewards

// Streak rewards

struct CfgRewardStreak {
	quint32 point;
	quint32 hp;
};

static const QHash<quint32, CfgRewardStreak> cfgRewardStreak = {
	{ 3, {.point=15, .hp=0} },
	{ 5, {.point=20, .hp=1} },
	{ 7, {.point=30, .hp=1} },
	{ 10, {.point=50, .hp=2} },
	{ 12, {.point=75, .hp=2} },
	{ 15, {.point=100, .hp=2} },
};







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
		.maxHp = 4,
		.radius = 150,
		.repeaterDelay = 20,
		.actionsToHpLoss = 2,
	};

	const quint32 push = 700;
	const quint32 pushDist = base.radius * 2;

	RpgStream::EntityConfig toEntityConfig() const {
		RpgStream::EntityConfig cfg;
		cfg.setPush(push);
		cfg.setPushDist(pushDist);
		cfg.setMaxHp(base.maxHp);
		return cfg;
	}
};



struct CfgDefenderAttack
{
	const CfgDefenderBase base = {
		.maxHp = 1,
		.radius = 300,
		.repeaterDelay = 120,
		.actionsToHpLoss = 0,
	};

	const quint32 force = 2;					// Ennyi HP-t sebez
};


static inline const CfgDefenderPulse cfgDefenderPulse = {};
static inline const CfgDefenderBase cfgDefenderFog = { .radius = 250 };
static inline const CfgDefenderBase cfgDefenderMultiplier = { .maxHp = 2 };
static inline const CfgDefenderAttack cfgDefenderElectric = {
	.base = { .maxHp = 5, .actionsToHpLoss = 1 },
	.force = 2
};
static inline const CfgDefenderAttack cfgDefenderQuestionnaire = {
	.base =  { .maxHp = 1,
			   .repeaterDelay = 200,
			   .actionsToHpLoss = 0
	},
	.force = 1
};
static inline const CfgDefenderAttack cfgDefenderHpHealer = {
	.base =  { .maxHp = 2,
			   .radius = 250,
			   .repeaterDelay = 60,
			   .actionsToHpLoss = 8
	},
	.force = 1
};

static inline const QHash<RpgStream::BaseDefenderObject::Type, int> cfgRequiredMpDefender = {
	{ RpgStream::BaseDefenderObject::Fog,					10 },
	{ RpgStream::BaseDefenderObject::Multiplier1,			8 },
	{ RpgStream::BaseDefenderObject::Pulse,					12 },
	{ RpgStream::BaseDefenderObject::Electric,				15 },
	{ RpgStream::BaseDefenderObject::Questionnaire,			18 },
	{ RpgStream::BaseDefenderObject::HpHealer,				12 },
};






/// UTILITY


struct CfgUtilityDistance
{
	const quint32 dist = 200;
};

struct CfgUtilityDuration
{
	const quint32 duration = 10*60;									// tick
};


static inline const CfgUtilityDistance cfgUtilityMissionary = {};
static inline const CfgUtilityDistance cfgUtilitySniper = { .dist = 800 };
static inline const CfgUtilityDuration cfgUtilityInvisible = { .duration = 20*60 };
static inline const CfgUtilityDuration cfgUtilityBlockMpPick = { .duration = 15*60 };
static inline const CfgUtilityDuration cfgUtilityBlockMpConvert = { .duration = 15*60 };
static inline const CfgUtilityDuration cfgUtilityBlockAttack = { .duration = 15*60 };
static inline const CfgUtilityDuration cfgUtilityBoostPoint = { .duration = 20*60 };

static inline const QHash<RpgStream::PlayerConfig::Utility, int> cfgRequiredMpUtility = {
	{ RpgStream::PlayerConfig::UtilityMissionary,					15 },
	{ RpgStream::PlayerConfig::UtilitySniper,						15 },
	{ RpgStream::PlayerConfig::UtilityInvisible,					15 },
	{ RpgStream::PlayerConfig::UtilityBlockMpPick,					20 },
	{ RpgStream::PlayerConfig::UtilityBlockMpConvert,				20 },
	{ RpgStream::PlayerConfig::UtilityBlockAttack,					20 },
	{ RpgStream::PlayerConfig::UtilityBoostAttackTower,				22 },
	{ RpgStream::PlayerConfig::UtilityBoostPoint,					22 },
};




















#endif // RPGCONFIG_H
