import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "JScript.js" as JS

Item {
    id: _control

    required property Pass pass

    property bool showPlaceholders: true


    implicitWidth: 600
    implicitHeight: Math.max(250, col.height + 30)

    height: implicitHeight




    BorderImage {
        id: border1
        source: "qrc:/internal/img/borderPass.svg"
        visible: false

        //sourceSize.height: 222
        //sourceSize.width: 127

        anchors.fill: parent
        border.top: 105
        border.left: 18
        border.right: 23
        border.bottom: 22

        horizontalTileMode: BorderImage.Round
        verticalTileMode: BorderImage.Repeat
    }


    Image {
        source: "qrc:/internal/img/paper_texture.png"
        fillMode: Image.Tile
        anchors.fill: parent

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: BorderImage {
                id: border2
                source: "qrc:/internal/img/borderPassFull.svg"
                visible: false

                width: _control.width
                height: _control.height

                border.top: border1.border.top
                border.left: border1.border.left
                border.right: border1.border.right
                border.bottom: border1.border.bottom

                horizontalTileMode: BorderImage.Round
                verticalTileMode: BorderImage.Repeat
            }
        }
    }

    ColorOverlay {
        anchors.fill: border1
        source: border1
        color: "burlywood"
    }


    Column {
        id: col
        x: 30
        width: parent.width-2*x
        y: 2

        Row {
            width: parent.width
            spacing: 10

            bottomPadding: 40

            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width-_labelResult.width-parent.spacing

                spacing: -3

                Qaterial.Label {
                    text: pass ? (pass.title != "" ? pass.title : qsTr("Call Pass #%1").arg(pass.passid)) : ""
                    width: parent.width
                    wrapMode: Text.NoWrap
                    elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone
                    font.family: "HVD Peace"
                    font.pixelSize: Qaterial.Style.textTheme.headline4.pixelSize*0.9
                    color: "saddlebrown"
                }

                Qaterial.LabelCaption {
                    text: pass ? JS.readableTimestampMin(pass.startTime) + " – "
                                 + (pass.endTime.getTime() ? JS.readableTimestampMin(pass.endTime) : "")
                               : ""
                    width: parent.width
                    wrapMode: Text.NoWrap
                    elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone
                    color: "saddlebrown"
                }
            }

            Row {
                id: _labelResult

                anchors.verticalCenter: parent.verticalCenter

                spacing: 5

                Repeater {
                    model: pass ? pass.gradeList : null

                    delegate: Label {
                        anchors.verticalCenter: parent.verticalCenter

                        text: modelData.shortname
                        color: Qaterial.Colors.red700
                        font.family: Qaterial.Style.textTheme.headline5.family
                        font.pixelSize: Qaterial.Style.textTheme.headline5.pixelSize
                        font.capitalization: Font.AllUppercase
                        font.weight: Font.Bold
                        topPadding: 5
                        bottomPadding: 5
                    }
                }
            }

        }


        Column {
            id: _placeholderList

            width: parent.width

            visible: showPlaceholders

            Repeater {
                model: 5

                delegate: QPlaceholderItem {
                    width: parent.width
                    horizontalAlignment: Qt.AlignLeft
                    height: 24
                    heightRatio: 0.8
                }

            }

        }

        Column {
            id: _categoryList

            visible: pass && !showPlaceholders

            width: parent.width

            Repeater {
                model: pass ? pass.categoryList : null

                delegate: Column {
                    id: _category

                    readonly property int categoryId: id

                    width: parent.width

                    Qaterial.LabelCaption {
                        width: parent.width
                        text: description
                        color: "saddlebrown"
                        topPadding: index > 0 ? 15 : 0
                        //bottomPadding: 5
                        font.pixelSize: Qaterial.Style.textTheme.caption.pixelSize
                        font.family: Qaterial.Style.textTheme.caption.family
                        font.capitalization: Font.AllUppercase
                        font.weight: Qaterial.Style.textTheme.caption.weight
                    }

                    Repeater {
                        model: SortFilterProxyModel {
                            sourceModel: pass ? pass.itemList : null

                            filters: [
                                ValueFilter {
                                    roleName: "categoryId"
                                    value: _category.categoryId
                                }
                            ]

                            sorters: RoleSorter {
                                roleName: "itemid"
                            }
                        }

                        delegate: Row {
                            readonly property PassItem passItem: model.qtObject

                            Qaterial.LabelBody2 {
                                anchors.verticalCenter: parent.verticalCenter

                                width: _category.width - _labelPts.width
                                leftPadding: 15

                                wrapMode: Text.NoWrap
                                elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone

                                color: passItem.extra ? Qaterial.Colors.green800 : Qaterial.Colors.black
                                text: (passItem.extra ? "+" : "") + passItem.description
                            }

                            Qaterial.LabelBody2 {
                                id: _labelPts

                                anchors.verticalCenter: parent.verticalCenter

                                textFormat: Text.StyledText

                                color: passItem.extra ? Qaterial.Colors.green800 : Qaterial.Colors.black
                                text: (passItem.extra ? "<b>+</b>" : "") + qsTr("<b>%1</b> / %2").arg(pass.round(passItem.pts)).arg(passItem.maxPts)
                            }
                        }
                    }
                }
            }
        }

    }


}
