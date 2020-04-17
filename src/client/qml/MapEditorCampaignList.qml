import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null


	title: qsTr("Hadjáratok")

	rightLoader.sourceComponent: QCloseButton {
		visible: modelIndex > 0
		onClicked: if (view) {
					   view.model.remove(modelIndex)
				   }
	}

	QPageHeader {
		id: header

		height: col.height

		Column {
			id: col
			width: parent.width


			QTextField {
				id: newCampaignName
				width: parent.width

				placeholderText: qsTr("új hadjárat hozzáadása")
				onAccepted: {
					if (map.campaignAdd({ "name": newCampaignName.text }))
						newCampaignName.clear()
				}
			}
		}
	}

	QListItemDelegate {
		id: list
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		modelTitleRole: "name"

		//			modelTitleSet: true
		//			modelSubtitleSet: true

		onClicked: pageEditor.campaignSelected(modelIndex, list.model[index].id)
	}


	Connections {
		target: map
		onCampaignListUpdated: getList()
	}

	Component.onCompleted: getList()


	function getList() {
		list.model = map.campaignListGet()
	}
}
