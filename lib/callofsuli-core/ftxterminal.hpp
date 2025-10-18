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
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/loop.hpp"
#include "qcborvalue.h"
#include "qcoreapplication.h"
#include "qelapsedtimer.h"
#include "qeventloop.h"
#include "qlocalserver.h"
#include "qlocalsocket.h"



class FtxServer
{
public:
	FtxServer() = default;
	~FtxServer();

	bool start(QObject *parent, const QString &name);
	void writeToSocket(const QCborValue &cbor);

protected:
	QLocalServer m_localServer;
	QList<QLocalSocket*> m_localSockets;

};




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
	QElapsedTimer m_lastUpdate;
};




/**
 * @brief The FtxTerminal class
 */

class FtxTerminal
{

public:
	FtxTerminal() = default;

	int run(FtxLoop &loop, const QString &serverName);

	static void msgError(const QString &str);

private:
	bool connect(const QString &serverName, const int msecs = 5000);

	QEventLoop m_eventLoop;
	QLocalSocket m_socket;

};





/**
 * @brief The ToggleButton class
 */

class ToggleButton
{
public:
	ToggleButton(const QString &text, const bool &checked = false)
	{
		using namespace ftxui;

		m_checked = checked;
		m_button = Button(text.toStdString(), [this]() { m_checked = !m_checked; });
		m_renderer = ftxui::Renderer(m_button, [this]() {
			if (m_checked) {
				auto cmp = m_button->Render();
				cmp |= bold;

				if (m_bgColor != Color::Default)
					cmp |= bgcolor(m_bgColor);

				if (m_color != Color::Default)
					cmp |= ftxui::color(m_color);

				return cmp;
			} else
				return m_button->Render();
		});
	}

	const ftxui::Component &renderer() { return m_renderer; }
	const ftxui::Component &button() { return m_button; }


	bool checked() const { return m_checked; }
	void setChecked(bool newChecked) { m_checked = newChecked; }

	ftxui::Color color() const { return m_color; }
	void setColor(const ftxui::Color &newColor) { m_color = newColor; }

	ftxui::Color bgColor() const { return m_bgColor; }
	void setBgColor(const ftxui::Color &newBgColor) { m_bgColor = newBgColor; }

private:
	bool m_checked = false;
	ftxui::Component m_button;
	ftxui::Component m_renderer;
	ftxui::Color m_color = ftxui::Color::Default;
	ftxui::Color m_bgColor = ftxui::Color::Default;

};




/**
 * @brief The ScrollText class
 */

class ScrollText : public ftxui::ComponentBase
{
public:
	ScrollText(const QString &title, const QString &text = {})
		: ComponentBase()
		, m_title(title)
		, m_text(text)
	{

	}

	bool Focusable() const override { return true; }

	ftxui::Element OnRender() override {
		using namespace ftxui;

		Element pg = paragraph(m_text.toStdString()) | focusPosition(m_posX, m_posY) | vscroll_indicator | frame;

		if (Focused() && m_focusTextColor != Color::Default)
			pg |= color(m_focusTextColor);
		else if (m_textColor != Color::Default)
			pg |= color(m_textColor);

		Element w = window(ftxui::text(m_title.toStdString()), pg);

		if (Focused() && m_focusBorderColor != Color::Default)
			w |= color(m_focusBorderColor);
		else if (m_borderColor != Color::Default)
			w |= color(m_borderColor);

		return w;
	}


	bool OnEvent (ftxui::Event event) override {
		using namespace ftxui;

		if (!Focused())
			return false;

		if (event.mouse().button == Mouse::WheelDown || event == Event::PageDown) {
			addPosY(10);
		} else if (event.mouse().button == Mouse::WheelUp  || event == Event::PageUp) {
			addPosY(-10);
		} else if (event == Event::ArrowDown) {
			addPosY(1);
		} else if (event == Event::ArrowUp) {
			addPosY(-1);
		} else {
			return false;
		}

		return true;
	};


	void addPosX(const int &delta) {
		m_posX = std::max(0, m_posX+delta);
	}

	void addPosY(const int &delta) {
		const int n = m_text.count('\n');
		m_posY = std::max(0, std::min(m_posY+delta, n));
	}


	QString title() const { return m_title; }
	void setTitle(const QString &newTitle) { m_title = newTitle; }

	QString text() const { return m_text; }
	void setText(const QString &newText) { m_text = newText; }

	ftxui::Color borderColor() const { return m_borderColor; }
	void setBorderColor(const ftxui::Color &newBorderColor) { m_borderColor = newBorderColor; }

	ftxui::Color textColor() const { return m_textColor; }
	void setTextColor(const ftxui::Color &newTextColor) { m_textColor = newTextColor; }

	ftxui::Color focusBorderColor() const { return m_focusBorderColor; }
	void setFocusBorderColor(const ftxui::Color &newFocusBorderColor) { m_focusBorderColor = newFocusBorderColor; }

	ftxui::Color focusTextColor() const { return m_focusTextColor; }
	void setFocusTextColor(const ftxui::Color &newFocusTextColor) { m_focusTextColor = newFocusTextColor; }

private:
	QString m_title;
	QString m_text;

	ftxui::Color m_borderColor = ftxui::Color::Default;
	ftxui::Color m_textColor = ftxui::Color::Default;
	ftxui::Color m_focusBorderColor = ftxui::Color::Default;
	ftxui::Color m_focusTextColor = ftxui::Color::Default;

	int m_posX = 0;
	int m_posY = 0;
};







/// ----------------------------------------------------------------------- ///





/**
 * @brief FtxServer::~FtxServer
 */

inline FtxServer::~FtxServer()
{

}



/**
 * @brief FtxServer::start
 * @param parent
 * @param name
 * @return
 */

inline bool FtxServer::start(QObject *parent, const QString &name)
{
	Q_ASSERT(parent);

	QObject::connect(qApp, &QCoreApplication::aboutToQuit, parent, [this](){
		for (QLocalSocket *s : m_localSockets) {
			s->disconnectFromServer();
			s->deleteLater();
		}

		m_localSockets.clear();

		if (m_localServer.isListening())
			m_localServer.close();
	});

	QObject::connect(&m_localServer, &QLocalServer::newConnection, parent, [this, parent](){
		while (m_localServer.hasPendingConnections()) {
			QLocalSocket *s = m_localServer.nextPendingConnection();

			QObject::connect(s, &QLocalSocket::disconnected, parent, [this, s]() {
				if (qApp->closingDown())
					return;
				m_localSockets.removeAll(s);
				s->deleteLater();
			});

			m_localSockets.append(s);
		}
	});

	if (!m_localServer.listen(name))
		return false;

	return true;
}




/**
 * @brief FtxServer::writeToSocket
 * @param cbor
 */

inline void FtxServer::writeToSocket(const QCborValue &cbor)
{
	if (m_localSockets.isEmpty())
		return;

	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	out << cbor;

	for (QLocalSocket *s : m_localSockets) {
		s->write(data);
		s->flush();
	}
}




/**
 * @brief FtxLoop::runOnce
 * @param localSocket
 * @return
 */

inline bool FtxLoop::runOnce(QLocalSocket &localSocket)
{
	if (!localSocket.isValid() || localSocket.state() == QLocalSocket::UnconnectedState)
		return true;//false;

	if (localSocket.bytesAvailable() <= 0)
		return true;

	while (true) {
		if (const auto &ptr = getCbor(localSocket)) {
			if (!runCbor(ptr.value()))
				return false;
		} else {
			break;
		}
	}

	if (!m_lastUpdate.isValid() || m_lastUpdate.elapsed() >= 50) {
		update();
		if (m_lastUpdate.isValid())
			m_lastUpdate.restart();
		else
			m_lastUpdate.start();
	}

	return true;
}




/**
 * @brief FtxLoop::getCbor
 * @param localSocket
 * @return
 */

inline std::optional<QCborValue> FtxLoop::getCbor(QLocalSocket &localSocket)
{
	QDataStream in(&localSocket);

	in.startTransaction();

	QCborValue cbor;
	in >> cbor;

	if (!in.commitTransaction())
		return std::nullopt;

	return cbor;
}




/**
 * @brief FtxLoop::updateScreen
 */

inline void FtxLoop::updateScreen()
{
	if (m_screen)
		m_screen->Post(ftxui::Event::Custom);
}







/**
 * @brief FtxTerminal::run
 * @param loop
 * @param serverName
 * @return
 */

inline int FtxTerminal::run(FtxLoop &loop, const QString &serverName)
{
	if (!connect(serverName.isEmpty() ? QStringLiteral("callofsuli") : serverName))
		return -1;

	ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
	ftxui::Loop lp(&screen, loop.createComponent(&screen));

	while (!lp.HasQuitted()) {
		if (!loop.runOnce(m_socket))
			break;

		m_eventLoop.processEvents(QEventLoop::AllEvents);

		lp.RunOnce();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	m_socket.disconnectFromServer();
	m_socket.waitForDisconnected(1000);

	return 0;
}



/**
 * @brief FtxTerminal::msgError
 * @param str
 */

inline void FtxTerminal::msgError(const QString &str)
{
	ftxui::Element txt = ftxui::text(str.toStdString()) | ftxui::bold | color(ftxui::Color::Red);

	ftxui::Screen screen = ftxui::Screen::Create(ftxui::Dimension::Full(), ftxui::Dimension::Fit(txt));

	Render(screen, txt);

	screen.Print();
}



/**
 * @brief FtxTerminal::connect
 * @param serverName
 * @param msecs
 * @return
 */

inline bool FtxTerminal::connect(const QString &serverName, const int msecs)
{
	m_socket.connectToServer(serverName);

	if (m_socket.waitForConnected(msecs))
		return true;
	else {
		msgError(QObject::tr("Connection failed: ").append(serverName));
		return false;
	}
}

#endif // FTXTERMINAL_H
