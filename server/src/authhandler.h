/*
 * ---- Call of Suli ----
 *
 * authhandler.h
 *
 * Created on: 2023. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AuthHandler
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

#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include "abstracthandler.h"

class AuthHandler : public AbstractHandler
{
	Q_OBJECT

public:
	AuthHandler(Client *client);

protected:
	void handleRequest();
	void handleRequestResponse();
	void handleEvent();

private slots:
	void loginGoogle();
	void test();

	void onOAuthFailed();
	void onOAuthSuccess(const QVariantMap &data);

private:
	void loginUser(const QString &username);


};

#endif // AUTHHANDLER_H
