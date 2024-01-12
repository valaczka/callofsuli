import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: root

	stackPopFunction: function() {
		/*if (_campaignList.view.selectEnabled) {
			_campaignList.view.unselectAll()
			return false
		}

		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
			return false
		}*/

		return true
	}

	title: mission ? mission.name : ""
	subtitle: editor ? editor.displayName : ""

	property MapEditorMission mission: null
	readonly property MapEditor editor: mission && mission.map ? mission.map.mapEditor : null


	signal missionLevelLoadRequest(MapEditorMissionLevel missionLevel)

	appBar.backButtonVisible: true
	appBar.rightComponent: MapEditorToolbarComponent {
		editor: root.editor
	}



	QScrollable {
		anchors.fill: parent

		QFormColumn {
			id: _form

			Row {
				width: parent.width
				spacing: 10

				Qaterial.RawMaterialButton {
					id: _buttonMedal
					icon.source: mission ? mission.fullMedalImage : ""
					icon.color: "transparent"
					icon.width: 2.2 * Qaterial.Style.pixelSize
					icon.height: 2.2 * Qaterial.Style.pixelSize
					width: 3.5 * Qaterial.Style.pixelSize
					height: 3.5 * Qaterial.Style.pixelSize
					flat: true
					outlined: false

					anchors.verticalCenter: parent.verticalCenter

					onClicked: {
						if (mission)
							_modelMedal.setCurrentIndex(mission.medalImage)
						Qaterial.DialogManager.openFromComponent(_cmpDialogMedal)
					}
				}

				Qaterial.TextField {
					id: _textFieldName
					visible: mission
					enabled: mission

					width: parent.width-parent.spacing-_buttonMedal.width

					anchors.verticalCenter: parent.verticalCenter

					font: Qaterial.Style.textTheme.headline4
					placeholderText: qsTr("A küldetés neve")
					backgroundBorderHeight: 1
					backgroundColor: "transparent"
					text: mission ? mission.name : ""

					onEditingFinished: editor.missionModify(mission, function() {
						mission.name = text
					})

				}
			}



			QFormTextArea {
				id: _textAreaDescription
				width: parent.width
				title: qsTr("Leírás")
				placeholderText: qsTr("Rövid tájékoztató a küldetésről")
				text: mission ? mission.description : ""

				onEditingFinished: editor.missionModify(mission, function() {
					mission.description = text
				})

				Connections {
					target: mission

					function onDescriptionChanged() {
						_textAreaDescription.text = mission.description
					}
				}

			}


			Item {
				width: parent.width
				height: 20
			}


			Row
			{
				width: parent.width

				spacing: 5

				Repeater {
					model: SortFilterProxyModel {
						sourceModel: mission ? mission.levelList : null
						sorters: [
							RoleSorter {
								roleName: "level"
								sortOrder: Qt.AscendingOrder
							}
						]
					}

					delegate: Qaterial.OutlineButton
					{
						property MapEditorMissionLevel missionLevel: model.qtObject
						text: missionLevel ? qsTr("Level %1").arg(missionLevel.level) : ""
						highlighted: false
						foregroundColor: Qaterial.Style.accentColor
						onClicked: root.missionLevelLoadRequest(missionLevel)
					}
				}

				Qaterial.FlatButton {
					visible: mission && mission.levelList.length < 3
					text: qsTr("Létrehozás")
					icon.source: Qaterial.Icons.plus
					//icon.color: foregroundColor
					foregroundColor: Qaterial.Colors.green400

					onClicked: {
						var m = editor.missionLevelAdd(mission)
						root.missionLevelLoadRequest(m)
					}
				}
			}



			Item {
				width: parent.width
				height: 20
			}


			QExpandableHeader {
				width: parent.width
				text: qsTr("Lehetséges játékmódok")
				icon: Qaterial.Icons.googleController
				button.visible: false
			}


			QIndentedItem {
				width: parent.width
				Column {
					width: parent.width

					QFormCheckButton
					{
						id: _isAction
						text: qsTr("Akciójáték")
						checked: mission && (mission.modes & GameMap.Action)
						onToggled: _form.updateCheckButtons()
						enabled: !_isExam.checked
					}

					QFormCheckButton
					{
						id: _isLite
						text: qsTr("Feladatmegoldás")
						checked: mission && (mission.modes & GameMap.Lite)
						onToggled: _form.updateCheckButtons()
						enabled: !_isExam.checked
					}

					QFormCheckButton
					{
						id: _isTest
						text: qsTr("Teszt")
						checked: mission && (mission.modes & GameMap.Test)
						onToggled: _form.updateCheckButtons()
						enabled: !_isExam.checked
					}

					QFormCheckButton
					{
						id: _isPractice
						text: qsTr("Gyakorlás")
						checked: mission && (mission.modes & GameMap.Practice)
						onToggled: _form.updateCheckButtons()
						enabled: !_isExam.checked
					}

					/*QFormCheckButton
					{
						id: _isMultiPlayer
						text: qsTr("Multiplayer")
						checked: mission && (mission.modes & GameMap.MultiPlayer)
						onToggled: _form.updateCheckButtons()
						enabled: !_isExam.checked
					}*/




					QFormCheckButton
					{
						id: _isExam
						text: qsTr("Dolgozat")
						checked: mission && (mission.modes & GameMap.Exam)
						onToggled: _form.updateCheckButtons()
					}

				}
			}

			function updateCheckButtons() {
				var c = 0

				if (_isExam.checked)
					c |= GameMap.Exam
				else {
					if (_isAction.checked)
						c |= GameMap.Action

					if (_isLite.checked)
						c |= GameMap.Lite

					if (_isTest.checked)
						c |= GameMap.Test

					if (_isPractice.checked)
						c |= GameMap.Practice

					if (_isMultiPlayer.checked)
						c |= GameMap.MultiPlayer
				}

				editor.missionModify(mission, function() {
					mission.modes = c
				})
			}





			Item {
				width: parent.width
				height: 20
			}


			QExpandableHeader {
				width: parent.width
				text: qsTr("Zárolások")
				icon: Qaterial.Icons.keyChain
				button.visible: false
			}

			QIndentedItem {
				width: parent.width
				Column {
					id: _col
					width: parent.width

					Repeater {
						model: mission ? mission.lockList : null

						delegate: Qaterial.LoaderItemDelegate {
							property MapEditorMissionLevel missionLevel: modelData

							width: _col.width

							leftSourceComponent: Qaterial.RoundColorIcon
							{
								iconSize: Qaterial.Style.delegate.iconWidth
								source: Qaterial.Icons.lock
								color: Qaterial.Style.iconColor()

								fill: true
								width: roundIcon ? roundSize : iconSize
								height: roundIcon ? roundSize : iconSize
							}

							text: (missionLevel.mission ? missionLevel.mission.name : qsTr("???"))+
								  qsTr(" - level %1").arg(missionLevel ? missionLevel.level : 0)

							rightSourceComponent: Qaterial.SquareButton {
								icon.source: Qaterial.Icons.lockOpenRemove
								icon.color: Qaterial.Colors.red400

								onClicked: editor.missionLockRemove(mission, missionLevel)
							}
						}
					}

					Qaterial.ItemDelegate {
						width: _col.width

						text: qsTr("Hozzáadás")
						icon.source: Qaterial.Icons.lockPlus
						iconColor: textColor
						textColor: Qaterial.Colors.green400

						onClicked: {
							_sortedMissionLevelModel.reload()

							Qaterial.DialogManager.openRadioListView(
										{
											onAccepted: function(index)
											{
												if (index < 0)
													return

												let ml = _sortedMissionLevelModel.get(index)
												if (ml)
													editor.missionLockAdd(mission, ml.uuid, ml.level)

											},
											title: qsTr("Zárolás hozzáadása"),
											model: _sortedMissionLevelModel
										})
						}

					}
				}
			}

		}

	}

	ListModel {
		id: _sortedMissionLevelModel

		function reload() {
			clear()

			if (!editor || !editor.map)
				return

			if (!mission)
				return

			for (let i=0; i<editor.map.missionList.length; ++i) {
				let m = editor.map.missionList.get(i)

				if (m === mission)
					continue

				let list = []
				let j=0

				for (j=0; j<m.levelList.length; ++j) {
					var ml = m.levelList.get(j)

					if (mission.lockList.includes(ml)) {
						list = []
						break
					} else {
						list.push(ml)
					}
				}

				if (list.length) {
					for (j=0; j<list.length; ++j)
						_sortedMissionLevelModel.append({
															text: m.name,
															secondaryText: qsTr("Level %1").arg(list[j].level),
															uuid: m.uuid,
															level: list[j].level
														})
				}
			}
		}
	}


	onEditorChanged: {
		if (editor) {
			_modelMedal.clear()

			let m = editor.availableMedals

			for (let i=0; i<m.length; ++i) {
				_modelMedal.append(m[i])
			}
		}
	}

	ListModel {
		id: _modelMedal

		property int _currentIndex: -1

		function setCurrentIndex(_name) {
			let idx = -1
			for (let i=0; i<count; ++i) {
				if (get(i).name === _name) {
					idx = i
					break
				}
			}

			_currentIndex = idx
		}
	}

	Component {
		id: _cmpDialogMedal

		QGridDialog {
			title: qsTr("Pálya medálképe")

			model: _modelMedal
			currentIndex: _modelMedal._currentIndex

			onAccepted: {
				let name = model.get(currentIndex).name
				if (editor)
					editor.missionModify(mission, function() {
						mission.medalImage = name
					})
			}

		}
	}

	onMissionChanged: if (!mission) Client.stackPop(root)
}
