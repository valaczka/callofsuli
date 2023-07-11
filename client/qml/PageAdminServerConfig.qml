import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	title: qsTr("Konfiguráció")
	subtitle: Client.server ? Client.server.serverName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: Row {
		visible: _hasModified
		Qaterial.AppBarButton
		{
			icon.source: Qaterial.Icons.archiveCancel
			ToolTip.text: qsTr("Mindet elvet")
		}
		Qaterial.AppBarButton
		{
			icon.source: Qaterial.Icons.contentSaveAll
			ToolTip.text: qsTr("Mindet ment")
		}
	}


	property var _items: []
	property bool _hasModified: false
	property var _data: ({})


	on_DataChanged: loadData()

	QScrollable {
		anchors.fill: parent

		QFormColumn {
			id: _form

			spacing: 3

			QExpandableHeader {
				width: parent.width
				text: qsTr("Szerver és authentikáció")
				icon: Qaterial.Icons.serverOutline
				button.visible: false
				topPadding: 10 * Qaterial.Style.pixelSizeRatio
			}

			AdminServerConfigTextField {
				field: "serverName"
				title: qsTr("A szerver neve")
			}

			AdminServerConfigCombo {
				field: "registrationEnabled"
				label: qsTr("Regisztráció:")
			}

			AdminServerConfigCombo {
				field: "oauth2RegistrationForced"
				label: qsTr("OAuth2 regisztráció kényszerítése:")
			}

			AdminServerConfigTextField {
				field: "oauth2DomainList"
				title: qsTr("Domain korlátozások:")
				helperText: qsTr("OAuth2 regisztrációnál lehetséges domainek vesszővel elválasztva")
			}




			QExpandableHeader {
				width: parent.width
				text: qsTr("Felhasználói adatok")
				icon: Qaterial.Icons.accountMultipleOutline
				button.visible: false
				topPadding: 30 * Qaterial.Style.pixelSizeRatio
			}

			AdminServerConfigCombo {
				field: "nameUpdateEnabled"
				label: qsTr("Vezeték- és keresznevek módosítása:")
			}

			AdminServerConfigCombo {
				field: "oauth2NameUpdate"
				label: qsTr("Vezeték- és keresznevek felülírása OAuth2 bejelentkezéskor:")
			}

			AdminServerConfigCombo {
				field: "pictureUpdateEnabled"
				label: qsTr("Profilképek módosítása:")
			}

		}

	}




	function checkModified() {
		let m = false

		for (let i=0; i<_items.length; ++i) {
			if (_items[i].active) {
				m = true
				break
			}
		}

		_hasModified = m
	}


	function loadData() {
		for (let i=0; i<_items.length; ++i) {
			let f = _items[i].field
			if (f === "serverName")
				_items[i].set(Client.server.serverName)
			else if (f !== "" && _data[f] !== undefined) {
				_items[i].set(_data[f])
			}
		}
	}


	Component.onCompleted: _data = Client.server ? Client.server.config : null

}
