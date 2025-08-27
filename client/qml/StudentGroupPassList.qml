import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel


QItemGradient {
	id: control

	property StudentMapHandler mapHandler: null
	property StudentGroupList groupList: null
	property StudentGroup group: null

	title: group ? group.name+qsTr(" | Call Pass") : ""

	signal changeGroup(StudentGroup group)

	appBar.rightComponent: StudentGroupButton {
		anchors.verticalCenter: parent.verticalCenter
		groupList: control.groupList
		group: control.group
		onChangeGroup: group => control.changeGroup(group)
	}


	QScrollable {
		anchors.fill: parent
		topPadding: Math.max(Client.safeMarginTop, control.paddingTop + 5)
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		refreshEnabled: true
		onRefreshRequest: {
			Client.reloadCache("passList")

			for (let i=0; i<view.count; ++i) {
				let e = view.itemAtIndex(i)

				let p = e.pass

				if (e.expanded && p)
					p.reload()
			}

		}


		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: false

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: SortFilterProxyModel {
				id: _passList

				sourceModel: Client.cache("passList")

				filters: [
					ValueFilter {
						roleName: "groupid"
						value: group ? group.groupid : -1
					}

				]

				sorters: [
					RoleSorter {
						roleName: "childless"
						sortOrder: Qt.DescendingOrder
						priority: 4
					},
					RoleSorter {
						roleName: "isActive"
						sortOrder: Qt.DescendingOrder
						priority: 3
					},
					RoleSorter {
						roleName: "endTime"
						sortOrder: Qt.AscendingOrder
						priority: 2
					},
					RoleSorter {
						roleName: "passid"
						sortOrder: Qt.AscendingOrder
						priority: 1
					}
				]
			}


			delegate: Qaterial.Expandable {
				id: _expandable

				width: view.width

				property Pass pass: model.qtObject
				property bool showPlaceholder: true

				expanded: false

				onExpandedChanged: if (expanded && pass)
									   pass.reload()

				Connections {
					target: pass

					function onItemsLoaded() {
						_expandable.showPlaceholder = false
					}
				}

				header: Qaterial.LoaderItemDelegate {
					id: _delegate

					width: _expandable.width

					text: _expandable.pass ? (_expandable.pass.title != "" ? _expandable.pass.title
																		   : qsTr("Call Pass #%1").arg(_expandable.pass.passid)) : ""

					leftSourceComponent: Qaterial.RoundColorIcon
					{
						source: _expandable.pass && _expandable.pass.childless ?
									Qaterial.Icons.tagText :
									Qaterial.Icons.tagMultipleOutline

						color: _expandable.pass && _expandable.pass.childless ?
								   Qaterial.Style.accentColor :
								   Qaterial.Style.iconColor()

						iconSize: Qaterial.Style.delegate.iconWidth

						fill: true
						width: roundIcon ? roundSize : iconSize
						height: roundIcon ? roundSize : iconSize
					}



					rightSourceComponent: Column {
						spacing: -2

						Row {
							anchors.right: parent.right

							spacing: 5

							Repeater {
								model: _expandable.pass ? _expandable.pass.gradeList : null

								delegate: Qaterial.LabelHeadline5 {
									color: Qaterial.Colors.red400
									anchors.verticalCenter: parent.verticalCenter
									text: modelData.shortname
								}
							}
						}

						Qaterial.LabelHint1 {
							anchors.right: parent.right
							text: _expandable.pass ?
									  "<b>" + _expandable.pass.round(_expandable.pass.pts) + "</b>/" + _expandable.pass.maxPts + qsTr(" pt") +
									  (_expandable.pass.result>=0 ? " (" + _expandable.pass.round(_expandable.pass.result*100)+"%)" : "") :
									  ""
							color: Qaterial.Style.primaryTextColor()
						}
					}

					onClicked: _expandable.expanded = !_expandable.expanded
				}


				delegate: Item {
					width: _expandable.width
					height: _passItem.height + 30 * Qaterial.Style.pixelSizeRatio

					StudentPass {
						id: _passItem

						pass: _expandable.pass
						width: Math.min(parent.width - 12 * Qaterial.Style.pixelSizeRatio, Qaterial.Style.maxContainerSize * 0.8,
										650)

						showPlaceholders: _expandable.showPlaceholder

						anchors.centerIn: parent
					}
				}


				Component.onCompleted: {
					if (pass && pass.isActive && pass.childless)
						_expandable.expanded = true
				}

			}


		}

	}


	StackView.onActivated: {
		Client.contextHelper.setCurrentContext(ContextHelperData.ContextStudentGroupCallPass)
		Client.reloadCache("passList")
	}

	StackView.onDeactivating: {
		Client.contextHelper.unsetContext(ContextHelperData.ContextStudentGroupCallPass)
	}
}
