/*
 * ---- Call of Suli ----
 *
 * teacherexam.h
 *
 * Created on: 2024. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherExam
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

#ifndef TEACHEREXAM_H
#define TEACHEREXAM_H


#include "exam.h"
#include "gamemap.h"
#include "qjsonarray.h"
#include "qlambdathreadworker.h"
#include "qprocess.h"
#include "qtemporarydir.h"
#include "qtextdocument.h"
#include "SBarcodeDecoder.h"
#include "qjsonobject.h"
#include "teachergroup.h"
#include <QObject>
#include <selectableobject.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"


/**
 * @brief The ExamScanData class
 */

class ExamScanData : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged FINAL)
	Q_PROPERTY(ScanFileState state READ state WRITE setState NOTIFY stateChanged FINAL)
	Q_PROPERTY(int examId READ examId WRITE setExamId NOTIFY examIdChanged FINAL)
	Q_PROPERTY(int contentId READ contentId WRITE setContentId NOTIFY contentIdChanged FINAL)
	Q_PROPERTY(QJsonObject result READ result WRITE setResult NOTIFY resultChanged FINAL)
	Q_PROPERTY(QString outputPath READ outputPath WRITE setOutputPath NOTIFY outputPathChanged FINAL)
	Q_PROPERTY(QJsonArray serverAnswer READ serverAnswer WRITE setServerAnswer NOTIFY serverAnswerChanged FINAL)
	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged FINAL)
	Q_PROPERTY(bool upload READ upload WRITE setUpload NOTIFY uploadChanged FINAL)

public:
	explicit ExamScanData(QObject *parent = nullptr);

	enum ScanFileState {
		ScanFileLoaded,
		ScanFileReadQR,
		ScanFileReadingOMR,
		ScanFileInvalid,
		ScanFileError,
		ScanFileFinished
	};

	Q_ENUM(ScanFileState);

	QString path() const;
	void setPath(const QString &newPath);

	ScanFileState state() const;
	void setState(const ScanFileState &newState);

	int examId() const;
	void setExamId(int newExamId);

	int contentId() const;
	void setContentId(int newContentId);

	QJsonObject result() const;
	void setResult(const QJsonObject &newResult);

	QString outputPath() const;
	void setOutputPath(const QString &newOutputPath);

	QJsonArray serverAnswer() const;
	void setServerAnswer(const QJsonArray &newServerAnswer);

	QString username() const;
	void setUsername(const QString &newUsername);

	bool upload() const;
	void setUpload(bool newUpload);

signals:
	void pathChanged();
	void stateChanged();
	void examIdChanged();
	void contentIdChanged();
	void resultChanged();
	void outputPathChanged();
	void serverAnswerChanged();
	void usernameChanged();
	void uploadChanged();

private:
	QString m_path;
	ScanFileState m_state = ScanFileLoaded;
	int m_examId = -1;
	int m_contentId = -1;
	QJsonObject m_result;
	QString m_outputPath;
	QJsonArray m_serverAnswer;
	QString m_username;
	bool m_upload = false;
};

using ExamScanDataList = qolm::QOlm<ExamScanData>;
Q_DECLARE_METATYPE(ExamScanDataList*)




/**
 * @brief The ExamUser class
 */

class ExamUser : public User
{
	Q_OBJECT

	Q_PROPERTY(QJsonArray examData READ examData WRITE setExamData NOTIFY examDataChanged FINAL)
	Q_PROPERTY(int contentId READ contentId WRITE setContentId NOTIFY contentIdChanged FINAL)

public:
	ExamUser(QObject *parent = nullptr);

	QJsonArray examData() const;
	void setExamData(const QJsonArray &newExamData);

	int contentId() const;
	void setContentId(int newContentId);

signals:
	void examDataChanged();
	void contentIdChanged();

private:
	QJsonArray m_examData;
	int m_contentId = 0;
};


using ExamUserList = qolm::QOlm<ExamUser>;
Q_DECLARE_METATYPE(ExamUserList*)



/**
 * @brief The TeacherExam class
 */

class TeacherExam : public QObject
{
	Q_OBJECT

	Q_PROPERTY(ExamScanDataList* scanData READ scanData CONSTANT FINAL)
	Q_PROPERTY(ScanState scanState READ scanState WRITE setScanState NOTIFY scanStateChanged FINAL)
	Q_PROPERTY(QVariantList acceptedExamIdList READ acceptedExamIdList WRITE setAcceptedExamIdList NOTIFY acceptedExamIdListChanged FINAL)
	Q_PROPERTY(int uploadableCount READ uploadableCount NOTIFY uploadableCountChanged FINAL)
	Q_PROPERTY(TeacherGroup *teacherGroup READ teacherGroup WRITE setTeacherGroup NOTIFY teacherGroupChanged FINAL)
	Q_PROPERTY(TeacherMapHandler *mapHandler READ mapHandler WRITE setMapHandler NOTIFY mapHandlerChanged FINAL)
	Q_PROPERTY(Exam *exam READ exam WRITE setExam NOTIFY examChanged FINAL)
	Q_PROPERTY(GameMap *gameMap READ gameMap NOTIFY gameMapChanged FINAL)
	Q_PROPERTY(QString missionUuid READ missionUuid WRITE setMissionUuid NOTIFY missionUuidChanged FINAL)
	Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged FINAL)
	Q_PROPERTY(ExamUserList*examUserList READ examUserList CONSTANT FINAL)

public:
	explicit TeacherExam(QObject *parent = nullptr);
	virtual ~TeacherExam();

	struct PdfConfig {
		int examId = 0;
		QString title;
		QString subject;
		int fontSize = 8;
	};

	enum ScanState {
		ScanIdle = 0,
		ScanRunning,
		ScanErrorFileSystem,
		ScanErrorOmrInProgress,
		ScanErrorOmrNotFound,
		ScanErrorOmr,
		ScanFinished
	};

	Q_ENUM(ScanState);

	void createPdf(const QList<ExamUser *> &list, const PdfConfig &pdfConfig);

	Q_INVOKABLE void createPdf(const QList<ExamUser *> &list, const QVariantMap &pdfConfig);
	Q_INVOKABLE void scanImageDir(const QUrl &path);

	Q_INVOKABLE void remove(ExamScanData *scan);
	Q_INVOKABLE void removeSelected();

	Q_INVOKABLE void uploadResult();

	Q_INVOKABLE QVariantList getMissionLevelList();
	Q_INVOKABLE void loadContentFromJson(const QJsonObject &object);

	Q_INVOKABLE void generateExamContent(const QList<ExamUser*> &list);
	Q_INVOKABLE void reloadExamContent();

	ExamScanDataList* scanData() const;

	ScanState scanState() const;
	void setScanState(const ScanState &newScanState);

	QVariantList acceptedExamIdList() const;
	void setAcceptedExamIdList(const QVariantList &newAcceptedExamIdList);

	TeacherGroup *teacherGroup() const;
	void setTeacherGroup(TeacherGroup *newTeacherGroup);

	int uploadableCount() const;

	TeacherMapHandler *mapHandler() const;
	void setMapHandler(TeacherMapHandler *newMapHandler);

	Exam *exam() const;
	void setExam(Exam *newExam);

	GameMap *gameMap() const;
	void setGameMap(std::unique_ptr<GameMap> newGameMap);

	QString missionUuid() const;
	void setMissionUuid(const QString &newMissionUuid);

	int level() const;
	void setLevel(int newLevel);

	ExamUserList*examUserList() const;

signals:
	void pdfFileGenerated(QString filename);
	void scanQRfinished();
	void scanOMRfinished();
	void scanStateChanged();
	void updateFromServerRequest();
	void acceptedExamIdListChanged();
	void teacherGroupChanged();
	void uploadableCountChanged();
	void mapHandlerChanged();
	void examChanged();
	void missionLevelChanged();
	void gameMapChanged();
	void missionUuidChanged();
	void levelChanged();

private:
	static QString pdfTitle(const PdfConfig &pdfConfig, const QString &username, const int &contentId, QTextDocument *document);
	static QString pdfSheet(const bool &addResource, const int &width, QTextDocument *document);
	static QString pdfQuestion(const QJsonArray &list);

	void loadUserList();
	void loadGameMap();

	void scanImages();
	bool scanHasPendingQR();
	void processQRdata(const QString &path, const QString &qr);
	void scanPreapareOMR();
	void runOMR();
	void onOmrFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void processOMRdata(const QJsonArray &data);
	void generateAnswerResult(const QJsonObject &content);
	QJsonArray getResult(const QJsonArray &qList, const QJsonObject &answer) const;
	void updateResultFromServer();

	static QVector<int> letterToOptions(const QString &options) {
		QVector<int> list;
		for (const QChar &ch : options)
			list.append(m_optionLetters.indexOf(ch));
		return list;
	}

	static QVector<int> letterToOptions(const QJsonObject &data, const int &number) {
		return letterToOptions(data.value(QStringLiteral("q")+QString::number(number)).toString());
	}

	static QJsonValue letterToInt(const QString &options) {
		const auto &l = letterToOptions(options);
		if (l.size() != 1 || l.at(0) == -1)
			return QJsonValue{};
		else
			return l.at(0);
	}

	static QJsonValue letterToInt(const QJsonObject &data, const int &number) {
		return letterToInt(data.value(QStringLiteral("q")+QString::number(number)).toString());
	}

	static QJsonArray letterToArray(const QString &options) {
		const auto &l = letterToOptions(options);
		QJsonArray r;
		for (const int &i : l) {
			if (i != -1)
				r.append(i);
			else
				r.append(QJsonValue{});
		}
		return r;
	}

	static QJsonArray letterToArray(const QJsonObject &data, const int &number) {
		return letterToArray(data.value(QStringLiteral("q")+QString::number(number)).toString());
	}


	static const QString m_optionLetters;

	std::unique_ptr<ExamScanDataList> m_scanData;
	std::unique_ptr<ExamUserList> m_examUserList;
	std::unique_ptr<QTemporaryDir> m_scanTempDir;
	std::unique_ptr<QProcess> m_omrProcess = nullptr;
	QLambdaThreadWorker m_worker;
	QRecursiveMutex m_mutex;
	ScanState m_scanState = ScanIdle;
	QJsonArray m_scannedIdList;
	QVariantList m_acceptedExamIdList;
	QPointer<TeacherGroup> m_teacherGroup;
	QPointer<TeacherMapHandler> m_mapHandler;
	QPointer<Exam> m_exam;
	std::unique_ptr<GameMap> m_gameMap;
	QString m_missionUuid;
	int m_level = -1;
};




/**
 * @brief The ExmSBarcodeDecoder class
 */

class ExamSBarcodeDecoder : public SBarcodeDecoder
{
	Q_OBJECT

public:
	ExamSBarcodeDecoder(QObject *parent = nullptr) : SBarcodeDecoder(parent) {}

	void setPath(const QString &p) { m_path = p; }
	const QString &path() const { return m_path; }

private:
	QString m_path;
};


#endif // TEACHEREXAM_H
