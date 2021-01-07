import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: control

	property alias textStorageModuleName: textStorageModuleName
	property alias comboChapter: comboChapter

	QGridLayout {
		width: parent.width

		watchModification: false


		QGridLabel { text: qsTr("Szakasz") }

		QGridComboBox {
			id: comboChapter

			model: SortFilterProxyModel {
				sourceModel: mapEditor.modelChapterList

				filters: [
					ValueFilter {
						roleName: "type"
						value: 0
					}
				]

				sorters: [
					RoleSorter {
						roleName: "name"
					}
				]
			}

			textRole: "name"
			valueRole: "id"

			onActivated: {
				mapEditor.run("objectiveModify", {uuid: objectiveUuid, data: { chapter: model.get(index).id }})
			}
		}


		QGridLabel {
			field: textStorageModuleName
			visible: textStorageModuleName.visible
		}

		QGridTextField {
			id: textStorageModuleName
			fieldName: qsTr("Storage")

			readOnly: true

			visible: text.length
		}

		QGridButton {
			visible: textStorageModuleName.visible
			text: qsTr("Szerkesztés")
			icon.source: CosStyle.iconEdit
		}
	}

	Component.onCompleted: {
		if (!moduleData)
			return

		comboChapter.setData(moduleData.chapter)

		if (moduleData.storage)
			textStorageModuleName.setData(mapEditor.storageInfo(moduleData.storageModule).name)
	}
}
