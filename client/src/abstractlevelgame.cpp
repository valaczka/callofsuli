/*
 * ---- Call of Suli ----
 *
 * abstractlevelgame.cpp
 *
 * Created on: 2022. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractLevelGame
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

#include "abstractlevelgame.h"
#include "question.h"



/**
 * @brief AbstractLevelGame::AbstractLevelGame
 * @param mode
 * @param client
 */

AbstractLevelGame::AbstractLevelGame(const Mode &mode, GameMapMissionLevel *missionLevel, Client *client)
	: AbstractGame(mode, client)
	, m_missionLevel(missionLevel)
	, m_timerLeft(new QTimer(this))
{
	m_timerLeft->setInterval(50);
	connect(m_timerLeft, &QTimer::timeout, this, &AbstractLevelGame::onTimerLeftTimeout);
}


/**
 * @brief AbstractLevelGame::~AbstractLevelGame
 */

AbstractLevelGame::~AbstractLevelGame()
{
	delete m_timerLeft;
}


/**
 * @brief AbstractLevelGame::createQuestions
 */

QVector<Question> AbstractLevelGame::createQuestions()
{
	qCDebug(lcGame).noquote() << tr("Create questions");

	QVector<Question> list;

	if (!m_missionLevel) {
		qCWarning(lcGame).noquote() << tr("Missing game map, don't created any question");
		return list;
	}

	foreach (GameMapChapter *chapter, m_missionLevel->chapters()) {
		foreach (GameMapObjective *objective, chapter->objectives()) {
			int n = (objective->storageId() != -1 ? objective->storageCount() : 1);

			for (int i=0; i<n; ++i) {
				Question q(objective);

				//q.generate();

				list.append(q);
			}
		}
	}

	qCDebug(lcGame).noquote() << tr("Created %1 question(s)").arg(list.size());

	return list;
}


/**
 * @brief AbstractLevelGame::onTimerLeftTimeout
 */

void AbstractLevelGame::onTimerLeftTimeout()
{
	qreal t = m_msecLeft - m_timerLeft->interval();

	if (t<=0) {
		qCDebug(lcGame).noquote() << tr("Game timeout");

		m_timerLeft->stop();
		setMsecLeft(0);
		emit gameTimeout();
		return;
	}

	setMsecLeft(t);
}


/**
 * @brief AbstractLevelGame::msecLeft
 * @return
 */

int AbstractLevelGame::msecLeft() const
{
	return m_msecLeft;
}

void AbstractLevelGame::setMsecLeft(int newMsecLeft)
{
	if (m_msecLeft == newMsecLeft)
		return;
	int d = newMsecLeft - m_msecLeft;
	m_msecLeft = newMsecLeft;
	emit msecLeftChanged(d);
}




/**
 * @brief AbstractLevelGame::deathmatch
 * @return
 */

bool AbstractLevelGame::deathmatch() const
{
	return m_deathmatch;
}

void AbstractLevelGame::setDeathmatch(bool newDeathmatch)
{
	if (m_deathmatch == newDeathmatch)
		return;
	m_deathmatch = newDeathmatch;
	emit deathmatchChanged();
}



GameMapMissionLevel *AbstractLevelGame::missionLevel() const
{
	return m_missionLevel;
}



/**
 * @brief AbstractLevelGame::uuid
 * @return
 */

QString AbstractLevelGame::uuid() const
{
	return m_missionLevel ? m_missionLevel->mission()->uuid() : "";
}

QString AbstractLevelGame::name() const
{
	return m_missionLevel ? m_missionLevel->mission()->name() : "";
}

QString AbstractLevelGame::description() const
{
	return m_missionLevel ? m_missionLevel->mission()->description() : "";
}

int AbstractLevelGame::level() const
{
	return m_missionLevel ? m_missionLevel->level() : -1;
}

QString AbstractLevelGame::medalImage() const
{
	return m_missionLevel ? m_missionLevel->mission()->medalImage() : "";
}

QString AbstractLevelGame::terrain() const
{
	return m_missionLevel ? m_missionLevel->terrain() : "";
}

int AbstractLevelGame::startHP() const
{
	return m_missionLevel ? m_missionLevel->startHP() : 1;
}

int AbstractLevelGame::duration() const
{
	return m_missionLevel ? m_missionLevel->duration() : 1;
}




/**
 * @brief AbstractLevelGame::backgroundImage
 * @return
 */

QUrl AbstractLevelGame::backgroundImage() const
{
	/*if (m_bgImage.isEmpty() || m_imageDbName.isEmpty())
					return "qrc:/internal/game/bg.png";
			else if (m_bgImage.startsWith("qrc:/"))
					return m_bgImage;
			else
					return "image://"+m_imageDbName+"/"+m_bgImage;

					return m_missionLevel ? m_missionLevel->image() : "";
*/

	return QUrl(QStringLiteral("qrc:/internal/game/bg.png"));
}

