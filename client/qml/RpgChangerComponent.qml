import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


ColumnLayout {
	id: root

	required property RpgChangerImpl changer
	required property string type

	property int modelIdx: -1

	property bool replaceMode: false

	readonly property color mainColor: type == "weapon" ?
										   Qaterial.Colors.red400 :
										   type == "defender" ?
											   Qaterial.Colors.green400 :
											   type == "utility" ?
												   Qaterial.Colors.amber400 :
												   Qaterial.Colors.white
	implicitWidth: 250


	ListModel {
		id: _model
	}

	RpgChangerTumbler {
		id: _tumbler

		Layout.fillHeight: true
		Layout.fillWidth: true

		model: _model
		visible: replaceMode
		currentIndex: modelIdx

		mainColor: root.mainColor
	}


	Qaterial.IconLabel {
		id: _content

		readonly property bool _hasNoUtility: type == "utility" && _model.count == 0

		visible: !replaceMode

		display: IconLabel.Display.TextUnderIcon

		font: Qaterial.Style.textTheme.body1
		icon.width: 32 * Qaterial.Style.pixelSizeRatio
		icon.height: 32 * Qaterial.Style.pixelSizeRatio
		icon.source: _hasNoUtility ? Qaterial.Icons.lockOutline
								   : modelIdx >= 0 ? _model.get(modelIdx).icon : ""

		text: _hasNoUtility ? qsTr("Unlock at power level 4") :
							  modelIdx >= 0 ? _model.get(modelIdx).description : ""


		Layout.fillHeight: true
		Layout.fillWidth: true

		color: _hasNoUtility ? Qaterial.Colors.gray500 : mainColor

		icon.color: type == "weapon" ? "transparent" : _content.color

		wrapMode: Text.Wrap
	}



	QButton {
		id: _btnChange

		visible: !replaceMode

		highlightedBgColor: root.mainColor
		highlightedTextColor: Qaterial.Colors.black
		highlighted: enabled


		readonly property bool hasItem: changer.player ?
											(type == "defender" ? changer.player.hasDefender :
																  type == "utility" ? changer.player.hasUtility :
																					  false) :
											false

		enabled: changer.player && modelIdx >= 0 &&
				 changer.player.mp >= _model.get(modelIdx).cost &&
				 !hasItem && !changer.isBlocked

		icon.source: enabled ? Qaterial.Icons.shimmer : Qaterial.Icons.lock
		text: modelIdx >= 0 ? qsTr("%1 MP").arg(_model.get(modelIdx).cost) : "---"

		leftPadding: 18 * Qaterial.Style.pixelSizeRatio
		rightPadding: 18 * Qaterial.Style.pixelSizeRatio
		topPadding: 20 * Qaterial.Style.pixelSizeRatio
		bottomPadding: 20 * Qaterial.Style.pixelSizeRatio

		onClicked: type == "defender" ?
					   changer.useDefender() :
					   type == "utility" ?
						   changer.useUtility() :
						   type == "weapon" ?
							   changer.useWeapon(true) :
							   console.warn("Invalid type", type)

		Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
	}


	QButton {
		id: _btnReplace

		enabled: changer.replaceEnabled  && type != "weapon" && _model.count > 1 && !_btnChange.hasItem

		visible: !replaceMode

		opacity: type == "weapon" ? 0.0 : 1.0

		icon.source: Qaterial.Icons.refresh
		text: qsTr("Csere")

		outlined: false
		flat: true

		textColor: Qaterial.Style.iconColor()

		onClicked: replaceMode = true

		Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
	}


	QButton {
		id: _btnReplaceOk

		visible: replaceMode

		icon.source: Qaterial.Icons.checkBold
		text: qsTr("Csere")

		bgColor: Qaterial.Colors.green600
		textColor: Qaterial.Colors.white

		onClicked: {
			modelIdx = _tumbler.currentIndex
			changer.set(type, _model.get(modelIdx).key)

			replaceMode = false
		}

		Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
	}



	function reset() {
		replaceMode = false

		if (type == "weapon") {
			modelIdx = _model.count-1
			return
		}

		let t = -1

		if (type == "defender")
			t = changer.currentDefender()
		else if (type == "utility")
			t = changer.currentUtility()
		else {
			modelIdx = -1
			console.error("Invalid type", type)
			return
		}

		for (let idx=0; idx<_model.count; ++idx) {
			if (_model.get(idx).key === t) {
				modelIdx = idx
				return
			}
		}

		if (_model.count > 0)
			modelIdx = 0
		else
			modelIdx = -1

		if (modelIdx != -1)
			_tumbler.positionViewAtIndex(modelIdx, Tumbler.Center)
	}


	function reload() {
		if (!changer)
			return

		let model = []

		if (type == "defender")
			model = changer.availableDefenders
		else if (type == "utility")
			model = changer.availableUtilites
		else if (type == "weapon")
			model = [ changer.availableWeapon() ]
		else {
			console.error("Invalid type", type)
			return
		}

		_model.clear()

		for (let i=0; i<model.length; ++i)
			_model.append(model[i])

		reset()
	}


	Connections {
		target: changer

		function onPlayerReloaded() {
			reload()
		}
	}

	onTypeChanged: reload()

	Component.onCompleted: reload()
}
