import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Qaterial.Expandable {
	id: root

	property MapEditorChapter chapter: null
	property bool separatorVisible: true
	property bool actionAddVisible: false
	property bool chapterDeleteAction: false

	readonly property MapEditor editor: chapter && chapter.map ? chapter.map.mapEditor : null

	property QListView _objectiveView: null

	signal removeActionRequest()

	header: Item {
		width: root.width
		height: _headerRow.height

		RowLayout {
			id: _headerRow
			width: parent.width
			spacing: 5

			Qaterial.RoundButton {
				icon.source: root.expanded ? Qaterial.Icons.plus : Qaterial.Icons.minus
				Layout.alignment: Qt.AlignCenter
				onClicked: root.expanded = !root.expanded
			}

			Qaterial.LabelHeadline6 {
				text: chapter ? chapter.name : ""
				elide: Text.ElideRight
				color: Qaterial.Style.accentColor
				Layout.fillWidth: true
				Layout.fillHeight: true
				Layout.alignment: Qt.AlignCenter

				verticalAlignment: Text.AlignVCenter

				MouseArea {
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton
					onClicked: root.expanded = !root.expanded
				}
			}

			QBanner {
				num: chapter ? chapter.objectiveCount : 0
				visible: num > 0
				color: Qaterial.Colors.cyan900
				Layout.alignment: Qt.AlignCenter
			}

			Qaterial.RoundButton {
				Layout.alignment: Qt.AlignCenter
				icon.source: Qaterial.Icons.dotsVertical
				icon.color: Qaterial.Style.iconColor()
				onClicked: root._objectiveView ? contextMenuFull.popup() : contextMenuSimple.popup()


				Qaterial.Menu {
					id: contextMenuSimple
					QMenuItem { action: actionChapterRename }
					QMenuItem { action: chapterDeleteAction ? actionChapterDelete : actionChapterRemove }
				}

				Qaterial.Menu {
					id: contextMenuFull

					QMenuItem { action: actionChapterRename }
					QMenuItem { action: chapterDeleteAction ? actionChapterDelete : actionChapterRemove }

					Qaterial.MenuSeparator {}

					Qaterial.Menu {
						title: qsTr("Feladatok")
						QMenuItem { action: root._objectiveView ? root._objectiveView.actionSelectAll : null}
						QMenuItem { action: root._objectiveView ? root._objectiveView.actionSelectNone : null }
						Qaterial.MenuSeparator {}
						QMenuItem { action: actionObjectiveAdd }
						QMenuItem {
							text: qsTr("Törlés")
							icon.source: Qaterial.Icons.trashCan
							enabled: root._objectiveView
							onClicked: {
								if (!editor)
									return

								let l = root._objectiveView.getSelected()

								//if (l.length)
								//	editor.missionLevelInventoryRemove(missionLevel, l)

								root._objectiveView.unselectAll()
							}
						}
					}
				}
			}
		}

		Qaterial.HorizontalLineSeparator {
			width: parent.width
			anchors.bottom: parent.bottom
			visible: root.separatorVisible
		}

	}


	delegate: QIndentedItem {
		width: root.width
		QListView {
			id: _objectiveView
			width: parent.width
			height: contentHeight
			boundsBehavior: Flickable.StopAtBounds

			autoSelectChange: true

			model: chapter ? chapter.objectiveList : null

			delegate: MapEditorObjectiveItem {
				objective: model.qtObject
				width: ListView.view.width

				onClicked: {
					Client.stackPushPage("MapEditorObjectiveEditor.qml", {
											 chapter: chapter,
											 objective: objective,
											 storage: objective.storage
										 })
				}
			}

			footer: Qaterial.ItemDelegate {
				width: ListView.view.width
				height: visible ? implicitHeight : 0
				visible: actionAddVisible
				textColor: Qaterial.Colors.cyan700
				iconColor: textColor
				action: actionObjectiveAdd
				text: qsTr("Új feladat létrehozása")
			}

			Component.onCompleted: root._objectiveView = _objectiveView
			Component.onDestruction: root._objectiveView = null
		}
	}

	Action {
		id: actionObjectiveAdd
		text: qsTr("Létrehozás")
		icon.source: Qaterial.Icons.plus
		onTriggered: if (editor) editor.objectiveDialogRequest(chapter)
	}

	Action {
		id: actionChapterRename
		text: qsTr("Átnevezés")
		icon.source: Qaterial.Icons.renameBox
		enabled: chapter

		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Feladatcsoport neve"),
														   title: qsTr("Feladatcsoport átnevezése"),
														   text: chapter.name,
														   standardButtons: Dialog.Cancel | Dialog.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length) {
																   editor.chapterModify(chapter, function(){
																	   chapter.name = _text
																   })
															   }
														   }
													   })
		}
	}

	Action {
		id: actionChapterRemove
		text: qsTr("Eltávolítás")
		icon.source: Qaterial.Icons.minus
		onTriggered: root.removeActionRequest()
	}

	Action {
		id: actionChapterDelete
		text: qsTr("Törlés")
		icon.source: Qaterial.Icons.trashCan
		onTriggered: if (editor) editor.chapterRemove(chapter)
	}
}
