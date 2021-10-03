/*
 * ---- Call of Suli ----
 *
 * profile.cpp
 *
 * Created on: 2021. 10. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * profile
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

#include "profile.h"



Profile::Profile(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassStudent, parent)
	, m_characterList()
{
	m_characterList.append(QVariantMap({{"dir", "default"}, {"name", "Default"}}));
}


/**
 * @brief Profile::~Profile
 */

Profile::~Profile()
{

}

void Profile::setCharacterList(QVariantList characterList)
{
	if (m_characterList == characterList)
		return;

	m_characterList = characterList;
	emit characterListChanged(m_characterList);
}


/**
 * @brief Profile::findCharacter
 * @param name
 * @return
 */

int Profile::findCharacter(const QString &name)
{
	for (int i=0; i<m_characterList.size(); i++) {
		QVariantMap m = m_characterList.at(i).toMap();

		if (m.value("dir").toString() == name)
			return i;
	}

	return 0;
}


/**
 * @brief Profile::clientSetup
 */

void Profile::clientSetup()
{
	if (!m_client)
		return;

	if (m_client->userRoles().testFlag(CosMessage::RoleTeacher))
		setDefaultClass(CosMessage::ClassTeacher);

	setCharacterList(Client::mapToList(m_client->characterData(), "dir"));
}
