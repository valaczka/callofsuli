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

	appBar.rightComponent: StudentGroupButton {
		anchors.verticalCenter: parent.verticalCenter
		groupList: control.groupList
		group: control.group
		onGroupChanged: control.group = group
	}


	QScrollable {
		anchors.fill: parent
		topPadding: Math.max(Client.safeMarginTop, control.paddingTop + 5)
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		refreshEnabled: true
		onRefreshRequest: Client.reloadCache("passList")

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

				/*filters: [
					ValueFilter {
						roleName: "groupid"
						value: group ? group.groupid : -1
					}

				]

				sorters: [
					RoleSorter {
						roleName: "isActive"
						sortOrder: Qt.AscendingOrder
						priority: 3
					},
					RoleSorter {
						roleName: "childless"
						sortOrder: Qt.AscendingOrder
						priority: 2
					},
					RoleSorter {
						roleName: "endTime"
						sortOrder: Qt.AscendingOrder
						priority: 3
					}
				]*/
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

					text: _expandable.pass ? (_expandable.pass.title != "" ? ""
																		   : qsTr("Call Pass #%1").arg(_expandable.pass.passid)) : ""

					secondaryText: {
						if (!_expandable.pass)
							return ""

						if (_expandable.pass.startTime.getTime()) {
							return _expandable.pass.startTime.toLocaleString(Qt.locale(), "yyyy. MMM d. – ")
									+ (_expandable.pass.endTime.getTime() ? _expandable.pass.endTime.toLocaleString(Qt.locale(), "yyyy. MMM d.") : "")
						}

						return ""
					}

					/*textColor: iconColor
					secondaryTextColor: !campaign || campaign.state == Campaign.Finished ?
											Qaterial.Style.disabledTextColor() : Qaterial.Style.colorTheme.secondaryText*/

					leftSourceComponent: Qaterial.RoundColorIcon
					{
						source: Qaterial.Icons.passportCheck //_delegate.iconSource
						//color: _delegate.iconColor
						iconSize: Qaterial.Style.delegate.iconWidth

						fill: true
						width: roundIcon ? roundSize : iconSize
						height: roundIcon ? roundSize : iconSize
					}

					rightSourceComponent: Qaterial.LabelHeadline5 {
						visible: text != ""
						text: {
							if (!_expandable.pass)
								return ""

							return _expandable.pass.pts + " / " +_expandable.pass.maxPts

							/*let l = []

							if (_delegate.campaign.maxPts > 0)
								l.push(qsTr("%1 pt").arg(Number(Math.round(_delegate.campaign.maxPts * _delegate.campaign.progress).toLocaleString())))

							if (_delegate.campaign.resultXP > 0)
								l.push(qsTr("%1 XP").arg(Number(_delegate.campaign.resultXP).toLocaleString()))

							if (_delegate.campaign.resultGrade)
								l.push(_delegate.campaign.resultGrade.shortname)

							return l.join(" / ")*/
						}
						color: Qaterial.Style.accentColor
					}

					onClicked: _expandable.expanded = !_expandable.expanded

					/*onClicked: Client.stackPushPage("PageStudentCampaign.qml", {
														user: Client.server ? Client.server.user : null,
														campaign: campaign,
														studentMapHandler: control.mapHandler,
														withResult: true,
														title: group ? group.name : ""
													})*/
				}


				delegate: Item {
					width: _expandable.width
					height: _passItem.height + 30 * Qaterial.Style.pixelSizeRatio

					StudentPass {
						id: _passItem

						pass: _expandable.pass
						width: Math.min(parent.width - 12 * Qaterial.Style.pixelSizeRatio, Qaterial.Style.maxContainerSize * 0.8)

						showPlaceholders: _expandable.showPlaceholder

						anchors.centerIn: parent
					}
				}


				Component.onCompleted: {
					if (pass && pass.isActive)
						_expandable.expanded = true
				}

			}


		}

	}


}
