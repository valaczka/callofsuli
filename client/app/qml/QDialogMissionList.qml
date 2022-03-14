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
	readonly property alias model: proxyModel
	property alias sourceModel: proxyModel.sourceModel


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
				color: CosStyle.colorWarningDark

				required property string section

				QLabel {
					text: parent.section
					font.pixelSize: CosStyle.pixelSize
					font.weight: Font.Medium
					//font.capitalization: Font.AllUppercase
					color: "white"

					leftPadding: 5
					topPadding: 2
					bottomPadding: 2
					rightPadding: 5

					elide: Text.ElideRight
				}
			}
		}

		model: SortFilterProxyModel {
			id: proxyModel
			sorters: [
				StringSorter { roleName: "name"; priority: 2 },
				RoleSorter { roleName: "level"; priority: 1 }
			]

			proxyRoles: [
				ExpressionRole {
					name: "fullname"
					expression: model.level+qsTr(". szint")
				}
			]
		}

		modelTitleRole: "fullname"

		autoSelectorChange: false

		section.property: "name"
		section.criteria: ViewSection.FullString
		section.delegate: sectionHeading

		colorTitle: CosStyle.colorWarningLight
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

			text: qsTr("OK")
			icon.source: CosStyle.iconOK
			themeColors: CosStyle.buttonThemeGreen

			onClicked: {
				acceptedData = true
				dlgClose()
			}
		}
	}



	function populated() {
		list.forceActiveFocus()
	}

}
