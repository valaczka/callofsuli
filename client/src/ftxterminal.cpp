/*
 * ---- Call of Suli ----
 *
 * terminal.cpp
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

#include "ftxterminal.h"

#include <string>

//#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for operator|, Maybe, Checkbox, Radiobox, Renderer, Vertical
#include "ftxui/component/component_base.hpp"      // for Component
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/component/event.hpp"
#include "ftxui/dom/elements.hpp"  // for Element, border, color, operator|, text
#include "ftxui/screen/color.hpp"  // for Color, Color::Red
#include "qeventloop.h"
#include "qlocalsocket.h"
#include "QCborValue"
#include <QCborMap>
#include <QJsonObject>
#include <QJsonDocument>


using namespace ftxui;


class ToggleButton
{
public:
	ToggleButton(const QString &text, const bool &checked = false)
	{
		m_checked = checked;
		m_button = Button(text.toStdString(), [this]() { m_checked = !m_checked; });
		m_renderer = Renderer(m_button, [this]() {
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

	const Component &renderer() { return m_renderer; }
	const Component &button() { return m_button; }


	bool checked() const { return m_checked; }
	void setChecked(bool newChecked) { m_checked = newChecked; }

	Color color() const { return m_color; }
	void setColor(const Color &newColor) { m_color = newColor; }

	Color bgColor() const { return m_bgColor; }
	void setBgColor(const Color &newBgColor) { m_bgColor = newBgColor; }

private:
	bool m_checked = false;
	Component m_button;
	Component m_renderer;
	Color m_color = Color::Default;
	Color m_bgColor = Color::Default;

};




/**
 * @brief The ScrollText class
 */

class ScrollText : public ComponentBase
{
public:
	ScrollText(const QString &title, const QString &text = {})
		: ComponentBase()
		, m_title(title)
		, m_text(text)
	{

	}

	bool Focusable() const override { return true; }

	Element OnRender() override {
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


	bool OnEvent (Event event) override {
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

	Color borderColor() const { return m_borderColor; }
	void setBorderColor(const Color &newBorderColor) { m_borderColor = newBorderColor; }

	Color textColor() const { return m_textColor; }
	void setTextColor(const Color &newTextColor) { m_textColor = newTextColor; }

	Color focusBorderColor() const { return m_focusBorderColor; }
	void setFocusBorderColor(const Color &newFocusBorderColor) { m_focusBorderColor = newFocusBorderColor; }

	Color focusTextColor() const { return m_focusTextColor; }
	void setFocusTextColor(const Color &newFocusTextColor) { m_focusTextColor = newFocusTextColor; }

private:
	QString m_title;
	QString m_text;

	Color m_borderColor = Color::Default;
	Color m_textColor = Color::Default;
	Color m_focusBorderColor = Color::Default;
	Color m_focusTextColor = Color::Default;

	int m_posX = 0;
	int m_posY = 0;
};




class DefaultLoop : public FtxLoop
{
public:
	DefaultLoop();

	virtual Component createComponent(ScreenInteractive *screen) override;

private:
	virtual bool runCbor(const QCborValue &cbor) override;

	std::shared_ptr<ScrollText> m_textLeft;
	std::shared_ptr<ScrollText> m_textRight;

	Component m_mainContainer;

	Component m_btnExit;

	std::unique_ptr<ToggleButton> m_btnSendPause;
	std::unique_ptr<ToggleButton> m_btnSendPlayer;
	std::unique_ptr<ToggleButton> m_btnSendEnemies;


	std::unique_ptr<ToggleButton> m_btnReceivePause;
	std::unique_ptr<ToggleButton> m_btnReceivePlayer;
	std::unique_ptr<ToggleButton> m_btnReceiveEnemies;

};




/**
 * @brief FtxTerminal::run
 * @param serverName
 * @return
 */

int FtxTerminal::run(const QString &serverName)
{
	DefaultLoop loop;
	return run(loop, serverName);
}



/**
 * @brief FtxTerminal::run
 * @param serverName
 * @return
 */

int FtxTerminal::run(FtxLoop &loop, const QString &serverName)
{
	if (!connect(serverName.isEmpty() ? QStringLiteral("callofsuli") : serverName))
		return -1;

	ScreenInteractive screen = ScreenInteractive::Fullscreen();

	Loop lp(&screen, loop.createComponent(&screen));

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

void FtxTerminal::msgError(const QString &str)
{
	Element txt = text(str.toStdString()) | bold | color(Color::Red);

	Screen screen = Screen::Create(Dimension::Full(), Dimension::Fit(txt));

	Render(screen, txt);

	screen.Print();
}



/**
 * @brief FtxTerminal::connect
 * @param serverName
 * @param msecs
 */

bool FtxTerminal::connect(const QString &serverName, const int msecs)
{
	m_socket.connectToServer(serverName);

	if (m_socket.waitForConnected(msecs))
		return true;
	else {
		msgError(QObject::tr("Connection failed: ").append(serverName));
		return false;
	}
}




/**
 * @brief DefaultLoop::DefaultLoop
 */

DefaultLoop::DefaultLoop()
	: FtxLoop()
{


}






/**
 * @brief DefaultLoop::createComponent
 * @return
 */

Component DefaultLoop::createComponent(ftxui::ScreenInteractive *screen)
{
	m_screen = screen;

	m_btnExit = Button("Exit", m_screen->ExitLoopClosure());


	m_btnSendPause = std::make_unique<ToggleButton>("Pause", false);
	m_btnSendPause->setBgColor(Color::Yellow);

	m_btnSendPlayer = std::make_unique<ToggleButton>("Player", true);
	m_btnSendPlayer->setColor(Color::Blue);

	m_btnSendEnemies = std::make_unique<ToggleButton>("Enemies", true);
	m_btnSendEnemies->setColor(Color::Red);


	m_btnReceivePause = std::make_unique<ToggleButton>("Pause", false);
	m_btnReceivePause->setBgColor(Color::Yellow);

	m_btnReceivePlayer = std::make_unique<ToggleButton>("Player", true);
	m_btnReceivePlayer->setColor(Color::Blue);

	m_btnReceiveEnemies = std::make_unique<ToggleButton>("Enemies", true);
	m_btnReceiveEnemies->setColor(Color::Red);

	m_textLeft = std::make_shared<ScrollText>(" Sent ");
	m_textRight = std::make_shared<ScrollText>(" Received ");

	m_textLeft->setTextColor(Color::Cyan1);
	m_textLeft->setFocusBorderColor(Color::Cyan2);

	m_textRight->setFocusBorderColor(Color::Yellow2);



	m_mainContainer = Container::Vertical({
											  Container::Horizontal({
												  m_textLeft,
												  m_textRight
											  }),
											  Container::Horizontal({
												  m_btnSendPause->button(),
												  m_btnSendPlayer->button(),
												  m_btnSendEnemies->button(),
												  m_btnExit,
												  m_btnReceivePlayer->button(),
												  m_btnReceiveEnemies->button(),
												  m_btnReceivePause->button(),
											  })
										  });

	auto main = Renderer(m_mainContainer, [this](){
		return vbox({
						text("Call of Suli terminal") | bold | color(Color::White) | hcenter,
						separatorHeavy(),
						hbox({
							m_textLeft->Render() | size(WIDTH, EQUAL, Dimension::Full().dimx * 0.5),
							m_textRight->Render() | flex ,
						}) | flex,
						hbox({
							vtext("SND") | yflex | vcenter,
							separatorEmpty(),
							m_btnSendPause->renderer()->Render(),
							m_btnSendPlayer->renderer()->Render(),
							m_btnSendEnemies->renderer()->Render(),
							filler(),
							m_btnExit->Render(),
							filler(),
							m_btnReceivePlayer->renderer()->Render(),
							m_btnReceiveEnemies->renderer()->Render(),
							m_btnReceivePause->renderer()->Render(),
							separatorEmpty(),
							vtext("RCV") | yflex | vcenter,
						})

					});
	});

	main |= CatchEvent([this](Event event) {
			if (event == Event::Character('q') || event == Event::Character('Q')) {
			m_screen->ExitLoopClosure();
			return false;
}

			return false;
});

	return main;
}



/**
 * @brief DefaultLoop::runCbor
 * @param cbor
 * @return
 */

bool DefaultLoop::runCbor(const QCborValue &cbor)
{
	QCborMap m = cbor.toMap();

	if (m.value("0op") == "SND") {
		if (m_btnSendPause->checked())
			return true;

		if (!m_btnSendEnemies->checked())
			m.remove(QStringLiteral("ee"));

		if (!m_btnSendPlayer->checked())
			m.remove(QStringLiteral("pp"));

		m_textLeft->setText(QJsonDocument(m.toJsonObject()).toJson());
	} else {
		if (m_btnReceivePause->checked())
			return true;

		if (!m_btnReceiveEnemies->checked())
			m.remove(QStringLiteral("ee"));

		if (!m_btnReceivePlayer->checked())
			m.remove(QStringLiteral("pp"));

		m_textRight->setText(QJsonDocument(m.toJsonObject()).toJson());
	}

	updateScreen();

	return true;
}





/**
 * @brief FtxLoop::updateScreen
 */

bool FtxLoop::runOnce(QLocalSocket &localSocket)
{
	if (!localSocket.isValid() || localSocket.state() == QLocalSocket::UnconnectedState)
		return false;

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

	update();

	return true;
}



/**
 * @brief FtxLoop::getCbor
 * @param localSocket
 * @return
 */

std::optional<QCborValue> FtxLoop::getCbor(QLocalSocket &localSocket)
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

void FtxLoop::updateScreen()
{
	if (m_screen)
		m_screen->Post(Event::Custom);
}

