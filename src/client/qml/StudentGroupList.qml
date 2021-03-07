import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	layoutFillWidth: true

	title: qsTr("Csoportok")
	icon: "image://font/School/\uf19d"

	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: studentMaps.modelGroupList.count

		model: SortFilterProxyModel {
			sourceModel: studentMaps.modelGroupList
			sorters: [
				StringSorter { roleName: "name" }
			]

			proxyRoles: ExpressionRole {
				name: "details"
				expression: (model.readableClassList.length ? model.readableClassList+" - " : "")
							+ model.teacherfirstname+" "+model.teacherlastname
			}
		}


		modelTitleRole: "name"
		modelSubtitleRole: "details"

		autoSelectorChange: false

		refreshEnabled: true

		delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: "image://font/School/\uf19d"

			visible: model

			color: CosStyle.colorPrimary
		}


		onRefreshRequest: studentMaps.send("groupListGet")

		onClicked: {
			var o = list.model.get(index)
			studentMaps.selectGroup(o.id)
		}
	}

	onPopulated: list.forceActiveFocus()
}



