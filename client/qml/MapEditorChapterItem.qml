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

	signal removeActionRequest()

	header: Item {
		width: root.width
		height: _headerRow.height

		RowLayout {
			id: _headerRow
			width: parent.width
			spacing: 5

			Qaterial.RoundButton {
				icon.source: root.expanded ? Qaterial.Icons.chevronDown : Qaterial.Icons.chevronRight
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
				icon.color: Qaterial.Style.accentColor
				onClicked: contextMenuSimple.popup()

				Qaterial.Menu {
					id: contextMenuSimple
					QMenuItem { action: actionChapterRename }
					QMenuItem { action: chapterDeleteAction ? actionChapterDelete : actionChapterRemove }
					QMenuItem { action: actionObjectiveAdd }
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

				onMenuRequest: _objectiveView.menuOpenFromDelegate(button)
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

			Qaterial.Menu {
				id: contextMenu
				QMenuItem { action: _objectiveView.actionSelectAll }
				QMenuItem { action: _objectiveView.actionSelectNone }
				Qaterial.MenuSeparator {}
				QMenuItem { action: actionObjectiveAdd }
				QMenuItem {
					text: qsTr("Kettőzés")
					icon.source: Qaterial.Icons.clipboardMultipleOutline
					enabled: _objectiveView && editor && chapter
					onClicked: {
						let l = _objectiveView.getSelected()

						if (l.length)
							editor.objectiveDuplicate(chapter, l)

						_objectiveView.unselectAll()
					}
				}
				QMenuItem {
					text: qsTr("Törlés")
					icon.source: Qaterial.Icons._delete
					enabled: _objectiveView && editor && chapter
					onClicked: {
						let l = _objectiveView.getSelected()

						if (l.length)
							editor.objectiveRemove(chapter, l)

						_objectiveView.unselectAll()
					}
				}

				Qaterial.MenuSeparator {}

				Qaterial.Menu {
					id: _copyMenu
					title: qsTr("Másolás")

					QMenuItem {
						text: qsTr("Új feladatcsoport")
						icon.source: Qaterial.Icons.folderPlus
						onClicked: {
							Qaterial.DialogManager.showTextFieldDialog({
																		   textTitle: qsTr("Feladatcsoport neve"),
																		   title: qsTr("Új feladatcsoport létrehozása"),
																		   standardButtons: Dialog.Cancel | Dialog.Ok,
																		   onAccepted: function(_text, _noerror) {
																			   if (_noerror && _text.length) {
																				  editor.objectiveCopyOrMove(chapter, _objectiveView.getSelected(),
																											 -1, true, _text)
																				   _objectiveView.unselectAll()
																			   }
																		   }
																	   })
						}
					}

					Qaterial.MenuSeparator {}

					Instantiator {
						model: _filteredChapterModel

						QMenuItem {
							text: model.name
							onClicked: {
								editor.objectiveCopyOrMove(chapter, _objectiveView.getSelected(), model.chapterid, true)
								_objectiveView.unselectAll()
							}
						}

						onObjectAdded: _copyMenu.insertItem(index+2, object)
						onObjectRemoved: _copyMenu.removeItem(object)
					}

				}

				Qaterial.Menu {
					id: _moveMenu
					title: qsTr("Áthelyezés")

					QMenuItem {
						text: qsTr("Új feladatcsoport")
						icon.source: Qaterial.Icons.folderPlus
						onClicked: {
							Qaterial.DialogManager.showTextFieldDialog({
																		   textTitle: qsTr("Feladatcsoport neve"),
																		   title: qsTr("Új feladatcsoport létrehozása"),
																		   standardButtons: Dialog.Cancel | Dialog.Ok,
																		   onAccepted: function(_text, _noerror) {
																			   if (_noerror && _text.length) {
																				  editor.objectiveCopyOrMove(chapter, _objectiveView.getSelected(),
																											 -1, false, _text)
																				   _objectiveView.unselectAll()
																			   }
																		   }
																	   })
						}
					}

					Qaterial.MenuSeparator {}

					Instantiator {
						model: _filteredChapterModel

						QMenuItem {
							text: model.name
							onClicked: {
								editor.objectiveCopyOrMove(chapter, _objectiveView.getSelected(), model.chapterid, false)
								_objectiveView.unselectAll()
							}
						}

						onObjectAdded: _moveMenu.insertItem(index+2, object)
						onObjectRemoved: _moveMenu.removeItem(object)
					}
				}

			}

			onRightClickOrPressAndHold: {
				if (index != -1)
					currentIndex = index

				_filteredChapterModel.reload()
				contextMenu.popup(mouseX, mouseY)
			}

			function menuOpenFromDelegate(_item) {
				var p = mapFromItem(_item, _item.x, _item.y+_item.height)

				_filteredChapterModel.reload()
				contextMenu.popup(p.x, p.y)
			}
		}
	}


	ListModel {
		id: _filteredChapterModel

		function reload() {
			clear()
			for (let i=0; i<editor.map.chapterList.length; i++) {
				let d = editor.map.chapterList.get(i)
				if (d !== chapter)
					append({chapterid: d.chapterid, name: d.name})
			}
		}
	}

	Action {
		id: actionObjectiveAdd
		text: qsTr("Új feladat")
		icon.source: Qaterial.Icons.clipboardPlusOutline
		onTriggered: if (editor) editor.objectiveDialogRequest(chapter)
	}


	Action {
		id: actionChapterRename
		text: qsTr("Átnevezés")
		icon.source: Qaterial.Icons.folderEditOutline
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
		icon.source: Qaterial.Icons.folderRemove
		onTriggered: root.removeActionRequest()
	}

	Action {
		id: actionChapterDelete
		text: qsTr("Törlés")
		icon.source: Qaterial.Icons._delete
		onTriggered: if (editor) editor.chapterRemove(chapter)
	}
}
