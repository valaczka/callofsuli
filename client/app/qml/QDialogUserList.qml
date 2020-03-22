import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import SortFilterProxyModel 0.2
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property alias list: list
	property alias selectorSet: list.selectorSet
	readonly property bool simpleSelect: !list.selectorSet
	property alias model: model.sourceModel
	/*property var roles: [username, firstname, lastname, active, "
"classname, classid, isTeacher, isAdmin]*/

	maximumHeight: 0
	maximumWidth: 700

	acceptedData: -1

	QObjectListView {
		id: list

		anchors.fill: parent

		Component {
			id: sectionHeading
			Rectangle {
				width: list.width
				height: childrenRect.height
				color: CosStyle.colorPrimaryDarker

				required property string section

				QLabel {
					text: parent.section
					font.pixelSize: CosStyle.pixelSize*0.85
					font.weight: Font.DemiBold
					font.capitalization: Font.AllUppercase
					color: CosStyle.colorPrimaryLight

					leftPadding: 5
					topPadding: 2
					bottomPadding: 2
					rightPadding: 5

					elide: Text.ElideRight
				}
			}
		}

		model: SortFilterProxyModel {
			id: model

			filters: [
				RegExpFilter {
					enabled: toolbar.searchBar.text.length
					roleName: "fullname"
					pattern: toolbar.searchBar.text
					caseSensitivity: Qt.CaseInsensitive
					syntax: RegExpFilter.FixedString
				}
			]

			sorters: [
				StringSorter { roleName: "classname"; priority: 2 },
				RoleSorter { roleName: "classid"; priority: 1 },
				StringSorter { roleName: "fullname" }
			]

			proxyRoles: [
				ExpressionRole {
					name: "fullname"
					expression: firstname+" "+lastname
				}
			]
		}

		modelTitleRole: "fullname"

		autoSelectorChange: false

		section.property: "classname"
		section.criteria: ViewSection.FullString
		section.delegate: sectionHeading

		onClicked: if (simpleSelect) {
					   acceptedData = modelObject(index)
					   dlgClose()
				   }
	}

	QPagePanelSearch {
		id: toolbar

		listView: list

		enabled: model.sourceModel.count
		labelCountText: list.objectModel ? list.objectModel.selectedCount : ""
		onSelectAll: JS.selectAllProxyModelToggle(model)
	}


	buttons: Row {
		id: buttonRow
		spacing: 10

		anchors.horizontalCenter: parent.horizontalCenter

		QButton {
			id: buttonNo
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Mégsem")
			icon.source: CosStyle.iconCancel
			themeColors: CosStyle.buttonThemeRed

			onClicked: dlgClose()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			visible: !simpleSelect

			text: qsTr("OK")
			icon.source: CosStyle.iconOK
			themeColors: CosStyle.buttonThemeGreen

			onClicked: {
				dlgClose()
			}
		}
	}



	function populated() {
		list.forceActiveFocus()
	}

}
