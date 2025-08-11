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
#include "rpgcontrol.h"
#include "rpgenemy.h"
#include "rpgpickable.h"
#include "rpgplayer.h"
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
	{}

	QS_SERIALIZABLE

	// Base

	QS_FIELD(QString, name)
	QS_FIELD(QString, minVersion)

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




class ActionRpgGame;
class RpgGamePrivate;


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
	Q_PROPERTY(int currency READ currency WRITE setCurrency NOTIFY currencyChanged FINAL)
	Q_PROPERTY(int winnerStreak READ winnerStreak WRITE setWinnerStreak NOTIFY winnerStreakChanged FINAL)
	Q_PROPERTY(QList<RpgQuest> quests READ quests NOTIFY questsChanged FINAL)

	Q_PROPERTY(QScatterSeries *scatterSeriesPlayers READ scatterSeriesPlayers WRITE setScatterSeriesPlayers NOTIFY scatterSeriesPlayersChanged FINAL)
	Q_PROPERTY(QScatterSeries *scatterSeriesEnemies READ scatterSeriesEnemies WRITE setScatterSeriesEnemies NOTIFY scatterSeriesEnemiesChanged FINAL)
	Q_PROPERTY(QScatterSeries *scatterSeriesPoints READ scatterSeriesPoints WRITE setScatterSeriesPoints NOTIFY scatterSeriesPointsChanged FINAL)

public:
	explicit RpgGame(QQuickItem *parent = nullptr);
	virtual ~RpgGame();

	Q_INVOKABLE bool load(const RpgGameDefinition &def, const int &playerCount = 1);

	static const QHash<QString, RpgGameDefinition> &terrains();
	static void reloadTerrains();

	static const QHash<QString, RpgPlayerCharacterConfig> &characters();
	static void reloadCharacters();

	static void reloadWorld();

	static std::optional<RpgGameDefinition> readGameDefinition(const QString &map);

	TiledObjectBody *findBody(const TiledObjectBody::ObjectId &objectId);

	void onPlayerDead(TiledObject *player) override final;
	void onEnemyDead(TiledObject *enemy) override final;
	void onEnemySleepingStart(TiledObject *enemy) override final;
	void onEnemySleepingEnd(TiledObject *enemy) override final;

	bool playerTryUseControl(RpgPlayer *player, RpgActiveIface *control);

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

	static QRect loadTextureSprites(TiledSpriteHandler *handler, const QString &path,
									QHash<QString, RpgArmory::LayerData> *layerPtr = nullptr);


	static RpgGameData::Inventory getInventoryFromPropertyValue(const QString &value);


	static const QVector<TextureSpriteMapper> &baseEntitySprite();

	static QString getAttackSprite(const RpgGameData::Weapon::WeaponType &weaponType);
	static RpgEnemyMetricDefinition defaultEnemyMetric();

	Q_INVOKABLE virtual void onMouseClick(const qreal &x, const qreal &y, const int &buttons, const int &modifiers) override;

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

	QScatterSeries *scatterSeriesPlayers() const;
	void setScatterSeriesPlayers(QScatterSeries *newScatterSeriesPlayers);

	QScatterSeries *scatterSeriesEnemies() const;
	void setScatterSeriesEnemies(QScatterSeries *newScatterSeriesEnemies);

	QScatterSeries *scatterSeriesPoints() const;
	void setScatterSeriesPoints(QScatterSeries *newScatterSeriesPoints);

	int currency() const;
	void setCurrency(int newCurrency);

	int winnerStreak() const;
	void setWinnerStreak(int newWinnerStreak);

	const QList<RpgQuest> &quests() const;

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgControlBase, T>::value>::type>
	T* controlFind(const T2 &baseData) const;

	template <typename T,
			  typename = std::enable_if<std::is_base_of<RpgControlBase, T>::value>::type,
			  class... Args>
	T* controlAdd(Args&& ...);

	void controlRemove(RpgControlBase *control);
	void controlRemove(const QList<RpgControlBase *> &controls);

	void controlAppeared(RpgActiveIface *iface);


	RpgCollectionData getCollectionImageData(const int &id) const;

	int getMetric(const RpgPlayerCharacterConfig::CastType &cast) const;
	EnemyMetric getMetric(EnemyMetric baseMetric, const RpgGameData::EnemyBaseData::EnemyType &type, const QString &subtype = QString());

	ActionRpgGame *actionRpgGame() const;
	void setActionRpgGame(ActionRpgGame *game);

	void playerAttack(RpgPlayer *player, RpgWeapon *weapon, const std::optional<QPointF> &dest);
	void playerUseCurrentControl(RpgPlayer *player);
	void playerExitHiding(RpgPlayer *player);

	bool playerShot(RpgPlayer *player, RpgWeapon *weapon, const qreal &angle);
	bool playerHit(RpgPlayer *player, RpgEnemy *enemy, RpgWeapon *weapon);
	bool playerAttackEnemy(RpgPlayer *player, RpgEnemy *enemy,
						   const RpgGameData::Weapon::WeaponType &weaponType, const int &weaponSubtype);
	bool enemyHit(RpgEnemy *enemy, RpgPlayer *player, RpgWeapon *weapon);
	bool enemyShot(RpgEnemy *enemy, RpgWeapon *weapon, const qreal &angle);
	bool enemyAttackPlayer(RpgEnemy *enemy, RpgPlayer *player,
						   const RpgGameData::Weapon::WeaponType &weaponType, const int &weaponSubtype);
	bool bulletImpact(RpgBullet *bullet, TiledObjectBody *other);

signals:
	void minimapToggleRequest();
	void questsRequest();
	void gameSuccess();
	void playerDead(RpgPlayer *player);
	void controlledPlayerChanged();
	void playersChanged();
	void gameQuestionChanged();
	void scatterSeriesPlayersChanged();
	void scatterSeriesEnemiesChanged();
	void scatterSeriesPointsChanged();
	void currencyChanged();
	void winnerStreakChanged();
	void questsChanged();

protected:
	RpgPlayer *createPlayer(TiledScene *scene, const RpgPlayerCharacterConfig &config, const int &ownerId,
							const bool &isDynamic = true);
	RpgEnemy *createEnemy(const RpgGameData::EnemyBaseData::EnemyType &type, const QString &subtype, TiledScene *scene, const int &id);
	RpgEnemy *createEnemy(const RpgGameData::EnemyBaseData::EnemyType &type, TiledScene *scene, const int &id) {
		return createEnemy(type, QString(), scene, id);
	}

	RpgBullet *createBullet(const RpgGameData::Weapon::WeaponType &type,
							TiledScene *scene, const int &id, const int &ownerId, const bool &isDynamic);

	RpgBullet *createBullet(RpgWeapon *weapon, TiledScene *scene, const int &id, const int &ownerId, const bool &isDynamic);

	QList<RpgPickable*> extractEnemyInventory(RpgEnemy *enemy);


	void updateRandomizer(const RpgGameData::Randomizer &randomizer);

	virtual void onShapeAboutToDeletePrivate(cpShape *shape) override;

	virtual void loadTileLayer(TiledScene *scene, Tiled::TileLayer *layer, Tiled::MapRenderer *renderer) override;
	virtual void loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer) override;
	virtual bool loadObjectLayer(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer) override;
	virtual void loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, const QString &groupClass, Tiled::MapRenderer *renderer) override;
	virtual void loadImageLayer(TiledScene *scene, Tiled::ImageLayer *image, Tiled::MapRenderer *renderer) override;
	virtual void joystickStateEvent(const JoystickState &state) override;
	virtual void keyPressEvent(QKeyEvent *event) override final;
	virtual bool loadLights(TiledScene *scene, const QList<Tiled::MapObject *> &objects, Tiled::MapRenderer *renderer) override;

	virtual void timeStepPrepareEvent() override;
	virtual void timeBeforeWorldStepEvent(const qint64 &tick) override;
	virtual void worldStep(TiledObjectBody *body) override;
	virtual void worldStep() override;
	virtual void timeAfterWorldStepEvent(const qint64 &tick) override;
	virtual void timeSteppedEvent() override;
	virtual void sceneDebugDrawEvent(TiledDebugDraw *debugDraw, TiledScene *scene) override;

	QList<RpgGameData::PlayerPosition> playerPositions() const;
	QList<QPointF> playerPositions(const int &sceneId) const;
	const RpgGameData::Collection &collection() const;
	RpgGameData::Collection &collection();
	RpgGameData::Randomizer randomizer() const;


private:
	void loadMetricDefinition();

	void addPlayerPosition(TiledScene *scene, const QPointF &position);
	void addCollection(TiledScene *scene, Tiled::GroupLayer *groupLayer, Tiled::MapRenderer *renderer);
	void addCollection(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer);

	void playerUseControl(RpgPlayer *player, RpgActiveIface *control, const bool &success);

	void loadEnemy(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	void addLocationSound(TiledObjectBody *object, const QString &sound,
						  const qreal &baseVolume = 1.,
						  const Sound::ChannelType &channel = Sound::Music2Channel);


	void onGameQuestionSuccess(const QVariantMap &answer);
	void onGameQuestionFailed(const QVariantMap &answer);
	void onGameQuestionStarted();
	void onGameQuestionFinished();
	void onMarketLoaded();
	void onMarketUnloaded();

	void updateScatterEnemies();
	void updateScatterPlayers();
	void updateScatterPoints();

	struct EnemyData {
		TiledObject::ObjectId objectId;
		RpgGameData::EnemyBaseData::EnemyType type = RpgGameData::EnemyBaseData::EnemyInvalid;
		QString subtype;
		EnemyMotorData motor;
		int defaultAngle = 0;
		QPointer<TiledScene> scene;
		QPointer<IsometricEnemy> enemy;
		bool dieForever = false;
		RpgGameData::Inventory inventory;
		QString displayName;
	};

	RpgGamePrivate *q = nullptr;

	QVector<EnemyData>::iterator enemyFind(IsometricEnemy *enemy);
	QVector<EnemyData>::const_iterator enemyFind(IsometricEnemy *enemy) const;

	RpgGameDefinition m_gameDefinition;
	QVector<RpgEnemyMetricDefinition> m_metricDefinition;

	QVector<EnemyData> m_enemyDataList;

	std::vector<std::unique_ptr<RpgControlBase>> m_controls;

	QList<RpgPlayer*> m_players;
	QPointer<RpgPlayer> m_controlledPlayer;
	QPointer<GameQuestion> m_gameQuestion;
	RpgQuestion *m_rpgQuestion = nullptr;

	int m_currency = 0;
	int m_winnerStreak = 0;
	int m_baseMp = 150;

	int m_level = 0;

	std::vector<std::unique_ptr<TiledGameSfxLocation>> m_sfxLocations;


	QVector<RpgWallet> m_usedWallet;

	QPointer<QScatterSeries> m_scatterSeriesPlayers;
	QPointer<QScatterSeries> m_scatterSeriesEnemies;
	QPointer<QScatterSeries> m_scatterSeriesPoints;


	static QHash<QString, RpgGameDefinition> m_terrains;
	static QHash<QString, RpgPlayerCharacterConfig> m_characters;

	friend class ActionRpgGame;
	friend class ActionRpgMultiplayerGame;
	friend class RpgGamePrivate;
};






/**
 * @brief RpgGame::controlFind
 * @param baseData
 * @return
 */

template<typename T, typename T2, typename T3>
inline T *RpgGame::controlFind(const T2 &baseData) const
{
	T *r = nullptr;

	for (const auto &ptr : m_controls) {
		if (T *c = dynamic_cast<T*>(ptr.get());
				c && c->baseData().isBaseEqual(baseData)) {
			r = c;
			break;
		}
	}
	return r;
}



/**
 * @brief RpgGame::createControl
 * @return
 */

template<typename T, typename T2, class ...Args>
inline T *RpgGame::controlAdd(Args &&...args)
{
	m_controls.emplace_back(new T(std::forward<Args>(args)...));
	return dynamic_cast<T*>(m_controls.back().get());

}


#endif // RPGGAME_H
