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
#include "qdebug.h"
#include "application.h"
#include "qandroidfunctions.h"
#include "qurl.h"


MobileUtils* MobileUtils::m_instance = nullptr;
QString MobileUtils::m_pendingArg;

/**
 * @brief MobileUtils::MobileUtils
 */

MobileUtils::MobileUtils()
{

}


/**
 * @brief MobileUtils::initialize
 */

void MobileUtils::initialize()
{

}



void openUrl(const std::string &url)
{
	QUrl _url(QString::fromStdString(url));
	Application::instance()->selectUrl(_url);
}



/**
 * @brief MobileUtils::vibrate
 */

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



QString MobileUtils::checkPendingIntents()
{
	QAndroidJniObject uri = QtAndroid::androidActivity().callObjectMethod<jstring>("checkPendingIntents");

	QString uriStr = uri.toString();

	LOG_CDEBUG("client") << "Check pending intents:" << uriStr;

	return uriStr;
}






#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_hu_piarista_vjp_callofsuli_ClientActivity_setUrl(JNIEnv *env,
													  jobject ,
													  jstring url)
{
	const char *urlStr = env->GetStringUTFChars(url, NULL);

	QUrl _url(QString::fromUtf8(urlStr));
	Application::instance()->selectUrl(_url);

	env->ReleaseStringUTFChars(url, urlStr);
	return;
}

#ifdef __cplusplus
}
#endif

