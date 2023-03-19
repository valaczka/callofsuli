/*
 * ---- Call of Suli ----
 *
 * terminalhandler.h
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

#ifndef TERMINALHANDLER_H
#define TERMINALHANDLER_H

#include <QObject>
#include <QtService/Terminal>

class ServerService;


class StringListMapFunc : public
		QMap<QString, std::function<bool(const QStringList &)>>
{
																public:
																StringListMapFunc() {}
																virtual ~StringListMapFunc() {}

																void map(const char *arg, const std::function<bool(const QStringList &)> &func) {
																insert(QString::fromUtf8(arg), func);
																}

																};

/**
 * @brief The TerminalHandler class
 */

class TerminalHandler : public QObject
{
	Q_OBJECT

public:
	explicit TerminalHandler(ServerService *service, QtService::Terminal *terminal);
	virtual ~TerminalHandler();


	void start();

	void writeLine(const char *txt) const;
	void write(const char *txt) const;
	void writeInvalidCmd(const QByteArray &data) const;
	void prompt();


	bool call(const StringListMapFunc &map, QStringList args);

	void parseRoot(const QByteArray &data);

private slots:
	void onServicePaused(bool success);
	void onServiceResumed(bool success);
	void onServiceReloaded(bool success);
	void onServiceStopped(bool success);

	void onReadyRead();

private:
	ServerService *const m_service;
	QtService::Terminal *const m_terminal;
	std::function<void(const QByteArray &)> m_readLineFunc = nullptr;
	bool m_isStopParse = false;
	bool m_promptReady = false;

};

#endif // TERMINALHANDLER_H
