import qbs
import qbs.TextFile

### Quaterial CMAKE_PREFIX_PATH=/usr/lib/mxe/usr/x86_64-w64-mingw32.shared/qt5/lib/.. cmake .. -DQATERIAL_ICONS="*.svg_out" -DQATERIAL_ENABLE_ROBOTO=OFF -DQATERIAL_ENABLE_ROBOTOMONO=OFF

Project {
	name: "box2d";

property int myversion: 4

	StaticLibrary {
		name: "box2d"
		files: ["Box2D/*.h", "Box2D/*/*.h", "Box2D/*/*/*.h", "Box2D/*.cpp", "Box2D/*/*.cpp", "Box2D/*/*/*.cpp"]
		Depends { name: "cpp" }
		cpp.includePaths: ["Box2D", "."]

		Group { qbs.install: true; fileTagsFilter: product.type;}
	}

	DynamicLibrary {
		name: "box2d_lib"
		files: ["*.h", "*.cpp"]
		Depends { name: "cpp" }
		Depends { name: "box2d" }
		Depends { name: "Qt"; submodules: ["core", "qml", "quick"]; }
		cpp.includePaths: ["Box2D", "."]

cpp.defines: [
			"MYname=" + project.myversion
		]

		Group { qbs.install: true; fileTagsFilter: product.type;}

		Export {
			Depends { name: "cpp" }
			cpp.includePaths: "."
		}
	}

Product {
	name: "loremipsum"
	type: "txt_output"

property string mypath: Qt.core.libPath

		Depends { name: "Qt"; submodules: ["core"]; }

//        Group { qbs.install: true; fileTagsFilter: product.type;}

	Group {
		name: "lorem_ipsum"
		files: "lorem_ipsum.txt"
		fileTags: "txt_input"
	}

	Rule {
		multiplex: false
	alwaysRun: true
		inputs: ["txt_input"]
		Artifact {
			filePath: input.fileName + ".out"
			fileTags: ["txt_output"]
		}
		prepare: {
			var cmd = new JavaScriptCommand();
			cmd.description = "generating" + output.fileName + " from " + input.fileName;
			cmd.highlight = "codegen";
			cmd.sourceCode = function() {
				var file = new TextFile(input.filePath);
				var content = file.readAll();
				file.close()
				content = content.toUpperCase();
//                file = new TextFile(output.filePath, TextFile.WriteOnly);
//                file.write(content);
//                file.close();

		var n = Number(content);
		project.myversion = n+1;

		console.info("### "+product.mypath);

		console.info("*** "+n+" "+(n+1)+" --- "+project.myversion);
			}
			return [cmd];
		}
	}
}

}
