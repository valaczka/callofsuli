import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "JScript.js" as JS

QPage {
	id: control

	property OfflineClientEngine engine: null

	header: null

	StudentOfflineDashboard {
		id: _dashboard
		anchors.fill: parent
		engine: control.engine
		visible: engine && engine.engineState == OfflineClientEngine.EngineActive
	}


	Qaterial.LabelHeadline6 {
		visible: !control.engine || control.engine.engineState != OfflineClientEngine.EngineActive
		anchors.centerIn: parent
		text: qsTr("Hiba történt")
		color: Qaterial.Style.errorColor
	}


	StackView.onActivated: { }

	StackView.onRemoved: {
		if (engine)
			engine.unloadPage()

		if (Client.httpConnection)
			Client.httpConnection.close()
	}
}

