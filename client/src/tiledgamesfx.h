/*
 * ---- Call of Suli ----
 *
 * sfx.h
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

#ifndef TILEDGAMESFX_H
#define TILEDGAMESFX_H

#include "qdeadlinetimer.h"
#include "qtimer.h"
#include "tiledobject.h"
#include <QObject>
#include <QString>
#include "sound.h"

class TiledGameSfx : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QStringList soundList READ soundList WRITE setSoundList NOTIFY soundListChanged FINAL)
	Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)
	Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)
	Q_PROPERTY(TiledObject *tiledObject READ tiledObject WRITE setTiledObject NOTIFY tiledObjectChanged FINAL)
	Q_PROPERTY(qint64 playOneDeadline READ playOneDeadline WRITE setPlayOneDeadline NOTIFY playOneDeadlineChanged FINAL)
	Q_PROPERTY(bool followPosition READ followPosition WRITE setFollowPosition NOTIFY followPositionChanged FINAL)

public:
	TiledGameSfx(const QStringList &soundList, const int &interval, const float &volume,
				 TiledObject *tiledObject = nullptr);

	TiledGameSfx(const QStringList &soundList, const int &interval, TiledObject *tiledObject) :
		TiledGameSfx(soundList, interval, 1.0, tiledObject) {}

	TiledGameSfx(const QStringList &soundList, TiledObject *tiledObject) :
		TiledGameSfx(soundList, 0, tiledObject) {}

	TiledGameSfx(TiledObject *tiledObject = nullptr) :
	  TiledGameSfx({}, tiledObject) {}

	virtual ~TiledGameSfx() {}

	Q_INVOKABLE void start();
	Q_INVOKABLE void startFromBegin();
	Q_INVOKABLE void stop();
	Q_INVOKABLE void restart();

	Q_INVOKABLE void playOne();
	Q_INVOKABLE void playOneDeadline(const qint64 &deadlineMsec);

	Q_INVOKABLE int interval() const { return m_timer.interval(); }
	Q_INVOKABLE void setInterval(const int &msec) { m_timer.setInterval(msec); }

	Q_INVOKABLE bool isActive() const { return m_timer.isActive(); }

	QStringList soundList() const;
	void setSoundList(const QStringList &newSoundList);

	float volume() const;
	void setVolume(float newVolume);

	int currentIndex() const;
	void setCurrentIndex(int newCurrentIndex);

	TiledObject *tiledObject() const;
	void setTiledObject(TiledObject *newTiledObject);

	qint64 playOneDeadline() const;
	void setPlayOneDeadline(qint64 newPlayOneDeadline);

	bool followPosition() const;
	void setFollowPosition(bool newFollowPosition);

signals:
	void soundListChanged();
	void volumeChanged();
	void currentIndexChanged();
	void tiledObjectChanged();
	void playOneDeadlineChanged();
	void followPositionChanged();

private:
	void onTimeout();
	void resetIndex();
	void startTimer();

	QPointer<TiledObject> m_tiledObject;
	QStringList m_soundList;
	float m_volume = 1.0;
	qint64 m_playOneDeadline = 0.;
	bool m_followPosition = true;

	QTimer m_timer;
	QDeadlineTimer m_deadlineTimer;

	int m_currentIndex = -1;
};





/**
 * @brief The TiledGameSfxLocation class
 */

class TiledGameSfxLocation : public QObject
{
	Q_OBJECT

	Q_PROPERTY(float baseVolume READ baseVolume WRITE setBaseVolume NOTIFY baseVolumeChanged FINAL)

public:
	TiledGameSfxLocation(const QString &path, const float &baseVolume, TiledObject *tiledObject,
						 const Sound::ChannelType &channel = Sound::Music2Channel);
	virtual ~TiledGameSfxLocation();

	float baseVolume() const;
	void setBaseVolume(float newBaseVolume);

signals:
	void baseVolumeChanged();

private:
	void onSceneChanged();
	void updateSound();
	void checkPosition();

	QPointer<TiledObject> m_object;
	std::unique_ptr<Sound::ExternalSound> m_sound;
	float m_baseVolume = 1.;

	TiledScene *m_connectedScene = nullptr;
	QPointF m_lastPoint;
	QRectF m_lastVisibleArea;
	qreal m_lastVolume = 0.;
};




#endif // TILEDGAMESFX_H
