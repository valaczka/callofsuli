/*
 * ---- Call of Suli ----
 *
 * gamequestioncomponent.cpp
 *
 * Created on: 2022. 12. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameQuestionComponent
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

#include "gamequestioncomponent.h"
#include "Logger.h"
#include "gamequestion.h"


GameQuestionComponent::GameQuestionComponent(QQuickItem *parent)
	: QQuickItem(parent)
{
	LOG_CTRACE("game") << "Game question component created" << this;
}

GameQuestionComponent::~GameQuestionComponent()
{
	LOG_CTRACE("game") << "Game question component destroyed" << this;
}

/**
 * @brief GameQuestionComponent::questionData
 * @return
 */

QVariantMap GameQuestionComponent::questionData() const
{
	if (m_question)
		return m_question->questionData();
	else
		return QVariantMap();
}



/**
 * @brief GameQuestionComponent::question
 * @return
 */

GameQuestion *GameQuestionComponent::question() const
{
	return m_question;
}

void GameQuestionComponent::setQuestion(GameQuestion *newQuestion)
{
	if (m_question == newQuestion)
		return;
	m_question = newQuestion;
	emit questionChanged();
	emit questionDataChanged();
	emit toggleModeChanged();
	emit storedAnswerChanged();
}



/**
 * @brief GameQuestionComponent::toggleMode
 * @return
 */

bool GameQuestionComponent::toggleMode() const
{
	return m_question ? m_question->toggleMode() : false;
}

QVariantMap GameQuestionComponent::storedAnswer() const
{
	return m_question ? m_question->storedAnswer() : QVariantMap();
}
