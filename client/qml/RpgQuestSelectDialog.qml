import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


RpgDialog {
	id: panel

	required property RpgGame game
	required property var questData

	dialogImplicitHeight: 500

	anchors.fill: parent

	ListModel {
		id: _modelDefender
	}

	ListModel {
		id: _modelUtility
	}

	ListModel {
		id: _modelQuest
	}


	ColumnLayout {
		anchors.fill: parent

		spacing: 2

		Qaterial.LabelHeadline6 {
			id: _title
			text: qsTr("Choose your quest & equipment")
			padding: 3
		}

		Qaterial.HorizontalLineSeparator {
			Layout.fillWidth: true
		}

		RpgChangerTumbler {
			id: _tumblerQuest

			visibleItemCount: 3

			Layout.fillHeight: true
			Layout.fillWidth: true

			model: _modelQuest

			delegate: Rectangle {
				id: _delegateQuest
				readonly property bool isCurrent: Tumbler.displacement === 0

				color: isCurrent ? _tumblerQuest.mainColor : "transparent"

				RowLayout {
					anchors.fill: parent
					spacing: 20

					Qaterial.LabelHeadline5 {
						Layout.fillHeight: false
						Layout.fillWidth: false
						Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

						leftPadding: Qaterial.Style.horizontalPadding

						text: model.description

						color: _delegateQuest.isCurrent ? Qaterial.Colors.black : _tumblerQuest.mainColor
					}

					Qaterial.IconLabel {
						Layout.fillHeight: false
						Layout.fillWidth: false
						Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

						visible: model.pts > 0

						icon.source: Qaterial.Icons.flashCircle
						icon.color: color
						//icon.source: "qrc:/rpg/coin/coins.png"
						//icon.color: "transparent"
						text: model.pts

						font: Qaterial.Style.textTheme.body1
						icon.width: 32 * Qaterial.Style.pixelSizeRatio
						icon.height: 32 * Qaterial.Style.pixelSizeRatio

						color: Qaterial.Colors.amber500//_delegateQuest.isCurrent ? Qaterial.Colors.black : _tumblerQuest.mainColor
					}

					Qaterial.IconLabel {
						Layout.fillHeight: false
						Layout.fillWidth: false
						Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

						icon.source: Qaterial.Icons.headQuestionOutline
						text: model.question

						font: Qaterial.Style.textTheme.body1
						icon.width: 32 * Qaterial.Style.pixelSizeRatio
						icon.height: 32 * Qaterial.Style.pixelSizeRatio

						color: _delegateQuest.isCurrent ? Qaterial.Colors.black : _tumblerQuest.mainColor
					}

					Qaterial.IconLabel {
						Layout.fillHeight: false
						Layout.fillWidth: false
						Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

						icon.source: Qaterial.Icons.chartTimelineVariantShimmer
						text: model.streak

						font: Qaterial.Style.textTheme.body1
						icon.width: 32 * Qaterial.Style.pixelSizeRatio
						icon.height: 32 * Qaterial.Style.pixelSizeRatio

						color: Qaterial.Colors.yellow500// _delegateQuest.isCurrent ? Qaterial.Colors.black : _tumblerQuest.mainColor
					}

					Item {
						Layout.fillHeight: true
						Layout.fillWidth: true

						Qaterial.Icon {
							anchors.centerIn: parent

							icon: Qaterial.Icons.arrowRightThin
						}
					}

					Qaterial.IconLabel {
						Layout.fillHeight: false
						Layout.fillWidth: false
						Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

						icon.source: Qaterial.Icons.shieldCrown
						text: model.token

						font: Qaterial.Style.textTheme.body1
						icon.width: 32 * Qaterial.Style.pixelSizeRatio
						icon.height: 32 * Qaterial.Style.pixelSizeRatio

						color: _delegateQuest.isCurrent ? Qaterial.Colors.black : Qaterial.Colors.blue500
					}

					Qaterial.LabelBody1 {
						Layout.fillHeight: false
						Layout.fillWidth: false
						Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

						rightPadding: Qaterial.Style.horizontalPadding

						text: model.xp+" XP"

						color: _delegateQuest.isCurrent ? Qaterial.Colors.black : Qaterial.Style.textColor
					}
				}


				opacity: 1.0 - Math.abs(Tumbler.displacement) / (_tumblerQuest.visibleItemCount / 2)
			}

			visible: true

			mainColor: Qaterial.Colors.blue400

			onCurrentIndexChanged: game.questSelect({quest: currentIndex})
		}

		Qaterial.HorizontalLineSeparator {
			Layout.fillWidth: true
		}

		RowLayout {
			Layout.fillWidth: true
			Layout.fillHeight: true

			spacing: 5

			RpgChangerTumbler {
				id: _tumblerDefender

				Layout.fillHeight: true
				Layout.fillWidth: true

				visibleItemCount: 3

				model: _modelDefender

				visible: true

				mainColor: Qaterial.Colors.green500

				onCurrentIndexChanged: game.questSelect({defender: currentIndex})
			}

			Qaterial.VerticalLineSeparator {
				Layout.fillHeight: true
			}

			RpgChangerTumbler {
				id: _tumblerUtility

				visibleItemCount: 3

				Layout.fillHeight: true
				Layout.fillWidth: true

				model: _modelUtility

				visible: true

				mainColor: Qaterial.Colors.amber500

				onCurrentIndexChanged: game.questSelect({utility: currentIndex})
			}
		}

		QButton {
			bgColor: Qaterial.Colors.green500
			icon.source: Qaterial.Icons.check
			text: qsTr("START")

			onClicked: {
				panel.enabled = false

				if (game.gameMode === RpgGame.MultiPlayer) {
					_title.text = qsTr("Waiting for other players...")
				}

				game.questSelect({
									 defender: _tumblerDefender.currentIndex,
									 utility: _tumblerUtility.currentIndex,
									 quest: _tumblerQuest.currentIndex,
									 ready: true
								 })
			}

			Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
		}
	}

	Component.onCompleted: {
		let idxD = -1
		let idxU = -1
		let idxQ = -1

		let keyD = -1
		let keyU = -1

		let levelNames = [
				qsTr("Easy"),
				qsTr("Tricky"),
				qsTr("Insane"),
				qsTr("Hyper hard"),
				qsTr("Scary"),
				qsTr("Impossible"),
				qsTr("Death"),
				qsTr("Null"),
				qsTr("Easy")
			]

		for (let i=0; i<questData.defenders.length; ++i) {
			_modelDefender.append(questData.defenders[i])
			if (i==0 || questData.defenders[i].key == keyD)
				idxD = i
		}

		for (let i=0; i<questData.utilities.length; ++i) {
			_modelUtility.append(questData.utilities[i])

			if (i==0 || questData.utilities[i].key == keyU)
				idxU = i
		}

		for (let i=0; i<questData.quests.length; ++i) {
			let m = questData.quests[i]
			m.description = levelNames[i]
			_modelQuest.append(m)

			if (i==0)
				idxQ = i
		}

		if (idxD != -1) {
			game.questSelect({defender: idxD})
			_tumblerDefender.currentIndex = idxD
			_tumblerDefender.positionViewAtIndex(idxD, Tumbler.Center)
		}


		if (idxU != -1) {
			game.questSelect({utility: idxU})
			_tumblerUtility.currentIndex = idxU
			_tumblerUtility.positionViewAtIndex(idxU, Tumbler.Center)
		}

		if (idxQ != -1) {
			game.questSelect({quest: idxQ})
			_tumblerQuest.currentIndex = idxQ
			_tumblerQuest.positionViewAtIndex(idxQ, Tumbler.Center)
		}

	}

}

