import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

Qaterial.Expandable {
	id: root

	property UserLogListImpl userLogList: null

	expanded: true

	onVisibleChanged: evaluateDelegateClipperHeight()

	QFetchLoaderGroup {
		id: _loaderGroup
	}

	header: QExpandableHeader {
		text: qsTr("Számlálók")
		icon: Qaterial.Icons.counter
		expandable: root
	}

	delegate: Column {
		width: root.width
		height: implicitHeight

		Column {
			width: root.width
			visible: _loaderGroup.showPlaceholders

			Repeater {
				model: 5
				delegate: Qaterial.FullLoaderItemDelegate {
					id: _delegatePlaceholder

					width: root.width

					height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

					spacing: 10
					leftPadding: 0
					rightPadding: 0

					contentSourceComponent: QPlaceholderItem {
						horizontalAlignment: Qt.AlignLeft
						heightRatio: 0.5
					}

					leftSourceComponent: QPlaceholderItem {
						contentComponent: ellipseComponent
						fixedHeight: _delegatePlaceholder.height-6
						fixedWidth: fixedHeight
					}

					rightSourceComponent: QPlaceholderItem {
						width: 90
						height: Qaterial.Style.textTheme.caption.pixelSize
						heightRatio: 1.0
						widthRatio: 1.0
					}
				}
			}
		}

		Column {
			width: parent.width
			visible: !_loaderGroup.showPlaceholders

			Repeater {
				id: _rptr
				model: null
				delegate: QFetchLoader {
					group: _loaderGroup
					Qaterial.FullLoaderItemDelegate {
						id: _delegate

						spacing: 10
						leftPadding: 0
						rightPadding: 0

						width: root.width

						height: Qaterial.Style.textTheme.body2.pixelSize*2 + topPadding+bottomPadding+topInset+bottomInset

						highlighted: false

						contentSourceComponent: Label {
							font: Qaterial.Style.textTheme.body2
							verticalAlignment: Label.AlignVCenter

							text: {
								switch (modelData.mode) {
								case GameMap.Action:
									qsTr("Akciójáték")
									break
								case GameMap.Rpg:
									qsTr("RPG")
									break
								case GameMap.Lite:
									qsTr("Feladatmegoldás")
									break
								case GameMap.Test:
									qsTr("Teszt")
									break
								case GameMap.Quiz:
									qsTr("Kvíz")
									break
								case GameMap.Exam:
									qsTr("Dolgozat")
									break
								case GameMap.Conquest:
									qsTr("Multiplayer")
									break
								default:
									"???"
								}
							}

						}

						rightSourceComponent: Row {
							spacing: 2

							Qaterial.Icon {
								visible: modelData.trophy
								anchors.verticalCenter: parent.verticalCenter
								icon: Qaterial.Icons.trophy
								color: Qaterial.Colors.green500
								width: Qaterial.Style.smallIcon*0.8
								height: Qaterial.Style.smallIcon*0.8
							}

							Qaterial.LabelCaption {
								visible: modelData.trophy
								anchors.verticalCenter: parent.verticalCenter
								text: modelData.trophy
								color: Qaterial.Colors.green300
								rightPadding: modelData.duration ? 10 * Qaterial.Style.pixelSizeRatio : 0
							}

							Qaterial.Icon {
								visible: modelData.duration
								anchors.verticalCenter: parent.verticalCenter
								icon: Qaterial.Icons.clock
								color: Qaterial.Colors.cyan500
								width: Qaterial.Style.smallIcon*0.8
								height: Qaterial.Style.smallIcon*0.8
							}

							Qaterial.LabelCaption {
								visible: modelData.duration
								anchors.verticalCenter: parent.verticalCenter
								text: Client.Utils.formatMSecs(modelData.duration, 0, true)
								color: Qaterial.Colors.cyan300
							}

						}
					}
				}
			}
		}


		Connections {
			target: userLogList

			function onCountersChanged() {
				_rptr.model = userLogList.counters
				if (!_rptr.model || !_rptr.model.length)
					_loaderGroup.showPlaceholders = false
			}
		}
	}

}
