import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

Item {
    id: control

    property Pass pass: null
    property TeacherPass teacherPass: null

    property string closeQuestion: _expGrading.modified ? qsTr("Biztosan eldobod a módosításokat?") : ""


    property alias actionGradingCopy: _actionGradingCopy
    property alias actionGradingPaste: _actionGradingPaste


    onPassChanged: _expGrading.loadData()


    SortFilterProxyModel {
        id: _sortedGradeList
        sourceModel: Client.cache("gradeList")
        sorters: [
            RoleSorter {
                roleName: "gradeid"
            }
        ]
    }


    SortFilterProxyModel {
        id: _filteredPassList
        sourceModel: teacherPass ? teacherPass.passList : null
        filters: [
            ValueFilter {
                roleName: "passid"
                value: pass ? pass.passid : -1
                enabled: pass
                inverted: true
            }
        ]

    }


    QMenu {
        id: _menuFilteredPass

        Instantiator {
            model: _filteredPassList

            delegate: QMenuItem {
                text: model.title + " (" + model.passid + ")"
                onTriggered: {
                    if (!teacherPass.addChild(pass, model.passid))
                        Client.messageWarning(qsTr("Nem sikerült beágyazni a kért elemet"), qsTr("Sikertelen művelet"))
                }
            }

            onObjectAdded: (index, object) => _menuFilteredPass.insertItem(index, object)
            onObjectRemoved: (index, object) => _menuFilteredPass.removeItem(object)
        }
    }

    QScrollable {
        anchors.fill: parent

        QFormColumn {
            id: _form

            spacing: 3

            QExpandableHeader {
                width: parent.width
                text: qsTr("Alapadatok")
                icon: Qaterial.Icons.serverOutline
                button.visible: false
                topPadding: 10 * Qaterial.Style.pixelSizeRatio
            }

            Qaterial.TextField {
                id: _tfName

                width: parent.width
                leadingIconSource: Qaterial.Icons.renameBox
                leadingIconInline: true
                title: qsTr("A Call Pass neve")
                readOnly: !pass

                trailingContent: Qaterial.TextFieldButtonContainer
                {
                    QTextFieldInPlaceButtons {
                        setTo: pass ? pass.title : ""
                        onSaveRequest: text => {
                                           Client.send(HttpConnection.ApiTeacher, "pass/%1/update".arg(pass.passid),
                                                       {
                                                           title: text
                                                       })
                                           .done(control, function(r){
                                               reloadPass()
                                               saved()
                                           })
                                           .fail(control, function(err) {
                                               Client.messageWarning(err, qsTr("Módosítás sikertelen"))
                                               revert()
                                           })
                                       }
                    }
                }
            }


            QDateTimePicker {
                width: parent.width
                canEdit: pass
                title: qsTr("Kezdete")
                onSaveRequest: text => {
                                   Client.send(HttpConnection.ApiTeacher, "pass/%1/update".arg(pass.passid),
                                               {
                                                   starttime: hasDate ? Math.floor(getDateTime()/1000) : -1
                                               })
                                   .done(control, function(r){
                                       reloadPass()
                                       saved()
                                   })
                                   .fail(control, function(err) {
                                       Client.messageWarning(err, qsTr("Módosítás sikertelen"))
                                       revert()
                                   })
                               }
                Component.onCompleted: {
                    hasDate = pass.startTime.getTime()
                    if (hasDate)
                        setFromDateTime(pass.startTime)
                }
            }

            QDateTimePicker {
                width: parent.width
                canEdit: pass
                title: qsTr("Vége")
                hour: 23
                minute: 59
                onSaveRequest: text => {
                                   Client.send(HttpConnection.ApiTeacher, "pass/%1/update".arg(pass.passid),
                                               {
                                                   endtime: hasDate ? Math.floor(getDateTime()/1000) : -1
                                               })
                                   .done(control, function(r){
                                       reloadPass()
                                       saved()
                                   })
                                   .fail(control, function(err) {
                                       Client.messageWarning(err, qsTr("Módosítás sikertelen"))
                                       revert()
                                   })
                               }
                Component.onCompleted: {
                    hasDate = pass.endTime.getTime()
                    if (hasDate)
                        setFromDateTime(pass.endTime)
                }
            }

        }


        QExpandableHeader {
            width: _view.width
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Tartalmi elemek")
            icon: Qaterial.Icons.newspaperVariantMultiple
            button.visible: false
            topPadding: 15 * Qaterial.Style.pixelSizeRatio
        }

        QListView {
            id: _view

            currentIndex: -1
            autoSelectChange: true

            height: contentHeight
            width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
            anchors.horizontalCenter: parent.horizontalCenter

            boundsBehavior: Flickable.StopAtBounds

            model: SortFilterProxyModel {
                id: _itemList

                sourceModel: pass ? pass.itemList : null

                sorters: [
                    RoleSorter {
                        roleName: "itemid"
                        sortOrder: Qt.AscendingOrder
                        priority: 1
                    }
                ]
            }


            delegate: QLoaderItemDelegate {
                id: _delegate

                readonly property PassItem passItem: model.qtObject

                selectableObject: passItem

                highlighted: _view.selectEnabled ? selected : ListView.isCurrentItem
                iconSource: passItem && passItem.includePass > 0 ?
                                Qaterial.Icons.tagMultipleOutline :
                                passItem && passItem.extra ?
                                    Qaterial.Icons.tagPlus :
                                    Qaterial.Icons.tagText

                iconColorBase: passItem && passItem.includePass > 0 ?
                                   Qaterial.Style.iconColor() :
                                   passItem && passItem.extra ?
                                       Qaterial.Colors.green400 :
                                       Qaterial.Style.accentColor

                textColor: passItem && passItem.extra ? Qaterial.Colors.green400 : Qaterial.Style.colorTheme.primaryText

                text: passItem ? passItem.description : ""
                secondaryText: passItem && passItem.categoryId > 0 ?
                                   (passItem.category != "" ? passItem.category : qsTr("Category #%1").arg(passItem.categoryId)) :
                                   ""



                rightSourceComponent: Row {
                    Qaterial.Icon {
                        visible: passItem && passItem.linkType !== PassItem.LinkNone
                        size: Qaterial.Style.largeIcon
                        icon: passItem ? (passItem.linkType == PassItem.LinkCampaign ?
                                              Qaterial.Icons.trophyOutline :
                                              passItem.linkType == PassItem.LinkExam ?
                                                  Qaterial.Icons.paperRoll :
                                                  Qaterial.Icons.beakerQuestion) :
                                         ""

                    }

                    Qaterial.LabelHeadline5 {
                        color: passItem && passItem.extra ?
                                   Qaterial.Colors.green400 :
                                   Qaterial.Style.accentColor
                        visible: passItem && passItem.maxPts > 0
                        text: passItem ? (passItem.extra ? "+" : "") + passItem.maxPts + qsTr(" pt") : ""

                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                onClicked: {
                    let o = null

                    if (passItem.includePass > 0) {
                        let p = Client.findOlmObject(teacherPass.passList, "passid", passItem.includePass)

                        if (p)
                            o = Client.stackPushPage("PageTeacherPass.qml", {
                                                         pass: p,
                                                         teacherPass: teacherPass
                                                     })
                    } else {
                        o = Client.stackPushPage("TeacherPassItemEdit.qml", {
                                                     pass: pass,
                                                     passItem: passItem,
                                                     teacherPass: teacherPass
                                                 })
                    }

                    if (o)
                        o.Component.destruction.connect(reloadPass)
                }


            }

            footer: Row {
                anchors.horizontalCenter: parent.horizontalCenter

                Qaterial.ToolButton {
                    action: _actionAddItem
                    display: AbstractButton.IconOnly
                }

                Qaterial.ToolButton {
                    id: _btnImport

                    icon.source: Qaterial.Icons.plusBoxMultipleOutline
                    display: AbstractButton.IconOnly

                    onClicked: _menuFilteredPass.popup(_btnImport, 0, _btnImport.height)
                }
            }

            Qaterial.Menu {
                id: _contextMenu
                QMenuItem { action: _view.actionSelectAll }
                QMenuItem { action: _view.actionSelectNone }
                Qaterial.MenuSeparator {}
                QMenuItem { action: _actionDelete }
            }

            onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
                                            if (index != -1)
                                            currentIndex = index
                                            _contextMenu.popup(mouseX, mouseY)
                                        }
        }


        Qaterial.Expandable {
            id: _expGrading
            width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
            anchors.horizontalCenter: parent.horizontalCenter
            expanded: true


            property var gradingData: []
            property Repeater gradeModelRepeater: null
            property bool modified: false


            function loadData() {
                gradingData = []

                if (pass)
                    gradingData = pass.getGradingFromData(pass.grading)

                modified = false
            }


            function getData(idx) {
                var d = []

                for (let i=0; i<gradeModelRepeater.model.length; ++i) {

                    if (i === idx)
                        continue

                    let l = []

                    let model = gradeModelRepeater.itemAt(i).model
                    let key = gradeModelRepeater.itemAt(i).progress

                    for (let j=0; j<model.length; ++j) {
                        l.push(model[j])
                    }

                    d.push({
                               key: key,
                               list: l
                           })
                }

                return pass.getGradingFromUi(d)
            }

            header: QExpandableHeader {
                text: qsTr("Értékelés")
                icon: Qaterial.Icons.bullseyeArrow
                expandable: _expGrading
                topPadding: 20 * Qaterial.Style.pixelSizeRatio

                rightSourceComponent: Row {
                    visible: _expGrading.modified

                    Qaterial.AppBarButton {
                        icon.source: Qaterial.Icons.check
                        foregroundColor: Qaterial.Colors.lightGreen400
                        anchors.verticalCenter: parent.verticalCenter

                        onClicked: {
                            Client.send(HttpConnection.ApiTeacher, "pass/%1/update".arg(pass.passid),
                                        {
                                            grading: pass.getMapFromUi(_expGrading.getData())
                                        })
                            .done(control, function(r){
                                reloadPass()
                            })
                            .fail(control, function(err) {
                                Client.messageWarning(err, qsTr("Módosítás sikertelen"))
                            })
                        }
                    }

                    Qaterial.AppBarButton {
                        icon.source: Qaterial.Icons.close
                        foregroundColor: Qaterial.Colors.red400
                        anchors.verticalCenter: parent.verticalCenter
                        ToolTip.text: qsTr("Mégsem")

                        onClicked: _expGrading.loadData()
                    }
                }
            }

            delegate: Column {
                width: _expGrading.width

                Repeater {
                    id: _gradeValueRptr

                    Component.onCompleted: _expGrading.gradeModelRepeater = _gradeValueRptr

                    model: _expGrading.gradingData

                    delegate: Row {
                        readonly property int progress: _spinGrade.value
                        readonly property alias model: _gradeRptr.model

                        spacing: 5

                        Qaterial.SquareButton {
                            icon.source: Qaterial.Icons.delete_
                            icon.color: Qaterial.Colors.red400
                            outlined: false

                            anchors.verticalCenter: parent.verticalCenter

                            onClicked: {
                                let d = _expGrading.getData(index)

                                _expGrading.modified = true
                                _expGrading.gradingData = d
                            }
                        }

                        QSpinBox {
                            id: _spinGrade
                            anchors.verticalCenter: parent.verticalCenter

                            from: 0
                            to: 1000
                            stepSize: 5
                            editable: true

                            value: modelData.key

                            onValueModified: {
                                _expGrading.modified = true
                            }
                        }


                        Row {
                            anchors.verticalCenter: parent.verticalCenter

                            Repeater {
                                id: _gradeRptr

                                model: modelData.list

                                delegate: QButton {
                                    property Grade grade: Client.findCacheObject("gradeList", modelData)

                                    anchors.verticalCenter: parent.verticalCenter
                                    text: grade ? grade.shortname : "???"

                                    onClicked: {
                                        _expGrading.modified = true
                                        _gradeRptr.model.splice(index, 1)
                                    }

                                }
                            }
                        }

                        Qaterial.SquareButton {
                            id: _btnPlus

                            icon.source: Qaterial.Icons.plus
                            icon.color: Qaterial.Colors.green400
                            foregroundColor: icon.color
                            text: qsTr("Jegy")
                            outlined: false
                            anchors.verticalCenter: parent.verticalCenter

                            onClicked: _menuGrade.popup(_btnPlus, 0, _btnPlus.height)

                            QMenu {
                                id: _menuGrade

                                Instantiator {
                                    model: _sortedGradeList

                                    delegate: QMenuItem {
                                        text: model.longname + " (" + model.shortname + ")"
                                        onTriggered: {
                                            _expGrading.modified = true
                                            _gradeRptr.model.push(model.gradeid)
                                        }
                                    }

                                    onObjectAdded: (index, object) => _menuGrade.insertItem(index, object)
                                    onObjectRemoved: (index, object) => _menuGrade.removeItem(object)
                                }
                            }
                        }

                    }

                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter

                    Qaterial.ToolButton {
                        icon.source: Qaterial.Icons.plus

                        onClicked: {
                            let d = _expGrading.getData()

                            d.push({
                                       key: 0,
                                       list: []
                                   })

                            _expGrading.gradingData = d
                            _expGrading.modified = true
                        }
                    }

                    Qaterial.ToolSeparator { }

                    Qaterial.ToolButton {
                        action: _actionGradingCopy
                        display: AbstractButton.IconOnly
                    }

                    Qaterial.ToolButton {
                        action: _actionGradingPaste
                        display: AbstractButton.IconOnly
                    }
                }
            }

        }


    }



    QFabButton {
        visible: _actionAddItem.enabled
        action: _actionAddItem
    }


    Action {
        id: _actionAddItem
        text: qsTr("Új elem")
        icon.source: Qaterial.Icons.plus
        enabled: pass
        onTriggered: {
            if (!pass)
                return

            if (pass.endTime.getTime() && pass.endTime.getTime() < new Date().getTime()) {
                JS.questionDialog(
                            {
                                onAccepted: function()
                                {
                                    addTask()
                                },
                                text: qsTr("A Call Pas már véget ért, biztosan hozzáadsz egy új elemet?"),
                                title: qsTr("Új elem"),
                                iconSource: Qaterial.Icons.progressClose
                            })
                return
            }

            addTask()
        }

        function addTask() {
            let o = Client.stackPushPage("TeacherPassItemEdit.qml", {
                                             pass: pass,
                                             passItem: null,
                                             teacherPass: teacherPass
                                         })

            if (o)
                o.Component.destruction.connect(reloadPass)
        }
    }

    Action {
        id: _actionDelete
        icon.source: Qaterial.Icons.delete_
        text: qsTr("Törlés")
        enabled: _view.currentIndex != -1 || _view.selectEnabled
        onTriggered: {
            var l = _view.getSelected()

            if (!l.length)
                return

            JS.questionDialogPlural(l, qsTr("Biztosan törlöd a kijelölt %1 elemet?"), "description",
                                    {
                                        onAccepted: function()
                                        {
                                            Client.send(HttpConnection.ApiTeacher,  "passItem/delete", {
                                                            list: JS.listGetFields(l, "itemid")
                                                        })
                                            .done(control, function(r){
                                                reloadPass()
                                                _view.unselectAll()
                                            })
                                            .fail(control, JS.failMessage("Törlés sikertelen"))

                                        },
                                        title: qsTr("Tartalmi elem törlése"),
                                        iconSource: Qaterial.Icons.delete_
                                    })

        }
    }

    Action {
        id: _actionGradingCopy
        enabled: _expGrading.gradingData.length > 0
        icon.source: Qaterial.Icons.contentCopy
        text: qsTr("Értékelés másolása")
        onTriggered: {
            let t = {
                type: "grading",
                list: _expGrading.getData()
            }

            Client.Utils.setClipboardText(JSON.stringify(t))

            Client.snack(qsTr("Értékelés a vágólapra másolva"))
        }
    }


    Action {
        id: _actionGradingPaste
        icon.source: Qaterial.Icons.contentPaste
        text: qsTr("Értékelés beillesztése")
        onTriggered: {
            let d = {}

            try {
                d = JSON.parse(Client.Utils.clipboardText())
            } catch (error) {
                Client.snack(qsTr("Érvénytelen tartalom a vágólapon"))
                return
            }

            if (d.type === undefined || d.type !== "grading") {
                Client.snack(qsTr("Érvénytelen tartalom a vágólapon"))
                return
            }

            _expGrading.modified = true
            _expGrading.gradingData = d.list
        }
    }




    Connections {
        target: pass

        function onItemsLoaded() {
            _expGrading.loadData()
        }
    }


    function reloadPass() {
        if (pass)
            pass.reload(HttpConnection.ApiTeacher)
    }

    Component.onCompleted: _expGrading.loadData()

}
