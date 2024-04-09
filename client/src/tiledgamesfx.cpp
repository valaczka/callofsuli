/*
 * ---- Call of Suli ----
 *
 * sfx.cpp
 *
 * Created on: 2024. 03. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Sfx
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

#include "tiledgamesfx.h"
#include "tiledgame.h"



/**
 * @brief TiledGameSfx::TiledGameSfx
 * @param soundList
 * @param interval
 * @param volume
 * @param tiledObject
 */

TiledGameSfx::TiledGameSfx(const QStringList &soundList, const int &interval, const float &volume, TiledObject *tiledObject)
	: QObject(tiledObject)
	, m_tiledObject(tiledObject)
	, m_soundList(soundList)
	, m_volume(volume)
{
	m_deadlineTimer.setRemainingTime(-1);
	m_timer.setInterval(interval);
	connect(&m_timer, &QTimer::timeout, this, &TiledGameSfx::onTimeout);
	resetIndex();
}



/**
 * @brief TiledGameSfx::start
 */

void TiledGameSfx::start()
{
	if (m_timer.isActive())
		return;

	startTimer();
}


/**
 * @brief TiledGameSfx::startFromBegin
 */

void TiledGameSfx::startFromBegin()
{
	if (m_timer.isActive())
		return;

	restart();
}


/**
 * @brief TiledGameSfx::stop
 */

void TiledGameSfx::stop()
{
	m_timer.stop();
}


/**
 * @brief TiledGameSfx::restart
 */

void TiledGameSfx::restart()
{
	m_timer.stop();
	setCurrentIndex(0);
	startTimer();
}



/**
 * @brief TiledGameSfx::playOne
 */

void TiledGameSfx::playOne()
{
	if (!m_deadlineTimer.isForever() && !m_deadlineTimer.hasExpired())
		return;

	onTimeout();

	if (m_playOneDeadline > 0)
		m_deadlineTimer.setRemainingTime(m_playOneDeadline);
}


/**
 * @brief TiledGameSfx::playOne
 * @param deadlineMsec
 */

void TiledGameSfx::playOneDeadline(const qint64 &deadlineMsec)
{
	if (!m_deadlineTimer.isForever() && !m_deadlineTimer.hasExpired())
		return;

	onTimeout();

	m_deadlineTimer.setRemainingTime(deadlineMsec);
}


/**
 * @brief TiledGameSfx::soundList
 * @return
 */

QStringList TiledGameSfx::soundList() const
{
	return m_soundList;
}

void TiledGameSfx::setSoundList(const QStringList &newSoundList)
{
	if (m_soundList == newSoundList)
		return;
	m_soundList = newSoundList;
	emit soundListChanged();
	resetIndex();
}

float TiledGameSfx::volume() const
{
	return m_volume;
}

void TiledGameSfx::setVolume(float newVolume)
{
	if (qFuzzyCompare(m_volume, newVolume))
		return;
	m_volume = newVolume;
	emit volumeChanged();
}

int TiledGameSfx::currentIndex() const
{
	return m_currentIndex;
}

void TiledGameSfx::setCurrentIndex(int newCurrentIndex)
{
	if (m_currentIndex == newCurrentIndex)
		return;
	m_currentIndex = newCurrentIndex;
	emit currentIndexChanged();
}

/**
 * @brief TiledGameSfx::onTimeout
 */

void TiledGameSfx::onTimeout()
{
	if (m_soundList.isEmpty()) {
		m_timer.stop();
		LOG_CWARNING("sound") << "Emptys SoundSfx sound list";
		setCurrentIndex(-1);
		return;
	}

	if (!m_tiledObject) {
		m_timer.stop();
		LOG_CWARNING("sound") << "Missing SoundSfx TiledObject";
		setCurrentIndex(-1);
		return;
	}

	TiledScene *scene = m_tiledObject->scene();

	if (!scene) {
		m_timer.stop();
		LOG_CWARNING("sound") << "Invalid SoundSfx scene";
		setCurrentIndex(-1);
		return;
	}

	if (m_currentIndex < 0 || m_currentIndex >= m_soundList.size())
		setCurrentIndex(0);

	const QString &s = m_soundList.at(m_currentIndex);


	if (m_followPosition)
		m_tiledObject->game()->playSfx(s, scene, m_tiledObject->body()->bodyPosition(), m_volume);
	else
		m_tiledObject->game()->playSfx(s, scene, m_volume);

	setCurrentIndex(m_currentIndex+1);
}


/**
 * @brief TiledGameSfx::resetIndex
 */

void TiledGameSfx::resetIndex()
{
	if (m_soundList.isEmpty())
		setCurrentIndex(-1);
	else
		setCurrentIndex(0);
}


/**
 * @brief TiledGameSfx::startTimer
 */

void TiledGameSfx::startTimer()
{
	if (m_soundList.isEmpty()) {
		LOG_CWARNING("sound") << "Emptys SoundSfx sound list";
		setCurrentIndex(-1);
		return;
	}

	if (m_timer.interval() <= 0) {
		LOG_CWARNING("sound") << "Invalid SoundSfx interval";
		return;
	}

	onTimeout();
	m_timer.start();
}

bool TiledGameSfx::followPosition() const
{
	return m_followPosition;
}

void TiledGameSfx::setFollowPosition(bool newFollowPosition)
{
	if (m_followPosition == newFollowPosition)
		return;
	m_followPosition = newFollowPosition;
	emit followPositionChanged();
}

qint64 TiledGameSfx::playOneDeadline() const
{
	return m_playOneDeadline;
}

void TiledGameSfx::setPlayOneDeadline(qint64 newPlayOneDeadline)
{
	if (m_playOneDeadline == newPlayOneDeadline)
		return;
	m_playOneDeadline = newPlayOneDeadline;
	emit playOneDeadlineChanged();
}

TiledObject *TiledGameSfx::tiledObject() const
{
	return m_tiledObject;
}

void TiledGameSfx::setTiledObject(TiledObject *newTiledObject)
{
	if (m_tiledObject == newTiledObject)
		return;
	m_tiledObject = newTiledObject;
	emit tiledObjectChanged();
}
