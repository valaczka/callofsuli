import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Popup {
	id: popupItem

	closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
	modal: false
	focus: false

	x: parent.width-width
	y: 0

	property int openedWidth: Math.min(600, parent.width-10)
	property int openedHeight: Math.min(450, parent.height-10)
	readonly property bool running: transEnter.running || transExit.running

	readonly property real scaleFactor: if (parent.width < 600 || parent.height < 450)
											0.75
										else
											1.0

	background: Rectangle {
		id: rect
		color: "transparent"

		DropShadow {
			id: dropshadow
			anchors.fill: border2
			horizontalOffset: 3
			verticalOffset: 3
			color: JS.setColorAlpha("black", 0.75)
			source: border2
		}

		BorderImage {
			id: border2
			source: "qrc:/internal/img/border2.svg"
			visible: false

			//sourceSize.height: 141
			//sourceSize.width: 414

			height: parent.height-5
			width: parent.width
			anchors.rightMargin: dropshadow.horizontalOffset
			border.top: 10
			border.left: 5
			border.right: 80
			border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}


		Image {
			id: metalbg
			source: "qrc:/internal/img/metalbg3.png"
			visible: false
			fillMode: Image.Tile
			anchors.fill: border2
		}

		OpacityMask {
			id: opacity1
			anchors.fill: border2
			source: metalbg
			maskSource: border2

			opacity: 0.7
		}

		BorderImage {
			id: border1
			source: "qrc:/internal/img/border1.svg"
			visible: false

			//sourceSize.height: 141
			//sourceSize.width: 414

			anchors.fill: parent
			border.top: 15
			border.left: 10
			border.right: 60
			border.bottom: 25

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}

		ColorOverlay {
			anchors.fill: border1
			source: border1
			color: CosStyle.colorAccentLighter
		}

	}

	Column {
		id: col
		anchors.centerIn: parent

		spacing: 20*scaleFactor

		Row {
			id: row
			anchors.horizontalCenter: parent.horizontalCenter

			spacing: 20*scaleFactor

			Image {
				id: img
				source: cosClient.rankImageSource(cosClient.userRank, -1, cosClient.userRankImage)

				width: 90*scaleFactor
				height: 90*scaleFactor

				fillMode: Image.PreserveAspectFit

				anchors.verticalCenter: parent.verticalCenter
			}

			Column {
				anchors.verticalCenter: parent.verticalCenter

				QLabel {
					font.pixelSize: CosStyle.pixelSize*1.7*scaleFactor
					font.weight: Font.Normal
					color: CosStyle.colorAccentLight
					text: (cosClient.userRoles & Client.RoleGuest) ? qsTr("Vendég") :
																	 cosClient.userNickName.length ?
																		 cosClient.userNickName :
																		 cosClient.userFirstName+" "+cosClient.userLastName

					anchors.left: parent.left
					elide: Text.ElideRight
					width: Math.min(implicitWidth, col.parent.width-row.spacing-img.width-20)
				}

				QLabel {
					font.pixelSize: CosStyle.pixelSize
					font.weight: Font.Medium
					color: CosStyle.colorAccentLighter
					visible: !(cosClient.userRoles & Client.RoleGuest) && cosClient.userNickName.length

					text: cosClient.userFirstName+" "+cosClient.userLastName

					anchors.left: parent.left
				}
			}
		}

		QLabel {
			anchors.horizontalCenter: parent.horizontalCenter
			font.pixelSize: CosStyle.pixelSize*1.3*scaleFactor
			font.weight: Font.Normal
			color: CosStyle.colorAccentLighter
			text: cosClient.userRankName+(cosClient.userRankLevel > 0 ? "<br><small>(lvl "+cosClient.userRankLevel+")</small>" : "")
			visible: !(cosClient.userRoles & Client.RoleGuest)
			horizontalAlignment: Text.AlignHCenter
			textFormat: Text.RichText
			elide: Text.ElideRight
			width: Math.min(implicitWidth, col.parent.width)
		}


		QLabel {
			id: xpLabel
			property int xp: 0
			anchors.horizontalCenter: parent.horizontalCenter
			font.pixelSize: CosStyle.pixelSize*1.8*scaleFactor
			font.weight: Font.DemiBold
			color: CosStyle.colorPrimaryLighter
			text: Number(xp).toLocaleString()+" XP"
			visible: !(cosClient.userRoles & Client.RoleGuest)
		}

		Item {
			width: col.parent.width*0.75

			height: xpProgress.height+xpFrom.height+rankTo.height

			anchors.horizontalCenter: parent.horizontalCenter

			ProgressBar {
				id: xpProgress
				width: parent.width
				from: 0
				to: 0
				value: xpLabel.xp
			}

			QLabel {
				id: xpFrom
				text: Number(xpProgress.from).toLocaleString()+" XP"
				anchors.left: xpProgress.left
				anchors.top: xpProgress.bottom
				font.pixelSize: CosStyle.pixelSize*0.7
				font.weight: Font.DemiBold
				color: CosStyle.colorPrimary
			}

			QLabel {
				id: xpTo
				text: Number(xpProgress.to).toLocaleString()+" XP"
				anchors.right: xpProgress.right
				anchors.top: xpProgress.bottom
				font.pixelSize: CosStyle.pixelSize*0.7
				font.weight: Font.DemiBold
				color: CosStyle.colorPrimary
			}

			QLabel {
				id: rankTo

				anchors.right: xpProgress.right
				anchors.top: xpTo.bottom

				font.pixelSize: CosStyle.pixelSize*0.8
				font.weight: Font.Medium
				color: CosStyle.colorPrimary
			}
		}

		Row {
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 10

			QButton {
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Profil")
				icon.source: CosStyle.iconUserWhite
				visible: !(cosClient.userRoles & Client.RoleGuest)

				onClicked: {
					cosClient.sendMessageInfo("Erre még várni kell", "Ez a funkció még nem elérhető... :(")
				}
			}

			QButton {
				anchors.verticalCenter: parent.verticalCenter
				text: (cosClient.userRoles & Client.RoleGuest) ? qsTr("Bejelentkezés") : qsTr("Kijelentkezés")

				onClicked: {
					popupItem.close()
					if (cosClient.userRoles & Client.RoleGuest)
						mainStack.loginRequest()
					else
						cosClient.logout()
				}
			}

			QButton {
				text: qsTr("Regisztráció")
				enabled: cosClient.registrationEnabled
				visible: (cosClient.userRoles & Client.RoleGuest) && cosClient.registrationEnabled
				anchors.verticalCenter: parent.verticalCenter

				onClicked: {
					popupItem.close()
					cosClient.registrationRequest()
				}
			}
		}

	}


	enter: Transition {
		id: transEnter
		SequentialAnimation {
			ScriptAction {
				script: {
					var n = cosClient.nextRank(cosClient.userRank)
					if ((cosClient.userRoles & Client.RoleGuest) || !Object.keys(n).length) {
						xpProgress.visible = false
						xpFrom.visible = false
						xpTo.visible = false
						rankTo.visible = false
						xpLabel.xp = 0
					} else {
						xpProgress.visible = true
						xpFrom.visible = true
						xpTo.visible = true
						rankTo.visible = true
						xpProgress.from = n.current.xp
						xpProgress.to = n.next.xp
						xpLabel.xp = n.current.xp
						rankTo.text = n.next.rankname+(n.next.ranklevel > 0 ? " (lvl "+n.next.ranklevel+")" : "")
					}
				}
			}

			ParallelAnimation {
				NumberAnimation {
					property: "opacity"
					duration: 75
					from: 0.0
					to: 1.0
					easing.type: Easing.InOutQuad
				}
				NumberAnimation {
					property: "width"
					duration: 150
					from: 0.0
					to: openedWidth
					easing.type: Easing.InOutQuad
				}
				NumberAnimation {
					property: "height"
					duration: 150
					from: 0.0
					to: openedHeight
					easing.type: Easing.InOutQuad
				}
			}


			NumberAnimation {
				target: contentItem
				property: "opacity"
				duration: 150
				from: 0.0
				to: 1.0
				easing.type: Easing.InOutQuad
			}

			NumberAnimation {
				target: xpLabel
				property: "xp"
				duration: 1750
				to: cosClient.userXP
				easing.type: Easing.OutQuart
			}
		}
	}

	exit: Transition {
		id: transExit
		SequentialAnimation {
			NumberAnimation {
				target: contentItem
				property: "opacity"
				duration: 120
				to: 0.0
				easing.type: Easing.InOutQuad
			}

			NumberAnimation {
				properties: "opacity,width,height"
				duration: 150
				to: 0.0
				easing.type: Easing.InOutQuad
			}
		}
	}

}
