import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: root

	property TeacherGroup group: null
	property User user: null
	property TeacherMapHandler mapHandler: null

	property bool _firstRun: true

	title: user ? user.fullName : ""
	subtitle: group ? group.fullName : ""

	stackPopFunction: function() {
		if (!_scrollable.flickable.atYBeginning) {
			_scrollable.flickable.contentY = 0
			return false
		}

		return true
	}



	QScrollable {
		id: _scrollable
		anchors.fill: parent
		spacing: 15
		contentCentered: false

		visible: user

		refreshEnabled: true

		onRefreshRequest: _view.offsetModel.reload()


		Qaterial.LabelHeadline5 {
			id: _fullName
			anchors.horizontalCenter: parent.horizontalCenter

			topPadding: 15
			text: user ? user.fullName : ""
		}

		QOffsetListView {
			id: _view

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			refreshEnabled: false
			refreshProgressVisible: false

			visible: user

			delegate: Qaterial.LoaderItemDelegate {
				id: _delegate
				width: _view.width

				text: model.readableMap+" | "+model.readableMission+" [%1%2]".arg(model.level).arg(model.deathmatch ? qsTr(" SD") : "")
				secondaryText: {
					let t = JS.readableTimestamp(model.timestamp)
					switch (model.mode) {
					case GameMap.Action:
						t += qsTr(" [akció]")
						break
					case GameMap.Rpg:
						t += qsTr(" [RPG]")
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
					case GameMap.Conquest:
						t += qsTr(" [multiplayer]")
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
				username: root.user ? root.user.username : ""
				groupid: root.group ? root.group.groupid : -1
			}

		}

		flickable.onAtYEndChanged: {
			if (flickable.atYEnd && flickable.moving)
				_view.offsetModel.fetch()
		}
	}


	StackView.onActivated: _view.offsetModel.reload()
}


