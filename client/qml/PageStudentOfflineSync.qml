import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

QPage {
	id: control

	property OfflineClientEngine engine: null

	property int _syncResult: 0

	title: qsTr("Szinkronizálás")

	closeQuestion: qsTr("Biztosan megszakítod?")

	Qaterial.LabelHeadline6 {
		visible: engine && engine.engineState == OfflineClientEngine.EngineError
		anchors.centerIn: parent
		text: qsTr("Hiba történt")
		color: Qaterial.Style.errorColor
	}


	Connections {
		target: engine

		function onSyncFinished(_result) {
			if (_result) {
				_syncResult = 1
				_closeTimer.start()
			} else
				_syncResult = 2

			control.closeQuestion = ""
		}
	}

	QScrollable {
		anchors.fill: parent

		visible: engine && engine.engineState != OfflineClientEngine.EngineError

		Column {
			id: col
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize) //parent.width-(Math.max(Client.safeMarginLeft, Client.safeMarginRight, 10)*2)
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 10

			topPadding: 20
			bottomPadding: 20


			Qaterial.LabelBody2 {
				horizontalAlignment: Qt.AlignHCenter
				width: parent.width

				visible: engine && engine.engineState == OfflineClientEngine.EngineUpload

				color: Qaterial.Colors.green400

				text: qsTr("Szinkronizálás...")
			}


			Qaterial.LabelBody2 {
				horizontalAlignment: Qt.AlignHCenter
				width: parent.width

				visible: engine && engine.engineState == OfflineClientEngine.EngineUpdate

				text: qsTr("Frissítés...")
			}

			Qaterial.LabelBody2 {
				horizontalAlignment: Qt.AlignHCenter
				width: parent.width

				visible: _syncResult == 2

				color: Qaterial.Style.errorColor

				text: qsTr("A szinkronizálás nem sikerült")
			}

			Row {
				spacing: 15
				anchors.horizontalCenter: parent.horizontalCenter

				visible: _syncResult > 0 && !engine.allPermitValid

				QButton {
					icon.source: Qaterial.Icons.refresh
					text: qsTr("Újra")
					onClicked: {
						_syncResult = 0
						engine.synchronize()
					}
				}

				QButton {
					icon.source: Qaterial.Icons.delete_
					text: qsTr("Játékok törlése")
					onClicked: JS.questionDialog(
								   {
									   onAccepted: function()
									   {
										   engine.removeFailedReceipts()
									   },
									   text: qsTr("Biztosan törlöd a sikertelenül szinkronizált játékokat?"),
									   title: qsTr("Játékok törlése"),
									   iconSource: Qaterial.Icons.delete_

								   })
				}
			}


			QButton {
				id: _btnOk
				anchors.horizontalCenter: parent.horizontalCenter
				visible: _syncResult == 1
				icon.source: Qaterial.Icons.check
				text: qsTr("Tovább")
				enabled: control.StackView.view && control.StackView.view.currentItem == control
				onClicked: {
					if (control.StackView.status == StackView.Active)
						Client.stackPop()
				}

				Timer {
					id: _closeTimer
					interval: 1250
					repeat: false
					running: false
					onTriggered: _btnOk.clicked()
				}
			}

			Qaterial.LabelBody2 {
				horizontalAlignment: Qt.AlignHCenter
				width: parent.width

				visible: _syncResult == 1

				color: engine && engine.allPermitValid ? Qaterial.Colors.green400 : Qaterial.Colors.amber400

				text: engine && engine.allPermitValid ? qsTr("A szinkronizálás sikerült") : qsTr("A szinkronizálás csak részben sikerült")
			}

		}

		QListView {
			id: _view

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			height: contentHeight
			anchors.horizontalCenter: parent.horizontalCenter

			refreshEnabled: false
			refreshProgressVisible: false


			model: engine ? engine.receiptModel : null

			delegate: Qaterial.LoaderItemDelegate {
				id: _delegate
				width: _view.width

				text: model.readableMission+" | "+model.readableMap+" [%1]".arg(model.level)

				secondaryText: {
					let t = JS.readableTimestamp(model.timestamp)
					switch (model.mode) {
					case GameMap.Action:
						t += qsTr(" [régi akció]")
						break
					case GameMap.Rpg:
						t += qsTr(" [akció]")
						break
					case GameMap.Lite:
						t += qsTr(" [feladatmegoldás]")
						break
					case GameMap.Quiz:
						t += qsTr(" [kvíz]")
						break
					case GameMap.Test:
						t += qsTr(" [teszt]")
						break
					case GameMap.Exam:
						t += qsTr(" [dolgozat]")
						break
						/*case GameMap.Conquest:
						t += qsTr(" [multiplayer]")
						break*/
					default:
						break
					}

					t += " "+Client.Utils.formatMSecs(model.duration)

					return t
				}



				textColor: switch (model.uploadState) {
						   case OfflineReceipt.UploadSuccess:
							   return Qaterial.Colors.green500
						   case OfflineReceipt.UploadFailed:
							   return Qaterial.Colors.red500
						   default:
							   return model.success ? Qaterial.Style.colorTheme.primaryText : Qaterial.Style.colorTheme.disabledText
						   }


				secondaryTextColor: model.success ? Qaterial.Style.colorTheme.secondaryText : Qaterial.Style.colorTheme.disabledText


				leftSourceComponent: Qaterial.RoundColorIcon
				{
					source: switch (model.uploadState) {
							case OfflineReceipt.Uploading:
								return Qaterial.Icons.upload
							case OfflineReceipt.UploadSuccess:
								return Qaterial.Icons.checkCircle
							case OfflineReceipt.UploadFailed:
								return Qaterial.Icons.close
							default:
								return Qaterial.Icons.dotsHorizontal
							}

					color: switch (model.uploadState) {
						   case OfflineReceipt.UploadSuccess:
							   return Qaterial.Colors.green500
						   case OfflineReceipt.UploadFailed:
							   return Qaterial.Colors.red500
						   default:
							   return Qaterial.Style.disabledTextColor()
						   }

					iconSize: Qaterial.Style.delegate.iconWidth

					fill: true
					width: roundIcon ? roundSize : iconSize
					height: roundIcon ? roundSize : iconSize
				}

				rightSourceComponent: Qaterial.LabelSubtitle1 {
					anchors.verticalCenter: parent.verticalCenter
					text: qsTr("%1 XP").arg(Number(model.xp).toLocaleString())
					color: Qaterial.Style.accentColor
				}

			}

		}

	}


	StackView.onActivated: {
		if (engine)
			engine.synchronize()
	}

	StackView.onRemoved: {
		if (engine)
			engine.loadNextPage()
	}
}
