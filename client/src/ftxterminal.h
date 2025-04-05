/*
 * ---- Call of Suli ----
 *
 * terminal.h
 *
 * Created on: 2025. 04. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Terminal
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

#ifndef FTXTERMINAL_H
#define FTXTERMINAL_H

#include <QString>
#include "qeventloop.h"
#include "qlocalsocket.h"


#include "ftxui/component/loop.hpp"

/**
 * @brief The FtxLoop class
 */

class FtxLoop
{

public:
	FtxLoop() = default;

	virtual ftxui::Component createComponent(ftxui::ScreenInteractive *screen) = 0;

	virtual bool runOnce(QLocalSocket &localSocket);

protected:
	virtual bool runCbor(const QCborValue &cbor) { Q_UNUSED(cbor); return true; }
	virtual void update() { updateScreen(); }
	virtual std::optional<QCborValue> getCbor(QLocalSocket &localSocket);

	void updateScreen();

	ftxui::ScreenInteractive *m_screen = nullptr;
};




/**
 * @brief The FtxTerminal class
 */

class FtxTerminal
{

public:
	FtxTerminal() = default;

	int run(FtxLoop &loop, const QString &serverName);
	int run(const QString &serverName);

	static void msgError(const QString &str);

private:
	bool connect(const QString &serverName, const int msecs = 5000);

	QEventLoop m_eventLoop;
	QLocalSocket m_socket;

};

#endif // FTXTERMINAL_H
