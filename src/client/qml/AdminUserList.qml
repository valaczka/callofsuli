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

			QTextField {
				id: mainSearch
				width: parent.width

				placeholderText: qsTr("Keresés...")

				onTextChanged: header.searchText.text = mainSearch.text
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
				}

				QCheckBox {
					id: checkTeacher
					text: qsTr("Tanár")
				}

				QCheckBox {
					id: checkStudent
					text: qsTr("Diák")
				}

				QCheckBox {
					id: checkAdmin
					text: qsTr("Admin")
				}

				QCheckBox {
					id: checkActive
					text: qsTr("Aktív")
				}

				QCheckBox {
					id: checkInactive
					text: qsTr("Nem aktív")
				}

				QCheckBox {
					id: checkNoClass
					text: qsTr("Osztály nélkül")
				}
			}
		}


		rightLoader.sourceComponent: QMenuButton {
			id: userMenu
			MenuItem {
				text: qsTr("Új felhasználó")
				onClicked: {
					pageAdminUsers.userSelected("")
				}
			}

			QMenu {
				title: qsTr("Aktív")
				enabled: userList.selectorSet

				MenuItem {
					text: qsTr("Igen")
				}

				MenuItem {
					text: qsTr("Nem")
				}
			}

			QMenu {
				title: qsTr("Tanár")
				enabled: userList.selectorSet

				MenuItem {
					text: qsTr("Igen")
				}

				MenuItem {
					text: qsTr("Nem")
				}
			}

			QMenu {
				title: qsTr("Admin")
				enabled: userList.selectorSet

				MenuItem {
					text: qsTr("Igen")
				}

				MenuItem {
					text: qsTr("Nem")
				}
			}

			QMenu {
				id: menuClass
				enabled: userList.selectorSet
				title: qsTr("Osztályba")

				MenuItem {
					text: qsTr("Osztály nélkül")
				}

				MenuSeparator { }

				Repeater {
					model: _classList
					MenuItem {
						text: modelData.name
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

		onClicked: pageAdminUsers.userSelected(model.get(index).username)
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

		onUserListLoaded: getList(list)
		onClassListLoaded: _classList = list
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
}

