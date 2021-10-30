/*
 * ---- Call of Suli ----
 *
 * androidshareutils.h
 *
 * Created on: 2021. 10. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AndroidShareUtils
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

#ifndef ANDROIDSHAREUTILS_H
#define ANDROIDSHAREUTILS_H

#include <QObject>

#ifdef Q_OS_ANDROID
#include <jni.h>
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#endif


class AndroidShareUtils : public QObject
{
	Q_OBJECT

public:
	explicit AndroidShareUtils(QObject *parent = nullptr);
	virtual ~AndroidShareUtils();

	static AndroidShareUtils *instance();

	bool forceLandscape();
	bool resetLandscape();

public slots:
	bool checkPendingIntents();

signals:
	void urlSelected(const QString &ulr);

private:
	static AndroidShareUtils *m_instance;
	bool m_pendingIntentsChecked;

#ifdef Q_OS_ANDROID
	jint m_screenOrientationRequest;
#endif


};

#endif // ANDROIDSHAREUTILS_H
