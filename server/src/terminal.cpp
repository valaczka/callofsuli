/*
 * ---- Call of Suli ----
 *
 * terminal.cpp
 *
 * Created on: 2025. 04. 25.
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

#include "terminal.h"
#include "ftxui/component/component.hpp"
#include <QCborMap>



/**
 * @brief The DefaultLoop class
 */

class DefaultLoop : public FtxLoop
{
public:
	DefaultLoop();

	virtual ftxui::Component createComponent(ftxui::ScreenInteractive *screen) override;

private:
	virtual bool runCbor(const QCborValue &cbor) override;

	std::shared_ptr<ScrollText> m_textSend;
	std::shared_ptr<ScrollText> m_textReceive;

	ftxui::Component m_mainContainer;

	ftxui::Component m_btnExit;

	std::unique_ptr<ToggleButton> m_btnSendPause;
	std::unique_ptr<ToggleButton> m_btnReceivePause;

	std::unique_ptr<ToggleButton> m_btnPauseAll;

};




Terminal::Terminal()
	: FtxTerminal()
{

}


/**
 * @brief Terminal::run
 * @param serverName
 * @return
 */

int Terminal::run(const QString &serverName)
{
	DefaultLoop loop;
	return FtxTerminal::run(loop, serverName);
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

ftxui::Component DefaultLoop::createComponent(ftxui::ScreenInteractive *screen)
{
	using namespace ftxui;

	m_screen = screen;

	m_btnExit = Button("Exit", m_screen->ExitLoopClosure());


	m_btnSendPause = std::make_unique<ToggleButton>("Pause", false);
	m_btnSendPause->setBgColor(Color::Green);

	m_btnReceivePause = std::make_unique<ToggleButton>("Pause", false);
	m_btnReceivePause->setBgColor(Color::Green);

	m_btnPauseAll = std::make_unique<ToggleButton>("Pause all", false);
	m_btnPauseAll->setBgColor(Color::Red);

	m_textSend = std::make_shared<ScrollText>(" Sent ");
	m_textReceive = std::make_shared<ScrollText>(" Received ");

	m_textSend->setTextColor(Color::Cyan1);
	m_textSend->setFocusBorderColor(Color::Cyan2);

	m_textReceive->setTextColor(Color::Green1);
	m_textReceive->setFocusBorderColor(Color::Yellow2);



	m_mainContainer = Container::Vertical({
											  Container::Horizontal({
												  m_textReceive,
												  m_textSend,
											  }),
											  Container::Horizontal({
												  m_btnReceivePause->button(),
												  m_btnPauseAll->button(),
												  m_btnExit,
												  m_btnSendPause->button(),
											  })
										  });

	auto main = Renderer(m_mainContainer, [this](){
		return vbox({
						text("Call of Suli server terminal") | bold | color(Color::White) | hcenter,
						separatorHeavy(),
						hbox({
							m_textReceive->Render() | size(WIDTH, EQUAL, Dimension::Full().dimx * 0.5),
							m_textSend->Render() | flex ,
						}) | flex,
						hbox({
							vtext("RCV") | yflex | vcenter,
							separatorEmpty(),
							m_btnReceivePause->renderer()->Render(),
							filler(),
							m_btnPauseAll->renderer()->Render(),
							m_btnExit->Render(),
							filler(),
							m_btnSendPause->renderer()->Render(),
							separatorEmpty(),
							vtext("SND") | yflex | vcenter,
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

	if (m.value("mode") == "SND") {
		if (m_btnSendPause->checked() || m_btnPauseAll->checked())
			return true;

		/*if (!m_btnSendEnemies->checked())
			m.remove(QStringLiteral("ee"));

		if (!m_btnSendPlayer->checked())
			m.remove(QStringLiteral("pp"));

		m_textLeft->setText(QJsonDocument(m.toJsonObject()).toJson());*/

		m_textSend->setText(m.value(QStringLiteral("txt")).toString());
	} else {
		if (m_btnReceivePause->checked() || m_btnPauseAll->checked())
			return true;

		/*if (!m_btnReceiveEnemies->checked())
			m.remove(QStringLiteral("ee"));

		if (!m_btnReceivePlayer->checked())
			m.remove(QStringLiteral("pp"));*/

		m_textReceive->setText(m.value(QStringLiteral("txt")).toString());
	}

	///updateScreen();

	return true;
}



