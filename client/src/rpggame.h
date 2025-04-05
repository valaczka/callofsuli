/*
 * ---- Call of Suli ----
 *
 * rpggame.h
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
#include "rpgenemy.h"
#include "rpgplayer.h"
#include "rpgpickableobject.h"
#include "tiledgame.h"
#include "isometricenemy.h"
#include "rpgconfig.h"
#include <QQmlEngine>
#include <QScatterSeries>


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
		, playerHP(0)
	{}

	QS_SERIALIZABLE

	// Base

	QS_FIELD(QString, name)
	QS_FIELD(QString, minVersion)

	// Inventory

	QS_COLLECTION(QList, QString, inventory)
	QS_COLLECTION(QList, QString, inventoryOnce)

	// Player HP
	QS_FIELD(int, playerHP)

	// Required tileset

	QS_COLLECTION(QList, QString, required)

	// Quests

	QS_COLLECTION_OBJECTS(QList, RpgQuest, quests)
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
	QS_QT_DICT_OBJECTS(QHash, QString, EnemyMetric, butcher)
	QS_QT_DICT_OBJECTS(QHash, QString, EnemyMetric, smith)
	QS_QT_DICT_OBJECTS(QHash, QString, EnemyMetric, barbarian)

	QS_QT_DICT(QHash, RpgPlayerCharacterConfig::CastType, int, playerCast)
};




typedef std::function<void(TiledObjectBody *body)> FuncBodyStep;
typedef std::function<void()> FuncTimeStep;
typedef std::function<bool(RpgPlayer*, RpgPickableObject*)> FuncPlayerPick;
typedef std::function<bool(RpgPlayer*, RpgContainer*)> FuncPlayerUseContainer;
typedef std::function<bool(RpgPlayer*, RpgEnemy*, const RpgGameData::Weapon::WeaponType &)> FuncPlayerAttackEnemy;
typedef std::function<bool(RpgPlayer*, RpgEnemy*, RpgWeapon*)> FuncPlayerHit;
typedef std::function<bool(RpgPlayer*, RpgWeapon*, const qreal &)> FuncPlayerShot;
typedef std::function<bool(RpgPlayer*)> FuncPlayerUseCast;
typedef std::function<bool(RpgEnemy*, RpgPlayer *, const RpgGameData::Weapon::WeaponType &)> FuncEnemyAttackPlayer;
typedef std::function<bool(RpgEnemy*, RpgPlayer*, RpgWeapon*)> FuncEnemyHit;
typedef std::function<bool(RpgEnemy*, RpgWeapon*, const qreal &)> FuncEnemyShot;



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
	Q_PROPERTY(int deadEnemyCount READ deadEnemyCount WRITE setDeadEnemyCount NOTIFY deadEnemyCountChanged FINAL)
	Q_PROPERTY(int currency READ currency WRITE setCurrency NOTIFY currencyChanged FINAL)
	Q_PROPERTY(int winnerStreak READ winnerStreak WRITE setWinnerStreak NOTIFY winnerStreakChanged FINAL)
	Q_PROPERTY(QList<RpgQuest> quests READ quests NOTIFY questsChanged FINAL)

	Q_PROPERTY(QScatterSeries *scatterSeriesPlayers READ scatterSeriesPlayers WRITE setScatterSeriesPlayers NOTIFY scatterSeriesPlayersChanged FINAL)
	Q_PROPERTY(QScatterSeries *scatterSeriesEnemies READ scatterSeriesEnemies WRITE setScatterSeriesEnemies NOTIFY scatterSeriesEnemiesChanged FINAL)
	Q_PROPERTY(QScatterSeries *scatterSeriesPoints READ scatterSeriesPoints WRITE setScatterSeriesPoints NOTIFY scatterSeriesPointsChanged FINAL)

public:
	explicit RpgGame(QQuickItem *parent = nullptr);
	virtual ~RpgGame();

	Q_INVOKABLE bool load(const RpgGameDefinition &def, const bool &replaceMpToShield = false);

	static const QHash<QString, RpgGameDefinition> &terrains();
	static void reloadTerrains();

	static const QHash<QString, RpgPlayerCharacterConfig> &characters();
	static void reloadCharacters();

	static void reloadWorld();

	static std::optional<RpgGameDefinition> readGameDefinition(const QString &map);

	void saveSceneState(RpgPlayer *player);
	void saveSceneState();

	void onPlayerDead(TiledObject *player) override final;
	void onEnemyDead(TiledObject *enemy) override final;
	void onEnemySleepingStart(TiledObject *enemy) override final;
	void onEnemySleepingEnd(TiledObject *enemy) override final;

	bool playerTryUseContainer(RpgPlayer *player, RpgContainer *container);

	Q_INVOKABLE bool transportPlayer();
	Q_INVOKABLE bool useContainer();

	RpgPlayer *controlledPlayer() const;
	void setControlledPlayer(RpgPlayer *newControlledPlayer);

	const QList<RpgPlayer *> &players() const;
	void setPlayers(const QList<RpgPlayer *> &newPlayers);


	// Sprite texture helper

	static bool loadBaseTextureSprites(TiledSpriteHandler *handler,
									   const QString &path,
									   const QString &layer = QStringLiteral("default"));

	static bool loadTextureSpritesWithHurt(TiledSpriteHandler *handler,
										   const QVector<TextureSpriteMapper> &mapper,
										   const QString &path,
										   const QString &layer = QStringLiteral("default"));


	static const QVector<TextureSpriteMapper> &baseEntitySprite();

	static QString getAttackSprite(const RpgGameData::Weapon::WeaponType &weaponType);
	static RpgEnemyMetricDefinition defaultEnemyMetric();

	Q_INVOKABLE virtual void onMouseClick(const qreal &x, const qreal &y, const int &buttons, const int &modifiers) override;

	int setQuestions(TiledScene *scene, qreal factor);

	void resurrectEnemiesAndPlayer(RpgPlayer *player);
	void resurrectEnemies(const QPointer<TiledScene> &scene);

	static void saveTerrainInfo();
	static std::optional<RpgMarket> saveTerrainInfo(const RpgGameDefinition &def);

	virtual TiledObjectBody *loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer) override;

	void useWeapon(const RpgGameData::Weapon::WeaponType &type);
	const QVector<RpgWallet> &usedWallet() const { return m_usedWallet; }
	QJsonArray usedWalletAsArray() const;

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

	FuncPlayerUseCast funcPlayerUseCast() const;
	void setFuncPlayerUseCast(const FuncPlayerUseCast &newFuncPlayerUseMana);

	FuncPlayerUseCast funcPlayerFinishCast() const;
	void setFuncPlayerFinishCast(const FuncPlayerUseCast &newFuncPlayerFinishCast);

	FuncPlayerUseCast funcPlayerCastTimeout() const;
	void setFuncPlayerCastTimeout(const FuncPlayerUseCast &newFuncPlayerCastTimeout);
	void onPlayerCastTimeout(RpgPlayer *player) const;

	FuncEnemyAttackPlayer funcEnemyAttackPlayer() const;
	void setFuncEnemyAttackPlayer(const FuncEnemyAttackPlayer &newFuncEnemyAttackPlayer);

	QScatterSeries *scatterSeriesPlayers() const;
	void setScatterSeriesPlayers(QScatterSeries *newScatterSeriesPlayers);

	QScatterSeries *scatterSeriesEnemies() const;
	void setScatterSeriesEnemies(QScatterSeries *newScatterSeriesEnemies);

	QScatterSeries *scatterSeriesPoints() const;
	void setScatterSeriesPoints(QScatterSeries *newScatterSeriesPoints);

	int deadEnemyCount() const;
	void setDeadEnemyCount(int newDeadEnemyCount);

	int currency() const;
	void setCurrency(int newCurrency);

	int winnerStreak() const;
	void setWinnerStreak(int newWinnerStreak);

	const QList<RpgQuest> &quests() const;

	int getMetric(const RpgPlayerCharacterConfig::CastType &cast) const;
	EnemyMetric getMetric(EnemyMetric baseMetric, const RpgGameData::EnemyBaseData::EnemyType &type, const QString &subtype = QStringLiteral(""));

	FuncBodyStep funcBodyStep() const;
	void setFuncBodyStep(const FuncBodyStep &newFuncBodyStep);

	FuncTimeStep funcTimeStep() const;
	void setFuncTimeStep(const FuncTimeStep &newFuncTimeStep);

	FuncPlayerHit funcPlayerHit() const;
	void setFuncPlayerHit(const FuncPlayerHit &newFuncPlayerHit);

	FuncPlayerShot funcPlayerShot() const;
	void setFuncPlayerShot(const FuncPlayerShot &newFuncPlayerShot);

	FuncEnemyHit funcEnemyHit() const;
	void setFuncEnemyHit(const FuncEnemyHit &newFuncEnemyHit);

	FuncEnemyShot funcEnemyShot() const;
	void setFuncEnemyShot(const FuncEnemyShot &newFuncEnemyShot);

signals:
	void minimapToggleRequest();
	void questsRequest();
	void gameSuccess();
	void playerDead(RpgPlayer *player);
	void controlledPlayerChanged();
	void playersChanged();
	void gameQuestionChanged();
	void enemyCountChanged();
	void scatterSeriesPlayersChanged();
	void scatterSeriesEnemiesChanged();
	void scatterSeriesPointsChanged();
	void deadEnemyCountChanged();
	void currencyChanged();
	void winnerStreakChanged();
	void questsChanged();

protected:
	RpgPlayer *createPlayer(TiledScene *scene, const RpgPlayerCharacterConfig &config, const int &ownerId);
	RpgEnemy *createEnemy(const RpgGameData::EnemyBaseData::EnemyType &type, const QString &subtype, TiledScene *scene, const int &id);
	RpgEnemy *createEnemy(const RpgGameData::EnemyBaseData::EnemyType &type, TiledScene *scene, const int &id) {
		return createEnemy(type, QString(), scene, id);
	}

	RpgPickableObject *createPickable(const RpgGameData::PickableBaseData::PickableType &type, const QString &name, TiledScene *scene,
									  const int &ownerId = 0, const int &id = 0);
	RpgPickableObject *createPickable(const RpgGameData::PickableBaseData::PickableType &type, TiledScene *scene,
									  const int &ownerId = 0, const int &id = 0) {
		return createPickable(type, QString(), scene, ownerId, id);
	}

	RpgBullet *createBullet(const RpgGameData::Weapon::WeaponType &type,
								  TiledScene *scene, const int &id, const int &ownerId);

	RpgBullet *createBullet(RpgWeapon *weapon, TiledScene *scene, const int &id, const int &ownerId);

	virtual void worldStep(TiledObjectBody *body) override;

	virtual void loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer) override;
	virtual void loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, const QString &groupClass, Tiled::MapRenderer *renderer) override;
	virtual void loadImageLayer(TiledScene *scene, Tiled::ImageLayer *image, Tiled::MapRenderer *renderer) override;
	virtual void joystickStateEvent(const JoystickState &state) override;
	virtual void keyPressEvent(QKeyEvent *event) override final;

	bool transportBeforeEvent(TiledObject *object, TiledTransport *transport) override;
	bool transportAfterEvent(TiledObject *object, TiledScene *newScene, TiledObject *newObject) override;
	bool transportDoor(TiledObject *object, TiledTransport *transport) override;

	virtual void timeSteppedEvent() override;
	virtual void sceneDebugDrawEvent(TiledDebugDraw *debugDraw, TiledScene *scene) override;

	std::optional<QPointF> playerPosition(const int &sceneId, const int &num) const;
	std::optional<QPointF> playerPosition(const TiledScene *scene, const int &num) const {
		return scene ? playerPosition(scene->sceneId(), num) : std::nullopt;
	}
	QList<QPointF> playerPosition(const int &sceneId) const;
	void addPlayerPosition(TiledScene *scene, const QPointF &position);

private:
	void loadMetricDefinition();

	void playerUseContainer(RpgPlayer *player, RpgContainer *container);

	void loadEnemy(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	void loadPickable(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	void addLocationSound(TiledObjectBody *object, const QString &sound,
						  const qreal &baseVolume = 1.,
						  const Sound::ChannelType &channel = Sound::Music2Channel);
	void loadDefaultQuests(const int &questions);

	void onGameQuestionSuccess(const QVariantMap &answer);
	void onGameQuestionFailed(const QVariantMap &answer);
	void onGameQuestionStarted();
	void onGameQuestionFinished();
	int recalculateEnemies();
	void onMarketLoaded();
	void onMarketUnloaded();

	void checkQuests();
	void checkEnemyQuests(const int &count);
	void checkWinnerQuests(const int &count);
	void checkFinalQuests();
	void questSuccess(RpgQuest *quest);

	void updateScatterEnemies();
	void updateScatterPlayers();
	void updateScatterPoints();

	static QVector<RpgGameData::PickableBaseData::PickableType> getPickablesFromPropertyValue(const QString &value);

	struct EnemyData {
		TiledObject::ObjectId objectId;
		RpgGameData::EnemyBaseData::EnemyType type = RpgGameData::EnemyBaseData::EnemyInvalid;
		QString subtype;
		QPolygonF path;
		int defaultAngle = 0;
		QPointer<TiledScene> scene;
		QPointer<IsometricEnemy> enemy;
		[[deprecated]] bool hasQuestion = false;
		bool dieForever = false;
		QVector<RpgGameData::PickableBaseData::PickableType> pickables;
		QVector<RpgGameData::PickableBaseData::PickableType> pickablesOnce;
		QString displayName;
	};


	struct PickableData {
		TiledObject::ObjectId objectId;
		RpgGameData::PickableBaseData::PickableType type = RpgGameData::PickableBaseData::PickableInvalid;
		QString name;
		QPointF position;
		QPointer<TiledScene> scene;
		QPointer<RpgPickableObject> pickableObject;
		QString displayName;
	};


	struct PlayerPosition {
		QPointer<TiledScene> scene;
		QPointF position;

		friend bool operator==(const PlayerPosition &p1, const PlayerPosition &p2) {
			return p1.scene == p2.scene && p1.position == p2.position;
		}
	};


	QVector<EnemyData>::iterator enemyFind(IsometricEnemy *enemy);
	QVector<EnemyData>::const_iterator enemyFind(IsometricEnemy *enemy) const;

	RpgGameDefinition m_gameDefinition;
	QVector<RpgEnemyMetricDefinition> m_metricDefinition;

	QVector<EnemyData> m_enemyDataList;
	QVector<PickableData> m_pickableDataList;
	std::vector<std::unique_ptr<RpgControlGroup>> m_controlGroups;

	QVector<PlayerPosition> m_playerPositionList;

	QList<RpgPlayer*> m_players;
	QPointer<RpgPlayer> m_controlledPlayer;
	QPointer<GameQuestion> m_gameQuestion;
	RpgQuestion *m_rpgQuestion = nullptr;

	int m_enemyCount = 0;
	int m_deadEnemyCount = 0;
	int m_currency = 0;
	int m_winnerStreak = 0;
	int m_lastWinnerStreak = 0;
	int m_baseMp = 150;

	int m_level = 0;

	std::vector<std::unique_ptr<TiledGameSfxLocation>> m_sfxLocations;


	QVector<RpgWallet> m_usedWallet;

	QPointer<QScatterSeries> m_scatterSeriesPlayers;
	QPointer<QScatterSeries> m_scatterSeriesEnemies;
	QPointer<QScatterSeries> m_scatterSeriesPoints;


	FuncBodyStep m_funcBodyStep;
	FuncTimeStep m_funcTimeStep;

	FuncPlayerPick m_funcPlayerPick;
	FuncPlayerAttackEnemy m_funcPlayerAttackEnemy;
	FuncPlayerUseContainer m_funcPlayerUseContainer;
	FuncPlayerUseCast m_funcPlayerUseCast;
	FuncPlayerUseCast m_funcPlayerCastTimeout;
	FuncPlayerUseCast m_funcPlayerFinishCast;
	FuncPlayerHit m_funcPlayerHit;
	FuncPlayerShot m_funcPlayerShot;

	FuncEnemyHit m_funcEnemyHit;
	FuncEnemyShot m_funcEnemyShot;
	FuncEnemyAttackPlayer m_funcEnemyAttackPlayer;

	static QHash<QString, RpgGameDefinition> m_terrains;
	static QHash<QString, RpgPlayerCharacterConfig> m_characters;

	friend class ActionRpgGame;
	friend class ActionRpgMultiplayerGame;
};



#endif // RPGGAME_H
