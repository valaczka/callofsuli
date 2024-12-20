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
#include "application.h"



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

	if (scene->game() && scene->game()->paused())
		return;

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



/**
 * @brief TiledGameSfxLocation::TiledGameSfxLocation
 * @param path
 * @param baseVolume
 * @param tiledObject
 */

TiledGameSfxLocation::TiledGameSfxLocation(const QString &path, const float &baseVolume, TiledObjectBase *tiledObject,
										   const Sound::ChannelType &channel)
	: QObject(tiledObject)
	, m_object(tiledObject)
	, m_sound(Application::instance()->client()->sound()->externalSoundAdd(path, channel))
	, m_baseVolume(baseVolume)
{
	Q_ASSERT(tiledObject);

	LOG_CTRACE("sound") << "Create TiledGameSfxLocation" << this;

	connect(tiledObject, &TiledObjectBase::sceneChanged, this, &TiledGameSfxLocation::onSceneChanged);

	onSceneChanged();
}


/**
 * @brief TiledGameSfxLocation::~TiledGameSfxLocation
 */

TiledGameSfxLocation::~TiledGameSfxLocation()
{
	LOG_CTRACE("sound") << "Delete TiledGameSfxLocation" << this;
}


/**
 * @brief TiledGameSfxLocation::updateSound
 */

void TiledGameSfxLocation::updateSound()
{
	if (!m_sound)
		return;

	ma_sound *snd = m_sound->sound();

	if (!snd)
		return;

	const bool isCurrentScene = m_object && m_object->scene() && m_object->game() && m_object->game()->currentScene() == m_object->scene();


	if (!isCurrentScene || m_baseVolume <= 0.) {
		if (ma_sound_is_playing(snd)) {
			ma_sound_stop(snd);
			ma_sound_seek_to_pcm_frame(snd, 0);
		}
		m_lastVolume = 0.;
		return;
	}

	const qreal vol = TiledGame::getSfxVolume(m_object->scene(), m_object->body()->bodyPosition(), m_baseVolume, m_object->game()->baseScale()).value_or(0.);

	if (qFuzzyCompare(vol, m_lastVolume))
		return;

	if (vol <= 0.) {
		if (ma_sound_is_playing(snd)) {
			ma_sound_stop(snd);
			ma_sound_seek_to_pcm_frame(snd, 0);
		}
		m_lastVolume = 0.;
		return;
	}

	ma_sound_set_volume(snd, vol);

	if (!ma_sound_is_playing(snd))
		ma_sound_start(snd);

	m_lastVolume = vol;
}





/**
 * @brief TiledGameSfxLocation::baseVolume
 * @return
 */

float TiledGameSfxLocation::baseVolume() const
{
	return m_baseVolume;
}

void TiledGameSfxLocation::setBaseVolume(float newBaseVolume)
{
	if (qFuzzyCompare(m_baseVolume, newBaseVolume))
		return;
	m_baseVolume = newBaseVolume;
	emit baseVolumeChanged();
}



/**
 * @brief TiledGameSfxLocation::onSceneChanged
 */

void TiledGameSfxLocation::onSceneChanged()
{
	TiledScene *scene = m_object ? m_object->scene() : nullptr;

	if (scene == m_connectedScene)
		return;

	if (m_connectedScene) {
		disconnect(m_connectedScene, &TiledScene::worldStepped, this, &TiledGameSfxLocation::checkPosition);
		disconnect(m_connectedScene, &TiledScene::onScreenAreaChanged, this, &TiledGameSfxLocation::checkPosition);
	}

	m_connectedScene = scene;

	if (m_connectedScene) {
		connect(m_connectedScene, &TiledScene::worldStepped, this, &TiledGameSfxLocation::checkPosition);
		connect(m_connectedScene, &TiledScene::onScreenAreaChanged, this, &TiledGameSfxLocation::checkPosition);
	}

	updateSound();
}







/**
 * @brief TiledGameSfxLocation::checkPosition
 */

void TiledGameSfxLocation::checkPosition()
{
	if (!m_connectedScene || !m_object)
		return;

	const QPointF &p = m_object->body()->bodyPosition();

	if (p != m_lastPoint) {
		m_lastPoint = p;
		updateSound();
		return;
	}

	const QRectF &r = m_connectedScene->onScreenArea();

	if (r != m_lastVisibleArea) {
		m_lastVisibleArea = r;
		updateSound();
		return;
	}
}

