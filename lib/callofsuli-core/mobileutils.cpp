/*
 * ---- Call of Suli ----
 *
 * mobileutils.cpp
 *
 * Created on: 2023. 07. 05.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MobileUtils
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

#include "mobileutils.h"
#include "Logger.h"
#include "qandroidfunctions.h"
#include "qdebug.h"

MobileUtils::MobileUtils()
{

}


/**
 * @brief MobileUtils::vibrate
 */

#ifdef Q_OS_ANDROID
void MobileUtils::vibrate(const int &milliseconds)
{
	QAndroidJniObject activity = QtAndroid::androidActivity();

	if (!activity.isValid()) {
		LOG_CWARNING("utils") << "Invalid AndroidActivity";
		return;
	}

	QAndroidJniObject service = QAndroidJniObject::getStaticObjectField("android/content/Context", "VIBRATOR_SERVICE", "Ljava/lang/String;");

	if (!service.isValid()) {
		LOG_CWARNING("utils") << "Invalid AndroidContext";
		return;
	}


	jint amplitude = QAndroidJniObject::getStaticField<jint>("android/os/VibrationEffect", "DEFAULT_AMPLITUDE");

	jlong ms = milliseconds;

	QAndroidJniObject effect = QAndroidJniObject::callStaticObjectMethod("android/os/VibrationEffect", "createOneShot",
																	  "(JI)Landroid/os/VibrationEffect;", ms, amplitude);

	if (!effect.isValid()) {
		LOG_CWARNING("utils") << "Invalid VibrationEffect";
		return;
	}

	QAndroidJniObject vibrator = activity.callObjectMethod("getSystemService",
														   "(Ljava/lang/String;)Ljava/lang/Object;",
														   service.object<jstring>());
	if (vibrator.isValid())
	{
		LOG_CDEBUG("utils") << "Call Android Vibrator" << milliseconds;

		vibrator.callMethod<void>("vibrate", "(Landroid/os/VibrationEffect;)V", effect.object());
	}
	else
	{
		LOG_CWARNING("utils") << "Invalid Android Vibrator";
		return;
	}




	/*


	  API 31:

	LOG_CDEBUG("utils") << "API" << QtAndroid::androidSdkVersion();

	if (!activity.isValid()) {
		LOG_CWARNING("utils") << "Invalid AndroidActivity";
		return;
	}

	QAndroidJniObject service = QAndroidJniObject::getStaticObjectField("android/content/Context", "VIBRATOR_MANAGER_SERVICE", "Ljava/lang/String;");

	if (!service.isValid()) {
		LOG_CWARNING("utils") << "Invalid AndroidContext";
		return;
	}


	jint amplitude = QAndroidJniObject::getStaticField<jint>("android/os/VibrationEffect", "DEFAULT_AMPLITUDE");

	jlong ms = milliseconds;

	QAndroidJniObject effect = QAndroidJniObject::callStaticObjectMethod("android/os/VibrationEffect", "createOneShot",
																	  "(JI)Landroid/os/VibrationEffect;", ms, amplitude);

	if (!effect.isValid()) {
		LOG_CWARNING("utils") << "Invalid VibrationEffect";
		return;
	}

	QAndroidJniObject vibratorManager = activity.callObjectMethod("getSystemService",
														   "(Ljava/lang/String;)Ljava/lang/Object;",
														   service.object<jstring>());

	if (!vibratorManager.isValid()) {
		LOG_CWARNING("utils") << "Invalid VibratorManager";
		return;
	}

	QAndroidJniObject vibrator = vibratorManager.callObjectMethod("getDefaultVibrator", "()Landroid/os/Vibrator;");

	if (vibrator.isValid())
	{
		LOG_CDEBUG("utils") << "Call Android Vibrator" << milliseconds;

		vibrator.callMethod<void>("vibrate", "(Landroid/os/VibrationEffect;)V", effect.object());
	}
	else
	{
		LOG_CWARNING("utils") << "Invalid Android Vibrator";
		return;
	}

	*/
}
#endif
