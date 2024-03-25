/*
 * ---- Call of Suli ----
 *
 * tiledactiongame.h
 *
 * Created on: 2024. 03. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgGame
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

#ifndef RPGGAME_H
#define RPGGAME_H

#include "gamequestion.h"
#include "rpgcontrolgroup.h"
#include "rpgenemyiface.h"
#include "rpgplayer.h"
#include "rpgpickableobject.h"
#include "tiledgame.h"
#include "isometricenemy.h"
#include <QQmlEngine>


class RpgQuestion;

/**
 * @brief The RpgGame class
 */

class RpgGame : public TiledGame
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(QList<RpgPlayer *> players READ players WRITE setPlayers NOTIFY playersChanged FINAL)
	Q_PROPERTY(RpgPlayer *controlledPlayer READ controlledPlayer WRITE setControlledPlayer NOTIFY controlledPlayerChanged FINAL)
	Q_PROPERTY(GameQuestion *gameQuestion READ gameQuestion WRITE setGameQuestion NOTIFY gameQuestionChanged FINAL)

public:
	explicit RpgGame(QQuickItem *parent = nullptr);
	virtual ~RpgGame();

	Q_INVOKABLE bool load();

	bool playerAttackEnemy(TiledObject *player, TiledObject *enemy, const TiledWeapon::WeaponType &weaponType) override final;
	bool enemyAttackPlayer(TiledObject *enemy, TiledObject *player, const TiledWeapon::WeaponType &weaponType) override final;
	bool playerPickPickable(TiledObject *player, TiledObject *pickable) override final;

	void onPlayerDead(TiledObject *player) override final;
	void onEnemyDead(TiledObject *enemy) override final;

	bool canAttack(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType);
	bool canAttack(IsometricEnemy *enemy, RpgPlayer *player, const TiledWeapon::WeaponType &weaponType);
	bool canTransport(RpgPlayer *player, TiledTransport *transport);

	IsometricEnemy *createEnemy(const RpgEnemyIface::RpgEnemyType &type, const QString &subtype, TiledScene *scene);
	IsometricEnemy *createEnemy(const RpgEnemyIface::RpgEnemyType &type, TiledScene *scene) {
		return createEnemy(type, QStringLiteral(""), scene);
	}


	RpgPickableObject *createPickable(const RpgPickableObject::PickableType &type, TiledScene *scene);

	RpgPlayer *controlledPlayer() const;
	void setControlledPlayer(RpgPlayer *newControlledPlayer);

	QList<RpgPlayer *> players() const;
	void setPlayers(const QList<RpgPlayer *> &newPlayers);

	static const QByteArray &baseEntitySprite0() { return m_baseEntitySprite0; }
	static const QByteArray &baseEntitySprite1() { return m_baseEntitySprite1; }
	static const QByteArray &baseEntitySprite2() { return m_baseEntitySprite2; }
	static QByteArray baseEntitySprite(const int &i) {
		if (i==0)
			return m_baseEntitySprite0;
		else if (i==1)
			return m_baseEntitySprite1;
		else if (i==2)
			return m_baseEntitySprite2;
		else
			return {};
	}


	static QString getAttackSprite(const TiledWeapon::WeaponType &weaponType);

	GameQuestion *gameQuestion() const;
	void setGameQuestion(GameQuestion *newGameQuestion);

	RpgQuestion *rpgQuestion() const;
	void setRpgQuestion(RpgQuestion *newRpgQuestion);

signals:
	void controlledPlayerChanged();
	void playersChanged();
	void gameQuestionChanged();

protected:
	virtual void loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer) override;
	virtual void loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer) override;
	virtual void joystickStateEvent(const JoystickState &state) override;
	virtual void keyPressEvent(QKeyEvent *event) override final;

private:
	void onGameQuestionSuccess(const QVariantMap &answer);
	void onGameQuestionFailed(const QVariantMap &answer);
	void onGameQuestionStarted();
	void onGameQuestionFinished();

	struct EnemyData {
		TiledObjectBase::ObjectId objectId;
		RpgEnemyIface::RpgEnemyType type = RpgEnemyIface::EnemyInvalid;
		QString subtype;
		QPolygonF path;
		int defaultAngle = 0;
		QPointer<TiledScene> scene;
		QPointer<IsometricEnemy> enemy;
		bool hasQuestion = false;
	};


	struct PickableData {
		TiledObjectBase::ObjectId objectId;
		RpgPickableObject::PickableType type = RpgPickableObject::PickableInvalid;
		QPointF position;
		QPointer<TiledScene> scene;
		QPointer<TiledObject> holder;
		QPointer<RpgPickableObject> pickableObject;
	};


	std::optional<EnemyData> getEnemyData(IsometricEnemy *enemy) const;

	QVector<EnemyData> m_enemyDataList;
	QVector<PickableData> m_pickableDataList;
	std::vector<std::unique_ptr<RpgControlGroup>> m_controlGroups;

	QList<RpgPlayer*> m_players;
	QPointer<RpgPlayer> m_controlledPlayer;
	QPointer<GameQuestion> m_gameQuestion;
	RpgQuestion *m_rpgQuestion = nullptr;

	static const QByteArray m_baseEntitySprite0;
	static const QByteArray m_baseEntitySprite1;
	static const QByteArray m_baseEntitySprite2;
};



#endif // RPGGAME_H
