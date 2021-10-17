import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSimpleContainer {
	id: panel

	isPanelVisible: false
	maximumWidth: -1

	title: qsTr("Küldetések")
	icon: "qrc:/internal/img/battle.png"

	property alias list: list


	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: studentMaps.modelMissionList
		sorters: [
			FilterSorter {
				ValueFilter { roleName: "lockDepth"; value: 0 }
				priority: 2
			},
			RoleSorter { roleName: "num"; priority: 1 },
			StringSorter { roleName: "name"; priority: 0 }
		]
		proxyRoles: [
			SwitchRole {
				name: "textColor"
				filters: [
					ExpressionFilter {
						expression: model.lockDepth === 0 && model.fullSolved
						SwitchRole.value: CosStyle.colorOKLighter
					},
					ExpressionFilter {
						expression: model.lockDepth > 0
						SwitchRole.value: JS.setColorAlpha(CosStyle.colorPrimary, 0.5)
					}
				]
				defaultValue: CosStyle.colorPrimary
			}
		]
	}


	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: studentMaps.modelMissionList.count

		model: userProxyModel
		modelTitleRole: "name"
		modelTitleColorRole: "textColor"
		fontFamilyTitle: "HVD Peace"

		pixelSizeTitle: CosStyle.pixelSize*1.3

		autoSelectorChange: false

		refreshEnabled: true

		delegateHeight: CosStyle.twoLineHeight*2

		leftComponent: Image {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			fillMode: Image.PreserveAspectFit

			source: model && model.medalImage.length ? cosClient.medalIconPath(model.medalImage) : ""

			opacity: model && model.lockDepth === 0 ? 1.0 : 0.3

			visible: model && model.medalImage.length
		}


		contentComponent: QLabel {
			font.pixelSize: CosStyle.pixelSize*0.8
			font.weight: Font.Normal
			color: model ? model.textColor : "white"
			visible: model && model.lockDepth === 0
			elide: Text.ElideRight
			maximumLineCount: 2
			wrapMode: Text.Wrap
			text: model ? model.description : ""
		}

		rightComponent: Column {
			QFontImage {
				anchors.right: parent.right

				width: visible ? list.delegateHeight : 0
				height: width*0.8
				size: Math.min(height*0.8, 32)

				icon: CosStyle.iconLock

				visible: model && model.lockDepth > 0

				color: model ? model.textColor : CosStyle.colorPrimary
			}


			Grid {
				id: rw

				spacing: 4

				columns: 3
				layoutDirection: Qt.RightToLeft

				visible: model.lockDepth === 0

				anchors.right: parent.right

				readonly property real rowHeight: CosStyle.pixelSize*1.6


				QMedalImage {
					visible: model.t1has
					opacity: model.t1>0 ? 1.0 : 0.2
					level: 1
					image: opacity == 1.0 && model && model.medalImage.length ? model.medalImage : ""
					isDeathmatch: false
					height: rw.rowHeight
					width: rw.rowHeight
				}

				QMedalImage {
					visible: model.d1has
					opacity: model.d1>0 ? 1.0 : 0.2
					level: 1
					image: opacity == 1.0 && model && model.medalImage.length ? model.medalImage : ""
					isDeathmatch: true
					height: rw.rowHeight
					width: rw.rowHeight
				}

				QMedalImage {
					visible: model.t2has
					opacity: model.t2>0 ? 1.0 : 0.2
					level: 2
					image: opacity == 1.0 && model && model.medalImage.length ? model.medalImage : ""
					isDeathmatch: false
					height: rw.rowHeight
					width: rw.rowHeight
				}

				QMedalImage {
					visible: model.d2has
					opacity: model.d2>0 ? 1.0 : 0.2
					level: 2
					image: opacity == 1.0 && model && model.medalImage.length ? model.medalImage : ""
					isDeathmatch: true
					height: rw.rowHeight
					width: rw.rowHeight
				}

				QMedalImage {
					visible: model.t3has
					opacity: model.t3>0 ? 1.0 : 0.2
					level: 3
					image: opacity == 1.0 && model && model.medalImage.length ? model.medalImage : ""
					isDeathmatch: false
					height: rw.rowHeight
					width: rw.rowHeight
				}

				QMedalImage {
					visible: model.d3has
					opacity: model.d3>0 ? 1.0 : 0.2
					level: 3
					image: opacity == 1.0 && model && model.medalImage.length ? model.medalImage : ""
					isDeathmatch: true
					height: rw.rowHeight
					width: rw.rowHeight
				}
			}

		}



		onRefreshRequest: studentMaps.getMissionList()

		onClicked: {
			studentMaps.missionSelected(list.model.mapToSource(index))
		}

		onLongPressed: {
			var o = list.model.get(index)

			var dd = JS.dialogCreateQml("Message", {
											title: o.name,
											text: o.description,
											maximumHeight: panel.height*0.8
										})
			dd.open()

		}

	}


	onPopulated: list.forceActiveFocus()

}



