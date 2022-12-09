import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
    id: control

    title: qsTr("Új feladat")
    icon: CosStyle.iconAdd

    property int contextAction: MapEditorAction.ActionTypeObjective
    property int actionContextType: -1
    property var actionContextId: null

    property MapEditor _mapEditor: null
    property GameMapEditorChapter chapter: null

    property GameMapEditorObjective objective: null
    property bool duplicate: false

    property string objectiveModule: ""
    property int storageId: -1
    property string storageModule: ""
    property int storageCount: 0
    property var objectiveData: null
    property var storageData: null

    property bool modified: false
    property bool _closeEnabled: false

    signal close()

    property ListModel availableObjectiveModel: ListModel {}
    property ListModel availableStorageModel: ListModel {}

    property var _availableStorageModules: []

    StackView {
        id: stack
        anchors.fill: parent
        clip: !control.compact
    }



    Component {
        id: cmpObjectiveModules

        QObjectListView {
            id: list

            isFullscreen: control.compact

            model: SortFilterProxyModel {
                sourceModel: availableObjectiveModel

                sorters: StringSorter {
                    roleName: "name"
                }
            }

            modelTitleRole: "name"

            autoSelectorChange: false

            leftComponent: QFontImage {
                width: list.delegateHeight+10
                height: list.delegateHeight
                size: height*0.85
                icon: model.icon
            }

            header: QTabHeader {
                tabContainer: control
                visible: control.compact
                flickable: list
            }

            onClicked: {
                var d = model.get(index)

                objectiveModule = d.module
                _availableStorageModules = []

                control.title = d.name
                control.icon = d.icon

                var i=0

                while (true) {
                    if (!d.storageModules[i])
                        break

                    _availableStorageModules.push(d.storageModules[i])

                    i++
                }

                if (_availableStorageModules.length) {
                    stack.replace(cmpStorages)
                } else {
                    storageId = -1
                    stack.replace(cmpEdit)
                }
            }
        }

    }



    Component {
        id: cmpStorages

        QListView {
            id: slist

            model: SortFilterProxyModel {
                sourceModel: availableStorageModel

                filters: ExpressionFilter {
                    expression: _availableStorageModules.includes(model.module) || model.module === ""
                }

                proxyRoles: [
                    SwitchRole {
                        name: "isNew"
                        defaultValue: false
                        filters: ExpressionFilter {
                            expression: model.id < 0
                            SwitchRole.value: true
                        }
                    },
                    SwitchRole {
                        name: "isNoStorage"
                        defaultValue: false
                        filters: ExpressionFilter {
                            expression: model.id === -1 && model.module === ""
                            SwitchRole.value: true
                        }
                    }
                ]

                sorters: [
                    FilterSorter {
                        filters: ValueFilter { roleName: "isNoStorage"; value: true }
                        priority: 3
                    },
                    FilterSorter {
                        filters: ValueFilter { roleName: "isNew"; value: true }
                        priority: 2
                    },
                    StringSorter {
                        roleName: "name"
                        priority: 1
                    },
                    RoleSorter {
                        roleName: "id"
                    }
                ]

            }

            delegate: Item {
                id: item
                width: slist.width
                height: CosStyle.twoLineHeight*1.2

                required property int id
                required property string module
                required property string name
                required property string icon
                required property string title
                required property string details
                required property string image
                required property int objectiveCount
                required property var storageData
                required property bool isNew


                QRectangleBg {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton

                    Item {
                        id: rect
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 20


                        QLabel {
                            id: labelName
                            text: item.name
                            color: CosStyle.colorPrimary
                            font.weight: Font.DemiBold
                            font.pixelSize: CosStyle.pixelSize*0.6
                            font.capitalization: Font.AllUppercase
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.leftMargin: 3
                            anchors.topMargin: 1
                        }

                        Row {
                            anchors.verticalCenter: parent.verticalCenter

                            spacing: 0

                            QFontImage {
                                id: imgModule
                                width: Math.max(rect.height*0.5, size*1.1)
                                size: CosStyle.pixelSize*1.5
                                anchors.verticalCenter: parent.verticalCenter
                                icon: item.icon
                                color: item.isNew ? CosStyle.colorAccent : CosStyle.colorPrimary
                            }


                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.verticalCenterOffset: (labelName.height/2)*(subtitle.lineCount-1)/3

                                QLabel {
                                    id: title
                                    anchors.left: parent.left
                                    width: rect.width-imgModule.width
                                           -(badge.visible ? badge.width : 0)
                                           -(img.visible ? img.width : 0)
                                    text: item.title
                                    color: item.isNew ? CosStyle.colorAccentLighter : CosStyle.colorPrimaryLighter
                                    font.pixelSize: CosStyle.pixelSize
                                    font.weight: Font.Normal
                                    maximumLineCount: 1
                                    lineHeight: 0.9
                                    elide: Text.ElideRight
                                    leftPadding: 10
                                }
                                QLabel {
                                    id: subtitle
                                    anchors.left: parent.left
                                    width: title.width
                                    text: item.details
                                    visible: text.length
                                    color: CosStyle.colorPrimary
                                    font.pixelSize: CosStyle.pixelSize*0.75
                                    font.weight: Font.Light
                                    maximumLineCount: 1
                                    lineHeight: 0.8
                                    wrapMode: Text.Wrap
                                    elide: Text.ElideRight
                                    leftPadding: 10
                                }
                            }

                            Item {
                                id: img
                                width: imgImg.width+20
                                height: imgImg.height
                                anchors.verticalCenter: parent.verticalCenter
                                visible: item.image.length

                                Image {
                                    id: imgImg
                                    anchors.centerIn: parent
                                    height: item.height*0.9
                                    width: height
                                    source: item.image
                                    fillMode: Image.PreserveAspectFit
                                }
                            }

                            QBadge {
                                id: badge
                                text: item.objectiveCount
                                color: CosStyle.colorWarningDark
                                anchors.verticalCenter: parent.verticalCenter
                                visible: item.objectiveCount > 0
                            }

                        }
                    }


                    mouseArea.onClicked: {
                        storageId = item.id > 0 ? item.id : -1
                        control.storageData = item.module === "" ? null : item.storageData
                        storageModule = item.module
                        stack.replace(cmpEdit)
                    }
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    width: parent.width
                    height: 1
                    color: CosStyle.colorPrimaryDark
                }
            }

            header: QTabHeader {
                tabContainer: control
                visible: control.compact
                flickable: slist
            }

        }
    }


    Component {
        id: cmpEdit

        QAccordion {
            anchors.fill: undefined

            QTabHeader {
                tabContainer: control
                visible: control.compact
                isPlaceholder: false
            }

            Loader {
                id: storageLoader
                width: parent.width

                Connections {
                    target: storageLoader.item
                    function onModified() {
                        control.modified = true
                    }
                }
            }

            Loader {
                id: objectiveLoader
                width: parent.width

                Connections {
                    target: objectiveLoader.item
                    function onModified() {
                        control.modified = true
                    }
                }
            }

            QButton {
                id: buttonYes

                anchors.horizontalCenter: parent.horizontalCenter

                text: qsTr("OK")
                icon.source: "qrc:/internal/icon/check-bold.svg"
                themeColors: CosStyle.buttonThemeGreen

                onClicked: {
                    save()
                    control.close()
                }
            }


            Component.onCompleted: {
                if (storageModule != "") {
                    var q = _mapEditor.storageQml(storageModule)
                    storageLoader.setSource(q, {
                                                mapEditor: _mapEditor,
                                                moduleData: control.storageData,
                                                editable: (control.storageId == -1)
                                            })

                }

                if (objectiveModule != "") {
                    var q2 = _mapEditor.objectiveQml(objectiveModule)
                    objectiveLoader.setSource(q2, {
                                                  mapEditor: _mapEditor,
                                                  moduleData: objectiveData,
                                                  storageData: control.storageData,
                                                  storageModule: storageModule,
                                                  storageCount: storageCount
                                              })
                }
            }


            Connections {
                target: storageLoader.item

                function onModuleDataChanged() {
                    control.storageData = storageLoader.item.moduleData
                }
            }

            Connections {
                target: control

                function onStorageDataChanged() {
                    if (objectiveLoader.status == Loader.Ready) {
                        objectiveLoader.item.setStorageData(control.storageData ? control.storageData : {})
                    }
                }
            }



            function getObjectiveData() {
                if (objectiveLoader.status == Loader.Ready)
                    return objectiveLoader.item.getData()

                return {}
            }

            function getStorageData() {
                if (storageLoader.status == Loader.Ready)
                    return storageLoader.item.getData()

                return {}
            }

            function getStorageCount() {
                if (objectiveLoader.status == Loader.Ready)
                    return objectiveLoader.item.storageCount

                return 0
            }
        }
    }



    Component.onCompleted: {
        var l = _mapEditor.availableObjectives

        for (var i=0; i<l.length; i++) {
            availableObjectiveModel.append(l[i])
        }


        var sl = _mapEditor.getStorages()

        for (var j=0; j<sl.length; j++) {
            availableStorageModel.append(sl[j])
        }


        // Add no storage

        availableStorageModel.append({
                                         id: -1,
                                         module: "",
                                         name: "",
                                         title: qsTr("Adatbank nélkül"),
                                         icon: "image://font/Material Icons/\ue06f",
                                         details: "",
                                         image: "",
                                         storageData: {},
                                         objectiveCount: 0
                                     })

        if (objective)
            loadObjective()
        else {
            stack.replace(cmpObjectiveModules)
        }
    }


    function loadObjective() {
        if (!objective)
            return

        objectiveModule = objective.module
        storageId = objective.storageId
        storageModule = objective.storageModule
        storageCount = objective.storageCount
        objectiveData = objective.data
        control.storageData = objective.storageData

        title = objective.info[0]
        icon = objective.info[1]

        stack.replace(cmpEdit)

        modified = false
    }


    backCallbackFunction: function () {
        if (modified && !_closeEnabled) {
            var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
            d.accepted.connect(function() {
                _closeEnabled = true
                mainStack.back()
            })
            d.open()
            return true
        }

        return false
    }


    closeCallbackFunction: function () {
        if (modified && !_closeEnabled) {
            var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
            d.accepted.connect(function() {
                _closeEnabled = true
                mainWindow.close()
            })
            d.open()
            return true
        }

        return false
    }


    function loadContextId(uuid) {
        if (objective && objective.uuid === uuid) {
            loadObjective()
        }

    }


    function save() {
        var odata = stack.currentItem.getObjectiveData()
        var sdata = stack.currentItem.getStorageData()
        var sc = stack.currentItem.getStorageCount()

        if (objective && !duplicate) {
            _mapEditor.objectiveModify(chapter, objective,
                                       {
                                           data: odata,
                                           storageCount: sc
                                       },
                                       sdata)
        } else {
            _mapEditor.objectiveAdd(chapter,
                                    {
                                        module: objectiveModule,
                                        data: odata,
                                        storageCount: sc
                                    },
                                    {
                                        id: storageId,
                                        module: storageModule,
                                        data: sdata
                                    })

        }

        modified = false
    }

}
