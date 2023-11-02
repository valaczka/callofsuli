import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "./JScript.js" as JS

Item
{
	id: control

	property TeacherGroup group: null
	property TeacherMapHandler mapHandler: null


	QScrollable {
		id: _scrollable

		anchors.fill: parent
		topPadding: 0
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		refreshEnabled: true

		onRefreshRequest: _view.offsetModel.reload()

		QOffsetListView {
			id: _view

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize * 1.2)
			height: Math.max(contentHeight, parent.height)
			anchors.horizontalCenter: parent.horizontalCenter

			refreshEnabled: false
			refreshProgressVisible: false

			//visible: user

			delegate: Qaterial.LoaderItemDelegate {
				id: _delegate
				width: _view.width

				text: Array(model.familyName, model.givenName).join(" ")

				secondaryText: {
					let t = model.readableMap+" | "+model.readableMission+" [%1%2]".arg(model.level).arg(model.deathmatch ? qsTr(" SD") : "")
						t += " @ "+JS.readableTimestamp(model.timestamp)
					switch (model.mode) {
					case GameMap.Action:
						t += qsTr(" [akció]")
						break
					case GameMap.Lite:
						t += qsTr(" [feladatmegoldás]")
						break
					case GameMap.Quiz:
						t += qsTr(" [kvíz]")
						break
					case GameMap.Test:
						t += qsTr(" [teszt]")
						break
					case GameMap.Exam:
						t += qsTr(" [dolgozat]")
						break
					default:
						break
					}

					t += " "+Client.Utils.formatMSecs(model.duration)

					return t
				}



				enabled: model.success

				Component {
					id: _cmpIcon
					Qaterial.RoundColorIcon
					{
						source: model.success ? Qaterial.Icons.checkBold : Qaterial.Icons.close
						color: model.success ? Qaterial.Colors.green400 : Qaterial.Style.disabledTextColor()
						iconSize: Qaterial.Style.delegate.iconWidth
						enabled: model.success

						fill: true
						width: roundIcon ? roundSize : iconSize
						height: roundIcon ? roundSize : iconSize
					}
				}

				Component {
					id: _cmpMedal
					MedalImage {
						width: 40 * Qaterial.Style.pixelSizeRatio
						height: 40 * Qaterial.Style.pixelSizeRatio
						deathmatch: model.deathmatch
						image: model.medal
						level: model.level
					}
				}

				leftSourceComponent: model.success && model.medal !== "" ? _cmpMedal : _cmpIcon

				rightSourceComponent: Qaterial.LabelSubtitle1 {
					anchors.verticalCenter: parent.verticalCenter
					text: qsTr("%1 XP").arg(Number(model.xp).toLocaleString())
					color: Qaterial.Style.accentColor
				}

			}


			offsetModel: StudentCampaignOffsetModelImpl {
				mapList: mapHandler ? mapHandler.mapList : null
				groupid: control.group ? control.group.groupid : -1
			}

		}

		flickable.onAtYEndChanged: {
			if (flickable.atYEnd && flickable.moving)
				_view.offsetModel.fetch()
		}
	}


	StackView.onActivated: _view.offsetModel.reload()

	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) _view.offsetModel.reload()
}