import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: root

	property real mapScale: width > height ? Math.min(1.0, width/2600) : Math.min(1.0, height/1200)
	property ConquestGame game: null
	property string world: ""

	Image {
		source: game && world != "" ? "qrc:/conquest/"+world+"/bg.png" : ""
		width: 2600*mapScale
		height: 1200*mapScale
		fillMode: Image.PreserveAspectFit
	}

	Repeater {
		model: game && world != "" ? 16 : null
		delegate: ConquestLand {
			world: root.world
			stateId: index+1
			mapScale: root.mapScale
			game: root.game
		}
	}


	Image {
		source: game && world != "" ? "qrc:/conquest/"+world+"/over.png" : ""
		width: 2600*mapScale
		height: 1200*mapScale
		fillMode: Image.PreserveAspectFit
	}
}
