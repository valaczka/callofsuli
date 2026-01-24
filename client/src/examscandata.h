/*
 * ---- Call of Suli ----
 *
 * %{Cpp:License:FileName}
 *
 * Created on: %{CurrentDate:yyyy. MM. dd.}
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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


#ifndef EXAMSCANDATA_H
#define EXAMSCANDATA_H

#include "QOlm/QOlm.hpp"
#include <selectableobject.h>
#include <QJsonObject>
#include <QJsonArray>



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



#endif // EXAMSCANDATA_H
