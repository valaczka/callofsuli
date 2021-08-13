/*
 * ---- Call of Suli ----
 *
 * gameenemy.h
 *
 * Created on: 2020. 10. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemy
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

#ifndef GAMEENEMY_H
#define GAMEENEMY_H

#include <QRectF>
#include <QObject>
#include <qquickitem.h>

#include "question.h"
#include "gameblock.h"

class GameEnemy;

class GameEnemyData : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QRectF boundRect READ boundRect WRITE setBoundRect NOTIFY boundRectChanged)
	Q_PROPERTY(GameBlock * block READ block WRITE setBlock NOTIFY blockChanged)

	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(int targetId READ targetId WRITE setTargetId NOTIFY targetIdChanged)
	Q_PROPERTY(QByteArray objectiveUuid READ objectiveUuid WRITE setObjectiveUuid NOTIFY objectiveUuidChanged)
	Q_PROPERTY(bool targetDestroyed READ targetDestroyed WRITE setTargetDestroyed NOTIFY targetDestroyedChanged)

	Q_PROPERTY(EnemyType enemyType READ enemyType NOTIFY enemyTypeChanged)
	Q_PROPERTY(QQuickItem * enemy READ enemy NOTIFY enemyChanged)

	Q_PROPERTY(PickableType pickableType READ pickableType WRITE setPickableType NOTIFY pickableTypeChanged)
	Q_PROPERTY(QVariantMap pickableData READ pickableData WRITE setPickableData NOTIFY pickableDataChanged)


public:

	enum EnemyType {
		EnemySoldier,
		EnemyOther
	};

	Q_ENUM(EnemyType)

	enum PickableType {
		PickableInvalid,
		PickableHealth,
		PickableTime,
		PickableShield
	};

	Q_ENUM(PickableType)


	struct InventoryType {
		InventoryType(QString name, QString icon, PickableType type, QVariantMap data) :
			name(name), icon(icon), type(type), data(data) {}

		InventoryType() :
			name(), icon(), type(PickableInvalid), data() {}

		QString name;
		QString icon;
		PickableType type;
		QVariantMap data;
	};

	explicit GameEnemyData(QObject *parent = nullptr);

	static QHash<QByteArray, InventoryType> inventoryTypes();
	static QVariantMap inventoryInfo(const QString &module);

	QRectF boundRect() const { return m_boundRect; }
	bool active() const { return m_active; }
	GameBlock * block() const { return m_block; }

	QQuickItem * enemy() const { return m_enemy; }
	EnemyType enemyType() const { return m_enemyType; }
	GameEnemy * enemyPrivate() const;
	int targetId() const { return m_targetId; }
	QByteArray objectiveUuid() const { return m_objectiveUuid; }
	PickableType pickableType() const { return m_pickableType; }
	QVariantMap pickableData() const { return m_pickableData; }
	bool targetDestroyed() const { return m_targetDestroyed; }


public slots:
	void setBoundRect(QRectF boundRect);
	void setActive(bool active);
	void setBlock(GameBlock * block);
	void setEnemy(QQuickItem * enemy);
	void setEnemyType(EnemyType enemyType);
	void enemyDied();
	void enemyKilled(GameEnemy *);
	void setTargetId(int targetId);
	void setObjectiveUuid(QByteArray objectiveUuid);
	Question generateQuestion();
	void setPickableType(PickableType pickableType);
	void setPickableData(QVariantMap pickableData);
	void setTargetDestroyed(bool targetDestroyed);

signals:
	void boundRectChanged(QRectF boundRect);
	void activeChanged(bool active);
	void blockChanged(GameBlock * block);
	void enemyChanged(QQuickItem * enemy);
	void enemyTypeChanged(EnemyType enemyType);
	void targetIdChanged(int targetId);
	void objectiveUuidChanged(QByteArray objectiveUuid);
	void pickableTypeChanged(PickableType pickableType);
	void pickableDataChanged(QVariantMap pickableData);
	void targetDestroyedChanged(bool targetDestroyed);

private:
	QRectF m_boundRect;
	bool m_active;
	GameBlock * m_block;
	QQuickItem *m_enemy;
	EnemyType m_enemyType;
	int m_targetId;
	QByteArray m_objectiveUuid;
	PickableType m_pickableType;
	QVariantMap m_pickableData;
	bool m_targetDestroyed;
};

#endif // GAMEENEMY_H
