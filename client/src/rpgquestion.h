/*
 * ---- Call of Suli ----
 *
 * rpgquestion.h
 *
 * Created on: 2024. 03. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgQuestion
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

#ifndef RPGQUESTION_H
#define RPGQUESTION_H

#include "question.h"
#include "rpgcontainer.h"
#include "rpgplayer.h"

class ActionRpgGame;


/**
 * @brief The RpgQuestion class
 */

class RpgQuestion
{

public:
	RpgQuestion(ActionRpgGame *game);
	~RpgQuestion() = default;

	void reloadQuestions();
	bool nextQuestion(RpgPlayer *player, RpgEnemy *enemy, const RpgGameData::Weapon::WeaponType &weaponType,
					  RpgContainer *container = nullptr);

	void questionSuccess(const QVariantMap &answer);
	void questionFailed(const QVariantMap &answer);
	void questionFinished();

	RpgEnemy *enemy() const { return m_enemy; }
	RpgPlayer *player() const { return m_player; }

	void initialize();

	bool emptyQuestions() const;
	qint64 duration() const;

private:
	ActionRpgGame *const m_game;
	QVector<Question> m_questionList;
	QVector<Question>::const_iterator m_questionIterator;

	QPointer<RpgPlayer> m_player;
	QPointer<RpgEnemy> m_enemy;
	QPointer<RpgContainer> m_container;
	RpgGameData::Weapon::WeaponType m_weaponType = RpgGameData::Weapon::WeaponInvalid;

	bool m_emptyQuestions = true;

	qint64 m_duration = 0;
};

#endif // RPGQUESTION_H
