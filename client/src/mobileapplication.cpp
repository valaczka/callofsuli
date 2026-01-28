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

#include <enet/enet.h>
#include "mobileapplication.h"
#include "standaloneclient.h"
#include "Logger.h"
#include "mobileutils.h"

#include <sodium.h>
#include "keychain.h"


#define KEYCHAIN_DOMAIN			QStringLiteral("hu.piarista.vjp.callofsuli")
#define KEYCHAIN_KEY_DEVICE		QStringLiteral("deviceSeed")

/**
 * @brief MobileApplication::MobileApplication
 * @param argc
 * @param argv
 */


MobileApplication::MobileApplication(QApplication *app)
	: Application(app)
{

}


/**
 * @brief MobileApplication::~MobileApplication
 */

MobileApplication::~MobileApplication()
{
	enet_deinitialize();
}


/**
 * @brief MobileApplication::initialize
 */

void MobileApplication::initialize()
{
	qmlRegisterUncreatableType<Sound>("CallOfSuli", 1, 0, "Sound", "Sound is uncreatable");

	qmlRegisterType<Server>("CallOfSuli", 1, 0, "Server");

	createStandardPath();

	if (enet_initialize() != 0) {
		LOG_CERROR("app") << "ENet initialize failed";
	} else {
		LOG_CDEBUG("app") << "ENet initialized";
	}
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
	return new StandaloneClient(this);
}



/**
 * @brief MobileApplication::getDeviceIdentityPlatform
 * @return
 */

QByteArray MobileApplication::getDeviceIdentityPlatform() const
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
	return MobileUtils::instance()->getApkSigningCertSha256();
#endif

	return {};
}


/**
 * @brief MobileApplication::getDeviceKeyPlatform
 * @return
 */

bool MobileApplication::getDeviceKeyPlatform()
{
	return deviceKeyRead();
}



/**
 * @brief MobileApplication::deviceKeyRead
 * @return
 */

bool MobileApplication::deviceKeyRead()
{
	static int tries = 0;

	if (tries++ > 20) {
		LOG_CERROR("app") << "Keychain read exceeds maximum tries";
		return false;
	}

	QKeychain::ReadPasswordJob *job = new QKeychain::ReadPasswordJob(KEYCHAIN_DOMAIN, m_application);

	job->setKey(KEYCHAIN_KEY_DEVICE);

	LOG_CDEBUG("app") << "Read device key";

	QObject::connect(job, &QKeychain::ReadPasswordJob::finished, m_application, [this](QKeychain::Job *job){
		if (job->error()) {
			LOG_CWARNING("app") << "Keychain error:" << job->errorString();

			deviceKeyCreate();
			return;
		}

		QKeychain::ReadPasswordJob *pjob = qobject_cast<QKeychain::ReadPasswordJob*>(job);

		Q_ASSERT(pjob);

		QByteArray key = pjob->binaryData();

		if (key.size() != (int) m_device.seed.size()) {
			LOG_CWARNING("app") << "Device seed length mismatch" << key.size();

			deviceKeyCreate();
			return;
		}

		m_device.moveToArray(key, &m_device.seed);

		QMutexLocker locker(&m_device.mutex);

		if (crypto_sign_seed_keypair(m_device.publicKey.data(), m_device.privateKey.data(), m_device.seed.data()) != 0) {
			LOG_CERROR("app") << "Keypair generation failed";
			return;
		}

	});

	job->start();

	return true;
}




/**
 * @brief MobileApplication::deviceKeyCreate
 */

void MobileApplication::deviceKeyCreate()
{
	QKeychain::WritePasswordJob *job = new QKeychain::WritePasswordJob(KEYCHAIN_DOMAIN, m_application);

	job->setKey(KEYCHAIN_KEY_DEVICE);

	QByteArray seed(crypto_sign_SEEDBYTES, Qt::Uninitialized);

	randombytes_buf(seed.data(), seed.size());

	job->setBinaryData(seed);

	LOG_CDEBUG("app") << "Create new device key";

	QObject::connect(job, &QKeychain::WritePasswordJob::finished, m_application, [this](QKeychain::Job *job){
		if (job->error()) {
			LOG_CERROR("app") << "Keychain write error:" << job->errorString();
			return;
		}

		deviceKeyRead();
	});

	job->start();

}
