import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Item {
	id: control

	implicitWidth: 300
	implicitHeight: 300

	property bool forceFullName: false


	ListModel {
		id: progressModel
	}


	QLabel {
		id: lblSelect
		visible: !flickable.visible
		anchors.centerIn: parent
		text: qsTr("Válassz felhasználót")
	}


	Flickable {
		id: flickable
		visible: false
		width: parent.width-20
		height: Math.min(parent.height, contentHeight)
		anchors.centerIn: parent

		contentWidth: col.width
		contentHeight: col.height

		clip: true

		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator { }

		Column {
			id: col
			width: flickable.width

			spacing: 10

			topPadding: 20
			bottomPadding: 20

			Row {
				id: row
				anchors.horizontalCenter: parent.horizontalCenter

				spacing: 15

				Image {
					id: imgRank
					source: ""

					width: 75
					height: 75

					fillMode: Image.PreserveAspectFit

					anchors.verticalCenter: parent.verticalCenter
				}

				QLabel {
					id: labelName
					font.pixelSize: CosStyle.pixelSize*1.7
					font.weight: Font.Normal
					color: CosStyle.colorAccentLight

					anchors.verticalCenter: parent.verticalCenter

					elide: Text.ElideRight
					width: Math.min(implicitWidth, col.width-row.spacing-imgRank.width)
				}
			}

			QLabel {
				id: labelRank
				anchors.horizontalCenter: parent.horizontalCenter
				font.pixelSize: CosStyle.pixelSize*1.3
				font.weight: Font.Normal
				color: CosStyle.colorAccentLighter

				horizontalAlignment: Text.AlignHCenter

				elide: Text.ElideRight
				width: Math.min(implicitWidth, col.width)

				bottomPadding: 35
			}

			Repeater {
				model: progressModel

				Row {
					required property string field
					required property string iicon
					required property int ilevel
					required property bool ideathmatch

					required property real min
					required property real max
					required property real v

					property color icolor: switch (field) {
										   case "xp": CosStyle.colorPrimaryLighter
											   break
										   case "cStreak": CosStyle.colorAccent
											   break
										   case "lStreak": CosStyle.colorWarning
											   break
										   case "t1": "goldenrod"
											   break
										   case "d1": "crimson"
											   break
										   case "t2": "limegreen"
											   break
										   case "d2": "dodgerblue"
											   break
										   case "t3": "gold"
											   break
										   case "d3": "orchid"
											   break
										   default:
											   CosStyle.colorPrimary
										   }

					spacing: 10

					property int imageSize: 35

					anchors.horizontalCenter: parent.horizontalCenter

					QFontImage {
						icon: iicon
						visible: iicon.length
						size: imageSize
						color: icolor
						anchors.verticalCenter: parent.verticalCenter
						width: lblNum.width
					}

					QTrophyImage {
						visible: ilevel > 0
						level: Math.max(ilevel, 1)
						isDeathmatch: ideathmatch
						anchors.verticalCenter: parent.verticalCenter
						height: imageSize*0.6
						width: lblNum.width
						opacity: v > 0 ? 1.0 : 0.3
					}

					ProgressBar {
						id: bar
						width: ilevel > 0 ? Math.min(col.width-4*(lblNum.width+10), 400-2*(lblNum.width+10))
										  : Math.min(col.width-2*(lblNum.width+10), 400)
						from: min
						to: max
						value: v
						Material.accent: icolor

						anchors.verticalCenter: parent.verticalCenter

						Behavior on value {
							NumberAnimation { duration: 450; easing.type: Easing.OutQuad }
						}
					}

					QLabel {
						id: lblNum
						width: 75
						font.pixelSize: 18
						font.weight: Font.DemiBold
						text: Math.floor(bar.value)+(field == "xp" ? " XP" : "")
						color: icolor
						anchors.verticalCenter: parent.verticalCenter
						horizontalAlignment: Label.AlignHCenter
					}
				}
			}

		}
	}


	Component.onCompleted: {
		progressModel.append({ field: "xp", iicon: CosStyle.iconXPgraph, min: 0, max: 1, v: 0, ilevel: 0, ideathmatch: false })
		progressModel.append({ field: "cStreak", iicon: CosStyle.iconStreak, min: 0, max: 1, v: 0, ilevel: 0, ideathmatch: false})
		progressModel.append({ field: "lStreak", iicon: CosStyle.iconStreakMax, min: 0, max: 1, v: 0, ilevel: 0, ideathmatch: false})
		progressModel.append({ field: "t1", iicon: "", min: 0, max: 1, v: 0, ilevel: 1, ideathmatch: false})
		progressModel.append({ field: "d1", iicon: "", min: 0, max: 1, v: 0, ilevel: 1, ideathmatch: true})
		progressModel.append({ field: "t2", iicon: "", min: 0, max: 1, v: 0, ilevel: 2, ideathmatch: false})
		progressModel.append({ field: "d2", iicon: "", min: 0, max: 1, v: 0, ilevel: 2, ideathmatch: true})
		progressModel.append({ field: "t3", iicon: "", min: 0, max: 1, v: 0, ilevel: 3, ideathmatch: false})
		progressModel.append({ field: "d3", iicon: "", min: 0, max: 1, v: 0, ilevel: 3, ideathmatch: true})
	}

	function loadUserScore(jsonData) {
		flickable.visible = true

		imgRank.source = cosClient.rankImageSource(jsonData.rankid, -1, jsonData.rankimage)

		labelName.text = jsonData.nickname.length && !forceFullName ? jsonData.nickname : jsonData.firstname+" "+jsonData.lastname
		labelRank.text = jsonData.rankname+(jsonData.ranklevel > 0 ? qsTr(" (lvl %1)").arg(jsonData.ranklevel) : "")

		progressModel.setProperty(0, "max", jsonData.maxXP)
		progressModel.setProperty(0, "v", jsonData.xp)

		progressModel.setProperty(1, "max", jsonData.maxStreak)
		progressModel.setProperty(1, "v", jsonData.currentStreak)

		progressModel.setProperty(2, "max", jsonData.maxStreak)
		progressModel.setProperty(2, "v", jsonData.longestStreak)

		progressModel.setProperty(3, "max", jsonData.maxT1)
		progressModel.setProperty(3, "v", jsonData.t1)

		progressModel.setProperty(4, "max", jsonData.maxD1)
		progressModel.setProperty(4, "v", jsonData.d1)

		progressModel.setProperty(5, "max", jsonData.maxT2)
		progressModel.setProperty(5, "v", jsonData.t2)

		progressModel.setProperty(6, "max", jsonData.maxD2)
		progressModel.setProperty(6, "v", jsonData.d2)

		progressModel.setProperty(7, "max", jsonData.maxT3)
		progressModel.setProperty(7, "v", jsonData.t3)

		progressModel.setProperty(8, "max", jsonData.maxD3)
		progressModel.setProperty(8, "v", jsonData.d3)
	}
}
