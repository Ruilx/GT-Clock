import qbs

Product {
    name: "fonts"
    Depends {name: "cpp"}

    cpp.includePaths: ["dist"]

    Export {
        Depends {name: "cpp"}
        Parameters {cpp.linkWholeArchive: true}
        cpp.includePaths: product.cpp.includePaths
    }

    Group {
        name: "Dist"
        files: ["dist/*.h"]
    }

    Group {
        name: "Files"
        files: ["*"]
    }
}
