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
#include "qdiriterator.h"
#include "question.h"
#include "gameterrain.h"
#include <QRandomGenerator>
#include <Logger.h>


QStringList AbstractLevelGame::m_availableMusic;
QStringList AbstractLevelGame::m_availableMedal;



/**
 * @brief AbstractLevelGame::AbstractLevelGame
 * @param mode
 * @param client
 */

AbstractLevelGame::AbstractLevelGame(const GameMap::GameMode &mode, GameMapMissionLevel *missionLevel, Client *client)
	: AbstractGame(mode, client)
	, m_missionLevel(missionLevel)
	, m_timerLeft(new QTimer(this))
{
	if (m_missionLevel)
		setMap(m_missionLevel->map());
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
	LOG_CDEBUG("game") << "Create questions";

	QVector<Question> list;

	if (!m_missionLevel) {
		LOG_CWARNING("game") << "Missing game map, don't created any question";
		return list;
	}

	foreach (GameMapChapter *chapter, m_missionLevel->chapters()) {
		foreach (GameMapObjective *objective, chapter->objectives()) {
			int n = (objective->storageId() > 0 ? objective->storageCount() : 1);

			for (int i=0; i<n; ++i)
				list.append(Question(objective));

		}
	}

	LOG_CDEBUG("game") << "Created " << list.size() << " questions";

	return list;
}




/**
 * @brief AbstractLevelGame::onTimerLeftTimeout
 */

void AbstractLevelGame::onTimerLeftTimeout()
{
	if (m_deadlineTimeout || m_closedSuccesfully) {
		m_timerLeft->stop();
		return;
	}

	emit msecLeftChanged();

	if (m_deadline.hasExpired()) {
		LOG_CDEBUG("game") << "Game timeout";

		m_deadlineTimeout = true;
		m_timerLeft->stop();
		emit gameTimeout();
		return;
	}

}

const QStringList &AbstractLevelGame::availableMedal()
{
	return m_availableMedal;
}

int AbstractLevelGame::xp() const
{
	return m_xp;
}

void AbstractLevelGame::setXp(int newXp)
{
	if (m_xp == newXp)
		return;
	m_xp = newXp;
	emit xpChanged();
}


/**
 * @brief AbstractLevelGame::msecLeft
 * @return
 */

int AbstractLevelGame::msecLeft() const
{
	if (m_deadlineTimeout)
		return 0;

	if (!m_timerLeft->isActive())
		return duration()*1000;

	if (m_deadline.hasExpired())
		return 0;

	return m_deadline.remainingTime();
}


/**
 * @brief AbstractLevelGame::reloadAvailableMusic
 */

void AbstractLevelGame::reloadAvailableMusic()
{
	LOG_CDEBUG("game") << "Reload available music...";

	m_availableMusic.clear();

	QDirIterator it(QStringLiteral(":/sound/music"), QDir::Files);

	while (it.hasNext())
		m_availableMusic.append(it.next());

	LOG_CDEBUG("game") << "...loaded " << m_availableMusic.size() << "music";
}


/**
 * @brief AbstractLevelGame::reloadAvailableMedal
 */

void AbstractLevelGame::reloadAvailableMedal()
{
	LOG_CDEBUG("game") << "Reload available medal...";

	m_availableMedal.clear();

	QDirIterator it(QStringLiteral(":/internal/medal"), QDir::Files);

	while (it.hasNext()) {
		it.next();
		m_availableMedal.append(it.fileName());
	}

	LOG_CDEBUG("game") << "...loaded " << m_availableMedal.size() << " medal";
}



/**
 * @brief AbstractLevelGame::medalImagePath
 * @param medal
 * @return
 */

QString AbstractLevelGame::medalImagePath(const QString &medal)
{
	if (m_availableMedal.contains(medal))
		return QStringLiteral("qrc:/internal/medal/")+medal;
	else
		return QLatin1String("");
}



/**
 * @brief AbstractLevelGame::medalImagePath
 * @param mission
 * @return
 */

QString AbstractLevelGame::medalImagePath(GameMapMission *mission)
{
	if (!mission)
		return QLatin1String("");

	return medalImagePath(mission->medalImage());
}


/**
 * @brief AbstractLevelGame::medalImagePath
 * @param image
 * @return
 */

QString AbstractLevelGame::medalImagePath(GameMapMissionLevel *missionLevel)
{
	if (!missionLevel)
		return QLatin1String("");
	else
		return medalImagePath(missionLevel->mission());
}


/**
 * @brief AbstractLevelGame::startWithRemainingTime
 * @param msec
 */

void AbstractLevelGame::startWithRemainingTime(const qint64 &msec)
{
	m_deadline.setRemainingTime(msec);
	m_deadlineTimeout = false;
	m_timerLeft->start();
	gameStart();
}


/**
 * @brief AbstractLevelGame::addToDeadline
 * @param msec
 */

void AbstractLevelGame::addToDeadline(const qint64 &msec)
{
	m_deadline += msec;
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
	return medalImagePath(m_missionLevel);
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
	if (!m_missionLevel)
		return QUrl(QStringLiteral("qrc:/internal/game/bg.png"));

	QString d;

	if (m_missionLevel->image() > 0)
		d = QStringLiteral("image://mapimage/%1").arg(m_missionLevel->image());

	if (d.isEmpty()) {
		const GameTerrain &t = GameTerrain::terrain(m_missionLevel->terrain());

		if (!t.name().isEmpty())
			d = t.backgroundImage();

		if (!d.isEmpty()) {
			if (d.startsWith(QStringLiteral(":")))
				d.prepend(QStringLiteral("qrc"));
			else if (!d.startsWith(QStringLiteral("qrc:")))
				d.prepend(QStringLiteral("qrc:"));
		}
	}

	return d.isEmpty() ? QStringLiteral("qrc:/internal/game/bg.png") : d;
}



/**
 * @brief AbstractLevelGame::backgroundMusic
 * @return
 */

QString AbstractLevelGame::backgroundMusic()
{
	if (!m_backgroundMusic.isEmpty())
		return m_backgroundMusic;

	QString d;

	if (m_missionLevel) {
		const GameTerrain &t = GameTerrain::terrain(m_missionLevel->terrain());

		if (!t.name().isEmpty())
			d = t.backgroundMusic();

		if (d.isEmpty() && m_availableMusic.size()) {
			d = m_availableMusic.at(QRandomGenerator::global()->bounded(m_availableMusic.size()));
		}

		if (d.isEmpty())
			d = QStringLiteral("qrc:/sound/music/default_bg_music.mp3");

		if (d.startsWith(QStringLiteral(":")))
			d.prepend(QStringLiteral("qrc"));
		else if (!d.startsWith(QStringLiteral("qrc:")))
			d.prepend(QStringLiteral("qrc:"));


		m_backgroundMusic = d;
	} else {
		d = QStringLiteral("qrc:/sound/music/default_bg_music.mp3");
	}

	return d;
}



/**
 * @brief AbstractLevelGame::isFlawless
 * @return
 */

bool AbstractLevelGame::isFlawless() const
{
	return m_isFlawless;
}

void AbstractLevelGame::setIsFlawless(bool newIsFlawless)
{
	if (m_isFlawless == newIsFlawless)
		return;
	m_isFlawless = newIsFlawless;
	emit isFlawlessChanged();
}
