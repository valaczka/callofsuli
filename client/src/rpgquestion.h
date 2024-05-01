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
	bool nextQuestion(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType,
					  TiledContainer *container = nullptr);

	void questionSuccess(const QVariantMap &answer);
	void questionFailed(const QVariantMap &answer);
	void questionFinished();

	IsometricEnemy *enemy() const { return m_enemy; }
	RpgPlayer *player() const { return m_player; }

private:
	ActionRpgGame *const m_game;
	QVector<Question> m_questionList;
	QVector<Question>::const_iterator m_questionIterator;

	QPointer<RpgPlayer> m_player;
	QPointer<IsometricEnemy> m_enemy;
	QPointer<TiledContainer> m_container;
	TiledWeapon::WeaponType m_weaponType = TiledWeapon::WeaponInvalid;
};

#endif // RPGQUESTION_H
