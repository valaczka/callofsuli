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
	, m_soundTypeSfx(PlayerSfx)
	, m_musicNextSource()
{
	LOG_CTRACE("sound") << "Sound object created" << this;

	/*m_fadeAnimation->setDuration(750);
	m_fadeAnimation->setEndValue(0);

	connect(m_fadeAnimation, &QVariantAnimation::finished, this, &Sound::musicLoadNextSource);
	connect(m_fadeAnimation, &QVariantAnimation::stateChanged, this, [](QAbstractAnimation::State newState, QAbstractAnimation::State oldState
			){ LOG_CTRACE("sound") << "###### STATE" << newState << oldState; });*/
}



/**
 * @brief CosClientSound::~CosClientSound
 */

Sound::~Sound()
{
	LOG_CTRACE("sound") << "Sound object destroyed" << this;

	QSettings s;
	s.beginGroup(QStringLiteral("sound"));
	s.setValue(QStringLiteral("volumeMusic"), volume(MusicChannel));
	s.setValue(QStringLiteral("volumeSfx"), volume(SfxChannel));
	s.setValue(QStringLiteral("volumeVoiceOver"), volume(VoiceoverChannel));
	s.endGroup();
}







/**
 * @brief CosSound::init
 */

void Sound::init()
{
	LOG_CTRACE("sound") << "Sound object init";

	m_mediaPlayerMusic = std::make_unique<QMediaPlayer>();
	m_mediaPlayerSfx = std::make_unique<QMediaPlayer>();
	m_mediaPlayerVoiceOver = std::make_unique<QMediaPlayer>();

#if QT_VERSION >= 0x060000
	m_audioOutputMusic = std::make_unique<QAudioOutput>();
	m_mediaPlayerMusic->setAudioOutput(m_audioOutputMusic.get());

	m_audioOutputSfx = std::make_unique<QAudioOutput>();
	m_mediaPlayerSfx->setAudioOutput(m_audioOutputSfx.get());

	m_audioOutputVoiceOver = std::make_unique<QAudioOutput>();
	m_mediaPlayerVoiceOver->setAudioOutput(m_audioOutputVoiceOver.get());
#endif


	m_soundTypeSfx = PlayerSfx;

#if QT_VERSION < 0x060000
	connect(m_mediaPlayerVoiceOver.get(), &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
		if (state == QMediaPlayer::StoppedState && !m_playlist.isEmpty()) {
			m_mediaPlayerVoiceOver->setMedia(QUrl(m_playlist.dequeue()));
			m_mediaPlayerVoiceOver->play();
		}
	});
#else
	connect(m_mediaPlayerVoiceOver.get(), &QMediaPlayer::playbackStateChanged, this, [this](const QMediaPlayer::PlaybackState &state) {
		if (state == QMediaPlayer::StoppedState && !m_playlist.isEmpty()) {
			m_mediaPlayerVoiceOver->setSource(QUrl());
			m_mediaPlayerVoiceOver->setSource(QUrl(m_playlist.dequeue()));
			m_mediaPlayerVoiceOver->play();
		}
	});
#endif


	QSettings s;
	s.beginGroup(QStringLiteral("sound"));
	setVolume(Sound::MusicChannel, s.value(QStringLiteral("volumeMusic"), 50).toInt());
	setVolume(Sound::SfxChannel, s.value(QStringLiteral("volumeSfx"), 50).toInt());
	setVolume(Sound::VoiceoverChannel, s.value(QStringLiteral("volumeVoiceOver"), 50).toInt());
	s.endGroup();

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

	if (soundType == Music) {
		musicPlay(source);
	} else if (soundType == VoiceOver) {
#if QT_VERSION < 0x060000
		if (!m_mediaPlayerVoiceOver->volume())
			return;
#else
		if (m_audioOutputVoiceOver->isMuted())
			return;
#endif

		m_playlist.append(source);

#if QT_VERSION < 0x060000
		if (m_mediaPlayerVoiceOver->state() != QMediaPlayer::PlayingState) {
			m_mediaPlayerVoiceOver->setMedia(QUrl(m_playlist.dequeue()));
			m_mediaPlayerVoiceOver->play();
		}
#else
		if (!m_mediaPlayerVoiceOver->isPlaying()) {
			m_mediaPlayerVoiceOver->setSource(QUrl());
			m_mediaPlayerVoiceOver->setSource(QUrl(m_playlist.dequeue()));
			m_mediaPlayerVoiceOver->play();
		}
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
#else
		if (m_audioOutputSfx->isMuted())
			return;

		if (m_mediaPlayerSfx->isPlaying()) {
			if (soundType == PlayerVoice && m_soundTypeSfx == GameSound)
				return;

			if (soundType == PlayerSfx && m_soundTypeSfx == GameSound)
				return;

			m_mediaPlayerSfx->stop();
		}

		m_mediaPlayerSfx->setSource(QUrl());

		m_soundTypeSfx = soundType;

		m_mediaPlayerSfx->setSource(QUrl(source));
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

	if (soundType != Music)
		return;

	m_musicNextSource = QStringLiteral("");
	musicLoadNextSource();

#if QT_VERSION < 0x060000
	/*if (m_mediaPlayerMusic->state() == QMediaPlayer::PlayingState && m_mediaPlayerMusic->currentMedia() == QUrl(source)
			&& m_fadeAnimation->state() != QAbstractAnimation::Running)
	{
		m_fadeAnimation->setStartValue(m_mediaPlayerMusic->volume());
		m_fadeAnimation->start();
	}*/
#else
	/*LOG_CTRACE("sound") << "----" << m_mediaPlayerMusic->isPlaying() << m_mediaPlayerMusic->source() << (m_mediaPlayerMusic->source() == QUrl(source))
						<< m_fadeAnimation->state();

	if (m_mediaPlayerMusic->isPlaying() && m_mediaPlayerMusic->source() == QUrl(source)
			&& m_fadeAnimation->state() != QAbstractAnimation::Running)
	{
		m_fadeAnimation->setStartValue(m_audioOutputMusic->volume());
		LOG_CTRACE("sound") << "++++++" << m_fadeAnimation->startValue();
		m_fadeAnimation->start();
	}*/
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
#else
	switch (channel) {
	case MusicChannel:
		return m_audioOutputMusic->volume()*100;
		break;
	case SfxChannel:
		return m_audioOutputSfx->volume()*100;
		break;
	case VoiceoverChannel:
		return m_audioOutputVoiceOver->volume()*100;
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
		emit volumeMusicChanged(newVolume);
		break;
	case SfxChannel:
		m_mediaPlayerSfx->setVolume(newVolume);
		emit volumeSfxChanged(newVolume);
		break;
	case VoiceoverChannel:
		m_mediaPlayerVoiceOver->setVolume(newVolume);
		emit volumeMusicChanged(newVolume);
		break;
	}
#else
	switch (channel) {
	case MusicChannel:
		m_audioOutputMusic->setVolume((qreal)newVolume/100.0);
		emit volumeMusicChanged(newVolume);
		break;
	case SfxChannel:
		m_audioOutputSfx->setVolume((qreal)newVolume/100.0);
		emit volumeSfxChanged(newVolume);
		break;
	case VoiceoverChannel:
		m_audioOutputVoiceOver->setVolume((qreal)newVolume/100.0);
		emit volumeVoiceOverChanged(newVolume);
		break;
	}
#endif
}



/**
 * @brief Sound::muted
 * @param channel
 * @return
 */

bool Sound::isMuted(const ChannelType &channel) const
{
#if QT_VERSION < 0x060000
	switch (channel) {
	case MusicChannel:
		return m_mediaPlayerMusic->volume() == 0;
		break;
	case SfxChannel:
		return m_mediaPlayerSfx->volume() == 0;
		break;
	case VoiceoverChannel:
		return m_mediaPlayerVoiceOver->volume() == 0;
		break;
	}
#else
	switch (channel) {
	case MusicChannel:
		return m_audioOutputMusic->isMuted();
		break;
	case SfxChannel:
		return m_audioOutputSfx->isMuted();
		break;
	case VoiceoverChannel:
		return m_audioOutputVoiceOver->isMuted();
		break;
	}
#endif

	return false;
}


/**
 * @brief Sound::setMuted
 * @param channel
 * @param newMuted
 */

void Sound::setMuted(const ChannelType &channel, bool newMuted)
{
#if QT_VERSION >= 0x060000
	switch (channel) {
	case MusicChannel:
		m_audioOutputMusic->setMuted(newMuted);
		break;
	case SfxChannel:
		m_audioOutputSfx->setMuted(newMuted);
		break;
	case VoiceoverChannel:
		m_audioOutputVoiceOver->setMuted(newMuted);
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
	return (m_mediaPlayerMusic->state() == QMediaPlayer::PlayingState);
#else
	return m_mediaPlayerMusic->isPlaying();
#endif
}



/**
 * @brief CosSound::musicPlay
 * @param source
 */

void Sound::musicPlay(const QString &source)
{
	LOG_CTRACE("sound") << "Play music" << source;

	m_musicNextSource = source;
/*
#if QT_VERSION < 0x060000
	if (m_mediaPlayerMusic->state() == QMediaPlayer::PlayingState) {
#else
	if (m_mediaPlayerMusic->isPlaying()) {
#endif
		LOG_CTRACE("sound") << "+++playing";
		if (m_fadeAnimation->state() != QAbstractAnimation::Running) {
#if QT_VERSION < 0x060000
			m_fadeAnimation->setStartValue(m_mediaPlayerMusic->volume());
#else
			m_fadeAnimation->setStartValue(m_audioOutputMusic->volume());
#endif
			LOG_CTRACE("sound") << "FADE FROM" << m_fadeAnimation->startValue();
			m_fadeAnimation->start();
		}
	} else {*/
		musicLoadNextSource();
	/*}*/
}



/**
 * @brief CosSound::musicLoadNextSource
 */

void Sound::musicLoadNextSource()
{
	LOG_CTRACE("sound") << "Music load next source" << m_musicNextSource;

	if (m_musicNextSource.isEmpty()) {
		m_mediaPlayerMusic->stop();
		return;
	}
#if QT_VERSION < 0x060000
	//m_mediaPlayerMusic->setVolume(m_musicVolume);

	QMediaPlaylist *playlist = m_mediaPlayerMusic->playlist();

	if (!playlist) {
		playlist = new QMediaPlaylist(m_mediaPlayerMusic.get());
		playlist->setPlaybackMode(QMediaPlaylist::Loop);
		m_mediaPlayerMusic->setPlaylist(playlist);
	}

	playlist->clear();
	playlist->addMedia(QUrl(m_musicNextSource));


#else
	//m_audioOutputMusic->setVolume((qreal)m_musicVolume/100.0);

	m_mediaPlayerMusic->setSource(QUrl(m_musicNextSource));
	m_mediaPlayerMusic->setLoops(QMediaPlayer::Infinite);
#endif

	m_musicNextSource = QStringLiteral("");
	m_mediaPlayerMusic->play();
}

