/*
 * ---- Call of Suli ----
 *
 * onlineapplication.cpp
 *
 * Created on: 2022. 12. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OnlineApplication
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

#include "onlineapplication.h"
#include "onlineclient.h"

/**
 * @brief OnlineApplication::OnlineApplication
 * @param argc
 * @param argv
 */

OnlineApplication::OnlineApplication(int &argc, char **argv)
	: Application(argc, argv)
{

}


/**
 * @brief OnlineApplication::~OnlineApplication
 */

OnlineApplication::~OnlineApplication()
{

}



/**
 * @brief OnlineApplication::loadResources
 * @return
 */

bool OnlineApplication::loadResources()
{
	return true;
}



bool OnlineApplication::loadMainQml()
{
	const QUrl url(QStringLiteral("qrc:/main.qml"));
	m_engine->load(url);

	return true;
}

/**
 * @brief OnlineApplication::createClient
 * @return
 */

Client *OnlineApplication::createClient()
{
	qCDebug(lcApp).noquote() << QObject::tr("Create online client");
	return new OnlineClient(this, m_application);
}


