import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	property StudentMaps studentMaps: null
	property Profile profile: null
	property string missionUuid: ""
	property int missionLevel: -1
	property bool missionDeathmatch: false
	property bool autoPlay: false
	property bool readOnly: true


	readonly property bool isHorizontal: width > height

	/*
	  state
	  0: info + play button
	  1: loading
	  2: missioncompleted
	*/

	property int _state: 0

	maximumWidth: -1


	property ListModel gameListModel: ListModel {}

	property int _minDuration: 0
	property int _maxDuration: 0
	property int _maxSuccess: 0

	Flickable {
		id: flick

		width: parent.width
		height: Math.min(contentHeight, parent.height)
		anchors.centerIn: parent

		flickableDirection: Flickable.VerticalFlick

		contentWidth: col.width
		contentHeight: col.height

		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator { }

		Column {
			id: col
			width: flick.width

			QTabHeader {
				tabContainer: control
				isPlaceholder: true
			}



			QLabel {
				id: labelTitle

				anchors.horizontalCenter: parent.horizontalCenter
				width: parent.width

				visible: _state < 2

				topPadding: isHorizontal ? 5 : 20
				bottomPadding: topPadding
				rightPadding: 20
				leftPadding: 20

				font.family: "HVD Peace"
				font.pixelSize: CosStyle.pixelSize*1.6

				color: CosStyle.colorAccentLighter

				horizontalAlignment: Text.AlignHCenter
				wrapMode: Text.Wrap
			}

			QLabel {
				id: labelLevel
				visible: _state < 2 && labelTitle.text != ""
				anchors.horizontalCenter: parent.horizontalCenter
				horizontalAlignment: Text.AlignHCenter
				bottomPadding: isHorizontal ? 10 : 100
				font.capitalization: Font.AllUppercase
				color: labelTitle.color
				font.weight: Font.DemiBold
				text: missionDeathmatch ? "Level %1%2Sudden death".arg(missionLevel).arg(isHorizontal ? " " : "\n") :
										  "Level %1".arg(missionLevel)
			}



			Loader {
				id: missionCompletedLoader
				anchors.horizontalCenter: parent.horizontalCenter
				width: isHorizontal ? parent.width-40 : parent.width-20

				visible: _state == 2

				onLoaded: item.populated()
			}




			QButton {
				id: buttonPlay

				visible: _state == 0 && !readOnly
				anchors.horizontalCenter: parent.horizontalCenter

				enabled: false
				themeColors: CosStyle.buttonThemeGreen
				text: qsTr("Play")
				icon.source: enabled ? CosStyle.iconPlay : CosStyle.iconLock
				font.pixelSize: CosStyle.pixelSize*1.4

				onClicked: {
					_state = 1
					studentMaps.playGame(missionUuid, missionLevel, missionDeathmatch)
				}
			}

			/*QImageSpinBox {
				id: spinCharacter
				from: 0
				to: profile.characterList.length-1
				//sqlField: "character"
				//sqlData: profile.characterList[value].dir

				visible: _state == 0
				anchors.horizontalCenter: parent.horizontalCenter

				imageSize: CosStyle.pixelSize*3

				textFromValue: function(value) {
					return profile.characterList[value].name
				}

				imageFromValue: function(value) {
					return "qrc:/character/%1/thumbnail.png".arg(profile.characterList[value].dir)
				}

				onValueModified: cosClient.userPlayerCharacter = profile.characterList[value].dir
			}*/

			QLabel {
				id: labelDescription
				visible: _state == 0
				width: Math.min(implicitWidth, parent.width)
				anchors.horizontalCenter: parent.horizontalCenter
				horizontalAlignment: Text.AlignHCenter
				wrapMode: Text.Wrap
				topPadding: isHorizontal ? 10 : 50
				leftPadding: 50
				rightPadding: 50
				color: CosStyle.colorPrimary
				font.weight: Font.Medium
				font.pixelSize: CosStyle.pixelSize*0.9
			}

			BusyIndicator {
				visible: _state == 1
				anchors.horizontalCenter: parent.horizontalCenter
				height: CosStyle.pixelSize*3
				width: CosStyle.pixelSize*3
				running: true
				Material.accent: CosStyle.colorErrorLighter
			}

			QLabel {
				visible: _state == 1
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Betöltés...")
				font.pixelSize: CosStyle.pixelSize*1.2
				color: CosStyle.colorErrorLighter
				topPadding: 20
			}




			QCollapsible {
				id: collapsibleSuccess
				title: qsTr("Megoldások")
				interactive: false
				collapsed: false
				backgroundColor: "transparent"
				visible: _state == 0 && !studentMaps.demoMode
				width: isHorizontal ? parent.width-20 : parent.width-10
				anchors.horizontalCenter: parent.horizontalCenter

				rightComponent: QToolButton {
					icon.source: CosStyle.iconAdd
					icon.width: CosStyle.pixelSize*0.8
					icon.height: CosStyle.pixelSize*0.8
					onClicked: {
						successFilter.enabled = false
						collapsibleSuccess.rightComponent = undefined
					}
				}

				QObjectListView {
					id: listSuccess

					width: parent.width

					model: SortFilterProxyModel {
						sourceModel: gameListModel

						sorters: [
							RoleSorter {
								roleName: "successNum"
								sortOrder: Qt.AscendingOrder
							}
						]

						proxyRoles: [
							ExpressionRole {
								name: "displayName"
								expression: "%1. %2".arg(model.successNum).arg(String(model.nickname !== "" ? model.nickname :
																											  model.firstname+" "+model.lastname).toLocaleUpperCase())
							},
							SwitchRole {
								name: "background"
								filters: ValueFilter {
									roleName: "username"
									value: cosClient.userName
									SwitchRole.value: JS.setColorAlpha(CosStyle.colorWarningDark, 0.4)
								}
								defaultValue: "transparent"
							}
						]

						filters: [
							AnyOf {
								id: successFilter
								RangeFilter {
									roleName: "successNum"
									maximumValue: 5
								}
								ValueFilter {
									roleName: "username"
									value: cosClient.userName
								}
							}

						]
					}

					delegateHeight: CosStyle.pixelSize*1.1

					highlightCurrentItem: false
					mouseAreaEnabled: false
					autoSelectorChange: false

					modelTitleRole: "displayName"
					modelBackgroundRole: "background"
					fontWeightTitle: Font.DemiBold
					pixelSizeTitle: CosStyle.pixelSize*0.8
					colorTitle: CosStyle.colorWarningLighter

					leftComponent: QProfileImage {
						rankId: model ? model.rankid : -1
						rankImage: model ? model.rankimage : ""
						width: listSuccess.delegateHeight
						height: listSuccess.delegateHeight*0.9
					}

					rightComponent: Row {
						anchors.verticalCenter: parent.verticalCenter

						spacing: 0

						ProgressBar {
							anchors.verticalCenter: parent.verticalCenter
							width: CosStyle.pixelSize*3
							from: 0
							to: _maxSuccess
							value: model ? Number(model.success) : 0

							Material.accent: CosStyle.colorWarningLight

							Behavior on value {
								NumberAnimation { duration: 750; easing.type: Easing.OutQuad }
							}
						}

						QLabel {
							anchors.verticalCenter: parent.verticalCenter
							text: model ? "%1x".arg(Number(model.success)) : ""
							width: CosStyle.pixelSize*2.5
							horizontalAlignment: Text.AlignRight
							font.pixelSize: CosStyle.pixelSize*0.8
							font.weight: Font.DemiBold
							color: CosStyle.colorWarningLight
						}

					}

				}



			}





			QCollapsible {
				id: collapsibleDuration
				title: qsTr("Leggyorsabb megoldások")
				interactive: false
				collapsed: false
				backgroundColor: "transparent"
				visible: _state == 0 && !studentMaps.demoMode
				width: isHorizontal ? parent.width-20 : parent.width-10
				anchors.horizontalCenter: parent.horizontalCenter

				rightComponent: QToolButton {
					icon.source: CosStyle.iconAdd
					icon.width: CosStyle.pixelSize*0.8
					icon.height: CosStyle.pixelSize*0.8
					onClicked: {
						durationFilter.enabled = false
						collapsibleDuration.rightComponent = undefined
					}
				}

				QObjectListView {
					id: listDuration

					width: parent.width

					model: SortFilterProxyModel {
						sourceModel: gameListModel

						sorters: [
							RoleSorter {
								roleName: "durationNum"
								sortOrder: Qt.AscendingOrder
							}
						]

						proxyRoles: [
							ExpressionRole {
								name: "displayName"
								expression: "%1. %2".arg(model.durationNum).arg(String(model.nickname !== "" ? model.nickname :
																											   model.firstname+" "+model.lastname).toLocaleUpperCase())
							},
							SwitchRole {
								name: "background"
								filters: ValueFilter {
									roleName: "username"
									value: cosClient.userName
									SwitchRole.value: JS.setColorAlpha(CosStyle.colorOKDark, 0.4)
								}
								defaultValue: "transparent"
							}
						]

						filters: [
							AnyOf {
								id: durationFilter
								RangeFilter {
									roleName: "durationNum"
									maximumValue: 5
								}
								ValueFilter {
									roleName: "username"
									value: cosClient.userName
								}
							}

						]
					}

					delegateHeight: CosStyle.pixelSize*1.1

					highlightCurrentItem: false
					mouseAreaEnabled: false
					autoSelectorChange: false

					modelTitleRole: "displayName"
					modelBackgroundRole: "background"
					fontWeightTitle: Font.DemiBold
					pixelSizeTitle: CosStyle.pixelSize*0.8
					colorTitle: CosStyle.colorOK

					leftComponent: QProfileImage {
						rankId: model ? model.rankid : -1
						rankImage: model ? model.rankimage : ""
						width: listDuration.delegateHeight
						height: listDuration.delegateHeight*0.9
					}

					rightComponent: Row {
						anchors.verticalCenter: parent.verticalCenter

						spacing: 0

						ProgressBar {
							anchors.verticalCenter: parent.verticalCenter
							width: CosStyle.pixelSize*3
							from: _minDuration
							to: _maxDuration
							value: model ? _maxDuration-(Number(model.duration)-_minDuration) : 0

							Material.accent: CosStyle.colorOKLighter

							Behavior on value {
								NumberAnimation { duration: 750; easing.type: Easing.OutQuad }
							}
						}

						QLabel {
							anchors.verticalCenter: parent.verticalCenter
							text: model ? JS.secToMMSS(Number(model.duration)) : ""
							width: CosStyle.pixelSize*2.5
							horizontalAlignment: Text.AlignRight
							font.pixelSize: CosStyle.pixelSize*0.8
							font.weight: Font.DemiBold
							color: CosStyle.colorOKLight
						}

					}

				}



			}

		}
	}


	Connections {
		target: studentMaps

		function onLevelInfoReady(info) {
			console.debug("!!!")
			buttonPlay.enabled = info.available
			labelTitle.text = info.name
			labelDescription.text = info.description/*+"\n\n"+
					qsTr("HP: %1\n").arg(info.hp)+
					qsTr("Idő: %1\n").arg(JS.secToMMSS(info.duration))+
					qsTr("Ellenfelek: %1").arg(info.enemies)*/

			if (autoPlay && info.available) {
				_state = 1
				studentMaps.playGame(missionUuid, missionLevel, missionDeathmatch)
			}
		}

		function onGamePlayReady(gameMatch) {
			var o = JS.createPage("Game", {
									  gameMatch: gameMatch,
									  studentMaps: studentMaps,
									  deleteGameMatch: true
								  })
		}

		function onGameStarted() {
			_state = 0
		}

		function onGameFinishDialogReady(data) {
			if (data.uuid !== missionUuid || data.level !== missionLevel || data.deathmatch !== missionDeathmatch)
				return

			_state = 2

			missionCompletedLoader.setSource("MapMissionLevelCompleted.qml", {gameData: data})
		}


		function onGameListUserMissionGet(jsonData, binaryData) {
			if (jsonData.missionid !== missionUuid || jsonData.level !== missionLevel || jsonData.deathmatch !== missionDeathmatch)
				return


			_minDuration = jsonData.minDuration
			_maxDuration = jsonData.maxDuration
			_maxSuccess = jsonData.maxSuccess

			JS.listModelReplace(gameListModel, jsonData.list)
		}
	}


	Connections {
		target: missionCompletedLoader.item

		function onPlay(uuid, level, deathmatch) {
			labelTitle.text = ""
			_state = 1
			missionUuid = uuid
			missionLevel = level
			missionDeathmatch = deathmatch
			autoPlay = true
			studentMaps.getLevelInfo(missionUuid, missionLevel, missionDeathmatch)
		}
	}

	onPopulated: {
		if (studentMaps) {
			studentMaps.getLevelInfo(missionUuid, missionLevel, missionDeathmatch)

			if (!studentMaps.demoMode)
				studentMaps.send(CosMessage.ClassStudent, "gameListUserMissionGet", {
									 missionid: missionUuid,
									 level: missionLevel,
									 deathmatch: missionDeathmatch
								 })
		}
	}

}



