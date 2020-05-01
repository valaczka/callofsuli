import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import SortFilterProxyModel 0.2
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property AdminUsers adminUsers: null

	property var _classList: []
	property var _selectedClasses: []
	property string _selectedClassesRegExp: ""

	title: qsTr("Felhasználók")

	QPageHeader {
		id: header

		isSelectorMode: userList.selectorSet

		labelCountText: userList.selectedItemCount

		searchText.onTextChanged: mainSearch.text = searchText.text

		Column {
			width: parent.width

			Row {
				id: row
				width: parent.width
				spacing: 5

				QTextField {
					id: mainSearch
					anchors.verticalCenter: parent.verticalCenter
					width: row.width-row.spacing-userMenu.width

					placeholderText: qsTr("Keresés...")

					onTextChanged: header.searchText.text = mainSearch.text
				}

				QMenuButton {
					id: userMenu
					anchors.verticalCenter: parent.verticalCenter
					MenuItem {
						icon.source: CosStyle.iconAdd
						text: qsTr("Új felhasználó")
						onClicked: {
							pageAdminUsers.userSelected("")
						}
					}
				}
			}

			Flow {
				width: parent.width

				QTag {
					id: classes
					checkable: true
					checked: true
					title: qsTr("Osztály:")
					width: parent.width
					defaultColor: CosStyle.colorAccentLight
					defaultBackground: CosStyle.colorAccentDark
					modelTextRole: "name"

					onClicked: loadDialogClasses()

					onCheckedChanged: if (checked && checkNoClass)
										  checkNoClass.checked = false
				}

				QCheckBox {
					id: checkTeacher
					text: qsTr("Tanár")

					onCheckedChanged: if (checked)
										  checkStudent.checked = false
				}

				QCheckBox {
					id: checkStudent
					text: qsTr("Diák")
					onCheckedChanged: if (checked)
										  checkTeacher.checked = false
				}

				QCheckBox {
					id: checkAdmin
					text: qsTr("Admin")
				}

				QCheckBox {
					id: checkActive
					text: qsTr("Aktív")
					onCheckedChanged: if (checked)
										  checkInactive.checked = false
				}

				QCheckBox {
					id: checkInactive
					text: qsTr("Nem aktív")
					onCheckedChanged: if (checked)
										  checkActive.checked = false
				}

				QCheckBox {
					id: checkNoClass
					text: qsTr("Osztály nélkül")
					onCheckedChanged: if (checked && classes)
										  classes.checked = false
				}
			}
		}

		selectorLoader.sourceComponent: Flow {
			id: flow
			property int buttonDisplay: AbstractButton.IconOnly

			QToolButton { action: actionActiveSet; display: flow.buttonDisplay }
			QToolButton { action: actionActiveUnset; display: flow.buttonDisplay  }
			QToolButton { action: actionTeacherSet; display: flow.buttonDisplay  }
			QToolButton { action: actionTeacherUnset; display: flow.buttonDisplay }
			QToolButton { action: actionAdminSet; display: flow.buttonDisplay  }
			QToolButton { action: actionAdminUnset; display: flow.buttonDisplay  }


			QMenuButton {
				icon.source: CosStyle.iconRemove
				ToolTip.text: qsTr("Osztályba sorol")

				MenuItem {
					text: qsTr("Osztály nélkül")
					onTriggered: runBatchFunction("classid", null)
				}

				MenuSeparator { }

				Repeater {
					model: _classList
					MenuItem {
						text: modelData.name
						onTriggered: runBatchFunction("classid", modelData.id)
					}
				}
			}


		}

		onSelectAll: userList.selectAll()
	}

	ListModel {
		id: baseUserModel
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: baseUserModel
		filters: [
			AnyOf {
				enabled: mainSearch.text.length
				RegExpFilter {
					roleName: "lastname"
					pattern: mainSearch.text
					caseSensitivity: Qt.CaseInsensitive
					syntax: RegExpFilter.FixedString
				}
				RegExpFilter {
					roleName: "firstname"
					pattern: mainSearch.text
					caseSensitivity: Qt.CaseInsensitive
					syntax: RegExpFilter.FixedString
				}
				RegExpFilter {
					roleName: "username"
					pattern: mainSearch.text
					caseSensitivity: Qt.CaseInsensitive
					syntax: RegExpFilter.FixedString
				}
			},
			ValueFilter {
				enabled: checkTeacher.checked
				roleName: "isTeacher"
				value: true
			},
			ValueFilter {
				enabled: checkStudent.checked
				roleName: "isTeacher"
				value: false
			},
			ValueFilter {
				enabled: checkAdmin.checked
				roleName: "isAdmin"
				value: true
			},
			ValueFilter {
				enabled: checkActive.checked
				roleName: "isActive"
				value: true
			},
			ValueFilter {
				enabled: checkInactive.checked
				roleName: "isActive"
				value: false
			},
			ValueFilter {
				enabled: checkNoClass.checked
				roleName: "classid"
				value: -1
			},
			RegExpFilter {
				enabled: classes.checked
				roleName: "classid"
				pattern: _selectedClassesRegExp
			}

		]
		sorters: [
			StringSorter { roleName: "firstname" },
			StringSorter { roleName: "lastname" }
		]

		proxyRoles: [
			JoinRole {
				name: "fullname"
				roleNames: ["firstname", "lastname"]
			},
			SwitchRole {
				name: "textColor"
				filters: ValueFilter {
					roleName: "isActive"
					value: false
					SwitchRole.value: "#88000000"
				}
				defaultValue: "black"
			}
		]

	}

	QListItemDelegate {
		id: userList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		model: userProxyModel
		isProxyModel: true

		delegateHeight: CosStyle.twoLineHeight

		isObjectModel: true
		modelTitleRole: "fullname"
		modelSubtitleRole: "username"
		modelTitleColorRole: "textColor"
		modelSubtitleColorRole: "textColor"

		autoSelectorChange: true

		rightComponent: Label {
			text: model && model["classname"] ? model["classname"] : ""
			color: "black"
			font.weight: Font.DemiBold
			font.pixelSize: CosStyle.pixelSize*0.9
			leftPadding: 5
			rightPadding: 5
		}


		onClicked: pageAdminUsers.userSelected(model.get(index).username)

		onRightClicked: {
			contextMenu.disableOwn = (model.get(index).username === cosClient.userName)
			contextMenu.popup()
		}

		QMenu {
			id: contextMenu

			property bool disableOwn: false

			MenuItem { action: actionActiveSet }
			MenuItem { action: actionActiveUnset; enabled: !contextMenu.disableOwn }
			MenuItem { action: actionTeacherSet }
			MenuItem { action: actionTeacherUnset }
			MenuItem { action: actionAdminSet }
			MenuItem { action: actionAdminUnset; enabled: !contextMenu.disableOwn }

			QMenu {
				title: qsTr("Osztályba sorol")

				MenuItem {
					text: qsTr("Osztály nélkül")
					onTriggered: runBatchFunction("classid", null)
				}

				MenuSeparator { }

				Repeater {
					model: _classList
					MenuItem {
						text: modelData.name
						onTriggered: runBatchFunction("classid", modelData.id)
					}
				}
			}
		}
	}


	Connections {
		target: pageAdminUsers
		onClassSelected: if (classes.checked ){
							 var ii = []
							 ii.push(id)
							 _selectedClasses = ii
							 var n = []
							 for (var i=0; i<_classList.length; ++i) {
								 if (_classList[i].id === id) {
									 _selectedClassesRegExp = id
									 n.push({"name": _classList[i].name})
									 classes.tags = n
									 return
								 }
							 }
						 }
	}

	Connections {
		target: adminUsers

		onUserCreated: reloadUserList()
		onUserUpdated: reloadUserList()

		onUserListLoaded: getList(list)
		onClassListLoaded: {
			_classList = list
			reloadUserList()
		}

		onUserBatchUpdated: {
			if (data.error)
				cosClient.sendMessageWarning(qsTr("Felhasználók módosítása"), qsTr("Szerver hiba"), data.error)
			else {
				userList.selectAll(false)
				reloadUserList()
			}
		}
	}




	Action {
		id: actionActiveSet
		icon.source: CosStyle.iconChecked
		text: qsTr("Aktivál")
		onTriggered: runBatchFunction("active", true)
	}

	Action {
		id: actionActiveUnset
		icon.source: CosStyle.iconUnchecked
		text: qsTr("Inaktivál")
		onTriggered: runBatchFunction("active", false)
	}

	Action {
		id: actionTeacherSet
		icon.source: CosStyle.iconChecked
		text: qsTr("Tanár jogot ad")
		onTriggered: runBatchFunction("isTeacher", true)
	}

	Action {
		id: actionTeacherUnset
		icon.source: CosStyle.iconUnchecked
		text: qsTr("Tanár jogot töröl")
		onTriggered: runBatchFunction("isTeacher", false)
	}

	Action {
		id: actionAdminSet
		icon.source: CosStyle.iconChecked
		text: qsTr("Admin jogot ad")
		onTriggered: runBatchFunction("isAdmin", true)
	}

	Action {
		id: actionAdminUnset
		icon.source: CosStyle.iconUnchecked
		text: qsTr("Admin jogot töröl")
		onTriggered: runBatchFunction("isAdmin", false)
	}



	function populated() {
	}


	function reloadUserList() {
		adminUsers.send({"class": "user", "func": "getAllUser"})
	}


	function getList(_list) {
		JS.setModel(baseUserModel, _list)
	}


	function loadDialogClasses() {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Szűrés osztályokra")
		d.item.newField.visible = false
		d.item.list.selectorSet = true
		d.item.list.modelTitleRole = "name"
		d.item.list.modelSelectedRole = "selected"

		for (var i=0; i<_classList.length; ++i) {
			var o = _classList[i]
			o.selected = _selectedClasses.includes(o.id)
			d.item.model.append(o)
		}

		d.accepted.connect(function() {
			_selectedClasses = JS.getSelectedIndices(d.item.model, "id")
			_selectedClassesRegExp = "("+_selectedClasses.join("|")+")"
			var cc = JS.getSelectedIndices(d.item.model, "name")
			var n = []
			for (i=0; i<cc.length; ++i)
				n.push({"name": cc[i]})
			classes.tags = n
		})
		d.open()
	}


	function runBatchFunction(p, value) {
		var l = []
		if (userList.selectorSet)
			l = JS.getSelectedIndices(baseUserModel, "username")
		else if (userList.currentIndex != -1)
			l.push(userList.model.get(userList.currentIndex).username)

		if (l.length === 0)
			return

		if (p==="active" || p==="isAdmin") {
			var i = l.indexOf(cosClient.userName)
			if (i !== -1)
				l.splice(i, 1)
		}

		var d = {}
		d["class"] = "user"
		d["func"] ="userBatchUpdate"
		d[p] = value
		d["users"] = l
		adminUsers.send(d)
	}
}

