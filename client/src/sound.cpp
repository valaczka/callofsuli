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
#include <QMediaPlaylist>

Q_LOGGING_CATEGORY(lcSound, "app.sound")

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
	qCDebug(lcSound).noquote() << tr("Sound object created") << this;

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

	if (m_mediaPlayerMusic && m_mediaPlayerSfx && m_mediaPlayerVoiceOver) {
		QSettings s(this);
		s.beginGroup("sound");
		s.setValue("volumeMusic", m_musicVolume);
		s.setValue("volumeSfx", volumeSfx());
		s.setValue("volumeVoiceOver", volumeVoiceOver());
		s.endGroup();
	}

	if (m_mediaPlayerSfx)
		delete m_mediaPlayerSfx;

	if (m_mediaPlayerMusic)
		delete m_mediaPlayerMusic;

	if (m_mediaPlayerVoiceOver)
		delete m_mediaPlayerVoiceOver;

	qCDebug(lcSound).noquote() << tr("Sound object destroyed") << this;
}




/**
 * @brief CosSound::init
 */

void Sound::init()
{
	qCDebug(lcSound).noquote() << tr("Sound object init");

	m_mediaPlayerMusic = new QMediaPlayer(this);
	m_mediaPlayerSfx = new QMediaPlayer(this);
	m_mediaPlayerVoiceOver = new QMediaPlayer(this);

	m_soundTypeSfx = PlayerSfx;

	/*connect (m_mediaPlayerMusic, &QMediaPlayer::volumeChanged, this, &CosSound::volumeMusicChanged);
	connect (m_mediaPlayerSfx, &QMediaPlayer::volumeChanged, this, &CosSound::volumeSfxChanged);
	connect (m_mediaPlayerVoiceOver, &QMediaPlayer::volumeChanged, this, &CosSound::volumeVoiceOverChanged);*/

	connect(m_mediaPlayerVoiceOver, &QMediaPlayer::stateChanged, this, [=](QMediaPlayer::State state) {
		if (state == QMediaPlayer::StoppedState && m_mediaPlayerVoiceOver->playlist()) {
			m_mediaPlayerVoiceOver->playlist()->clear();
		}
	});

	connect(m_fadeAnimation, &QVariantAnimation::valueChanged, m_mediaPlayerMusic, [=](const QVariant &value) {
		m_mediaPlayerMusic->setVolume(value.toInt());
	});
	connect(m_fadeAnimation, &QVariantAnimation::finished, this, [=]() {
		m_mediaPlayerMusic->setVolume(m_musicVolume);
	});

	QSettings s(this);
	s.beginGroup("sound");
	setVolumeMusic(s.value("volumeMusic", 50).toInt());
	setVolumeSfx(s.value("volumeSfx", 50).toInt());
	setVolumeVoiceOver(s.value("volumeVoiceOver", 50).toInt());
	s.endGroup();


	/*connect(m_mediaPlayerMusic, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
			[=](QMediaPlayer::Error error) {
		qWarning() << "Media error" << m_mediaPlayerMusic << error;
		sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Médialejátszó"), tr("Médialejátszási hiba %1").arg(error));
	});

	connect(m_mediaPlayerSfx, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
			[=](QMediaPlayer::Error error) {
		qWarning() << "Media error" << m_mediaPlayerSfx << error;
		sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Médialejátszó"), tr("Médialejátszási hiba %1").arg(error));
	});

	connect(m_mediaPlayerVoiceOver, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
			[=](QMediaPlayer::Error error) {
		qWarning() << "Media error" << m_mediaPlayerVoiceOver << error;
		sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Médialejátszó"), tr("Médialejátszási hiba %1").arg(error));
	});*/

}



/**
 * @brief CosClientSound::playSound
 * @param source
 * @param soundType
 */

void Sound::playSound(const QString &source, const SoundType &soundType)
{
	qCDebug(lcSound).noquote() << tr("Play sound") << source << soundType;

	if (!m_mediaPlayerMusic || !m_mediaPlayerSfx || !m_mediaPlayerVoiceOver)
		return;

	if (soundType == Music) {
		musicPlay(source);
	} else if (soundType == VoiceOver) {
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
	} else {
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
	}
}


/**
 * @brief CosClientSound::stopSound
 * @param source
 * @param soundType
 */

void Sound::stopSound(const QString &source, const SoundType &soundType)
{
	qCDebug(lcSound).noquote() << tr("Stop sound") << source << soundType;

	if (!m_mediaPlayerMusic)
		return;

	if (soundType != Music)
		return;

	if (m_mediaPlayerMusic->state() == QMediaPlayer::PlayingState && m_mediaPlayerMusic->currentMedia() == QUrl(source)
		&& m_fadeAnimation->state() != QAbstractAnimation::Running)
	{
		m_fadeAnimation->setStartValue(m_mediaPlayerMusic->volume());
		m_fadeAnimation->start();
	}
}


/**
 * @brief CosSound::setVolumeSfx
 * @param volume
 */

void Sound::setVolumeSfx(int volume)
{
	if (m_mediaPlayerSfx) m_mediaPlayerSfx->setVolume(volume);
	emit volumeSfxChanged(volume);
}

/**
 * @brief CosSound::setVolumeMusic
 * @param volume
 */

void Sound::setVolumeMusic(int volume)
{
	if (m_mediaPlayerMusic) m_mediaPlayerMusic->setVolume(volume);
	emit volumeMusicChanged(volume);
	m_musicVolume = volume;
}



/**
 * @brief CosSound::setVolumeVoiceOver
 * @param volume
 */

void Sound::setVolumeVoiceOver(int volume)
{
	if (m_mediaPlayerVoiceOver) m_mediaPlayerVoiceOver->setVolume(volume);
	emit volumeVoiceOverChanged(volume);
}


/**
 * @brief CosSound::musicPlay
 * @param source
 */

void Sound::musicPlay(const QString &source)
{
	qCDebug(lcSound).noquote() << tr("Play music") << source;

	if (!m_mediaPlayerMusic)
		return;

	m_musicNextSource = source;

	if (m_mediaPlayerMusic->state() == QMediaPlayer::PlayingState) {
		if (m_fadeAnimation->state() != QAbstractAnimation::Running) {
			m_fadeAnimation->setStartValue(m_mediaPlayerMusic->volume());
			m_fadeAnimation->start();
		}
	} else {
		musicLoadNextSource();
	}
}



/**
 * @brief CosSound::musicLoadNextSource
 */

void Sound::musicLoadNextSource()
{
	qCDebug(lcSound).noquote() << tr("Music load next source") << m_musicNextSource;

	if (!m_mediaPlayerMusic)
		return;

	if (m_musicNextSource.isEmpty()) {
		m_mediaPlayerMusic->stop();
		return;
	}

	m_mediaPlayerMusic->setVolume(m_musicVolume);

	QMediaPlaylist *playlist = m_mediaPlayerMusic->playlist();

	if (!playlist) {
		playlist = new QMediaPlaylist(m_mediaPlayerMusic);
		playlist->setPlaybackMode(QMediaPlaylist::Loop);
		m_mediaPlayerMusic->setPlaylist(playlist);
	}

	playlist->clear();
	playlist->addMedia(QUrl(m_musicNextSource));

	m_musicNextSource = "";

	m_mediaPlayerMusic->play();

}
