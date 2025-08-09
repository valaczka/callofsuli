import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	closeQuestion: _form.modified ? qsTr("Biztosan eldobod a módosításokat?") : ""

	property Pass pass: null
	property PassItem passItem: null
	property TeacherPass teacherPass: null

	title: passItem ? (passItem.description != "" ? passItem.description : qsTr("Elem #%1 szerkesztése").arg(passItem.itemid))
					: qsTr("Új elem")
	subtitle: pass ? (pass.title != "" ? pass.title : qsTr("Call Pass #%1").arg(pass.passid)) : ""

	appBar.rightComponent: Qaterial.AppBarButton
	{
		visible: passItem
		ToolTip.text: qsTr("Elem törlése")
		icon.source: Qaterial.Icons.delete_
		onClicked: JS.questionDialog(
					   {
						   onAccepted: function()
						   {
							   Client.send(HttpConnection.ApiTeacher, "passItem/%1/delete".arg(passItem.itemid))
							   .done(control, function(r){
								   _form.modified = false
								   Client.stackPop(control)
							   })
							   .fail(control, JS.failMessage("Törlés sikertelen"))
						   },
						   text: qsTr("Biztosan törlöd az elemet?"),
						   title: pass ? (pass.title != "" ? pass.title : qsTr("Call Pass #%1").arg(pass.passid)) : "",
						   iconSource: Qaterial.Icons.deleteCircle
					   })

	}


	TeacherPassResultModel {
		id: _model

		teacherPass: control.teacherPass
		mode: TeacherPassResultModel.ModePassItem
		contentId: control.passItem ? control.passItem.itemid : -1

		/*onModelReloaded: {
						root.forceLayout()
					}*/
	}

	QScrollable {
		anchors.fill: parent

		QFormColumn {
			id: _form

			spacing: 5

			QFormTextField {
				id: _tfDescription

				width: parent.width
				leadingIconSource: Qaterial.Icons.renameBox
				leadingIconInline: true
				title: qsTr("Leírás")

				field: "description"
				fieldData: passItem ? passItem.description : ""
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldClearButton {  }
				}
			}

			QFormSpinBox {
				id: _spinMaxPts

				text: qsTr("Maximális pont:")
				from: 0
				to: 1000
				stepSize: 1
				spin.editable: true

				field: "pts"
				fieldData: passItem ? passItem.maxPts : 0
			}

			QFormSwitchButton
			{
				id: _chExtra
				text: qsTr("Extra pont (nem számít bele a maximálisba)")

				field: "extra"

				fieldData: passItem ? passItem.extra : false
			}

			Row {
				id: _rowCategory

				spacing: 10

				visible: !_tfCategory.visible

				QFormComboBox
				{
					id: _comboCategory
					text: qsTr("Kategória:")

					anchors.verticalCenter: parent.verticalCenter

					combo.width: Math.max(combo.implicitWidth,
										  Math.min(350,
												   _form.width - _rowCategory.spacing - _btnAdd.width
												   - _comboCategory.spacing - label.width))
					combo.onActivated: _form.modified = true

					model: teacherPass ? teacherPass.categoryList : null

					textRole: "description"
					valueRole: "id"

					field: "category"

					fieldData: passItem ? passItem.categoryId : 0

				}

				Qaterial.ToggleButton {
					id: _btnAdd
					anchors.verticalCenter: parent.verticalCenter
					icon.source: Qaterial.Icons.plus
					ToolTip.text: qsTr("Új kategória létrehozása")
					onClicked: {
						_tfCategory.visible = true
						_tfCategory.forceActiveFocus()
					}
				}
			}

			QFormTextField {
				id: _tfCategory

				visible: false

				width: parent.width
				leadingIconSource: Qaterial.Icons.tagPlusOutline
				leadingIconInline: true
				title: qsTr("Új kategória")

				field: "categoryDescription"
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldClearButton {  }
				}
			}


			TeacherPassItemLink {
				id: _passLink
				passItem: control.passItem

				visible: passItem && passItem.linkType !== PassItem.LinkNone

				width: parent.width

				onUnlinked: {
					if (pass)
						pass.reload(HttpConnection.ApiTeacher)
					Client.stackPop(control)
				}

			}


			QButton
			{
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Mentés")
				icon.source: Qaterial.Icons.contentSave
				enabled: pass && _form.modified
				onClicked:
				{
					let d = _form.getItems([_tfDescription, _chExtra, _spinMaxPts, _tfCategory, _comboCategory])

					if (_tfCategory.visible)
						d.category = _tfCategory.text !== "" ? -1 : 0

					let path = ""

					if (passItem)
						path = "passItem/%1/update".arg(passItem.itemid)
					else
						path = "pass/%1/create".arg(pass.passid)

					_form.enabled = false

					Client.send(HttpConnection.ApiTeacher, path, d)
					.done(control, function(r){
						_form.modified = false
						if (_tfCategory.visible)
							teacherPass.reloadCategories()
						Client.stackPop(control)
					})
					.fail(control, function(err) {
						Client.messageWarning(err, passItem ? qsTr("Elem módosítása sikertelen") : qsTr("Eleme létrehozása sikertelen"))
						_form.enabled = true
					})
					.error(control, function(err) {
						_form.enabled = true
					})

				}
			}
		}


		QListView {
			id: _view

			currentIndex: -1
			autoSelectChange: true

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: SortFilterProxyModel {
				sourceModel: teacherPass ? teacherPass.userList : null

				sorters: [
					StringSorter {
						roleName: "fullName"
						sortOrder: Qt.AscendingOrder
						priority: 1
					}
				]
			}


			delegate: QLoaderItemDelegate {
				id: _delegate

				readonly property User passUser: model.qtObject
				property var userData: _model.getUserData(passUser)

				selectableObject: passUser

				highlighted: _view.selectEnabled ? selected : ListView.isCurrentItem
				iconSource: userData.assigned ? Qaterial.Icons.accountCheck : Qaterial.Icons.accountOffOutline
				iconColorBase: userData.assigned ? Qaterial.Style.iconColor() : Qaterial.Style.colorTheme.disabledText

				text: passUser ? passUser.fullName : ""
				//secondaryText: passUser ? passUser.className : ""

				textColor: userData.assigned ? Qaterial.Style.colorTheme.primaryText : Qaterial.Style.colorTheme.disabledText

				Connections {
					target: teacherPass

					function onUserDataChanged(mode, id, username) {
						if (!passItem || !_delegate.passUser)
							return

						if (mode == TeacherPassResultModel.ModePassItem &&
								id == passItem.itemid && username == _delegate.passUser.username)
							userData = _model.getUserData(passUser)
					}
				}


				rightSourceComponent: Column {
					spacing: -3

					Qaterial.LabelHeadline6 {
						anchors.right: parent.right
						color: Qaterial.Style.accentColor
						visible: userData.assigned
						text: teacherPass.round(userData.pts)
					}

					Qaterial.LabelHint1 {
						anchors.right: parent.right
						color: Qaterial.Style.secondaryTextColor()
						visible: userData.assigned
						text: teacherPass.round(userData.result*100) + "%"
					}
				}

				onClicked: {
					Qaterial.DialogManager.showTextFieldDialog({
																   textTitle: qsTr("Eredmény"),
																   title: qsTr("Eredmény módosítása"),
																   text: teacherPass.round(userData.pts),
																   helperText: qsTr("Eredmény beállítása (pont vagy százalék)"),
																   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
																   onAccepted: function(_text, _noerror) {
																	   if (_noerror) {
																		   teacherPass.assignResult(pass, passItem, _view.getSelected(), _text)
																		   _view.unselectAll()
																	   }
																   }
															   })
				}

			}

			Qaterial.Menu {
				id: _contextMenu
				QMenuItem { action: _view.actionSelectAll }
				QMenuItem { action: _view.actionSelectNone }
				Qaterial.MenuSeparator {}
				QMenuItem { action: _actionAssign }
				QMenuItem { action: _actionRemove }
			}

			onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
											if (index != -1)
											currentIndex = index
											_contextMenu.popup(mouseX, mouseY)
										}
		}

	}

	Action {
		id: _actionAssign
		icon.source: Qaterial.Icons.cardPlus
		text: qsTr("Kiosztás")
		enabled: passItem && teacherPass &&
				 (_view.currentIndex != -1 || _view.selectEnabled)
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Eredmény"),
														   title: qsTr("Elem kiosztása"),
														   helperText: qsTr("Eredmény beállítása (pont vagy százalék)"),
														   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror) {
																   teacherPass.assignResult(pass, passItem, _view.getSelected(), _text)
																   _view.unselectAll()
															   }
														   }
													   })

		}
	}

	Action {
		id: _actionRemove
		icon.source: Qaterial.Icons.cardMinus
		text: qsTr("Kiosztás törlése")
		enabled: passItem && teacherPass &&
				 (_view.currentIndex != -1 || _view.selectEnabled)
		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								teacherPass.removeResult(pass, passItem, _view.getSelected())
								_view.unselectAll()
							},
							text: qsTr("Biztosan törlöd az eredményeket?"),
							title: qsTr("Eredmények törlése"),
							iconSource: Qaterial.Icons.cardMinusOutline
						})
		}
	}

	Component.onCompleted: {
		if (teacherPass)
			teacherPass.reloadPassItemResult(passItem)
	}

}
