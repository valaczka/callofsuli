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

class TiledGameSfx : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QStringList soundList READ soundList WRITE setSoundList NOTIFY soundListChanged FINAL)
	Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)
	Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged FINAL)
	Q_PROPERTY(TiledObject *tiledObject READ tiledObject WRITE setTiledObject NOTIFY tiledObjectChanged FINAL)
	Q_PROPERTY(qint64 playOneDeadline READ playOneDeadline WRITE setPlayOneDeadline NOTIFY playOneDeadlineChanged FINAL)

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

signals:
	void soundListChanged();
	void volumeChanged();
	void currentIndexChanged();
	void tiledObjectChanged();
	void playOneDeadlineChanged();

private:
	void onTimeout();
	void resetIndex();
	void startTimer();

	QPointer<TiledObject> m_tiledObject;
	QStringList m_soundList;
	float m_volume = 1.0;
	qint64 m_playOneDeadline = 0.;

	QTimer m_timer;
	QDeadlineTimer m_deadlineTimer;

	int m_currentIndex = -1;
};

#endif // TILEDGAMESFX_H
