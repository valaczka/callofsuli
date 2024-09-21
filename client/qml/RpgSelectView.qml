import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

ListView {
	id: view

	property string selected: ""
	property var selectedList: []

	orientation: ListView.Horizontal

	implicitHeight: 110*Qaterial.Style.pixelSizeRatio

	spacing: 5 * Qaterial.Style.pixelSizeRatio

	clip: true

	snapMode: ListView.SnapToItem

	header: Item {
		width: Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
		height: view.height
	}

	footer: Item {
		width: Math.max(Client.safeMarginRight, Qaterial.Style.card.horizontalPadding)
		height: view.height
	}


	function selectOne(_wallet) {
		selectedList = []
		if (_wallet && _wallet.available)
			selected = _wallet.market.name
		else
			selected = ""
	}


	function selectMore(_wList) {
		selected = ""

		let l = selectedList

		for (let i=0; i<_wList.length; ++i) {
			let w = _wList[i]
			let idx = selectedList.indexOf(w.market.name)

			if (idx > -1) {
				if (!w.available)
					l.splice(idx, 1)
			} else if (w.available)
				l.push(w.market.name)
		}

		selectedList = l
	}


	function unselectMore(_wList) {
		selected = ""

		let l = selectedList

		for (let i=0; i<_wList.length; ++i) {
			let w = _wList[i]
			let idx = selectedList.indexOf(w.market.name)

			if (i > -1)
				l.splice(idx, 1)
		}

		selectedList = l
	}

}
