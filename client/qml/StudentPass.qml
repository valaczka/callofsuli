import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel


Item {
    id: _control

    required property Pass pass

    property bool showPlaceholders: true


    implicitWidth: 600
    implicitHeight: col.height+85//+50

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
        source: "qrc:/internal/img/paper_bw.png"
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
                    text: pass ? (pass.title != "" ? "" : qsTr("Call Pass #%1").arg(pass.passid)) : ""
                    width: parent.width
                    wrapMode: Text.NoWrap
                    elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone
                    font.family: "HVD Peace"
                    font.pixelSize: Qaterial.Style.textTheme.headline4.pixelSize*0.9
                    color: "saddlebrown"
                }

                Qaterial.LabelCaption {
                    //visible: campaingDetails
                    text: pass ? pass.startTime.toLocaleString(Qt.locale(), "yyyy. MMM d. – ")
                                 + (pass.endTime.getTime() ? pass.endTime.toLocaleString(Qt.locale(), "yyyy. MMM d.") : "")
                               : ""
                    width: parent.width
                    wrapMode: Text.NoWrap
                    elide: implicitWidth > width ? Text.ElideRight : Text.ElideNone
                    color: "saddlebrown"
                }
            }

            Label {
                id: _labelResult

                anchors.verticalCenter: parent.verticalCenter

                text: "???" /*campaign ? campaign.readableShortResult(campaign.resultGrade, campaign.resultXP,
                                                                                                                                                          campaign.maxPts > 0 ? Math.round(campaign.progress * campaign.maxPts) : -1) : ""
                                */
                wrapMode: Text.Wrap
                color: Qaterial.Colors.black
                font.family: Qaterial.Style.textTheme.headline5.family
                font.pixelSize: Qaterial.Style.textTheme.headline5.pixelSize
                font.capitalization: Font.AllUppercase
                font.weight: Font.DemiBold
                topPadding: 5
                bottomPadding: 5
            }
        }



        Column {
            id: _colItemList

            width: parent.width

            Repeater {
                id: _rptr

                model: pass ? pass.itemList : null

                delegate: Item {
                    width: parent.width
                    height: showPlaceholders || passItem ? _itemDelegate.implicitHeight : 15//_section.implicitHeight

                    readonly property PassItem passItem: model.qtObject

                    /*Qaterial.LabelCaption {
                                                                                id: _section
                                                                                visible: !_rptr.showPlaceholders && !task
                                                                                width: parent.width
                                                                                text: modelData.section !== undefined ? modelData.section : ""
                                                                                color: "saddlebrown"
                                                                                topPadding: 15
                                                                                bottomPadding: 5
                                                                                font.pixelSize: Qaterial.Style.textTheme.caption.pixelSize
                                                                                font.family: Qaterial.Style.textTheme.caption.family
                                                                                font.capitalization: Font.AllUppercase
                                                                                font.weight: Qaterial.Style.textTheme.caption.weight
                                                                        }*/

                    Row {
                        id: _itemDelegate

                        visible: pass && !showPlaceholders

                        width: parent.width

                        spacing: 10

                        Qaterial.LabelBody2 {
                            color: "black"
                            text: passItem.description+" / "+passItem.category + " - " +passItem.pts + ":" + passItem.result
                        }

                    }

                    QPlaceholderItem {
                        visible: showPlaceholders
                        anchors.fill: parent
                        horizontalAlignment: Qt.AlignLeft
                        height: 48
                        heightRatio: 0.8
                    }

                }
            }

        }

    }


}
