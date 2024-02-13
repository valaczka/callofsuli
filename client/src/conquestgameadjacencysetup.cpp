/*
 * ---- Call of Suli ----
 *
 * conquestgameadjacencysetup.cpp
 *
 * Created on: 2024. 02. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestGameAdjacencySetup
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

#include "conquestgameadjacencysetup.h"
#include "utils_.h"

/**
 * @brief ConquestGameAdjacencySetup::ConquestGameAdjacencySetup
 * @param client
 */

ConquestGameAdjacencySetup::ConquestGameAdjacencySetup(Client *client)
	: ConquestGame(nullptr, client)
{

}


/**
 * @brief ConquestGameAdjacencySetup::loadFromFile
 * @param filename
 */

void ConquestGameAdjacencySetup::loadFromFile(const QString &world)
{
	LOG_CINFO("game") << "Adjacency setup for world:" << world;
	ConquestConfig c;
	c.world.name = world;
	c.gameState = ConquestConfig::StatePrepare;
	setConfig(c);

	if (!m_matrix.isEmpty()) {
		const auto &k = m_matrix.constBegin();
		setCurrentLandId(k.key());
	}
}



/**
 * @brief ConquestGameAdjacencySetup::adjacencyAdd
 * @param id
 */

void ConquestGameAdjacencySetup::adjacencyAdd(const QString &id)
{
	if (!m_matrix.contains(m_currentLandId))
		m_matrix.insert(m_currentLandId, {id});
	else if (QStringList &list = m_matrix[m_currentLandId]; !list.contains(id))
		list.append(id);

	if (!m_matrix.contains(id))
		m_matrix.insert(id, {m_currentLandId});
	else if (QStringList &list = m_matrix[id]; !list.contains(m_currentLandId))
		list.append(m_currentLandId);

	emit currentAdjacencyChanged();
}


/**
 * @brief ConquestGameAdjacencySetup::adjacencyRemove
 * @param id
 */

void ConquestGameAdjacencySetup::adjacencyRemove(const QString &id)
{
	if (m_matrix.contains(m_currentLandId))
		m_matrix[m_currentLandId].removeAll(id);

	if (m_matrix.contains(id))
		m_matrix[id].removeAll(m_currentLandId);

	emit currentAdjacencyChanged();
}


/**
 * @brief ConquestGameAdjacencySetup::adjacencyToggle
 * @param id
 */

void ConquestGameAdjacencySetup::adjacencyToggle(const QString &id)
{
	if (m_matrix.contains(m_currentLandId) && m_matrix.value(m_currentLandId).contains(id))
		adjacencyRemove(id);
	else
		adjacencyAdd(id);
}


/**
 * @brief ConquestGameAdjacencySetup::save
 */

void ConquestGameAdjacencySetup::save() const
{
	QJsonObject obj;

	for (auto it = m_matrix.constBegin(); it != m_matrix.constEnd(); ++it) {
		obj.insert(it.key(), QJsonArray::fromStringList(it.value()));
	}

	static const QString file = QStringLiteral("/tmp/_output.json");

	Utils::jsonObjectToFile(obj, file);

	m_client->snack(tr("Mentés: ").append(file));
}



/**
 * @brief ConquestGameAdjacencySetup::currentLandId
 * @return
 */

QString ConquestGameAdjacencySetup::currentLandId() const
{
	return m_currentLandId;
}

void ConquestGameAdjacencySetup::setCurrentLandId(const QString &newCurrentLandId)
{
	if (m_currentLandId == newCurrentLandId)
		return;
	m_currentLandId = newCurrentLandId;
	emit currentLandIdChanged();
	emit currentAdjacencyChanged();
}


/**
 * @brief ConquestGameAdjacencySetup::currentAdjacency
 * @return
 */

QStringList ConquestGameAdjacencySetup::currentAdjacency() const
{
	return m_matrix.value(m_currentLandId);
}


/**
 * @brief ConquestGameAdjacencySetup::jsonOrigDataCheck
 * @param obj
 */

void ConquestGameAdjacencySetup::jsonOrigDataCheck(const QJsonObject &obj)
{
	LOG_CDEBUG("game") << "Load adjacency";

	const QJsonObject &data = obj.value(QStringLiteral("adjacency")).toObject();

	ConquestWorldHelper::adjacencyToMatrix(data, &m_matrix);
}
