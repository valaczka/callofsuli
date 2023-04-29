import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Item {
	id: root

	property var resultData: ({})
	property string name: ""

	implicitWidth: 200
	implicitHeight: 200

	width: Math.min(parent.width, 1200)
	height: col.height+85

	BorderImage {
		width: parent.width/scale
		height: parent.height/scale
		source: "qrc:/internal/img/paper_bw.png"
		border {
			left: 253
			top: 129
			right: 1261-1175
			bottom: 851-737
		}
		scale: Qaterial.Style.dense ? 0.3 : 0.15
		transformOrigin: Item.TopLeft
	}

	Column {
		id: col
		x: Qaterial.Style.dense ? 50 : 25
		width: parent.width-2*x
		y: Qaterial.Style.dense ? 35 : 20

		Qaterial.Label {
			text: name
			width: Math.min(implicitWidth, parent.width)
			anchors.horizontalCenter: parent.horizontalCenter
			horizontalAlignment: Text.AlignHCenter
			wrapMode: Text.Wrap
			font.family: "HVD Peace"
			font.pixelSize: Qaterial.Style.textTheme.headline4.pixelSize
			color: Qaterial.Colors.black
			visible: text != ""
			leftPadding: 30
			rightPadding: 30
		}

		Qaterial.LabelHeadline5 {
			id: _title
			anchors.horizontalCenter: parent.horizontalCenter
			horizontalAlignment: Text.AlignHCenter

			property int num: 0

			text: resultData.success ? qsTr("%1%\nSikeresen teljesítve").arg(num)
									 : qsTr("%1%\nSikertelen megoldás").arg(num)

			color: resultData.success ? Qaterial.Colors.green500 : Qaterial.Colors.red500

			topPadding: 10
			bottomPadding: 10
		}


		/*Qaterial.LabelCaption {
			visible: campaingDetails
			text: campaign ? campaign.startTime.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm – ")
							 + (campaign.endTime.getTime() ? campaign.endTime.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm") : "")
						   : ""
			width: parent.width
			wrapMode: Text.Wrap
			color: root.finished ? "black" :"saddlebrown"
		}*/

		Repeater {
			model: resultData.list ? resultData.list : null

			delegate: Column {
				width: col.width

				readonly property int _idx: index+1
				readonly property string _module: modelData.module
				readonly property int _point: modelData.question ? Number(modelData.question.xpFactor)*10 : 0
				readonly property bool _success: modelData.success
				readonly property var _question: modelData.question
				readonly property var _answer: modelData.answer
				readonly property string _qml: Client.moduleTestResult(_module)

				Row {
					width: parent.width
					spacing: 10

					Qaterial.LabelHeadline5 {
						width: parent.width-_labelPoint.width-parent.spacing
						wrapMode: Text.Wrap
						color: Qaterial.Colors.black

						text: qsTr("%1. %2").arg(_idx).arg(_question ? _question.question : "")
						anchors.bottom: _labelPoint.bottom
					}

					Qaterial.LabelSubtitle1 {
						id: _labelPoint
						color: Qaterial.Colors.black

						text: _success ? qsTr("<font color=\"#4CAF50\">%1</font> / %2").arg(_point).arg(_point)
									   : qsTr("<font color=\"#F44336\">0</font> / %1").arg(_point)

						topPadding: 25*Qaterial.Style.pixelSizeRatio
					}
				}

				Loader {
					width: parent.width-40
					anchors.left: parent.left
					anchors.leftMargin: 20
					source: _qml
					sourceComponent: _qml == "" ? _cmpPh : undefined
				}

				Component {
					id: _cmpPh

					GameTestResultBase {
						GameTestResultLabelAnswer {
							text: JSON.stringify(_question)
							success: _success
							width: parent.width
							wrapMode: Text.Wrap
						}
					}
				}
			}
		}

	}

	onResultDataChanged: {
		var p = Number(resultData.points)
		var max = Number(resultData.maxPoints)

		console.error("****", p, max, p/max, 100*(p/max), Math.floor(100*(p/max)), "---", resultData.points, resultData.maxPoints)

		_title.num = max > 0 ? Math.floor(100*(p/max)) : 0
	}

}
