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
#include "Logger.h"
#include "onlineclient.h"

/**
 * @brief OnlineApplication::OnlineApplication
 * @param argc
 * @param argv
 */

OnlineApplication::OnlineApplication(QApplication *app)
	: Application(app)
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
	LOG_CTRACE("app") << "Create online client";
	return new OnlineClient(this);
}



/**
 * @brief OnlineApplication::getDeviceIdentityPlatform
 * @return
 */

QByteArray OnlineApplication::getDeviceIdentityPlatform() const
{
	return QByteArrayLiteral("-");
}


/**
 * @brief OnlineApplication::getDeviceKeyPlatform
 * @return
 */

bool OnlineApplication::getDeviceKeyPlatform()
{
	QSettings s;
	QByteArray key = s.value(QStringLiteral("deviceSeed")).toByteArray();

	if (key.size() != (int) m_device.seed.size()) {
		LOG_CWARNING("app") << "Device seed length mismatch" << key.size();

		QByteArray k(crypto_sign_SEEDBYTES, Qt::Uninitialized);


		// Ez valamiért nem működött...
		///randombytes_buf(key.data(), key.size());

		for (int i=0; i<k.size(); ++i)
			k.data()[i] = QRandomGenerator::global()->bounded(256);

		key = k;

		LOG_CDEBUG("app") << "Create new device key" << key.size();



		s.setValue(QStringLiteral("deviceSeed"), key);
		s.sync();
	}

	m_device.moveToArray(key, &m_device.seed);

	std::array<unsigned char, crypto_sign_SEEDBYTES> keySeed = m_device.deriveFromSeed<crypto_sign_SEEDBYTES>(1, "PRIV_KEY");

	if (crypto_sign_seed_keypair(m_device.publicKey.data(), m_device.privateKey.data(), keySeed.data()) != 0) {
		LOG_CERROR("app") << "Keypair generation failed";
		return false;
	}

	return true;
}
