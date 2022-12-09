import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

Column {
    id: control
    property string fieldName: ""
    property string sqlField: ""
    property var sqlData: _generateData()
    property bool modified: false

    property bool watchModification: parent.watchModification

    property int initialCount: 5
    property string separator: "—"

    property bool readOnly: false

    signal modification()
    signal imageAddRequest()
    signal imageAddRequestSuccess(int imageId)

    Layout.fillWidth: true
    Layout.bottomMargin: parent.columns === 1 ? 10 : 0

    spacing: 5

    Column {
        id: fieldColumn
        width: parent.width

        spacing: 0

        property bool modified: false
        property bool watchModification: control.watchModification

        onModifiedChanged: if (control.watchModification) {
                               control.modified = true
                               control.parent.modified = true
                           }

        Component {
            id: fieldComponent

            QGridDoubleImageField {
                id: field
                width: parent.width
                separator: control.separator
                canDelete: parent.children && parent.children.length > 1 && !control.readOnly
                readOnly: control.readOnly

                onDeleteAction: {
                    modification()

                    if (parent.watchModification)
                        parent.modified = true

                    destroy()
                }

                onAcceptAction: if (parent.children && parent.children.length > 1) {
                                    if (parent.children[parent.children.length-1] === field) {
                                        addField()
                                        modification()
                                    } else {
                                        for (var i=0; i<parent.children.length; i++) {
                                            if (parent.children[i] === field) {
                                                parent.children[i+1].second.forceActiveFocus()
                                                break
                                            }
                                        }
                                    }

                                } else {
                                    addField()
                                    modification()
                                }

                onModifyAction: modification()

                onImageAddAction: {
                    imageAddRequest()
                }

                Connections {
                    target: control

                    function onImageAddRequestSuccess(imageId) {
                        if (field.waitForImage) {
                            field.first.imageId = imageId
                            field.waitForImage = false
                        }
                    }
                }

            }
        }
    }

    QToolButtonFooter {
        width: parent.width
        icon.source: CosStyle.iconAdd
        text: qsTr("Hozzáadás")
        visible: !control.readOnly
        onClicked: {
            addField()
            modification()
            if (control.watchModification) {
                control.modified = true
                control.parent.modified = true
            }
        }
    }


    Component.onCompleted: {
        for (var i=0; i<initialCount; i++)
            addField()
    }


    function addField(text1, text2) {
        var o = fieldComponent.createObject(fieldColumn)

        if (text1)
            o.first.setData(text1)

        if (text2)
            o.second.setData(text2)

        if (!readOnly)
            o.second.forceActiveFocus()
    }



    function setData(list) {
        for (var i=0; i<list.length; i++) {
            var o=list[i]

            if (fieldColumn.children && fieldColumn.children.length > i) {
                fieldColumn.children[i].first.setData(o.image)
                fieldColumn.children[i].second.setData(o.text)
            } else {
                addField(o.image, o.text)
            }
        }

        control.modified = false
    }

    function _generateData() {
        var d = []

        for (var i=0; fieldColumn.children && i<fieldColumn.children.length; i++) {
            var f = fieldColumn.children[i]
            if (f.first.imageId === -1 && f.second.text === "")
                continue

            var p = {}
            p.image = f.first.imageId
            p.text = f.second.text

            d.push(p)
        }

        return d
    }
}
