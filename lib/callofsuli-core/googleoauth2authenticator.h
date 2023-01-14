/*
 * ---- Call of Suli ----
 *
 * googleoauth2authenticator.h
 *
 * Created on: 2023. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GoogleOAuth2Authenticator
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

#ifndef GOOGLEOAUTH2AUTHENTICATOR_H
#define GOOGLEOAUTH2AUTHENTICATOR_H

#include "oauth2authenticator.h"

class GoogleOAuth2Authenticator : public OAuth2Authenticator
{
	Q_OBJECT

public:
	explicit GoogleOAuth2Authenticator(QObject *parent)
		: OAuth2Authenticator(OAuth2Authenticator::Google, parent) { }

	OAuth2CodeFlow *addCodeFlow(QObject *referenceObject);
	OAuth2CodeFlow *addCodeFlow(OAuth2CodeFlow *flow);
	void setCodeFlow(OAuth2CodeFlow *flow) const;

	static QMap<std::string, std::string> getInfoFromRequestAccess(const QVariantMap &data);

};

#endif // GOOGLEOAUTH2AUTHENTICATOR_H
