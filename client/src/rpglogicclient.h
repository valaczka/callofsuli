/*
 * ---- Call of Suli ----
 *
 * rpglogicclient.h
 *
 * Created on: 2026. 05. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLogicClient
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

#ifndef RPGLOGICCLIENT_H
#define RPGLOGICCLIENT_H

#include <rpglogic.h>

namespace Rpg {


/**
 * @brief The RpgEntityStatePull class
 */

template <typename T, std::size_t PULL_SIZE>
class RpgEntityStatePull
{
public:
	RpgEntityStatePull() = default;

	void reset() { m_head = 0; }
	void append(const T &content) {
		if (m_head > 1 && m_list[(m_head-1) % PULL_SIZE] == content)
			return;

		m_list[m_head % PULL_SIZE] = content;
		++m_head;
	}
	void append(T &&content) {
		if (m_head > 1 && m_list[(m_head-1) % PULL_SIZE] == content)
			return;

		m_list[m_head % PULL_SIZE] = std::move(content);
		++m_head;
	}

	bool extract(T &origPtr, std::vector<T> &listPtr) {
		if (m_head == 0)
			return false;

		const quint32 from = (m_head > PULL_SIZE ? m_head-PULL_SIZE : 0);

		origPtr = m_list[from % PULL_SIZE];
		listPtr.clear();
		listPtr.reserve(PULL_SIZE);

		for (quint32 i=from+1; i<m_head; ++i) {
			listPtr.push_back(m_list[i % PULL_SIZE]);
		}

		return true;
	}

protected:
	std::array<T, PULL_SIZE> m_list;
	quint32 m_head = 0;
};



typedef RpgEntityStatePull<RpgStream::PlayerState, 10> RpgPlayerStatePull;


class RpgLogicClient : public RpgLogic
{
public:
	RpgLogicClient();
};

}		// end of namespace

#endif // RPGLOGICCLIENT_H
