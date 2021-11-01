/*
 * ---- Call of Suli ----
 *
 * androidshareutils.cpp
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

#include <QDebug>

#include "androidshareutils.h"

#define FLAG_SCREEN_ORIENTATION_LANDSCAPE       0x00000000

AndroidShareUtils* AndroidShareUtils::m_instance = nullptr;



AndroidShareUtils::AndroidShareUtils(QObject *parent)
	: QObject(parent)
	, m_pendingIntentsChecked(false)
	#ifdef Q_OS_ANDROID
	, m_screenOrientationRequest(-1)
	#endif

{
	Q_ASSERT(m_instance == nullptr);

	m_instance = this;
}


/**
 * @brief AndroidShareUtils::~AndroidShareUtils
 */

AndroidShareUtils::~AndroidShareUtils()
{
	m_instance = nullptr;
}



/**
 * @brief AndroidShareUtils::instance
 * @return
 */

AndroidShareUtils *AndroidShareUtils::instance()
{
	if (!m_instance)
		m_instance = new AndroidShareUtils;
	return m_instance;
}


/**
 * @brief AndroidShareUtils::forceLandscape
 * @return
 */

bool AndroidShareUtils::forceLandscape()
{
#ifdef Q_OS_ANDROID
	m_screenOrientationRequest = QtAndroid::androidActivity().callMethod<jint>("getRequestedOrientation");

	QtAndroid::androidActivity().callMethod<void>("setRequestedOrientation",
												  "(I)V",
												  FLAG_SCREEN_ORIENTATION_LANDSCAPE);

	return true;
#endif

	return false;
}


/**
 * @brief AndroidShareUtils::resetLandscape
 * @return
 */

bool AndroidShareUtils::resetLandscape()
{
#ifdef Q_OS_ANDROID
	QtAndroid::androidActivity().callMethod<void>("setRequestedOrientation",
												  "(I)V",
												  m_screenOrientationRequest);

	return true;
#endif

	return false;
}


/**
 * @brief AndroidShareUtils::onApplicationStateChanged
 * @param applicationState
 */

bool AndroidShareUtils::checkPendingIntents()
{
	if (!m_pendingIntentsChecked) {
		m_pendingIntentsChecked = true;

#ifdef Q_OS_ANDROID
		QAndroidJniObject activity = QtAndroid::androidActivity();
		if (activity.isValid())  {
			jboolean ret = activity.callMethod<jboolean>("checkPendingIntents","()Z");
			qDebug() << "checkPendingIntents: " << ret;

			if (ret)
				return true;
		} else {
			qDebug() << "checkPendingIntents: Activity not valid";
		}
#endif
	}

	return false;
}



/**
 * @brief AndroidShareUtils::checkPermissions
 */

void AndroidShareUtils::checkPermissions()
{

#ifdef Q_OS_ANDROID
	QtAndroid::PermissionResult result1 = QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE");
	QtAndroid::PermissionResult result2 = QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");

	QStringList permissions;

	if (result1 == QtAndroid::PermissionResult::Denied)
		permissions.append("android.permission.READ_EXTERNAL_STORAGE");

	if (result2 == QtAndroid::PermissionResult::Denied)
		permissions.append("android.permission.WRITE_EXTERNAL_STORAGE");

	if (!permissions.isEmpty()) {
		QtAndroid::PermissionResultMap resultHash = QtAndroid::requestPermissionsSync(permissions, 30000);

		QList<QtAndroid::PermissionResult> results = resultHash.values();
		if (results.isEmpty() || results.contains(QtAndroid::PermissionResult::Denied)) {
			emit storagePermissionsDenied();
			return;
		}
	}
#endif

	emit storagePermissionsGranted();
}



#ifdef Q_OS_ANDROID

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_hu_piarista_vjp_callofsuli_ClientActivity_setUrl(JNIEnv *env,
													  jobject ,
													  jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);
	emit AndroidShareUtils::instance()->urlSelected(urlStr);

	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

#ifdef __cplusplus
}
#endif

#endif
