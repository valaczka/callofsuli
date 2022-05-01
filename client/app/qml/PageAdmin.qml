import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	property string page: ""

	activity: ServerSettings {
		id: serverSettings
	}

	Component {
		id: componentSettings
		AdminServerSettings { }
	}

	Component {
		id: componentClasses
		AdminClassList { }
	}


	Component.onCompleted: {
		if (page == "ServerSettings")
			replaceContent(componentSettings)
		else if (page == "ClassList")
			replaceContent(componentClasses)
		else
			replaceContent()
	}

}
