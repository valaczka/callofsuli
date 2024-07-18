import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Qaterial.Card {
	id: control

	property RpgUserWallet wallet: null
	property bool resourceLoaded: false
	property int availableCurrency: 0

	signal buyRequest()


	width: ListView.view ? ListView.view.implicitHeight * (180/300) : 50
	height: ListView.view ? ListView.view.implicitHeight : 50

	outlined: true


	/*Binding {
		target: control
		property: "backgroundColor"
		value: Qaterial.Colors.green600
		when: wallet && wallet.available
	}*/

	borderColor: "transparent"


	elevation: Qaterial.Style.card.activeElevation


	/*readonly property color textColor: selected ? Qaterial.Colors.cyan300 :
												  locked ? Qaterial.Style.colorTheme.disabledText :
														   Qaterial.Style.primaryTextColor()*/

	readonly property color textColor: Qaterial.Colors.white

	contentItem: Item {
		width: parent.width
		height: parent.height

		layer.enabled: true
		layer.effect: OpacityMask
		{
			maskSource: Rectangle
			{
				width: control.width
				height: control.height
				radius: control.radius
			}
		}

		Item {
			id: _imgHolder
			width: parent.width
			height: parent.height*0.6

			anchors.top: parent.top
			anchors.left: parent.left

			Image
			{
				id: _image
				source: wallet ? wallet.image : ""
				fillMode: Image.PreserveAspectFit
				width: Math.min(parent.width, sourceSize.width)
				height: Math.min(parent.height, sourceSize.height)
				anchors.centerIn: parent
				horizontalAlignment: Image.AlignHCenter
				verticalAlignment: Image.AlignVCenter
			}

			QTagList {
				id: _tagFree

				anchors.top: _image.top
				anchors.left: _image.left
				anchors.leftMargin: Qaterial.Style.card.horizontalPadding
				anchors.topMargin: Qaterial.Style.card.horizontalPadding

				model: [ {
						"text": qsTr("Free"),
						"color": Qaterial.Colors.green700
					}]

				visible: wallet && wallet.market.cost === 0 && wallet.market.rank === 0
			}
		}


		Qaterial.Label
		{
			id: _label

			text: wallet ? wallet.readableName : ""

			font: Qaterial.Style.textTheme.body1

			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: _imgHolder.bottom
			anchors.leftMargin: Qaterial.Style.card.horizontalPadding
			anchors.rightMargin: Qaterial.Style.card.horizontalPadding
			anchors.bottomMargin: Qaterial.Style.card.verticalPadding

			lineHeight: 0.8
			wrapMode: Text.Wrap

			color: control.textColor

			clip: true

			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
		}



		Glow {
			anchors.fill: _label
			source: _label
			//visible: !locked && selected
			color: "black"
			radius: 1
			spread: 0.9
			samples: 5
		}



		Rectangle {
			id: _costItem

			property int size: Math.min(120 * Qaterial.Style.pixelSizeRatio, parent.width*0.75)

			visible: wallet && wallet.market.cost > 0 && (!wallet.available || wallet.buyable)

			color: Qaterial.Colors.orange800

			width: size
			height: _costLabel.height + 4 * Qaterial.Style.pixelSizeRatio

			anchors.horizontalCenter: parent.left
			anchors.verticalCenter: parent.top
			anchors.horizontalCenterOffset: size*0.25
			anchors.verticalCenterOffset: size*0.25

			Row {
				id: _costLabel
				anchors.horizontalCenter: parent.horizontalCenter
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenterOffset: 5 * Qaterial.Style.pixelSizeRatio

				spacing: 0

				Qaterial.Label {
					id: _lbl
					anchors.verticalCenter: parent.verticalCenter

					font.family: Qaterial.Style.textTheme.body2.family
					font.pixelSize: Qaterial.Style.textTheme.body2.pixelSize
					font.weight: Font.Bold

					color: Qaterial.Colors.black

					text: wallet ? wallet.market.cost : 0
				}

				Image {
					source: resourceLoaded ? "qrc:/rpg/coin/coin.png" : ""
					fillMode: Image.PreserveAspectFit
					height: _lbl.height - 4
					width: _lbl.width - 4

					anchors.verticalCenter: parent.verticalCenter
				}
			}

			rotation: -45
			transformOrigin: Item.Center
		}


		Rectangle {
			color: "black"
			opacity: 0.8
			width: _amountLabel.width + 2*Qaterial.Style.card.horizontalPadding
			height: _amountLabel.height + 2*Qaterial.Style.card.horizontalPadding
			anchors.centerIn: _amountLabel

			visible: _amountLabel.visible && !_amountLabel._isCentered
		}

		Column {
			id: _amountLabel

			readonly property bool _isCentered: !_btnBuy.visible && !_expLabel.visible

			anchors.top: parent.top
			anchors.right: parent.right
			anchors.rightMargin: Qaterial.Style.card.horizontalPadding
			anchors.topMargin: Qaterial.Style.card.horizontalPadding

			states: State {
				when: _amountLabel._isCentered

				AnchorChanges {
					target: _amountLabel
					anchors.top: undefined
					anchors.right: undefined
					anchors.horizontalCenter: _btnBuy.horizontalCenter
					anchors.verticalCenter: _btnBuy.verticalCenter
				}
			}

			property color textColor: Qaterial.Colors.green300

			visible: wallet && wallet.available

			spacing: 5 * Qaterial.Style.pixelSizeRatio

			Qaterial.Icon {
				icon: Qaterial.Icons.checkCircle
				color: _amountLabel.textColor
				size: 35 * Qaterial.Style.pixelSizeRatio
				anchors.horizontalCenter: parent.horizontalCenter
			}

			Qaterial.Label {
				anchors.horizontalCenter: parent.horizontalCenter
				text: wallet && wallet.amount > 1 ? "×"+wallet.amount : ""
				color: _amountLabel.textColor
				font.family: Qaterial.Style.textTheme.subtitle1.family
				font.pixelSize: Qaterial.Style.textTheme.subtitle1.pixelSize
				font.weight: Font.Bold
			}
		}


		Row {
			id: _rankLabel

			anchors.horizontalCenter: parent.horizontalCenter
			anchors.top: _imgHolder.bottom
			anchors.topMargin: Qaterial.Style.card.verticalPadding

			visible: wallet && wallet.market.rank > 0

			spacing: 3 * Qaterial.Style.pixelSizeRatio

			UserImage {
				anchors.verticalCenter: parent.verticalCenter
				width: _rankName.height
				height: _rankName.height

				userData: {
					"rank": {
						"level": (wallet ? wallet.rank.level : -1),
						"sublevel": (wallet ? wallet.rank.sublevel : -1)
					}
				}
			}

			Qaterial.LabelHint1 {
				id: _rankName
				anchors.verticalCenter: parent.verticalCenter
				text: wallet ? wallet.rank.name : "_placeholder_"
				color: Qaterial.Style.iconColor()
			}

		}

		Row {
			id: _expLabel

			anchors.centerIn: _btnBuy

			visible: wallet && new Date(wallet.expiry).getTime() > 0 && wallet.available

			spacing: 5 * Qaterial.Style.pixelSizeRatio

			Qaterial.Icon {
				icon: Qaterial.Icons.clockAlertOutline
				color: _amountLabel.textColor
				size: 20 * Qaterial.Style.pixelSizeRatio
				anchors.verticalCenter: parent.verticalCenter
			}

			Qaterial.LabelHint1 {
				anchors.verticalCenter: parent.verticalCenter
				text: wallet ? JS.readableTimestampMin(wallet.expiry) : ""
				color: _amountLabel.textColor
			}

		}

		QButton {
			id: _btnBuy

			anchors.horizontalCenter: parent.horizontalCenter
			y: _imgHolder.height +
			   (parent.height-_imgHolder.height-height)/2

			text: qsTr("Vásárlás")
			icon.source: enabled ? Qaterial.Icons.cart : Qaterial.Icons.lockOutline
			enabled: wallet && wallet.buyable && wallet.market.cost <= availableCurrency

			visible: wallet && (!wallet.available || wallet.buyable)

			onClicked: control.buyRequest()
		}


		Qaterial.Label {
			anchors.bottom: parent.bottom
			anchors.bottomMargin: Qaterial.Style.card.verticalPadding
			anchors.horizontalCenter: parent.horizontalCenter

			font.family: Qaterial.Style.textTheme.body2.family
			font.pixelSize: Qaterial.Style.textTheme.body2.pixelSize
			font.weight: Font.Bold

			text: wallet && wallet.market.amount > 1 ? qsTr("%1 db").arg(wallet.market.amount) : ""
			color: Qaterial.Style.accentColor
		}

		Row {
			id: _expInfo

			anchors.bottom: parent.bottom
			anchors.right: parent.right
			anchors.rightMargin: Qaterial.Style.card.horizontalPadding
			anchors.bottomMargin: Qaterial.Style.card.verticalPadding

			visible: wallet && wallet.market.exp > 0 && (!wallet.available || wallet.buyable)

			spacing: 5 * Qaterial.Style.pixelSizeRatio

			Qaterial.Icon {
				icon: Qaterial.Icons.clockOutline
				color: Qaterial.Style.iconColor()
				size: 20 * Qaterial.Style.pixelSizeRatio
				anchors.verticalCenter: parent.verticalCenter
			}

			Qaterial.LabelHint1 {
				anchors.verticalCenter: parent.verticalCenter
				text: wallet ? Client.Utils.formatMSecs(wallet.market.exp*1000) : ""
				color: Qaterial.Style.iconColor()
			}
		}




		Rectangle {
			anchors.fill: parent
			color: "transparent"
			radius: control.radius
			border.width: wallet && wallet.available ? 3 : 1
			border.color: wallet && wallet.available ? Qaterial.Colors.green400 :
													   control.enabled ? Qaterial.Style.dividersColor() :
																		 Qaterial.Style.disabledDividersColor()

		}

	}


}
