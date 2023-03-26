/*
 * ---- Call of Suli ----
 *
 * terminalhandler.cpp
 *
 * Created on: 2023. 03. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TerminalHandler
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

#include "terminalhandler.h"
#include "Logger.h"
#include "serverservice.h"

using Terminal = QtService::Terminal;

/**
 * @brief TerminalHandler::TerminalHandler
 * @param service
 */

TerminalHandler::TerminalHandler(ServerService *service, QtService::Terminal *terminal)
	: QObject{service}
	, m_service(service)
	, m_terminal(terminal)
{
	Q_ASSERT(m_service);
	Q_ASSERT(m_terminal);

	LOG_CTRACE("service") << "Terminal handler created" << this;

	m_terminal->setAutoDelete(true);

	connect(m_terminal, &Terminal::destroyed, this, &TerminalHandler::deleteLater);
	connect(m_terminal, &Terminal::terminalDisconnected, this, &TerminalHandler::deleteLater);
	connect(m_terminal, &Terminal::readyRead, this, &TerminalHandler::onReadyRead);

	connect(m_service, &ServerService::paused, this, &TerminalHandler::onServicePaused);
	connect(m_service, &ServerService::resumed, this, &TerminalHandler::onServiceResumed);
	connect(m_service, &ServerService::reloaded, this, &TerminalHandler::onServiceReloaded);

	m_readLineFunc = std::bind(&TerminalHandler::parseRoot, this, std::placeholders::_1);

	start();
}


/**
 * @brief TerminalHandler::~TerminalHandler
 */

TerminalHandler::~TerminalHandler()
{
	LOG_CTRACE("service") << "Terminal handler destroyed" << this;
}




/**
 * @brief TerminalHandler::start
 */

void TerminalHandler::start()
{
	QStringList commands = m_terminal->command();

	if (commands.size()>1) {
		m_isStopParse = true;
		commands.removeFirst();
		const QByteArray &c = commands.join(QStringLiteral(" ")).toUtf8();
		if (m_readLineFunc)
			std::invoke(m_readLineFunc, c);
		else
			LOG_CERROR("service") << "No read line function";
	}

	if (m_isStopParse) {
		LOG_CTRACE("service") << "Stop terminal parsing";
		m_terminal->disconnectTerminal();
		return;
	}

	writeLine("Call of Suli terminal");
	writeLine("=====================");

	prompt();
}


/**
 * @brief TerminalHandler::writeLine
 * @param txt
 */

void TerminalHandler::writeLine(const char *txt) const
{
	m_terminal->writeLine(txt);
}


/**
 * @brief TerminalHandler::write
 * @param txt
 */

void TerminalHandler::write(const char *txt) const
{
	m_terminal->write(txt);
}


/**
 * @brief TerminalHandler::prompt
 */

void TerminalHandler::prompt()
{
	if (m_isStopParse)
		return;

	if (m_promptReady) {
		LOG_CTRACE("service") << "Prompt already open";
		return;
	}
	write("cmd> ");
	m_terminal->requestLine();
	m_promptReady = true;
}


/**
 * @brief TerminalHandler::call
 * @param map
 * @param args
 * @return
 */

bool TerminalHandler::call(const StringListMapFunc &map, QStringList args)
{
	const QString &cmd = args.isEmpty() ? QString() : args.at(0);

	if (cmd == QStringLiteral("help") || cmd.isEmpty()) {
		if (cmd.isEmpty())
			writeLine("Available arguments:");

		for (auto it = map.constBegin(); it != map.constEnd(); ++it)
			writeLine(it.key().toUtf8().prepend(QByteArrayLiteral("  ")));

		writeLine("");

		return !cmd.isEmpty();
	}

	if (map.contains(cmd)) {
		LOG_CDEBUG("service") << "Call" << args;
		args.removeFirst();
		if (map.value(cmd))
			return map.value(cmd)(args);
		else
			return true;
	} else {
		writeInvalidCmd(args.join(QStringLiteral(" ")).toUtf8());
		return false;
	}
}


/**
 * @brief TerminalHandler::writeInvalidCmd
 * @param data
 */

void TerminalHandler::writeInvalidCmd(const QByteArray &data) const
{
	write("*** unknown command: ");
	if (m_terminal) m_terminal->writeLine(data);
}


/**
 * @brief TerminalHandler::parseRoot
 * @param data
 */

void TerminalHandler::parseRoot(const QByteArray &data)
{
	const QStringList &list = QString::fromUtf8(data).split(QStringLiteral(" "));

	StringListMapFunc f;

	f.map("quit", [this](const QStringList &){
		m_terminal->disconnectTerminal();
		return true;
	});

	f.map("reload", [this](const QStringList &){
		m_service->reload();
		return true;
	});

	f.map("stop", [this](const QStringList &){
		m_terminal->disconnectTerminal();
		m_service->quit();
		return true;
	});


	f.map("api", [this](const QStringList &l){
		if (m_service->webServer() && m_service->webServer()->handler()) {
			Handler *h = m_service->webServer()->handler();

			for (auto it = h->apiHandlers().constBegin(); it != h->apiHandlers().constEnd(); ++it) {
				AbstractAPI *api = it.value();

				if (!api)
					continue;

				if (!l.isEmpty() && l.at(0) != it.key())
					continue;

				for (auto ait = api->maps().constBegin(); ait != api->maps().constEnd(); ++ait) {
					write(it.key());
					write(" ");
					writeLine(ait->regularExpression);
				}
			}
		}
		return true;
	});


	f.map("ls", [this](const QStringList &){
		QDirIterator it(QStringLiteral(":/"), QDirIterator::Subdirectories);
		while (it.hasNext()) {
			writeLine(it.next().toUtf8());
		}

		return true;
	});


	f.map("send", [this](const QStringList &l){
		if (l.size() < 2) {
			writeLine("*** missing parameters: event data");
			return true;
		}

		int n = 0;

		auto streams = m_service->eventStreams();

		for (auto it = streams.constBegin(); it != streams.constEnd(); ++it) {
			HttpEventStream *stream = *it;

			if (!stream)
				continue;

			stream->write(l.at(0).toUtf8(), l.at(1).toUtf8());
			++n;
		}


		writeLine(QStringLiteral("Sent to %1 stream").arg(n).toUtf8());

		return true;
	});


	f.map("panels", [this](const QStringList &){
		auto panels = m_service->panels();

		for (auto it = panels.constBegin(); it != panels.constEnd(); ++it) {
			Panel *p = *it;

			if (!p)
				continue;

			writeLine(QStringLiteral("Panel [%1] %2 - %3")
					  .arg(p->stream() ? QStringLiteral("X") : QStringLiteral(" "))
					  .arg(p->id())
					  .arg(p->owner()).toUtf8()
					  );
		}

		return true;
	});

	call(f, list);

	prompt();
}


/**
 * @brief TerminalHandler::onServicePaused
 */

void TerminalHandler::onServicePaused(bool success)
{
	if (success)
		writeLine("*** Service paused successful");
	else
		writeLine("*** Service pause failed");
}

/**
 * @brief TerminalHandler::onServiceResumed
 */

void TerminalHandler::onServiceResumed(bool success)
{
	if (success)
		writeLine("*** Service resumed successful");
	else
		writeLine("*** Service resume failed");
}

/**
 * @brief TerminalHandler::onServiceReloaded
 */

void TerminalHandler::onServiceReloaded(bool success)
{
	if (success)
		writeLine("*** Service reloaded successful");
	else
		writeLine("*** Service reload failed");
}


/**
 * @brief TerminalHandler::onServiceStopped
 * @param success
 */

void TerminalHandler::onServiceStopped(bool success)
{
	if (success)
		writeLine("*** Service stopped successful");
	else
		writeLine("*** Service stop failed");
}


/**
 * @brief TerminalHandler::onReadyRead
 */

void TerminalHandler::onReadyRead()
{
	m_promptReady = false;

	while (m_terminal->bytesAvailable()) {
		const QByteArray &line = m_terminal->readLine().simplified();

		if (m_readLineFunc)
			std::invoke(m_readLineFunc, line);
		else
			LOG_CERROR("service") << "No read line function";
	}
}

