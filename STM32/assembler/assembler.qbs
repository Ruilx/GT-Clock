import qbs

CppApplication {
    consoleApplication: true

    cpp.includePaths: ["../logic"]

    files: [
        "../logic/logic_layer_program_data.h",
        "main.cpp",
        "program_data.h",
        "program_inc.h",
    ]

    Group {     // Properties for the produced executable
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
