import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSwipeContainer {
	id: panel

	title: qsTr("Résztvevők")
	icon: CosStyle.iconGroup

	property alias list: userList
	property alias buttonEditAction: buttonEdit.action

	QVariantMapProxyView {
		id: userList
		anchors.fill: parent

		visible: teacherGroups.modelUserList.count

		refreshEnabled: true
		delegateHeight: CosStyle.twoLineHeight

		section.property: "classname"
		section.criteria: ViewSection.FullString
		section.delegate: Component {
			Rectangle {
				width: userList.width
				height: childrenRect.height
				color: CosStyle.colorPrimaryDark

				required property string section

				QLabel {
					text: parent.section
					font.pixelSize: CosStyle.pixelSize*0.8
					font.weight: Font.DemiBold
					font.capitalization: Font.AllUppercase
					color: "white"

					leftPadding: 5
					topPadding: 2
					bottomPadding: 2
					rightPadding: 5

					elide: Text.ElideRight
				}
			}
		}


		leftComponent: Image {
			source: model ? cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage) : ""
			width: userList.delegateHeight+10
			height: userList.delegateHeight*0.8
			fillMode: Image.PreserveAspectFit
		}

		model: SortFilterProxyModel {
			id: userProxyModel
			sourceModel: teacherGroups.modelUserList

			sorters: [
				StringSorter { roleName: "classname"; priority: 2 },
				StringSorter { roleName: "name"; priority: 1 }
			]

			proxyRoles: [
				ExpressionRole {
					name: "name"
					expression: model.firstname+" "+model.lastname
				},
				SwitchRole {
					name: "titlecolor"
					filters: ValueFilter {
						roleName: "activeClient"
						value: true
						SwitchRole.value: CosStyle.colorOK
					}
					defaultValue: CosStyle.colorPrimaryLighter
				}
			]
		}

		modelTitleRole: "name"
		modelSubtitleRole: "nickname"
		modelTitleColorRole: "titlecolor"
		modelSubtitleColorRole: "titlecolor"

		highlightCurrentItem: false

		onRefreshRequest: teacherGroups.send("groupGet", { id: teacherGroups.selectedGroupId })
	}

	QToolButtonBig {
		id: buttonEdit
		anchors.centerIn: parent
		visible: !userList.visible
	}

}



