import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



Loader {
    id: ldr
    width: parent.width

    property MapEditor mapEditor: null

    property var moduleData: ({})
    property var storageData: ({})
    property string storageModule: ""
    property int storageCount: 0

    signal modified()

    Component {
        id: cmpNone

        QGridLayout {
            id: layout

            watchModification: true
            onModifiedChanged: if (layout.modified)
                                   ldr.modified()

            QGridLabel { field: textQuestion }

            QGridTextField {
                id: textQuestion
                fieldName: qsTr("Kérdés")
                sqlField: "question"
                placeholderText: qsTr("Ez a kérdés fog megjelenni")
            }

            QGridLabel { field: textCorrectAnswer }

            QGridTextField {
                id: textCorrectAnswer
                fieldName: qsTr("Helyes válasz")
                sqlField: "correct"
                placeholderText: qsTr("Ez lesz a helyes válasz")
            }

            QGridLabel {
                field: areaAnswers
            }

            QGridTextArea {
                id: areaAnswers
                fieldName: qsTr("Helytelen válaszok")
                placeholderText: qsTr("Lehetséges helytelen válaszok (soronként)")
                minimumHeight: CosStyle.baseHeight*2
            }




            Component.onCompleted: {
                if (!moduleData)
                    return

                JS.setSqlFields([textQuestion, textCorrectAnswer], moduleData)
                areaAnswers.setData(moduleData.answers.join("\n"))
            }


            function getData() {
                var d = JS.getSqlFields([textQuestion, textCorrectAnswer])
                d.answers = areaAnswers.text.split("\n")

                moduleData = d
                return moduleData
            }

        }

    }


    Component {
        id: cmpBinding

        Column {

            QGridLayout {
                id: layout2
                watchModification: true
                onModifiedChanged: if (layout2.modified)
                                       ldr.modified()

                QGridText {
                    field: comboMode
                    text: qsTr("Kérdések készítése:")
                }

                QGridComboBox {
                    id: comboMode
                    sqlField: "mode"

                    valueRole: "value"
                    textRole: "text"

                    model: [
                        {value: "left", text: qsTr("Bal oldaliakhoz")},
                        {value: "right", text: qsTr("Jobb oldaliakhoz")}
                    ]

                    onActivated: preview.refresh()
                }



                QGridLabel { field: textQuestion2 }

                QGridTextField {
                    id: textQuestion2
                    fieldName: qsTr("Kérdés")
                    sqlField: "question"
                    placeholderText: qsTr("Ez a kérdés fog megjelenni. \%1 az összerendelésből kiválasztott tételre cserélődik ki.")

                    onTextModified: preview.refresh()
                }


                QGridText {
                    text: qsTr("Feladatok száma:")
                    field: spinCount
                }

                QGridSpinBox {
                    id: spinCount
                    from: 1
                    to: 99
                    editable: true

                    onValueModified: {
                        storageCount = value
                    }
                }


                Component.onCompleted: {
                    if (!moduleData)
                        return

                    JS.setSqlFields([comboMode, textQuestion2], moduleData)
                    spinCount.setData(storageCount)

                }

            }

            MapEditorObjectivePreview {
                id: preview

                refreshFunc: function() { return mapEditor.objectiveGeneratePreview("simplechoice", getData(), storageModule, storageData) }

                Connections {
                    target: ldr
                    function onStorageDataChanged() {
                        preview.refresh()
                    }
                }
            }


            function getData() {
                moduleData = JS.getSqlFields([comboMode, textQuestion2])

                return moduleData
            }
        }
    }





    Component {
        id: cmpImages

        Column {

            QGridLayout {
                id: layout3
                watchModification: true
                onModifiedChanged: if (layout3.modified)
                                       ldr.modified()

                QGridText {
                    field: comboModeImg
                    text: qsTr("Kérdések készítése:")
                }

                QGridComboBox {
                    id: comboModeImg
                    sqlField: "mode"

                    valueRole: "value"
                    textRole: "text"

                    model: [
                        {value: "image", text: qsTr("Képhez (szövegekből választhat)")},
                        {value: "text", text: qsTr("Szöveghez (képekből választhat)")}
                    ]

                    onActivated: previewImg.refresh()
                }



                QGridLabel {
                    field: textQuestionImgImg
                    visible: textQuestionImgImg.visible
                }

                QGridTextField {
                    id: textQuestionImgImg
                    fieldName: qsTr("Kérdés")
                    sqlField: "question"
                    placeholderText: qsTr("Ez a kérdés fog megjelenni.")
                    text: qsTr("Mit látsz a képen?")

                    visible: comboModeImg.currentValue === "image"

                    onTextModified: previewImg.refresh()
                }

                QGridLabel {
                    field: textQuestionImgText
                    visible: textQuestionImgText.visible
                }

                QGridTextField {
                    id: textQuestionImgText
                    fieldName: qsTr("Kérdés")
                    sqlField: "question"
                    placeholderText: qsTr("Ez a kérdés fog megjelenni. \%1 az összerendelésből kiválasztott tételre cserélődik ki.")
                    text: qsTr("Melyik képen látható: %1?")

                    visible: comboModeImg.currentValue === "text"

                    onTextModified: previewImg.refresh()
                }

                QGridLabel {
                    field: textAnswerImg
                    visible: textAnswerImg.visible
                }

                QGridTextField {
                    id: textAnswerImg
                    fieldName: qsTr("Válaszok")
                    sqlField: "answers"
                    placeholderText: qsTr("A válaszok formátuma. \%1 az összerendelésből kiválasztott tételre cserélődik ki.")

                    visible: comboModeImg.currentValue === "image"
                    onTextModified: previewImg.refresh()
                }


                QGridText {
                    text: qsTr("Feladatok száma:")
                    field: spinCountImg
                }

                QGridSpinBox {
                    id: spinCountImg
                    from: 1
                    to: 99
                    editable: true

                    onValueModified: {
                        storageCount = value
                    }
                }


                Component.onCompleted: {
                    if (!moduleData)
                        return

                    JS.setSqlFields([comboModeImg, textQuestionImgImg, textQuestionImgText, textAnswerImg], moduleData)
                    spinCountImg.setData(storageCount)

                }

            }

            MapEditorObjectivePreview {
                id: previewImg

                refreshFunc: function() { return mapEditor.objectiveGeneratePreview("simplechoice", getData(), storageModule, storageData) }

                Connections {
                    target: ldr
                    function onStorageDataChanged() {
                        previewImg.refresh()
                    }
                }
            }


            function getData() {
                moduleData = JS.getSqlFields([comboModeImg,
                                              comboModeImg.currentValue === "image" ?
                                                  textQuestionImgImg :
                                                  textQuestionImgText,
                                              textAnswerImg])

                return moduleData
            }
        }
    }

    Component.onCompleted: {
        if (storageModule == "binding" || storageModule == "numbers")
            ldr.sourceComponent = cmpBinding
        if (storageModule == "images")
            ldr.sourceComponent = cmpImages
        else
            ldr.sourceComponent = cmpNone
    }


    function getData() {
        if (ldr.status == Loader.Ready)
            return ldr.item.getData()

        return {}
    }

    function setStorageData(data) {
        storageData = data
    }

}



