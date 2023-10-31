/*
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
#if QT_VERSION < 0x060000
#include <QMediaPlaylist>
#else
#include <QAudioOutput>
#endif
#include <Logger.h>

Sound::Sound(QObject *parent)
	: QObject(parent)
	, m_mediaPlayerMusic (nullptr)
	, m_mediaPlayerSfx (nullptr)
	, m_mediaPlayerVoiceOver (nullptr)
	, m_soundTypeSfx(PlayerSfx)
	, m_musicNextSource()
	, m_fadeAnimation(new QVariantAnimation(this))
	, m_musicVolume(0)
{
	LOG_CTRACE("sound") << "Sound object created" << this;

	m_fadeAnimation->setDuration(750);
	m_fadeAnimation->setEndValue(0);

	connect(m_fadeAnimation, &QVariantAnimation::finished, this, &Sound::musicLoadNextSource);
}



/**
 * @brief CosClientSound::~CosClientSound
 */

Sound::~Sound()
{
	delete m_fadeAnimation;
	m_fadeAnimation = nullptr;

	if (m_mediaPlayerSfx)
		delete m_mediaPlayerSfx;

	m_mediaPlayerSfx = nullptr;

	if (m_mediaPlayerMusic)
		delete m_mediaPlayerMusic;

	m_mediaPlayerMusic = nullptr;

	if (m_mediaPlayerVoiceOver)
		delete m_mediaPlayerVoiceOver;

	m_mediaPlayerVoiceOver = nullptr;

	LOG_CTRACE("sound") << "Sound object destroyed" << this;
}







/**
 * @brief CosSound::init
 */

void Sound::init()
{
	LOG_CTRACE("sound") << "Sound object init";

	m_mediaPlayerMusic = new QMediaPlayer(this);
	m_mediaPlayerSfx = new QMediaPlayer(this);
	m_mediaPlayerVoiceOver = new QMediaPlayer(this);

#if QT_VERSION >= 0x060000
	m_audioOutput = new QAudioOutput(this);
	m_mediaPlayerMusic->setAudioOutput(m_audioOutput);
#endif


	m_soundTypeSfx = PlayerSfx;

#if QT_VERSION < 0x060000
	connect(m_mediaPlayerVoiceOver, &QMediaPlayer::stateChanged, this, [=](QMediaPlayer::State state) {
		if (state == QMediaPlayer::StoppedState && m_mediaPlayerVoiceOver->playlist()) {
			m_mediaPlayerVoiceOver->playlist()->clear();
		}
	});
#endif

	connect(m_fadeAnimation, &QVariantAnimation::valueChanged, m_mediaPlayerMusic, [=](const QVariant &value) {
#if QT_VERSION < 0x060000
		m_mediaPlayerMusic->setVolume(value.toInt());
#else
		m_audioOutput->setVolume(value.toDouble());
#endif
	});
	connect(m_fadeAnimation, &QVariantAnimation::finished, this, [=]() {
#if QT_VERSION < 0x060000
		m_mediaPlayerMusic->setVolume(m_musicVolume);
#else
		m_audioOutput->setVolume(m_musicVolume);
#endif
	});


	LOG_CTRACE("sound") << "Sound object initialized";
}



/**
 * @brief CosClientSound::playSound
 * @param source
 * @param soundType
 */

void Sound::playSound(const QString &source, const SoundType &soundType)
{
	LOG_CTRACE("sound") << "Play sound" << source << soundType;

	if (!m_mediaPlayerMusic || !m_mediaPlayerSfx || !m_mediaPlayerVoiceOver)
		return;

	if (soundType == Music) {
		musicPlay(source);
	} else if (soundType == VoiceOver) {
#if QT_VERSION < 0x060000
		if (!m_mediaPlayerVoiceOver->volume())
			return;

		QMediaPlaylist *playlist = m_mediaPlayerVoiceOver->playlist();

		if (!playlist) {
			playlist = new QMediaPlaylist(m_mediaPlayerMusic);
			playlist->setPlaybackMode(QMediaPlaylist::Sequential);
			m_mediaPlayerVoiceOver->setPlaylist(playlist);
		}
		playlist->addMedia(QUrl(source));

		if (m_mediaPlayerVoiceOver->state() != QMediaPlayer::PlayingState)
			m_mediaPlayerVoiceOver->play();
#endif
	} else {
#if QT_VERSION < 0x060000
		if (!m_mediaPlayerSfx->volume())
			return;

		if (m_mediaPlayerSfx->state() == QMediaPlayer::PlayingState) {
			if (soundType == PlayerVoice && m_soundTypeSfx == GameSound)
				return;

			if (soundType == PlayerSfx && m_soundTypeSfx == GameSound)
				return;

			m_mediaPlayerSfx->stop();
		}

		m_soundTypeSfx = soundType;

		m_mediaPlayerSfx->setMedia(QUrl(source));
		m_mediaPlayerSfx->play();
#endif
	}
}


/**
 * @brief CosClientSound::stopSound
 * @param source
 * @param soundType
 */

void Sound::stopSound(const QString &source, const SoundType &soundType)
{
	LOG_CTRACE("sound") << "Stop sound" << source << soundType;

	if (!m_mediaPlayerMusic)
		return;

	if (soundType != Music)
		return;
#if QT_VERSION < 0x060000
	if (m_mediaPlayerMusic->state() == QMediaPlayer::PlayingState && m_mediaPlayerMusic->currentMedia() == QUrl(source)
			&& m_fadeAnimation->state() != QAbstractAnimation::Running)
	{
		m_fadeAnimation->setStartValue(m_mediaPlayerMusic->volume());
		m_fadeAnimation->start();
	}
#endif
}


/**
 * @brief Sound::volume
 * @param channel
 * @return
 */

int Sound::volume(const ChannelType &channel) const
{
#if QT_VERSION < 0x060000
	switch (channel) {
	case MusicChannel:
		return m_mediaPlayerMusic->volume();
		break;
	case SfxChannel:
		return m_mediaPlayerSfx->volume();
		break;
	case VoiceoverChannel:
		return m_mediaPlayerVoiceOver->volume();
		break;
	}
#endif
	return 0;
}


/**
 * @brief Sound::setVolume
 * @param channel
 * @param newVolume
 */

void Sound::setVolume(const ChannelType &channel, int newVolume)
{
#if QT_VERSION < 0x060000
	switch (channel) {
	case MusicChannel:
		m_mediaPlayerMusic->setVolume(newVolume);
		m_musicVolume = newVolume;
		break;
	case SfxChannel:
		m_mediaPlayerSfx->setVolume(newVolume);
		break;
	case VoiceoverChannel:
		m_mediaPlayerVoiceOver->setVolume(newVolume);
		break;
	}
#endif
}


/**
 * @brief Sound::isPlayingMusic
 * @return
 */

bool Sound::isPlayingMusic() const
{
#if QT_VERSION < 0x060000
	return (m_mediaPlayerMusic && m_mediaPlayerMusic->state() == QMediaPlayer::PlayingState);
#else
	return false;
#endif
}



/**
 * @brief CosSound::musicPlay
 * @param source
 */

void Sound::musicPlay(const QString &source)
{

	LOG_CTRACE("sound") << "Play music" << source;

	if (!m_mediaPlayerMusic)
		return;

	m_musicNextSource = source;
#if QT_VERSION < 0x060000
	if (m_mediaPlayerMusic->state() == QMediaPlayer::PlayingState) {
		if (m_fadeAnimation->state() != QAbstractAnimation::Running) {
			m_fadeAnimation->setStartValue(m_mediaPlayerMusic->volume());
			m_fadeAnimation->start();
		}
	} else {
		musicLoadNextSource();
	}
#endif
}



/**
 * @brief CosSound::musicLoadNextSource
 */

void Sound::musicLoadNextSource()
{
	LOG_CTRACE("sound") << "Music load next source" << m_musicNextSource;

	if (!m_mediaPlayerMusic)
		return;

	if (m_musicNextSource.isEmpty()) {
		m_mediaPlayerMusic->stop();
		return;
	}
#if QT_VERSION < 0x060000
	m_mediaPlayerMusic->setVolume(m_musicVolume);

	QMediaPlaylist *playlist = m_mediaPlayerMusic->playlist();

	if (!playlist) {
		playlist = new QMediaPlaylist(m_mediaPlayerMusic);
		playlist->setPlaybackMode(QMediaPlaylist::Loop);
		m_mediaPlayerMusic->setPlaylist(playlist);
	}

	playlist->clear();
	playlist->addMedia(QUrl(m_musicNextSource));

	m_musicNextSource = QLatin1String("");

	m_mediaPlayerMusic->play();
#endif
}

