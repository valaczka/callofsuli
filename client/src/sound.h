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

#include "qtimer.h"
#include <QString>
#include <QObject>
#include <QQueue>
#include <QPointer>
#include <QMutex>
#include <miniaudio.h>
#include <QHash>
#include <QTemporaryFile>


/**
 * @brief The Sound class
 */

class Sound : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int volumeSfx READ volumeSfx WRITE setVolumeSfx NOTIFY volumeSfxChanged)
	Q_PROPERTY(int volumeVoiceOver READ volumeVoiceOver WRITE setVolumeVoiceOver NOTIFY volumeVoiceOverChanged)
	Q_PROPERTY(int volumeMusic READ volumeMusic WRITE setVolumeMusic NOTIFY volumeMusicChanged)
	Q_PROPERTY(bool sfxEnabled READ sfxEnabled WRITE setSfxEnabled NOTIFY sfxEnabledChanged)
	Q_PROPERTY(bool voiceOverEnabled READ voiceOverEnabled WRITE setVoiceOverEnabled NOTIFY voiceOverEnabledChanged)
	Q_PROPERTY(bool musicEnabled READ musicEnabled WRITE setMusicEnabled NOTIFY musicEnabledChanged)

public:
	enum ChannelType { MusicChannel, SfxChannel, VoiceoverChannel };
	Q_ENUM(ChannelType)

	explicit Sound(QObject *parent = nullptr);
	virtual ~Sound();

	Q_INVOKABLE void playSound(const QString &source, const Sound::ChannelType &channel);
	Q_INVOKABLE void stopSound(const QString &source, const Sound::ChannelType &channel);
	Q_INVOKABLE void stopMusic();

	Q_INVOKABLE bool isPlayingMusic() const;

	int volumeSfx() const;
	void setVolumeSfx(int newVolumeSfx);

	int volumeVoiceOver() const;
	void setVolumeVoiceOver(int newVolumeVoiceOver);

	int volumeMusic() const;
	void setVolumeMusic(int newVolumeMusic);

	bool sfxEnabled() const;
	void setSfxEnabled(bool newSfxEnabled);

	bool voiceOverEnabled() const;
	void setVoiceOverEnabled(bool newVoiceOverEnabled);

	bool musicEnabled() const;
	void setMusicEnabled(bool newMusicEnabled);

signals:
	void volumeSfxChanged();
	void volumeVoiceOverChanged();
	void volumeMusicChanged();
	void sfxEnabledChanged();
	void voiceOverEnabledChanged();
	void musicEnabledChanged();

private:

	/**
	 * @brief The MaSound class
	 */

	class MaSound {
	public:
		MaSound(ma_engine *engine, const QString &path, const ChannelType &channel, ma_sound_group *group);
		~MaSound();

		void uninit();
		void uninitChildren();

		const QString &path() const { return m_path; }

		ChannelType channel() const { return m_channel; }
		void setChannel(ChannelType newType) { m_channel = newType; }

		ma_sound *sound() const { return m_sound.get(); }

		ma_sound *duplicate(ma_sound_group *group);
		void garbage();

		const std::vector<std::unique_ptr<ma_sound> > &children() { return m_children; }

	private:
		void removeChild(ma_sound *ptr);

		ma_engine *m_engine = nullptr;
		QString m_path;
		ChannelType m_channel = SfxChannel;
		std::unique_ptr<ma_sound> m_sound;
		std::vector<std::unique_ptr<ma_sound> > m_children;
		std::vector<ma_sound*> m_garbage;

		QRecursiveMutex m_mutex;
		QByteArray m_tmpPath;
	};

	bool engineInit();
	bool engineUninit();
	void engineCheck();

	void playSound(MaSound *sound);
	QVector<MaSound *> currentMusic() const;
	void updateVolumes();
	void garbage();
	void playNextVoiceOver();

	int m_volumeSfx = 0;
	int m_volumeVoiceOver = 0;
	int m_volumeMusic = 0;
	bool m_sfxEnabled = true;
	bool m_voiceOverEnabled = true;
	bool m_musicEnabled = true;

	std::unique_ptr<ma_engine> m_engine;
	std::unique_ptr<ma_sound_group> m_groupSfx;
	std::unique_ptr<ma_sound_group> m_groupMusic;
	std::unique_ptr<ma_sound_group> m_groupVoiceOver;

	std::vector<std::unique_ptr<MaSound> > m_sound;

	QQueue<MaSound *> m_queue;

	QTimer m_garbageTimer;
	QRecursiveMutex m_mutex;
};





#endif // SOUND_H
