/*
 * ---- Call of Suli ----
 *
 * gameactivity.h
 *
 * Created on: 2020. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameActivity
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

#ifndef GAMEACTIVITY_H
#define GAMEACTIVITY_H

#include "abstractactivity.h"
#include "gameenemy.h"
#include "gameenemydata.h"
#include <QObject>

class CosGame;

class GameActivity : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(bool prepared READ prepared WRITE setPrepared NOTIFY preparedChanged)
	Q_PROPERTY(CosGame * game READ game WRITE setGame NOTIFY gameChanged)


public:
	GameActivity(QQuickItem *parent = nullptr);
	~GameActivity();

	Q_INVOKABLE void prepare();
	Q_INVOKABLE void createPickable(GameEnemy *enemy);

	bool createTarget(GameEnemy *enemy);
	bool createTargets(QVector<GameEnemy *> enemies);

	bool prepared() const { return m_prepared; }
	CosGame * game() const { return m_game; }

public slots:
	void onEnemyKilled(GameEnemy *enemy);
	void onEnemyKillMissed(GameEnemy *enemy);
	void setPrepared(bool prepared);
	void setGame(CosGame * game);

signals:
	void prepareFailed();
	void prepareSucceed();

	void preparedChanged(bool prepared);
	void gameChanged(CosGame * game);

protected slots:
	//void clientSetup() override;
	//void onMessageReceived(const CosMessage &message) override;
	//void onMessageFrameReceived(const CosMessage &message) override;

private slots:
	void prepareDb(QVariantMap = QVariantMap());

private:
	bool m_prepared;
	CosGame * m_game;
};

#endif // GAMEACTIVITY_H
