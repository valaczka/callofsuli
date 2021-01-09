import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import SortFilterProxyModel 0.2
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: panel

	property var _classList: []
	property var _selectedClasses: []
	property string _selectedClassesRegExp: ""

	property alias delegate: userList
	property alias model: baseUserModel

	property alias tagClasses: classes
	property alias checkTeacher: checkTeacher
	property alias checkStudent: checkStudent
	property alias checkAdmin: checkAdmin
	property alias checkActive: checkActive
	property alias checkInactive: checkInactive
	property alias checkNoClass: checkNoClass

	property alias filterButtonVisible: filterButton.visible

	property bool filterShow: false


	default property alias headerData: header.colData

	QPageHeader {
		id: header

		width: parent.width

		isSelectorMode: userList.selectorSet

		labelCountText: userList.selectedItemCount

		mainItem: Row {
			id: row1
			spacing: 0

			width: parent.width

			QTextField {
				id: mainSearch
				width: row1.width-filterButton.width-row1.spacing
				lineVisible: false

				anchors.verticalCenter: parent.verticalCenter

				placeholderText: qsTr("Keresés...")
				clearAlwaysVisible: true
			}

			QToolButton {
				id: filterButton

				anchors.verticalCenter: parent.verticalCenter

				icon.source: CosStyle.iconAdjust

				ToolTip.text: qsTr("Szűrők")

				onClicked: {
					if (filterShow) {
						classes.checked = false
						checkTeacher.checked = false
						checkStudent.checked = false
						checkAdmin.checked = false
						checkActive.checked = false
						checkInactive.checked = false
						checkNoClass.checked = false
					}

					filterShow = !filterShow
				}
			}
		}

		Flow {
			width: parent.width

			visible: filterShow

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

		onSelectAll: userList.selectAll()
		onDeselectAll: userList.selectAll(false)
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

		modelTitleRole: "fullname"
		modelSubtitleRole: "username"
		modelTitleColorRole: "textColor"
		modelSubtitleColorRole: "textColor"

		autoSelectorChange: true

		rightComponent: QLabel {
			text: model && model["classname"] ? model["classname"] : ""
			color: "black"
			font.weight: Font.DemiBold
			font.pixelSize: CosStyle.pixelSize*0.9
			leftPadding: 5
			rightPadding: 5
		}
	}


	function autoCreateClassList() {
		var l = []
		_classList = []
		for (var i=0; i<baseUserModel.count; ++i) {
			var n = baseUserModel.get(i)
			if (!l.includes(n["classid"])) {
				l.push(n["classid"])
				_classList.push({id: n["classid"], name: n["classname"]})
			}
		}
	}


	function loadDialogClasses() {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Szűrés osztályokra")
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

