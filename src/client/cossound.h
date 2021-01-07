/*
 * ---- Call of Suli ----
 *
 * cosclientsound.h
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

#ifndef COSSOUND_H
#define COSSOUND_H

#include <QObject>
#include <QMediaPlayer>
#include <QVariantAnimation>

class CosSound : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int volumeMusic READ volumeMusic WRITE setVolumeMusic NOTIFY volumeMusicChanged)
	Q_PROPERTY(int volumeSfx READ volumeSfx WRITE setVolumeSfx NOTIFY volumeSfxChanged)
	Q_PROPERTY(int volumeVoiceOver READ volumeVoiceOver WRITE setVolumeVoiceOver NOTIFY volumeVoiceOverChanged)


public:
	enum ChannelType { MusicChannel, SfxChannel, VoiceoverChannel };
	Q_ENUM(ChannelType)

	enum SoundType { Music, VoiceOver, PlayerSfx, EnemySfx, GameSfx, PlayerShoot, EnemyShoot, PlayerVoice, GameSound };
	Q_ENUM(SoundType)

	explicit CosSound(QObject *parent = nullptr);
	~CosSound();

public slots:
	void init();

	void playSound(const QString &source, const CosSound::SoundType &soundType);
	void stopSound(const QString &source, const CosSound::SoundType &soundType);

	int volumeMusic() const { return m_mediaPlayerMusic ? m_mediaPlayerMusic->volume() : 0;  }
	int volumeSfx() const { return m_mediaPlayerSfx ? m_mediaPlayerSfx->volume() : 0;  }
	int volumeVoiceOver() const { return m_mediaPlayerVoiceOver ? m_mediaPlayerVoiceOver->volume() : 0; }

	void setVolumeSfx(int volume) { if (m_mediaPlayerSfx) m_mediaPlayerSfx->setVolume(volume); }
	void setVolumeMusic(int volume) { if (m_mediaPlayerMusic) m_mediaPlayerMusic->setVolume(volume); }
	void setVolumeVoiceOver(int volume) { if (m_mediaPlayerVoiceOver) m_mediaPlayerVoiceOver->setVolume(volume); }

private slots:
	void musicPlay(const QString &source);
	void musicLoadNextSource();

signals:
	void volumeSfxChanged(int volumeSfx);
	void volumeMusicChanged(int volumeMusic);
	void volumeVoiceOverChanged(int volumeVoiceOver);

private:
	QMediaPlayer *m_mediaPlayerMusic;
	QMediaPlayer *m_mediaPlayerSfx;
	QMediaPlayer *m_mediaPlayerVoiceOver;
	SoundType m_soundTypeSfx;
	QString m_musicNextSource;
	QVariantAnimation *m_fadeAnimation;

};

#endif // COSSOUND_H
