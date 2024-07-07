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
#include <QScatterSeries>


#if QT_VERSION < 0x060000
using QScatterSeries = QtCharts::QScatterSeries;
#endif

class RpgQuestion;




/**
 * @brief The RpgGameDefinition class
 */

class RpgGameDefinition : public TiledGameDefinition
{
	Q_GADGET

public:
	RpgGameDefinition()
		: TiledGameDefinition()
	{}

	QS_SERIALIZABLE

	// Base

	QS_FIELD(QString, name)
	QS_FIELD(QString, minVersion)

	// Market

	QS_OBJECT(RpgMarket, market)

	// Inventory

	QS_COLLECTION(QList, QString, inventory)
	QS_COLLECTION(QList, QString, inventoryOnce)

	// Required tileset

	QS_COLLECTION(QList, QString, required)
};






/**
 * @brief The RpgEnemyMetricDefinition class
 */

class RpgEnemyMetricDefinition : public QSerializer
{
	Q_GADGET

public:
	RpgEnemyMetricDefinition() {}

	QS_SERIALIZABLE

	QS_QT_DICT_OBJECTS(QHash, QString, EnemyMetric, soldier)
	QS_QT_DICT_OBJECTS(QHash, QString, EnemyMetric, archer)
	QS_QT_DICT_OBJECTS(QHash, QString, EnemyMetric, werebear)
	QS_QT_DICT_OBJECTS(QHash, QString, EnemyMetric, skeleton)
};




typedef std::function<bool(RpgPlayer*, RpgPickableObject*)> FuncPlayerPick;
typedef std::function<bool(RpgPlayer*, TiledContainer*)> FuncPlayerUseContainer;
typedef std::function<bool(RpgPlayer*, IsometricEnemy*, const TiledWeapon::WeaponType &)> FuncPlayerAttackEnemy;



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
	Q_PROPERTY(int enemyCount READ enemyCount WRITE setEnemyCount NOTIFY enemyCountChanged FINAL)

	Q_PROPERTY(QScatterSeries *scatterSeriesPlayers READ scatterSeriesPlayers WRITE setScatterSeriesPlayers NOTIFY scatterSeriesPlayersChanged FINAL)
	Q_PROPERTY(QScatterSeries *scatterSeriesEnemies READ scatterSeriesEnemies WRITE setScatterSeriesEnemies NOTIFY scatterSeriesEnemiesChanged FINAL)

public:
	explicit RpgGame(QQuickItem *parent = nullptr);
	virtual ~RpgGame();

	Q_INVOKABLE bool load(const RpgGameDefinition &def);

	static const QHash<QString, RpgGameDefinition> &terrains();
	static void reloadTerrains();

	static const QHash<QString, RpgPlayerCharacterConfig> &characters();
	static void reloadCharacters();

	static std::optional<RpgGameDefinition> readGameDefinition(const QString &map);

	bool playerAttackEnemy(TiledObject *player, TiledObject *enemy, const TiledWeapon::WeaponType &weaponType) override final;
	bool enemyAttackPlayer(TiledObject *enemy, TiledObject *player, const TiledWeapon::WeaponType &weaponType) override final;
	bool playerPickPickable(TiledObject *player, TiledObject *pickable) override final;
	void saveSceneState(RpgPlayer *player);

	void onPlayerDead(TiledObject *player) override final;
	void onEnemyDead(TiledObject *enemy) override final;
	void onEnemySleepingStart(TiledObject *enemy) override final;
	void onEnemySleepingEnd(TiledObject *enemy) override final;

	bool canAttack(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType);
	bool canAttack(IsometricEnemy *enemy, RpgPlayer *player, const TiledWeapon::WeaponType &weaponType);
	bool canTransport(RpgPlayer *player, TiledTransport *transport);

	bool playerTryUseContainer(RpgPlayer *player, TiledContainer *container);
	void playerUseContainer(RpgPlayer *player, TiledContainer *container);

	IsometricEnemy *createEnemy(const RpgEnemyIface::RpgEnemyType &type, const QString &subtype, TiledScene *scene);
	IsometricEnemy *createEnemy(const RpgEnemyIface::RpgEnemyType &type, TiledScene *scene) {
		return createEnemy(type, QStringLiteral(""), scene);
	}

	RpgPickableObject *createPickable(const RpgPickableObject::PickableType &type, const QString &name, TiledScene *scene);
	RpgPickableObject *createPickable(const RpgPickableObject::PickableType &type, TiledScene *scene) {
		return createPickable(type, QStringLiteral(""), scene);
	}

	Q_INVOKABLE bool transportPlayer();
	Q_INVOKABLE bool useContainer();

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
	static RpgEnemyMetricDefinition defaultEnemyMetric();

	Q_INVOKABLE virtual void onMouseClick(const qreal &x, const qreal &y, const int &modifiers) override;

	int setQuestions(TiledScene *scene, qreal factor);

	bool enemySetDieForever(IsometricEnemy *enemy, const bool &dieForever);

	void resurrectEnemiesAndPlayer(RpgPlayer *player);
	void resurrectEnemies(const QPointer<TiledScene> &scene);

	virtual void onSceneWorldStepped(TiledScene *scene) override;

	GameQuestion *gameQuestion() const;
	void setGameQuestion(GameQuestion *newGameQuestion);

	RpgQuestion *rpgQuestion() const;
	void setRpgQuestion(RpgQuestion *newRpgQuestion);

	int enemyCount() const;
	void setEnemyCount(int newEnemyCount);

	FuncPlayerPick funcPlayerPick() const;
	void setFuncPlayerPick(const FuncPlayerPick &newFuncPlayerPick);

	FuncPlayerAttackEnemy funcPlayerAttackEnemy() const;
	void setFuncPlayerAttackEnemy(const FuncPlayerAttackEnemy &newFuncPlayerAttackEnemy);

	FuncPlayerUseContainer funcPlayerUseContainer() const;
	void setFuncPlayerUseContainer(const FuncPlayerUseContainer &newFuncPlayerUseContainer);

	QScatterSeries *scatterSeriesPlayers() const;
	void setScatterSeriesPlayers(QScatterSeries *newScatterSeriesPlayers);

	QScatterSeries *scatterSeriesEnemies() const;
	void setScatterSeriesEnemies(QScatterSeries *newScatterSeriesEnemies);


signals:
	void minimapToggleRequest();
	void gameSuccess();
	void playerDead(RpgPlayer *player);
	void controlledPlayerChanged();
	void playersChanged();
	void gameQuestionChanged();
	void enemyCountChanged();
	void scatterSeriesPlayersChanged();
	void scatterSeriesEnemiesChanged();

protected:
	virtual void loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer) override;
	virtual void loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, const QString &groupClass, Tiled::MapRenderer *renderer) override;
	virtual TiledObjectBasePolygon *loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer) override;
	virtual void joystickStateEvent(const JoystickState &state) override;
	virtual void keyPressEvent(QKeyEvent *event) override final;

	bool transportBeforeEvent(TiledObject *object, TiledTransport *transport) override;
	bool transportAfterEvent(TiledObject *object, TiledScene *newScene, TiledObjectBase *newObject) override;

private:
	void loadMetricDefinition();
	EnemyMetric getMetric(EnemyMetric baseMetric, const RpgEnemyIface::RpgEnemyType &type, const QString &subtype = QStringLiteral(""), const int &level = 1);

	void loadEnemy(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	void loadPickable(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	void addLocationSound(TiledObjectBase *object, const QString &sound,
						  const qreal &baseVolume = 1.,
						  const Sound::ChannelType &channel = Sound::Music2Channel);

	void onGameQuestionSuccess(const QVariantMap &answer);
	void onGameQuestionFailed(const QVariantMap &answer);
	void onGameQuestionStarted();
	void onGameQuestionFinished();
	int recalculateEnemies();

	void updateScatterEnemies();
	void updateScatterPlayers();

	struct EnemyData {
		TiledObjectBase::ObjectId objectId;
		RpgEnemyIface::RpgEnemyType type = RpgEnemyIface::EnemyInvalid;
		QString subtype;
		QPolygonF path;
		int defaultAngle = 0;
		QPointer<TiledScene> scene;
		QPointer<IsometricEnemy> enemy;
		bool hasQuestion = false;
		bool dieForever = false;
		QVector<RpgPickableObject::PickableType> pickables;
		QVector<RpgPickableObject::PickableType> pickablesOnce;
	};


	struct PickableData {
		TiledObjectBase::ObjectId objectId;
		RpgPickableObject::PickableType type = RpgPickableObject::PickableInvalid;
		QString name;
		QPointF position;
		QPointer<TiledScene> scene;
		QPointer<RpgPickableObject> pickableObject;
	};


	QVector<EnemyData>::iterator enemyFind(IsometricEnemy *enemy);
	QVector<EnemyData>::const_iterator enemyFind(IsometricEnemy *enemy) const;



	RpgGameDefinition m_gameDefinition;
	QVector<RpgEnemyMetricDefinition> m_metricDefinition;

	QVector<EnemyData> m_enemyDataList;
	QVector<PickableData> m_pickableDataList;
	std::vector<std::unique_ptr<RpgControlGroup>> m_controlGroups;

	QList<RpgPlayer*> m_players;
	QPointer<RpgPlayer> m_controlledPlayer;
	QPointer<GameQuestion> m_gameQuestion;
	RpgQuestion *m_rpgQuestion = nullptr;
	int m_enemyCount = 0;

	std::vector<std::unique_ptr<TiledGameSfxLocation>> m_sfxLocations;


	QPointer<QScatterSeries> m_scatterSeriesPlayers;
	QPointer<QScatterSeries> m_scatterSeriesEnemies;

	// TODO: FuncPlayerAttack, FuncEnemyAttack,...
	FuncPlayerPick m_funcPlayerPick;
	FuncPlayerAttackEnemy m_funcPlayerAttackEnemy;
	FuncPlayerUseContainer m_funcPlayerUseContainer;

	// 3 részre daraboljuk, hogy ne haladja meg a textúra a 4096 px méretet

	static const QByteArray m_baseEntitySprite0;
	static const QByteArray m_baseEntitySprite1;
	static const QByteArray m_baseEntitySprite2;

	static QHash<QString, RpgGameDefinition> m_terrains;
	static QHash<QString, RpgPlayerCharacterConfig> m_characters;

	friend class ActionRpgGame;
};



#endif // RPGGAME_H
