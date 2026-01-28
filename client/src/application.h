/*
 * ---- Call of Suli ----
 *
 * baseapplication.h
 *
 * Created on: 2022. 12. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * BaseApplication
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

#ifndef APPLICATION_H
#define APPLICATION_H

#include "client.h"
#include <QApplication>
#include <QtQml>
#include <sodium.h>
#include "../modules/interfaces.h"


/**
 * @brief The Application class
 */

class Application
{
public:
	Application(QApplication *app);
	virtual ~Application();

	enum CommandLine {
		Normal,
		License,
		Editor,
		Map,
		Play,
		Demo,
		DevPage,
		Adjacency [[deprecated]],
		Terminal
	};


	struct Device {
		static const QString userAgent;

#ifndef Q_OS_WASM
		mutable QMutex mutex;
#endif

		std::array<unsigned char, crypto_sign_SEEDBYTES> seed;
		std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> publicKey;
		std::array<unsigned char, crypto_sign_SECRETKEYBYTES> privateKey;

		mutable QByteArray identity;
		mutable QByteArray identitySignature;

		template <std::size_t N>
		void moveToArray(QByteArray& src, std::array<unsigned char, N> *dst)
		{
			Q_ASSERT(dst);

			if (src.size() != static_cast<int>(N)) {
				throw std::runtime_error("Invalid secret length");
			}

#ifndef Q_OS_WASM
			QMutexLocker locker(&mutex);
#endif

			std::memcpy(dst->data(), src.constData(), N);

			sodium_memzero(src.data(), src.size());
			src.clear();
		}
	};

	int run();

	static const QVersionNumber version();
	static bool debug();

	static void initialize();

	QApplication *application() const;
	QQmlApplicationEngine *engine() const;
	Client *client() const;

	static Application *instance();

	void messageInfo(const QString &text, const QString &title = "") const;
	void messageWarning(const QString &text, const QString &title = "") const;
	void messageError(const QString &text, const QString &title = "") const;

	const QHash<QString, ModuleInterface *> &objectiveModules() const;
	const QHash<QString, ModuleInterface *> &storageModules() const;

	const CommandLine &commandLine() const;
	const QString &commandLineData() const;

	void selectUrl(const QUrl &url);

	static const QString &userAgent();
	QByteArray userAgentSign(const QByteArray &content,
							 const QByteArray &sessionId);

	std::optional<std::pair<QByteArray, QByteArray> > deviceIdentity() const;
	void setOnDeviceIdentityReady(QObject *instance, const std::function<void(bool)> &func, const bool &startTimer = true);

protected:
	virtual bool loadMainQml();
	virtual bool loadResources();
	virtual Client *createClient() = 0;
	virtual QByteArray getDeviceIdentityPlatform() const = 0;
	virtual bool getDeviceKeyPlatform() = 0;

	void registerQmlTypes();
	void loadFonts();
	void loadQaterial();
	void loadModules();
	void loadDeviceIdentity();

	friend class Client;
	friend class OnlineClient;

	CommandLine m_commandLine = Normal;
	QString m_commandLineData;
	QApplication *const m_application;
	std::unique_ptr<QQmlApplicationEngine> m_engine;
	std::unique_ptr<Client> m_client;
	static Application *m_instance;
	static const bool m_debug;

	QHash<QString, ModuleInterface*> m_objectiveModules;
	QHash<QString, ModuleInterface*> m_storageModules;

	Device m_device;

private:
	QTimer m_deviceIdentityTimer;
	std::function<void(bool)> m_deviceIdentityFunc;
	QObject *m_deviceIdentityInst = nullptr;
};

#endif // APPLICATION_H
