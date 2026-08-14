/*
 * ---- Call of Suli ----
 *
 * rpglogicserver.h
 *
 * Created on: 2026. 06. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLogicServer
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

#ifndef RPGLOGICSERVER_H
#define RPGLOGICSERVER_H

#include <rpglogic.h>

class RpgEngine;



/**
 * @brief The RpgLogicServerConfig class
 */

class RpgLogicServerConfig
{
public:
	RpgLogicServerConfig() = default;

	void loadCharacterList(const RpgCharacterList &newCharacterList);

	const QHash<QString, RpgServerCharacter> &characters() const;
	const RpgStream::HashFnv1A64 &characterHash() const;

private:
	QHash<QString, RpgServerCharacter> m_characters;
	RpgStream::HashFnv1A64 m_characterHash;
};



/**
 * @brief The RpgLogicServer class
 */

class RpgLogicServer : public Rpg::RpgLogic
{
public:
	RpgLogicServer(RpgEngine *engine);

	RpgEngine *engine() const { return m_engine; }

	RpgStream::Full getRenderedState(const bool &isStageSelect);

	static QList<RpgQuestData> singlePlayerQuests();

protected:
	virtual void eventRealized(entt::entity entity) override;
	virtual void onNpcCreated(entt::entity entity, const quint32 &idTag, Rpg::Player *player) override;
	virtual std::vector<Rpg::Chest> initializeChests() override;
	virtual void checkState(const RpgStream::GameState &state) override;
	virtual Rpg::QuestList getQuestList() const override;
	virtual RpgStream::Result getResult() override;

	bool onStageChanged(const RpgStream::GameConfig::Stage &stage);

private:
	Logger *_logger() const;

	RpgEngine *const m_engine;
	std::map<int, int> m_heatSteps;
};

#endif // RPGLOGICSERVER_H
