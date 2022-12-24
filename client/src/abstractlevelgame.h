/*
 * ---- Call of Suli ----
 *
 * abstractlevelgame.h
 *
 * Created on: 2022. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractLevelGame
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

#ifndef ABSTRACTLEVELGAME_H
#define ABSTRACTLEVELGAME_H

#include "abstractgame.h"
#include "question.h"
#include "qtimer.h"


class AbstractLevelGame : public AbstractGame
{
	Q_OBJECT

	Q_PROPERTY(GameMapMissionLevel *missionLevel READ missionLevel CONSTANT)
	Q_PROPERTY(bool deathmatch READ deathmatch WRITE setDeathmatch NOTIFY deathmatchChanged)

	Q_PROPERTY(QString uuid READ uuid CONSTANT)
	Q_PROPERTY(QString name READ name CONSTANT)
	Q_PROPERTY(QString description READ description CONSTANT)
	Q_PROPERTY(int level READ level CONSTANT)
	Q_PROPERTY(QString medalImage READ medalImage CONSTANT)
	Q_PROPERTY(QString terrain READ terrain CONSTANT)
	Q_PROPERTY(int startHP READ startHP CONSTANT)
	Q_PROPERTY(int duration READ duration CONSTANT)

	Q_PROPERTY(QUrl backgroundImage READ backgroundImage CONSTANT);

	Q_PROPERTY(int msecLeft READ msecLeft WRITE setMsecLeft NOTIFY msecLeftChanged)

public:
	explicit AbstractLevelGame(const Mode &mode, GameMapMissionLevel *missionLevel, Client *client);
	virtual ~AbstractLevelGame();

	bool deathmatch() const;
	void setDeathmatch(bool newDeathmatch);

	GameMapMissionLevel *missionLevel() const;

	QString uuid() const;
	QString name() const;
	QString description() const;
	int level() const;
	QString medalImage() const;
	QString terrain() const;
	int startHP() const;
	int duration() const;
	QUrl backgroundImage() const;

	int msecLeft() const;
	void setMsecLeft(int newMsecLeft);

protected:
	QVector<Question> createQuestions();

private slots:
	void onTimerLeftTimeout();

signals:
	void gameTimeout();
	void deathmatchChanged();
	void msecLeftChanged(int diff);

protected:
	GameMapMissionLevel *const m_missionLevel = nullptr;
	bool m_deathmatch = false;
	int m_msecLeft = 0;
	QTimer *m_timerLeft = nullptr;

private:
};

#endif // ABSTRACTLEVELGAME_H
