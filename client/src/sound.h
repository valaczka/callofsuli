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

#ifndef SOUND_H
#define SOUND_H

#include <QObject>
#include <QMediaPlayer>
#include <QVariantAnimation>

/**
 * @brief The Sound class
 */

class Sound : public QObject
{
	Q_OBJECT

public:
	enum ChannelType { MusicChannel, SfxChannel, VoiceoverChannel };
	Q_ENUM(ChannelType)

	enum SoundType { Music, VoiceOver, PlayerSfx, PlayerVoice, GameSound };
	Q_ENUM(SoundType)

	explicit Sound(QObject *parent = nullptr);
	virtual ~Sound();

	void init();

	void playSound(const QString &source, const Sound::SoundType &soundType);
	void stopSound(const QString &source, const Sound::SoundType &soundType);

	int volume(const ChannelType &channel) const;
	void setVolume(const ChannelType &channel, int newVolume);

	bool isPlayingMusic() const;

private slots:
	void musicPlay(const QString &source);
	void musicLoadNextSource();

private:
	QMediaPlayer *m_mediaPlayerMusic = nullptr;
	QMediaPlayer *m_mediaPlayerSfx = nullptr;
	QMediaPlayer *m_mediaPlayerVoiceOver = nullptr;
	SoundType m_soundTypeSfx;
	QString m_musicNextSource;
	QVariantAnimation *m_fadeAnimation = nullptr;
	int m_musicVolume;

	friend class DesktopClient;
};



#endif // SOUND_H
