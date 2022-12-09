/*
 * ---- Call of Suli ----
 *
 * gameenemysniper.h
 *
 * Created on: 2022. 07. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemySniper
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

#ifndef GAMEENEMYSNIPER_H
#define GAMEENEMYSNIPER_H

#include "gameenemy.h"

class GameEnemySniper : public GameEnemy
{
	Q_OBJECT

	Q_PROPERTY(int msecBeforeTurn READ msecBeforeTurn WRITE setMsecBeforeTurn NOTIFY msecBeforeTurnChanged)
	Q_PROPERTY(QString shotSoundFile READ shotSoundFile WRITE setShotSoundFile NOTIFY shotSoundFileChanged)
	Q_PROPERTY(QString sniperType READ sniperType WRITE setSniperType NOTIFY sniperTypeChanged)

public:
	GameEnemySniper(QQuickItem *parent = 0);
	virtual ~GameEnemySniper();

	void setQrcDir() override;
	void createFixtures() override;

	int msecBeforeTurn() const;
	void setMsecBeforeTurn(int newMsecBeforeTurn);

	const QString &shotSoundFile() const;
	void setShotSoundFile(const QString &newShotSoundFile);

	const QString &sniperType() const;
	void setSniperType(const QString &newSniperType);

signals:
	void msecBeforeTurnChanged();
	void shotSoundFileChanged();
	void sniperTypeChanged();

private slots:
	void onGameDataReady(const QVariantMap &map) override;
	void onCosGameChanged(CosGame *);
	void onQrcDataChanged(QVariantMap);

private:
	int m_msecBeforeTurn;
	QString m_shotSoundFile;
	QString m_sniperType;
};


#endif // GAMEENEMYSNIPER_H
