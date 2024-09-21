import QtQuick
import QtQuick.Controls
import CallOfSuli
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Rectangle {
	id: root

	property ConquestGame game: null

	radius: 5

	implicitHeight: _col.implicitHeight
	implicitWidth: 600 * Qaterial.Style.pixelSizeRatio

	color: Client.Utils.colorSetAlpha(Qaterial.Colors.black, 0.85)

	Column {
		id: _col

		padding: 30 * Qaterial.Style.pixelSizeRatio

		anchors.fill: parent
		spacing: 5


		Repeater {
			model: SortFilterProxyModel {
				sourceModel: game ? game.playersModel : null

				sorters: [
					RoleSorter {
						roleName: "xp"
						sortOrder: Qt.DescendingOrder
						priority: 2
					},
					RoleSorter {
						roleName: "playerId"
						priority: 1
					}
				]

			}


			delegate: Row {
				spacing: 0

				Image {
					id: _img
					fillMode: Image.PreserveAspectFit
					width: 50 * Qaterial.Style.pixelSizeRatio
					height: 50 * Qaterial.Style.pixelSizeRatio
					anchors.verticalCenter: parent.verticalCenter
					source: "qrc:/character/%1/thumbnail.png".arg(model.character)

					Qaterial.Icon {
						anchors.right: parent.right
						anchors.bottom: parent.bottom
						anchors.margins: 3

						icon: model.success ? Qaterial.Icons.checkCircle : Qaterial.Icons.closeCircle
						color: model.success ? Qaterial.Colors.green400 : Qaterial.Colors.red400
						size: Qaterial.Style.smallIcon
					}
				}

				Qaterial.LabelSubtitle1 {
					id: _labelName

					anchors.verticalCenter: parent.verticalCenter

					width: _col.width -2*_col.padding -_img.width -_hpIcon.width -_hpLabel.width -_labelXP.width
					elide: width < implicitWidth ? Text.ElideRight : Text.ElideNone
					text: model.fullNickName
					leftPadding: 10 * Qaterial.Style.pixelSizeRatio
					rightPadding: 5 * Qaterial.Style.pixelSizeRatio
					color: model.success ? Qaterial.Style.primaryTextColor() : Qaterial.Style.secondaryTextColor()
				}

				Qaterial.LabelSubtitle1 {
					id: _labelXP
					anchors.verticalCenter: parent.verticalCenter
					color: model.success ? Qaterial.Style.accentColor : Qaterial.Style.secondaryTextColor()

					property int xp: model.xp

					text: "%1 XP".arg(xp)

					Behavior on xp {
						NumberAnimation { duration: 650; easing.type: Easing.OutQuad }
					}

					rightPadding: 10 * Qaterial.Style.pixelSizeRatio
				}

				Qaterial.Icon {
					id: _hpIcon
					anchors.verticalCenter: parent.verticalCenter
					icon: Qaterial.Icons.heartPulse
					color: model.success ? Qaterial.Colors.red400 : Qaterial.Colors.red800
					width: Qaterial.Style.smallIcon
					height: Qaterial.Style.smallIcon
				}

				Qaterial.LabelBody2 {
					id: _hpLabel
					anchors.verticalCenter: parent.verticalCenter
					text: model.hp
					color: model.success ? Qaterial.Colors.red400 : Qaterial.Colors.red800
					leftPadding: 3 * Qaterial.Style.pixelSizeRatio
					//rightPadding: 5 * Qaterial.Style.pixelSizeRatio
				}

			}
		}

	}

}
