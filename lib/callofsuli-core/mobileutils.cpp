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

#if QT_VERSION < 0x060000
#include "qandroidfunctions.h"
#endif

#include <QtCore/private/qandroidextras_p.h>
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
 * @brief initialize
 */

void MobileUtils::initialize() {}


/**
 * @brief openUrl
 * @param url
 */

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
#if QT_VERSION >= 0x060000
	using QAndroidJniObject = QJniObject;
	int apiLevel = QNativeInterface::QAndroidApplication::sdkVersion();
	QAndroidJniObject activity = QNativeInterface::QAndroidApplication::context();
#else
	int apiLevel = QtAndroid::androidSdkVersion();
	QAndroidJniObject activity = QtAndroid::androidActivity();
#endif


	if (!activity.isValid()) {
		LOG_CWARNING("utils") << "Invalid AndroidActivity";
		return;
	}

	QAndroidJniObject service = QAndroidJniObject::getStaticObjectField("android/content/Context", "VIBRATOR_SERVICE", "Ljava/lang/String;");

	if (!service.isValid()) {
		LOG_CWARNING("utils") << "Invalid AndroidContext";
		return;
	}


	QAndroidJniObject vibrator = activity.callObjectMethod("getSystemService",
														   "(Ljava/lang/String;)Ljava/lang/Object;",
														   service.object<jstring>());
	if (vibrator.isValid())
	{
		LOG_CDEBUG("utils") << "Call Android Vibrator" << milliseconds;

		jlong ms = milliseconds;

		if (apiLevel >= 26) {
			jint amplitude = QAndroidJniObject::getStaticField<jint>("android/os/VibrationEffect", "DEFAULT_AMPLITUDE");

			QAndroidJniObject effect = QAndroidJniObject::callStaticObjectMethod("android/os/VibrationEffect", "createOneShot",
																				 "(JI)Landroid/os/VibrationEffect;", ms, amplitude);

			if (!effect.isValid()) {
				LOG_CWARNING("utils") << "Invalid VibrationEffect";
				return;
			}

			vibrator.callMethod<void>("vibrate", "(Landroid/os/VibrationEffect;)V", effect.object());
		} else {
			vibrator.callMethod<void>("vibrate", "(J)V", ms);
		}
	}
	else
	{
		LOG_CWARNING("utils") << "Invalid Android Vibrator";
		return;
	}


}



/**
 * @brief MobileUtils::checkPendingIntents
 * @return
 */

QString MobileUtils::checkPendingIntents()
{
#if QT_VERSION >= 0x060000
	QJniObject activity = QNativeInterface::QAndroidApplication::context();
	const QJniObject &uri = activity.callObjectMethod<jstring>("checkPendingIntents");
#else
	QAndroidJniObject uri = QtAndroid::androidActivity().callObjectMethod<jstring>("checkPendingIntents");
#endif

	QString uriStr = uri.toString();

	LOG_CDEBUG("client") << "Check pending intents:" << uriStr;

	return uriStr;
}





/**
 * @brief MobileUtils::getSafeMargins
 * @return
 */


QMarginsF MobileUtils::getSafeMargins()
{
	QMarginsF margins;
	static const double devicePixelRatio = QApplication::primaryScreen()->devicePixelRatio();

#if QT_VERSION >= 0x060000
	QJniObject activity = QNativeInterface::QAndroidApplication::context();
	QJniObject rect = activity.callObjectMethod<jobject>("getSafeArea");
#else
	QAndroidJniObject rect = QtAndroid::androidActivity().callObjectMethod<jobject>("getSafeArea");
#endif

	const double left = static_cast<double>(rect.getField<jint>("left"));
	const double top = static_cast<double>(rect.getField<jint>("top"));
	const double right = static_cast<double>(rect.getField<jint>("right"));
	const double bottom = static_cast<double>(rect.getField<jint>("bottom"));

	margins.setTop(top/devicePixelRatio);
	margins.setBottom(bottom/devicePixelRatio);
	margins.setLeft(left/devicePixelRatio);
	margins.setRight(right/devicePixelRatio);

	return margins;
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

