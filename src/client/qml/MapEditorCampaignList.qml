import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property MapEditor map: null

	title: qsTr("Hadjáratok")


	QPageHeader {
		id: header

		isSelectorMode: list.selectorSet

		labelCountText: list.selectedItemCount

		QTextField {
			id: newCampaignName
			width: parent.width

			placeholderText: qsTr("új hadjárat hozzáadása")
			onAccepted: {
				map.undoLogBegin(qsTr("Hadjárat hozzáadása"))
				if (map.campaignAdd({ "name": newCampaignName.text }))
					newCampaignName.clear()
				map.undoLogEnd()
			}
		}

		rightLoader.sourceComponent: QCloseButton { }

		onSelectAll: list.selectAll()
	}

	QListItemDelegate {
		id: list
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		modelTitleRole: "name"

		autoSelectorChange: true

		onClicked: pageEditor.campaignSelected(list.model.get(index).id)
	}


	Connections {
		target: map
		onCampaignListUpdated: getList()
		onUndone: getList()
	}

	function populated() {
		getList()
	}


	function getList() {
		JS.setModel(list.model, map.campaignListGet())
	}
}
