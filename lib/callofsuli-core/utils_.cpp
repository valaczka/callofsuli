/*
 * ---- Call of Suli ----
 *
 * utils.cpp
 *
 * Created on: 2022. 12. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Utils
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

#include "utils_.h"
#include "Logger.h"
#include "qdesktopservices.h"
#include "qdir.h"
#include "qdiriterator.h"
#include "qfileinfo.h"
#include "qjsondocument.h"
#include "qmath.h"
#include "qsettings.h"
#include <QStandardPaths>
#include <random>
#include "../../version/version.h"

#ifdef CLIENT_UTILS
#include "selectableobject.h"
#include "qsdiffrunner.h"
#include "qclipboard.h"
#include "QApplication"
#endif

const quint32 Utils::m_versionMajor = VERSION_MAJOR;
const quint32 Utils::m_versionMinor = VERSION_MINOR;
const quint32 Utils::m_versionBuild = VERSION_BUILD;

#ifdef Q_OS_ANDROID
#if QT_VERSION < 0x060000
#include "qandroidfunctions.h"
#else
#include <QtCore/private/qandroidextras_p.h>
#endif
#include "QApplication"
#include "qscreen.h"
#endif


#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
#include "mobileutils.h"
#endif



/**
 * @brief Utils::Utils
 * @param client
 */

Utils::Utils(QObject *parent)
	: QObject{parent}
{

}


/**
 * @brief Utils::~Utils
 */

Utils::~Utils()
{

}


/**
 * @brief Utils::fileContent
 * @param filename
 * @param error
 * @return
 */

std::optional<QByteArray> Utils::fileContent(const QString &filename)
{
	QFile f(filename);

	if (!f.exists()) {
		LOG_CWARNING("utils") << "Can't read file:" << filename;
		return std::nullopt;
	}

	if (!f.open(QIODevice::ReadOnly)) {
		LOG_CWARNING("utils") << "Can't open file:" << filename;
		return std::nullopt;
	}

	QByteArray data = f.readAll();

	f.close();

	return data;
}


/**
 * @brief Utils::fileBaseName
 * @param filename
 * @return
 */

QString Utils::fileBaseName(const QString &filename)
{
	return QFileInfo(filename).baseName();
}


/**
 * @brief Utils::fileExists
 * @param filename
 * @return
 */

bool Utils::fileExists(const QUrl &file)
{
	return QFile::exists(file.toLocalFile());
}


/**
 * @brief Utils::jsonDocumentToFile
 * @param doc
 * @param filename
 * @return
 */

bool Utils::jsonDocumentToFile(const QJsonDocument &doc, const QString &filename, const QJsonDocument::JsonFormat &format)
{
	const QByteArray &b = doc.toJson(format);

	QFile f(filename);

	if (!f.open(QIODevice::WriteOnly)) {
		LOG_CWARNING("utils") << "Can't write file:" << filename;

		return false;
	}

	f.write(b);

	f.close();

	return true;
}


/**
 * @brief Utils::jsonObjectToFile
 * @param doc
 * @param filename
 * @param format
 * @return
 */

bool Utils::jsonObjectToFile(const QJsonObject &object, const QString &filename, const QJsonDocument::JsonFormat &format)
{
	return jsonDocumentToFile(QJsonDocument(object), filename, format);
}


/**
 * @brief Utils::jsonArrayToFile
 * @param array
 * @param filename
 * @param format
 * @return
 */

bool Utils::jsonArrayToFile(const QJsonArray &array, const QString &filename, const QJsonDocument::JsonFormat &format)
{
	return jsonDocumentToFile(QJsonDocument(array), filename, format);
}




/**
 * @brief Utils::byteArrayToJsonDocument
 * @param data
 * @return
 */

std::optional<QJsonDocument> Utils::byteArrayToJsonDocument(const QByteArray &data)
{
	if (data.isEmpty())
		return QJsonDocument();

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(data, &error);

	if (error.error != QJsonParseError::NoError) {
		LOG_CWARNING("utils") << "JSON parse error:" << error.errorString() << error.error;
		return std::nullopt;
	}

	return doc;
}


/**
 * @brief Utils::byteArrayToJsonObject
 * @param data
 * @return
 */

std::optional<QJsonObject> Utils::byteArrayToJsonObject(const QByteArray &data)
{
	if (data.isEmpty())
		return QJsonObject();

	const std::optional<QJsonDocument> &doc = byteArrayToJsonDocument(data);

	if (!doc || doc->isNull())
		return std::nullopt;

	return doc->object();
}


/**
 * @brief Utils::byteArrayToJsonArray
 * @param data
 * @return
 */

std::optional<QJsonArray> Utils::byteArrayToJsonArray(const QByteArray &data)
{
	if (data.isEmpty())
		return QJsonArray();

	const std::optional<QJsonDocument> &doc = byteArrayToJsonDocument(data);

	if (!doc || doc->isNull())
		return std::nullopt;

	return doc->array();
}


/**
 * @brief Utils::fileToJsonDocument
 * @param filename
 * @return
 */

std::optional<QJsonDocument> Utils::fileToJsonDocument(const QString &filename)
{
	QFile f(filename);

	if (!f.exists()) {
		LOG_CWARNING("utils") << "Can't read file:" << filename;
		return std::nullopt;
	}

	if (!f.open(QIODevice::ReadOnly)) {
		LOG_CWARNING("utils") << "Can't open file:" << filename;
		return std::nullopt;
	}

	QByteArray data = f.readAll();

	f.close();

	return byteArrayToJsonDocument(data);
}


/**
 * @brief Utils::fileToJsonObject
 * @param filename
 * @param error
 * @return
 */

std::optional<QJsonObject> Utils::fileToJsonObject(const QString &filename)
{
	const std::optional<QJsonDocument> &doc = fileToJsonDocument(filename);

	if (!doc || doc->isNull())
		return std::nullopt;

	return doc->object();
}



/**
 * @brief Utils::fileToJsonArray
 * @param filename
 * @param error
 * @return
 */

std::optional<QJsonArray> Utils::fileToJsonArray(const QString &filename)
{
	const std::optional<QJsonDocument> &doc = fileToJsonDocument(filename);

	if (!doc || doc->isNull())
		return std::nullopt;

	return doc->array();
}


/**
 * @brief Utils::colorSetAlpha
 * @param color
 * @param alpha
 * @return
 */

QColor Utils::colorSetAlpha(QColor color, const qreal &alpha)
{
	color.setAlphaF(alpha);
	return color;
}



/**
 * @brief Utils::formatMsecs
 * @param msec
 * @param decimals
 * @return
 */

QString Utils::formatMSecs(const int &msec, const int &decimals, const bool &withMinute)
{
	int s = qFloor((qreal)msec / 1000.0);
	int ms = msec - 1000*s;

	QString r;

	if (withMinute) {
		int h = qFloor((qreal)msec / (60*60*1000.0));
		int m = qFloor((qreal)msec / (60*1000.0)) - h*60;
		s -= m*60 + h*60*60;

		if (h > 0)
			r += QStringLiteral("%1:").arg(h, 2, 10, QChar('0'));

		r += QStringLiteral("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));

		if (decimals > 0) {
			QString n = QStringLiteral("%1").arg(ms, 3, 10, QChar('0'));
			r += QStringLiteral(".")+n.left(decimals);
		}
	} else {
		r = QString::number(s);
		if (decimals > 0) {
			QString n = QStringLiteral("%1").arg(ms, 3, 10, QChar('0'));
			r += QStringLiteral(".")+n.left(decimals);
		}
	}

	return r;
}




/**
 * @brief Utils::openUrl
 * @param url
 */
void Utils::openUrl(const QUrl &url)
{
	LOG_CDEBUG("utils") << "Open URL:" << url;
	QDesktopServices::openUrl(url);
}


/**
 * @brief Utils::standardPath
 * @param path
 * @return
 */

QString Utils::standardPath(const QString &path)
{
	if (!path.isEmpty())
		return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0)+QStringLiteral("/")+path;
	else
		return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0);
}




/**
 * @brief Utils::genericDataPath
 * @param path
 * @return
 */

QString Utils::genericDataPath(const QString &path)
{
#ifdef Q_OS_ANDROID
#if QT_VERSION >= 0x060000
	QJniObject mediaDir = QJniObject::callStaticObjectMethod("android/os/Environment", "getExternalStorageDirectory", "()Ljava/io/File;");
	QJniObject mediaPath = mediaDir.callObjectMethod( "getAbsolutePath", "()Ljava/lang/String;" );
#else
	QAndroidJniObject mediaDir = QAndroidJniObject::callStaticObjectMethod("android/os/Environment", "getExternalStorageDirectory", "()Ljava/io/File;");
	QAndroidJniObject mediaPath = mediaDir.callObjectMethod( "getAbsolutePath", "()Ljava/lang/String;" );
#endif
	if (!path.isEmpty())
		return QStringLiteral("file://")+mediaPath.toString()+QStringLiteral("/")+path;
	else
		return QStringLiteral("file://")+mediaPath.toString();
#endif

	if (!path.isEmpty())
		return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+QStringLiteral("/")+path;
	else
		return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
}





/**
 * @brief Utils::generateRandomString
 * @param length
 * @return
 */

QByteArray Utils::generateRandomString(quint8 length)
{
	const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	return generateRandomString(length, characters);
}


/**
 * @brief Utils::generateRandomString
 * @param length
 * @param characters
 * @return
 */

QByteArray Utils::generateRandomString(quint8 length, const char *characters)
{
	Q_ASSERT(characters && strlen(characters));

	static std::mt19937 randomEngine(QDateTime::currentDateTime().toMSecsSinceEpoch());
	std::uniform_int_distribution<int> distribution(0, strlen(characters)-1);
	QByteArray data;
	data.reserve(length);
	for (quint8 i = 0; i < length; ++i)
		data.append(characters[distribution(randomEngine)]);
	return data;
}


/**
 * @brief Utils::settingsGet
 * @param key
 * @param defaultValue
 * @return
 */

QVariant Utils::settingsGet(const QString &key, const QVariant &defaultValue)
{
	QSettings s;
	return s.value(key, defaultValue);
}


/**
 * @brief Utils::settingsSet
 * @param key
 * @param value
 */

void Utils::settingsSet(const QString &key, const QVariant &value)
{
	QSettings s;
	s.setValue(key, value);
}


/**
 * @brief Utils::settingsClear
 * @param key
 */

void Utils::settingsClear(const QString &key)
{
	QSettings s;
	s.remove(key);
}


#ifdef CLIENT_UTILS

/**
 * @brief Utils::noParent
 * @return
 */

const QModelIndex &Utils::noParent()
{
	static const QModelIndex ret = QModelIndex();
	return ret;
}


/**
 * @brief Utils::selectedCount
 * @param list
 * @return
 */


int Utils::selectedCount(qolm::QOlmBase *list)
{
	if (!list)
		return 0;

	int num = 0;

	foreach (QObject *o, list->children()) {
		SelectableObject *s = qobject_cast<SelectableObject*>(o);
		if (s && s->selected())
			++num;
	}

	return num;
}



/**
 * @brief Utils::getRolesFromObject
 * @param object
 * @return
 */

QStringList Utils::getRolesFromObject(const QMetaObject *object)
{
	QStringList roles;

	for (int i = 0 ; i < object->propertyCount(); i++) {
		const QMetaProperty &property = object->property(i);

		if (!property.isValid() || !property.isReadable() || !property.isStored())
			continue;

		const QString &p = property.name();

		if (p == QLatin1String("objectName"))
			continue;

		roles.append(p);
	}

	return roles;
}


/**
 * @brief Utils::patchSListModel
 * @param model
 * @param keyField
 */

void Utils::patchSListModel(QSListModel *model, const QVariantList &data, const QString &keyField)
{
	Q_ASSERT(model);

	QSDiffRunner runner;

	runner.setKeyField(keyField);

	const QList<QSPatch> &patches = runner.compare(model->storage(), data);

	runner.patch(model, patches);
}


/**
 * @brief Utils::setCliboardText
 * @param text
 */

void Utils::setClipboardText(const QString &text)
{
	QClipboard *clipboard = QApplication::clipboard();

	if (!clipboard) {
		LOG_CERROR("utils") << "Cliboard unavailable";
		return;
	}

	clipboard->setText(text);
}


/**
 * @brief Utils::clipboardText
 * @return
 */

QString Utils::clipboardText()
{
	QClipboard *clipboard = QApplication::clipboard();

	if (!clipboard) {
		LOG_CERROR("utils") << "Cliboard unavailable";
		return QStringLiteral("");
	}

	return clipboard->text();
}

#endif

/**
 * @brief Utils::versionMajor
 * @return
 */

quint32 Utils::versionMajor()
{
	return m_versionMajor;
}


/**
 * @brief Utils::versionMinor
 * @return
 */

quint32 Utils::versionMinor()
{
	return m_versionMinor;
}

quint32 Utils::versionBuild()
{
	return m_versionBuild;
}


/**
 * @brief Utils::versionCode
 * @return
 */

quint32 Utils::versionCode()
{
	return (1000*m_versionMajor)+m_versionMinor;
}


/**
 * @brief Utils::versionCode
 * @param major
 * @param minor
 * @return
 */

quint32 Utils::versionCode(const int &major, const int &minor)
{
	return (1000*major)+minor;
}



/**
 * @brief Utils::checkStoragePermissions
 */


void Utils::checkStoragePermissions()
{
#ifdef Q_OS_ANDROID

#if QT_VERSION < 0x060000
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
#else
	emit storagePermissionsDenied();
	return;
#endif
#else

#endif

	emit storagePermissionsGranted();
}





/**
 * @brief Utils::checkMediaPermissions
 */

void Utils::checkMediaPermissions()
{
#if defined(Q_OS_ANDROID) && QT_VERSION < 0x060000
	QtAndroid::PermissionResult result0 = QtAndroid::checkPermission("android.permission.CAMERA");

	QStringList permissions;

	if (result0 == QtAndroid::PermissionResult::Denied)
		permissions.append("android.permission.CAMERA");

	if (!permissions.isEmpty()) {
		QtAndroid::PermissionResultMap resultHash = QtAndroid::requestPermissionsSync(permissions, 30000);

		QList<QtAndroid::PermissionResult> results = resultHash.values();
		if (results.isEmpty() || results.contains(QtAndroid::PermissionResult::Denied)) {
			emit mediaPermissionsDenied();
			return;
		}
	}
#else
	switch (qApp->checkPermission(QCameraPermission{}))
	{
	case Qt::PermissionStatus::Undetermined:
		qApp->requestPermission(QCameraPermission{}, this, [this](const QPermission &permission) {
			if (permission.status() == Qt::PermissionStatus::Granted)
				emit mediaPermissionsGranted();
			else if (permission.status() == Qt::PermissionStatus::Denied)
				emit mediaPermissionsDenied();
		});
		break;

	case Qt::PermissionStatus::Granted:
		emit mediaPermissionsGranted();
		break;

	case Qt::PermissionStatus::Denied:
		emit mediaPermissionsDenied();
		break;
	}


#endif

	emit mediaPermissionsGranted();
}



/**
 * @brief Utils::vibrate
 */

void Utils::vibrate()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
	MobileUtils::vibrate();
#else
	LOG_CTRACE("utils") << "Vibrate not supported";
#endif
}




/**
 * @brief Utils::getDiskCacheSize
 * @param dir
 * @return
 */

qint64 Utils::getDiskCacheSize()
{
	QDir dir(standardPath());

	LOG_CTRACE("utils") << "Get disk cache size in:" << qPrintable(dir.absolutePath());

	if (!dir.exists())
		return 0;

	QDirIterator it(dir, QDirIterator::Subdirectories);
	qint64 total = 0;
	while (it.hasNext()) {
		it.next();
		total += it.fileInfo().size();
	}
	return total;
}



/**
 * @brief Utils::getFormattedDiskCacheSize
 * @param dir
 * @return
 */

QString Utils::getFormattedDiskCacheSize()
{
	return QLocale::system().formattedDataSize(getDiskCacheSize());
}



/**
 * @brief Utils::clearDiskCache
 * @param dir
 */

void Utils::clearDiskCache()
{
	const QString &basePath = standardPath();
	QDir dir(basePath);

	LOG_CDEBUG("utils") << "Clear disk cache:" << qPrintable(dir.absolutePath()) << "...";

	if (!dir.exists())
		return;

	QStringList preventList;

	preventList << basePath;
	preventList << basePath + QStringLiteral("/servers");
	preventList << basePath + QStringLiteral("/servers/[0-9]");
	preventList << basePath + QStringLiteral("/servers/[0-9][0-9]");
	preventList << basePath + QStringLiteral("/servers/[0-9][0-9][0-9]");
	preventList << basePath + QStringLiteral("/servers/[0-9][0-9][0-9][0-9]");
	preventList << basePath + QStringLiteral("/servers/[0-9]/config.json");
	preventList << basePath + QStringLiteral("/servers/[0-9][0-9]/config.json");
	preventList << basePath + QStringLiteral("/servers/[0-9][0-9][0-9]/config.json");
	preventList << basePath + QStringLiteral("/servers/[0-9][0-9][0-9][0-9]/config.json");


	QDirIterator it(basePath, QDir::AllEntries | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		it.next();
		const QFileInfo &info = it.fileInfo();

		if (info.isDir() || !info.absoluteFilePath().startsWith(basePath))
			continue;

		if (!QDir::match(preventList, info.absoluteFilePath())) {
			bool success = QFile::remove(info.absoluteFilePath());
			LOG_CDEBUG("utils") << "   -" << info.absoluteFilePath() << "->" << success;
		}
	}

	QDirIterator it2(basePath, QDir::AllEntries | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);

	while (it2.hasNext()) {
		it2.next();
		const QFileInfo &info = it2.fileInfo();

		if (!info.isDir() || !info.absoluteFilePath().startsWith(basePath))
			continue;

		if (!QDir::match(preventList, info.absoluteFilePath())) {
			QDir d(info.path());
			bool success = d.rmdir(info.fileName());
			LOG_CDEBUG("utils") << "   -" << info.absoluteFilePath() << "->" << success;
		}

	}



	LOG_CDEBUG("utils") << "...disk cache cleared";

}





/**
 * @brief Utils::getAndroidSafeMargins
 * @return
 */

#ifdef Q_OS_ANDROID

QMarginsF Utils::getAndroidSafeMargins()
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

#endif














/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <stdio.h>

#endif

#else
#error "Cannot define getPeakRSS( ) or getCurrentRSS( ) for an unknown OS."
#endif





/**
 * Returns the peak (maximum so far) resident set size (physical
 * memory use) measured in bytes, or zero if the value cannot be
 * determined on this OS.
 */
size_t Utils::getPeakRSS()
{
#if defined(_WIN32)
	/* Windows -------------------------------------------------- */
	PROCESS_MEMORY_COUNTERS info;
	GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
	return (size_t)info.PeakWorkingSetSize;

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
	/* AIX and Solaris ------------------------------------------ */
	struct psinfo psinfo;
	int fd = -1;
	if ( (fd = open( "/proc/self/psinfo", O_RDONLY )) == -1 )
		return (size_t)0L;      /* Can't open? */
	if ( read( fd, &psinfo, sizeof(psinfo) ) != sizeof(psinfo) )
	{
		close( fd );
		return (size_t)0L;      /* Can't read? */
	}
	close( fd );
	return (size_t)(psinfo.pr_rssize * 1024L);

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
	/* BSD, Linux, and OSX -------------------------------------- */
	struct rusage rusage;
	getrusage( RUSAGE_SELF, &rusage );
#if defined(__APPLE__) && defined(__MACH__)
	return (size_t)rusage.ru_maxrss;
#else
	return (size_t)(rusage.ru_maxrss * 1024L);
#endif

#else
	/* Unknown OS ----------------------------------------------- */
	return (size_t)0L;          /* Unsupported. */
#endif
}





/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 */
size_t Utils::getCurrentRSS()
{
#if defined(_WIN32)
	/* Windows -------------------------------------------------- */
	PROCESS_MEMORY_COUNTERS info;
	GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
	return (size_t)info.WorkingSetSize;

#elif defined(__APPLE__) && defined(__MACH__)
	/* OSX ------------------------------------------------------ */
	struct mach_task_basic_info info;
	mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
	if ( task_info( mach_task_self( ), MACH_TASK_BASIC_INFO,
					(task_info_t)&info, &infoCount ) != KERN_SUCCESS )
		return (size_t)0L;      /* Can't access? */
	return (size_t)info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
	/* Linux ---------------------------------------------------- */
	long rss = 0L;
	FILE* fp = NULL;
	if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
		return (size_t)0L;      /* Can't open? */
	if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
	{
		fclose( fp );
		return (size_t)0L;      /* Can't read? */
	}
	fclose( fp );
	return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);

#else
	/* AIX, BSD, Solaris, and Unknown OS ------------------------ */
	return (size_t)0L;          /* Unsupported. */
#endif
}
