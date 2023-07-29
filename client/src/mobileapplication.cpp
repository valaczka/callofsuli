/*
 * ---- Call of Suli ----
 *
 * mobileapplication.cpp
 *
 * Created on: 2023. 01. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MobileApplication
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

#include "mobileapplication.h"
#include "standaloneclient.h"
#include "Logger.h"


/**
 * @brief MobileApplication::MobileApplication
 * @param argc
 * @param argv
 */

MobileApplication::MobileApplication(int &argc, char **argv)
	: Application(argc, argv)
{

}


/**
 * @brief MobileApplication::~MobileApplication
 */

MobileApplication::~MobileApplication()
{

}


/**
 * @brief MobileApplication::initialize
 */

void MobileApplication::initialize()
{
	qRegisterMetaType<Sound::SoundType>("SoundType");
	qmlRegisterUncreatableType<Sound>("CallOfSuli", 1, 0, "Sound", "Sound is uncreatable");

	qmlRegisterType<Server>("CallOfSuli", 1, 0, "Server");

	createStandardPath();
}


/**
 * @brief MobileApplication::createStandardPath
 */

void MobileApplication::createStandardPath()
{
	QDir d(Utils::standardPath());

	if (!d.exists()) {
		LOG_CDEBUG("app") << "Create directory:" << qPrintable(d.absolutePath());
		d.mkpath(d.absolutePath());
	} else {
		LOG_CDEBUG("app") << "Standard path:" << qPrintable(d.absolutePath());
	}
}



/**
 * @brief MobileApplication::createClient
 * @return
 */

Client *MobileApplication::createClient()
{
	return new StandaloneClient(this, m_application);
}
