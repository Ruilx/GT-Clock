import qbs

StaticLibrary {
    name: "core"
    Depends {name: "CMSIS"}

    cpp.defines: {
        var flags = ["HWVER=" + project.hwver, "SWVER=" + project.swver];
        if (project.bootloader)
            flags.push("BOOTLOADER");
        return flags;
    }

    cpp.driverFlags: {
        var flags = [];
        //if (project.bootloader)
            flags.push("--specs=nano.specs");
        return flags;
    }

    cpp.commonCompilerFlags: [
        "-Wno-unused-parameter",
        "-Wno-unused-function",
        "-Wno-unused-variable",
    ]
    cpp.staticLibraries: ["m"]
    cpp.includePaths: ["common", "."]

    Properties {
        condition: qbs.buildVariant == "debug"
        cpp.defines: outer.concat(["DEBUG=5"])
        cpp.optimization: "none"
    }

    Properties {
        condition: qbs.buildVariant == "release"
        cpp.defines: outer.concat(["DEBUG=1"])
        cpp.optimization: "fast"
    }

    Properties {
        condition: project.bootloader
        cpp.defines: outer.concat(["BOOTLOADER"])
    }

    Export {
        Depends {name: "cpp"}
        Depends {name: "CMSIS"}
        Parameters {cpp.linkWholeArchive: true}
        cpp.includePaths: product.cpp.includePaths
        cpp.defines: product.cpp.defines
        cpp.driverFlags: product.cpp.driverFlags
        cpp.optimization: product.cpp.optimization
    }

    Group {
        name: "Memory test"
        condition: false
        cpp.optimization: "none"
        cpp.commonCompilerFlags: ["-ffast-math", "-O3"]
        files: ["system/mem_test.c"]
    }

    Group {
        name: "Critical"
        cpp.optimization: "none"
        cpp.commonCompilerFlags: ["-O3"]
        files: [
            "peripheral/matrix.c",
            "peripheral/matrix.h",
        ]
    }

    Group {
        name: "Fonts"
        files: [
            "fonts/*"
        ]
    }

    files: [
        "common/common.c",
        "common/common.h",
        "common/escape.h",
        "common/list.h",
        "common/macros.h",
        "common/debug.h",
        "common/device.h",
        "logic/logic_i2c_fb.c",
        "logic/logic_i2c_func.c",
        "logic/logic_i2c_layers.c",
        "logic/logic_layer_breath.c",
        "logic/logic_layer_const.c",
        "logic/logic_layer_gamma.c",
        "logic/logic_layer_sine.c",
        "logic/logic_layer_string.c",
        "logic/logic_layers.c",
        "logic/logic_layers.h",
        "peripheral/i2c_slave.c",
        "peripheral/i2c_slave.h",
        "system/syscall.c",
        "system/system.c",
        "system/system.h",
        "system/system.s",
        "system/clocks.h",
        "system/clocks.c",
        "system/systick.h",
        "system/systick.c",
        "system/irq.h",
        "system/dma.txt",
    ]
}
