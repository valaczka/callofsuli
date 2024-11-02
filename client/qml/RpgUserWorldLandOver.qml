import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Column {
	id: root

	property RpgWorldLandData landData: null
	property bool selected: false

	readonly property bool _selectable: landData && (
											landData.landState == RpgWorldLandData.LandSelectable ||
											landData.landState == RpgWorldLandData.LandAchieved)

	x: landData ? landData.posX + (landData.textX > 0 ? landData.textX : root.width/2) - width/2 : 0
	y: landData ? landData.posY + (landData.textY > 0 ? landData.textY : root.height/2) - height/2 : 0

	spacing: 5 * Qaterial.Style.pixelSizeRatio

	transformOrigin: Item.Center
	rotation: landData ? landData.rotate : 0

	Qaterial.Icon {
		color: Qaterial.Colors.gray600
		icon: Qaterial.Icons.lock
		size: 42  * Qaterial.Style.pixelSizeRatio
		visible: landData && landData.landState == RpgWorldLandData.LandLocked
		anchors.horizontalCenter: parent.horizontalCenter
	}

	Label {
		id: _label

		anchors.horizontalCenter: parent.horizontalCenter
		horizontalAlignment: Text.AlignHCenter

		font.family: Qaterial.Style.textTheme.headline5.family
		font.pixelSize: Qaterial.Style.textTheme.headline5.pixelSize
		font.weight: Font.Bold

		text: landData ? landData.name : ""
		color: selected ? Qaterial.Colors.amber200 :
						  _selectable ? Qaterial.Colors.cyan200 : Qaterial.Colors.gray600

		style: Text.Outline
		styleColor: Qaterial.Colors.black
	}
}

