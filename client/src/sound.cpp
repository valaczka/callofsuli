/*S
 * ---- Call of Suli ----
 *
 * cosclientsound.cpp
 *
 * Created on: 2021. 01. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CosClientSound
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sound.h"
#include <QSettings>
#include <Logger.h>
#include <QFile>
#include <QTemporaryFile>
#include <QDir>

#if !defined(Q_OS_IOS) && !defined(Q_OS_MACOS)
#define MINIAUDIO_IMPLEMENTATION
#endif

#include "miniaudio.h"


/**
 * @brief Sound::Sound
 * @param parent
 */

Sound::Sound(QObject *parent)
	: QObject(parent)
{
	LOG_CTRACE("sound") << "Sound object created" << this;

	QSettings s;
	s.beginGroup(QStringLiteral("sound"));
	setVolumeMusic(s.value(QStringLiteral("volumeMusic"), 50).toInt());
	setVolumeSfx(s.value(QStringLiteral("volumeSfx"), 50).toInt());
	setVolumeVoiceOver(s.value(QStringLiteral("volumeVoiceOver"), 50).toInt());

	setMusicEnabled(s.value(QStringLiteral("musicEnabled"), true).toBool());
	setSfxEnabled(s.value(QStringLiteral("sfxEnabled"), true).toBool());
	setVoiceOverEnabled(s.value(QStringLiteral("voiceOverEnabled"), true).toBool());
	s.endGroup();

	engineCheck();

	m_garbageTimer.setInterval(500);
	connect(&m_garbageTimer, &QTimer::timeout, this, &Sound::garbage);

	LOG_CTRACE("sound") << "Sound object initialized" << this;

}



/**
 * @brief CosClientSound::~CosClientSound
 */

Sound::~Sound()
{
	LOG_CTRACE("sound") << "Destory sound object" << this;

	engineUninit();

	QSettings s;
	s.beginGroup(QStringLiteral("sound"));
	s.setValue(QStringLiteral("volumeMusic"), m_volumeMusic);
	s.setValue(QStringLiteral("volumeSfx"), m_volumeSfx);
	s.setValue(QStringLiteral("volumeVoiceOver"), m_volumeVoiceOver);
	s.setValue(QStringLiteral("musicEnabled"), m_musicEnabled);
	s.setValue(QStringLiteral("sfxEnabled"), m_sfxEnabled);
	s.setValue(QStringLiteral("voiceOverEnabled"), m_voiceOverEnabled);
	s.endGroup();

	LOG_CTRACE("sound") << "Sound object destroyed" << this;
}



/**
 * @brief Sound::playSound
 * @param source
 * @param soundType
 */

void Sound::playSound(const QString &source, const ChannelType &channel, const float &volume)
{
	LOG_CDEBUG("sound") << "Play sound" << qPrintable(source) << channel;

	QMutexLocker locker(&m_mutex);

	if (!m_engine ||
			(channel == SfxChannel && !m_sfxEnabled) ||
			(channel == MusicChannel && !m_musicEnabled) ||
			(channel == Music2Channel && !m_musicEnabled) ||
			(channel == VoiceoverChannel && !m_voiceOverEnabled)) {
		LOG_CTRACE("sound") << "Sound disabled" << channel;
		return;
	}


	auto it = m_sound.begin();

	for ( ; it != m_sound.end(); ++it) {
		if (it->get()->path() == source && it->get()->channel() == channel)
			break;
	}

	MaSound *sndObj = nullptr;

	if (it == m_sound.end()) {
		ma_sound_group *group = nullptr;
		switch (channel) {
			case SfxChannel: group = m_groupSfx.get(); break;
			case MusicChannel: group = m_groupMusic.get(); break;
			case Music2Channel: group = m_groupMusic.get(); break;
			case VoiceoverChannel: group = m_groupVoiceOver.get(); break;
		}

		/*auto ptr = std::make_unique<MaSound>(m_engine.get(), source, channel, group);
		sndObj = ptr.get();
		m_sound.push_back(std::move(ptr));*/

		const auto &ptr = m_sound.emplace_back(new MaSound(m_engine.get(), source, channel, group));
		sndObj = ptr.get();

		//updateVolumes();	// Bug?

		if (channel == VoiceoverChannel && sndObj && sndObj->sound()) {
			ma_sound_set_end_callback(sndObj->sound(), [](void* pUserData, ma_sound *) {
				Sound *obj = static_cast<Sound*>(pUserData);
				if (obj)
					obj->playNextVoiceOver();
			}, this);
		}
	} else {
		sndObj = it->get();
	}

	if (!sndObj) {
		LOG_CERROR("sound") << "Internal error";
		return;
	}

	if (!sndObj->sound()) {
		LOG_CWARNING("sound") << "Can't play sound" << qPrintable(source);
		return;
	}

	playSound(sndObj, volume);
}


/**
 * @brief Sound::stopSound
 * @param source
 * @param soundType
 */

void Sound::stopSound(const QString &source, const ChannelType &channel)
{
	for (auto &ptr : m_sound) {
		if (!ptr->sound() || !ma_sound_is_playing(ptr->sound()))
			continue;

		if (ptr->channel() != channel)
			continue;

		if (!source.isEmpty() && ptr->path() != source)
			continue;

		if (channel == MusicChannel || channel == Music2Channel) {
			ma_sound_stop_with_fade_in_milliseconds(ptr->sound(), 750);
		} else {
			ma_sound_stop(ptr->sound());
			ma_sound_seek_to_pcm_frame(ptr->sound(), 0);
		}
	}
}


/**
 * @brief Sound::stopMusic
 */

void Sound::stopMusic()
{
	stopSound(QStringLiteral(""), MusicChannel);
}


/**
 * @brief Sound::stopMusic2
 */

void Sound::stopMusic2()
{
	stopSound(QStringLiteral(""), Music2Channel);
}


/**
 * @brief Sound::isPlayingMusic
 * @return
 */

bool Sound::isPlayingMusic() const
{
	auto s = currentMusic();

	LOG_CTRACE("sound") << "Current music:" << s;

	return !s.isEmpty();
}


/**
 * @brief Sound::isPlayingMusic2
 * @return
 */

bool Sound::isPlayingMusic2() const
{
	auto s = currentMusic2();

	LOG_CTRACE("sound") << "Current music2:" << s;

	return !s.isEmpty();
}




/**
 * @brief Sound::volumeSfx
 * @return
 */

int Sound::volumeSfx() const
{
	return m_volumeSfx;
}

void Sound::setVolumeSfx(int newVolumeSfx)
{
	if (m_volumeSfx == newVolumeSfx)
		return;
	m_volumeSfx = newVolumeSfx;
	emit volumeSfxChanged();
	updateVolumes();
}

/**
 * @brief Sound::volumeVoiceOver
 * @return
 */


int Sound::volumeVoiceOver() const
{
	return m_volumeVoiceOver;
}

void Sound::setVolumeVoiceOver(int newVolumeVoiceOver)
{
	if (m_volumeVoiceOver == newVolumeVoiceOver)
		return;
	m_volumeVoiceOver = newVolumeVoiceOver;
	emit volumeVoiceOverChanged();
	updateVolumes();
}


/**
 * @brief Sound::volumeMusic
 * @return
 */

int Sound::volumeMusic() const
{
	return m_volumeMusic;
}

void Sound::setVolumeMusic(int newVolumeMusic)
{
	if (m_volumeMusic == newVolumeMusic)
		return;
	m_volumeMusic = newVolumeMusic;
	emit volumeMusicChanged();
	updateVolumes();
}


/**
 * @brief Sound::sfxEnabled
 * @return
 */

bool Sound::sfxEnabled() const
{
	return m_sfxEnabled;
}

void Sound::setSfxEnabled(bool newSfxEnabled)
{
	if (m_sfxEnabled == newSfxEnabled)
		return;
	m_sfxEnabled = newSfxEnabled;
	emit sfxEnabledChanged();
	engineCheck();
}

bool Sound::voiceOverEnabled() const
{
	return m_voiceOverEnabled;
}

void Sound::setVoiceOverEnabled(bool newVoiceOverEnabled)
{
	if (m_voiceOverEnabled == newVoiceOverEnabled)
		return;
	m_voiceOverEnabled = newVoiceOverEnabled;
	emit voiceOverEnabledChanged();
	engineCheck();
}

bool Sound::musicEnabled() const
{
	return m_musicEnabled;
}

void Sound::setMusicEnabled(bool newMusicEnabled)
{
	if (m_musicEnabled == newMusicEnabled)
		return;
	m_musicEnabled = newMusicEnabled;
	emit musicEnabledChanged();
	engineCheck();
}




/**
 * @brief Sound::engineInit
 * @return
 */

bool Sound::engineInit()
{
	LOG_CDEBUG("sound") << "Init audio engine";

	QMutexLocker locker(&m_mutex);

	m_engine = std::make_unique<ma_engine>();

	if (auto r = ma_engine_init(NULL, m_engine.get()); r != MA_SUCCESS) {
		LOG_CERROR("sound") << "Engine init failed:" << r;
		m_engine.reset();
		return false;
	}

	m_groupMusic = std::make_unique<ma_sound_group>();
	ma_sound_group_init(m_engine.get(), 0, NULL, m_groupMusic.get());

	m_groupSfx = std::make_unique<ma_sound_group>();
	ma_sound_group_init(m_engine.get(), 0, NULL, m_groupSfx.get());

	m_groupVoiceOver = std::make_unique<ma_sound_group>();
	ma_sound_group_init(m_engine.get(), 0, NULL, m_groupVoiceOver.get());

	updateVolumes();

	m_garbageTimer.start();

	return true;
}



/**
 * @brief Sound::engineUninit
 * @return
 */

bool Sound::engineUninit()
{
	LOG_CDEBUG("sound") << "Uninit audio engine";

	QMutexLocker locker(&m_mutex);

	LOG_CTRACE("sound") << "Clear playlist queue";

	m_queue.clear();

	LOG_CTRACE("sound") << "Stop garbage timer";

	m_garbageTimer.stop();

	LOG_CTRACE("sound") << "Delete sounds";

	for (auto &ptr : m_sound) {
		if (!ptr)
			continue;

		ptr.reset();
	}

	LOG_CTRACE("sound") << "Clear sounds";

	m_sound.clear();

	LOG_CTRACE("sound") << "Delete group music";

	if (m_groupMusic) {
		ma_sound_group_uninit(m_groupMusic.get());
		m_groupMusic.reset();
	}

	LOG_CTRACE("sound") << "Delete group sfx";

	if (m_groupSfx) {
		ma_sound_group_uninit(m_groupSfx.get());
		m_groupSfx.reset();
	}

	LOG_CTRACE("sound") << "Delete group voiceover";

	if (m_groupVoiceOver) {
		ma_sound_group_uninit(m_groupVoiceOver.get());
		m_groupVoiceOver.reset();
	}

	LOG_CTRACE("sound") << "Delete engine";


	if (m_engine) {
		ma_engine_uninit(m_engine.get());
		m_engine.reset();
	}

	LOG_CTRACE("sound") << "Engine uninit finished";


	return true;
}



/**
 * @brief Sound::engineCheck
 */

void Sound::engineCheck()
{
	QMutexLocker locker(&m_mutex);

	bool req = m_musicEnabled || m_sfxEnabled || m_voiceOverEnabled;

	LOG_CTRACE("sound") << "Check audio engine: required ="	<< req << "exists:" << m_engine.get();

	if (req && !m_engine)
		engineInit();
	else if (!req && m_engine)
		engineUninit();
}



/**
 * @brief Sound::playSound
 * @param sound
 */

void Sound::playSound(MaSound *sound, const float &volume)
{
	if (!sound || !sound->sound()) {
		LOG_CERROR("sound") << "Invalid MaSound";
		return;
	}

	if (sound->channel() == MusicChannel) {
		bool isActive = false;

		for (const auto &ptr : m_sound) {
			if (ptr->channel() == MusicChannel && ptr->sound() && ma_sound_is_playing(ptr->sound())) {
				if (ptr->path() == sound->path()) {
					isActive = true;
				} else {
					LOG_CTRACE("sound") << "Stop currently playing music:" << qPrintable(ptr->path());
					ma_sound_stop(ptr->sound());
				}
			}
		}

		if (!isActive) {
			LOG_CTRACE("sound") << "Start playing music:" << qPrintable(sound->path());

			// Bug: https://github.com/mackron/miniaudio/issues/714
			ma_sound_set_stop_time_in_milliseconds(sound->sound(), ~(ma_uint64)0);
			ma_sound_set_fade_in_milliseconds(sound->sound(), 1.0, 1.0, 0);
			ma_sound_seek_to_pcm_frame(sound->sound(), 0);
			ma_sound_start(sound->sound());
		}

	} else if (sound->channel() == Music2Channel) {
		bool isActive = false;

		for (const auto &ptr : m_sound) {
			if (ptr->channel() == Music2Channel && ptr->sound() && ma_sound_is_playing(ptr->sound())) {
				if (ptr->path() == sound->path()) {
					isActive = true;
				} else {
					LOG_CTRACE("sound") << "Stop currently playing music2:" << qPrintable(ptr->path());
					ma_sound_stop(ptr->sound());
				}
			}
		}

		if (!isActive) {
			LOG_CTRACE("sound") << "Start playing music2:" << qPrintable(sound->path());

			// Bug: https://github.com/mackron/miniaudio/issues/714
			ma_sound_set_stop_time_in_milliseconds(sound->sound(), ~(ma_uint64)0);
			ma_sound_set_fade_in_milliseconds(sound->sound(), 1.0, 1.0, 0);
			ma_sound_seek_to_pcm_frame(sound->sound(), 0);
			ma_sound_start(sound->sound());
		}

	} else if (sound->channel() == SfxChannel) {
		if (ma_sound_is_playing(sound->sound())) {
			LOG_CTRACE("sound") << "Duplicate playing sfx:" << qPrintable(sound->path()) << "volume:" << volume;

			auto snd = sound->duplicate(m_groupSfx.get());

			if (snd) {
				ma_sound_set_volume(snd, volume);
				ma_sound_start(snd);
			}
		} else {
			LOG_CTRACE("sound") << "Start playing sfx:" << qPrintable(sound->path()) << "volume:" << volume;
			ma_sound_set_volume(sound->sound(), volume);
			ma_sound_start(sound->sound());
		}
	} else if (sound->channel() == VoiceoverChannel) {
		QMutexLocker locker(&m_mutex);
		m_queue.append(sound);

		for (const auto &ptr : m_sound) {
			if (ptr->channel() == VoiceoverChannel && ptr->sound() && ma_sound_is_playing(ptr->sound())) {
				return;
			}
		}
		playNextVoiceOver();
	}
}



/**
 * @brief Sound::currentMusic
 * @return
 */

QVector<Sound::MaSound *> Sound::currentMusic() const
{
	QVector<Sound::MaSound *> list;

	for (const auto &ptr : m_sound) {
		if (ptr->channel() == MusicChannel && ptr->sound() && ma_sound_is_playing(ptr->sound())) {
			list.append(ptr.get());
		}
	}

	return list;
}



/**
 * @brief Sound::currentMusic2
 * @return
 */

QVector<Sound::MaSound *> Sound::currentMusic2() const
{
	QVector<Sound::MaSound *> list;

	for (const auto &ptr : m_sound) {
		if (ptr->channel() == Music2Channel && ptr->sound() && ma_sound_is_playing(ptr->sound())) {
			list.append(ptr.get());
		}
	}

	return list;
}





/**
 * @brief Sound::updateVolumes
 */

void Sound::updateVolumes()
{
	if (m_groupMusic)
		ma_sound_group_set_volume(m_groupMusic.get(), qMax(0.0, qMin((float) m_volumeMusic / 100.0, 1.0)));

	if (m_groupSfx)
		ma_sound_group_set_volume(m_groupSfx.get(), qMax(0.0, qMin((float) m_volumeSfx / 100.0, 1.0)));

	if (m_groupVoiceOver)
		ma_sound_group_set_volume(m_groupVoiceOver.get(), qMax(0.0, qMin((float) m_volumeVoiceOver / 100.0, 1.0)));
}




/**
 * @brief Sound::garbage
 */

void Sound::garbage()
{
	QMutexLocker locker(&m_mutex);

	for (const auto &ptr : m_sound) {
		if (ptr)
			ptr->garbage();
	}
}



/**
 * @brief Sound::playNextVoiceOver
 */

void Sound::playNextVoiceOver()
{
	QMutexLocker locker(&m_mutex);

	if (m_queue.isEmpty())
		return;

	MaSound *sound = m_queue.dequeue();

	if (!sound)
		return;

	LOG_CTRACE("sound") << "Start playing voiceover:" << qPrintable(sound->path());
	ma_sound_start(sound->sound());

}



/**
 * @brief Sound::MaSound::MaSound
 * @param path
 * @param type
 */

Sound::MaSound::MaSound(ma_engine *engine, const QString &path, const ChannelType &channel, ma_sound_group *group)
	: m_engine(engine)
	, m_path(path)
	, m_channel(channel)
{
	LOG_CTRACE("sound") << "MaSound created" << qPrintable(path) << channel << this;

	QString s = path;
	if (s.startsWith(QStringLiteral("qrc:/")))
		s.replace(QStringLiteral("qrc:/"), QStringLiteral(":/"));

	QTemporaryFile *tmp = QTemporaryFile::createNativeFile(s);

	if (tmp) {
		tmp->setAutoRemove(false);
		m_tmpPath = QDir::toNativeSeparators(tmp->fileName()).toUtf8();
		delete tmp;
	} else
		m_tmpPath = QDir::toNativeSeparators(s).toUtf8();

	LOG_CTRACE("sound") << "MaSound ready:" << qPrintable(path) << "->" << m_tmpPath.constData();

	m_sound = std::make_unique<ma_sound>();

	if (auto r = ma_sound_init_from_file(m_engine, m_tmpPath, MA_SOUND_FLAG_DECODE, group, NULL, m_sound.get());
			r != MA_SUCCESS) {
		LOG_CERROR("sound") << "Sound error" << qPrintable(path) << "code" << r;
		m_sound.reset();
		return;
	}


	if (channel == MusicChannel || channel == Music2Channel) {
		ma_sound_set_looping(m_sound.get(), true);
	}
}


/**
 * @brief Sound::MaSound::~MaSound
 */

Sound::MaSound::~MaSound()
{
	LOG_CTRACE("sound") << "Destroying MaSound" << qPrintable(m_path) << m_channel << this;

	uninit();

	LOG_CTRACE("sound") << "MaSound destroyed" << this;
}


/**
 * @brief Sound::MaSound::uninit
 */

void Sound::MaSound::uninit()
{
	QMutexLocker locker(&m_mutex);

	LOG_CTRACE("sound") << "Uninit MaSound" << this;

	uninitChildren();

	if (m_sound) {
		ma_sound_stop(m_sound.get());
		ma_sound_uninit(m_sound.get());
		m_sound.reset();
	}

	if (!m_tmpPath.isEmpty()) {
		QFile::remove(m_tmpPath);
	}

	LOG_CTRACE("sound") << "Uninit MaSound finished" << this;
}


/**
 * @brief Sound::MaSound::uninitChildren
 */

void Sound::MaSound::uninitChildren()
{
	QMutexLocker locker(&m_mutex);

	for (auto &ptr : m_children) {
		if (!ptr)
			continue;
		ma_sound_stop(ptr.get());
		ma_sound_uninit(ptr.get());
		ptr.reset();
	}

	m_children.clear();
}



/**
 * @brief Sound::MaSound::duplicate
 * @return
 */

ma_sound *Sound::MaSound::duplicate(ma_sound_group *group)
{
	if (!m_sound)
		return nullptr;

	QMutexLocker locker(&m_mutex);

	LOG_CTRACE("sound") << "Add children" << this;

	if (m_children.size() > 5) {
		LOG_CDEBUG("sound") << "Children overload" << this;
		return nullptr;
	}

	auto child = std::make_unique<ma_sound>();

	if (auto r = ma_sound_init_copy(m_engine, m_sound.get(), 0, group, child.get()); r != MA_SUCCESS) {
		LOG_CERROR("sound") << "Duplicate error" << r;
		child.reset();
		return nullptr;
	}

	ma_sound *ptr = child.get();

	ma_sound_set_end_callback(ptr, [](void* pUserData, ma_sound *snd) {
		MaSound *obj = static_cast<MaSound*>(pUserData);

		if (obj) {
			QMutexLocker locker(&obj->m_mutex);
			obj->m_garbage.push_back(snd);
		}
	}, this);


	m_children.push_back(std::move(child));

	return ptr;
}



/**
 * @brief Sound::MaSound::garbage
 */

void Sound::MaSound::garbage()
{
	QMutexLocker locker(&m_mutex);

	for (const auto &ptr : m_garbage) {
		if (ptr)
			removeChild(ptr);
	}

	m_garbage.clear();
}



/**
 * @brief Sound::MaSound::removeChild
 * @param ptr
 */

void Sound::MaSound::removeChild(ma_sound *sound)
{
	LOG_CTRACE("sound") << "Remove duplicated child" << sound << "from" << this << m_children.size();

	QMutexLocker locker(&m_mutex);

	for (auto it = m_children.begin(); it != m_children.end(); ) {
		if (it->get() == sound) {
			ma_sound_uninit(it->get());
			it->reset();
			it = m_children.erase(it);
		} else {
			++it;
		}
	}
}

