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

#ifndef Q_OS_WASM
#include "qprocess.h"
#endif

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
	Q_PROPERTY(QJsonArray correction READ correction WRITE setCorrection NOTIFY correctionChanged FINAL)
	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged FINAL)
	Q_PROPERTY(bool upload READ upload WRITE setUpload NOTIFY uploadChanged FINAL)
	Q_PROPERTY(int maxPoint READ maxPoint WRITE setMaxPoint NOTIFY maxPointChanged FINAL)
	Q_PROPERTY(int point READ point WRITE setPoint NOTIFY pointChanged FINAL)
	Q_PROPERTY(int gradeId READ gradeId WRITE setGradeId NOTIFY gradeIdChanged FINAL)

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

	QJsonArray correction() const;
	void setCorrection(const QJsonArray &newCorrection);

	int maxPoint() const;
	void setMaxPoint(int newMaxPoint);

	int point() const;
	void setPoint(int newPoint);

	int gradeId() const;
	void setGradeId(int newGradeId);

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
	void correctionChanged();
	void maxPointChanged();
	void pointChanged();
	void gradeIdChanged();

private:
	QString m_path;
	ScanFileState m_state = ScanFileLoaded;
	int m_examId = -1;
	int m_contentId = -1;
	QJsonObject m_result;
	QString m_outputPath;
	QJsonArray m_serverAnswer;
	QJsonArray m_correction;
	QString m_username;
	bool m_upload = false;
	int m_maxPoint = 0;
	int m_point = 0;
	int m_gradeId = -1;
};


using ExamScanDataList = qolm::QOlm<ExamScanData>;
Q_DECLARE_METATYPE(ExamScanDataList*)



class TeacherExam;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_TeacherExam
#define OPAQUE_PTR_TeacherExam
  Q_DECLARE_OPAQUE_POINTER(TeacherExam*)
#endif

#endif


/**
 * @brief The ExamUser class
 */

class ExamUser : public User
{
	Q_OBJECT

	Q_PROPERTY(TeacherExam *teacherExam READ teacherExam WRITE setTeacherExam NOTIFY teacherExamChanged FINAL)
	Q_PROPERTY(QJsonArray examData READ examData WRITE setExamData NOTIFY examDataChanged FINAL)
	Q_PROPERTY(int contentId READ contentId WRITE setContentId NOTIFY contentIdChanged FINAL)
	Q_PROPERTY(Grade *grade READ grade WRITE setGrade NOTIFY gradeChanged FINAL)
	Q_PROPERTY(qreal result READ result WRITE setResult NOTIFY resultChanged FINAL)
	Q_PROPERTY(bool picked READ picked WRITE setPicked NOTIFY pickedChanged FINAL)
	Q_PROPERTY(bool joker READ joker WRITE setJoker NOTIFY jokerChanged FINAL)
	Q_PROPERTY(QJsonArray answer READ answer WRITE setAnswer NOTIFY answerChanged FINAL)
	Q_PROPERTY(QJsonArray correction READ correction WRITE setCorrection NOTIFY correctionChanged FINAL)
	Q_PROPERTY(QJsonArray pendingCorrection READ pendingCorrection WRITE setPendingCorrection NOTIFY pendingCorrectionChanged FINAL)
	Q_PROPERTY(Grade *pendingGrade READ pendingGrade WRITE setPendingGrade NOTIFY pendingGradeChanged FINAL)

public:
	ExamUser(QObject *parent = nullptr);

	Q_INVOKABLE void getContent(const int &index, QQuickTextDocument *document,
								QQuickItem *checkBoxSuccess = nullptr, QQuickItem *spinPoint = nullptr) const;
	Q_INVOKABLE bool isModified(const int &index) const;
	Q_INVOKABLE void modify(const int &index, const bool &success, const int &point);
	Q_INVOKABLE qreal recalculateResult();

	QJsonArray mergeCorrection() const;

	QJsonArray examData() const;
	void setExamData(const QJsonArray &newExamData);

	int contentId() const;
	void setContentId(int newContentId);

	Grade *grade() const;
	void setGrade(Grade *newGrade);

	qreal result() const;
	void setResult(qreal newResult);

	bool picked() const;
	void setPicked(bool newPicked);

	QJsonArray answer() const;
	void setAnswer(const QJsonArray &newAnswer);
	QJsonArray correction() const;
	void setCorrection(const QJsonArray &newCorrection);

	QJsonArray pendingCorrection() const;
	void setPendingCorrection(const QJsonArray &newPendingCorrection);

	TeacherExam *teacherExam() const;
	void setTeacherExam(TeacherExam *newTeacherExam);

	Grade *pendingGrade() const;
	void setPendingGrade(Grade *newPendingGrade);

	bool joker() const;
	void setJoker(bool newJoker);

signals:
	void examDataChanged();
	void contentIdChanged();
	void gradeChanged();
	void resultChanged();
	void pickedChanged();
	void answerChanged();
	void correctionChanged();
	void pendingCorrectionChanged();
	void teacherExamChanged();
	void pendingGradeChanged();
	void jokerChanged();

private:
	QJsonArray m_examData;
	QJsonArray m_answer;
	QJsonArray m_correction;
	QJsonArray m_pendingCorrection;
	int m_contentId = 0;
	qreal m_result = -1;
	Grade *m_grade = nullptr;
	Grade *m_pendingGrade = nullptr;
	bool m_picked = false;
	bool m_joker = false;
	TeacherExam *m_teacherExam = nullptr;
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
	Q_PROPERTY(bool hasPendingCorrection READ hasPendingCorrection NOTIFY hasPendingCorrectionChanged FINAL)
	Q_PROPERTY(GradingConfig *gradingConfig READ gradingConfig WRITE setGradingConfig NOTIFY gradingConfigChanged FINAL)

public:
	explicit TeacherExam(QObject *parent = nullptr);
	virtual ~TeacherExam();

	struct PdfConfig {
		int examId = 0;
		QString title;
		QString subject;
		int fontSize = 8;
		QString file;
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
	Q_INVOKABLE void pickUsers(QStringList userList, int count);

	Q_INVOKABLE void reload();
	Q_INVOKABLE void activate();
	Q_INVOKABLE void inactivate();
	Q_INVOKABLE void finish();
	Q_INVOKABLE void reclaim();

	Q_INVOKABLE void clearPendingCorrections();
	Q_INVOKABLE void savePendingCorrections(const QList<ExamUser *> &list);

	Q_INVOKABLE void clearPendingGrades();
	Q_INVOKABLE void savePendingGrades(const QList<ExamUser *> &list);

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

	ExamUserList* examUserList() const;

	bool hasPendingCorrection() const;

	GradingConfig *gradingConfig() const;
	void setGradingConfig(GradingConfig *newGradingConfig);

signals:
	void examListReloadRequest();
	void virtualListPicked(QList<ExamUser*> list);
	void examActivated();
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
	void hasPendingCorrectionChanged();
	void gradingConfigChanged();

private:
	static QString pdfTitle(const PdfConfig &pdfConfig, const QString &username, const int &contentId, QTextDocument *document);
	static QString pdfSheet(const bool &addResource, const int &width, QTextDocument *document);
	static QString pdfQuestion(const QJsonArray &list);

	void loadUserList();
	void loadGameMap();

	void pickUsersRandom(const int &count, const QStringList &userList, const QJsonObject &data);
	int getPicked(const QString &username, const QJsonArray &list) const;

	void scanImages();
	bool scanHasPendingQR();
	void processQRdata(const QString &path, const QString &qr);
	void scanPreapareOMR();
	void runOMR();
#ifndef Q_OS_WASM
	void onOmrFinished(int exitCode, QProcess::ExitStatus exitStatus);
#endif
	void processOMRdata(const QJsonArray &data);
	void generateAnswerResult(const QJsonObject &content);
	void getResult(const QJsonArray &qList, const QJsonObject &answer, QJsonArray *result, QJsonArray *correction,
				   int *ptrMax, int *ptrPoint) const;
	void updateResultFromServer();
	Q_INVOKABLE void uploadResultReal(QVector<QPointer<ExamScanData>> list);

	static QVector<int> letterToOptions(const QString &options) {
		QVector<int> list;
		for (const QChar &ch : options)
			list.append(m_optionLetters.indexOf(ch));
		return list;
	}

	static QVector<int> letterToOptions(const QJsonObject &data, const int &number) {
		return letterToOptions(data.value(QStringLiteral("q")+QString::number(number)).toString());
	}

	static const QString m_optionLetters;

	std::unique_ptr<ExamScanDataList> m_scanData;
	std::unique_ptr<ExamUserList> m_examUserList;
	std::unique_ptr<QTemporaryDir> m_scanTempDir;
#ifndef Q_OS_WASM
	std::unique_ptr<QProcess> m_omrProcess = nullptr;
	QLambdaThreadWorker m_worker;
#endif
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
	std::unique_ptr<GradingConfig> m_gradingConfig = nullptr;
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
