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

	anchors.fill: parent

	active: true

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

				mainColor: Qaterial.Colors.red400

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

				mainColor: Qaterial.Colors.green400

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

		for (let i=0; i<questData.defenders.length; ++i) {
			_modelDefender.append(questData.defenders[i])
			if (i==0 || questData.defenders[i].key == keyD)
				idxD = i
		}

		for (let i=0; i<questData.utilities.length; ++i) {
			_modelUtility.append(questData.utilities[i])

			if (i==0 || questData.utilities[i].key == keyU)
				idxD = i
		}

		for (let i=0; i<questData.quests.length; ++i) {
			_modelQuest.append({
								   icon: Qaterial.Icons.abacus,
								   description: "Szia "+i+": "+questData.quests[i].xp+" XP"
							   })

			if (i==0)
				idxQ = i
		}

		if (idxD != -1) {
			game.questSelect({defender: idxD})
			_tumblerDefender.positionViewAtIndex(idxD, Tumbler.Center)
		}


		if (idxU != -1) {
			game.questSelect({utility: idxU})
			_tumblerUtility.positionViewAtIndex(idxU, Tumbler.Center)
		}

		if (idxQ != -1) {
			game.questSelect({quest: idxQ})
			_tumblerQuest.positionViewAtIndex(idxQ, Tumbler.Center)
		}

	}

}

