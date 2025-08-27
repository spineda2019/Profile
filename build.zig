const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const modargparse = b.dependency("argparse", .{});

    const files: []const []const u8 = comptime &.{
        "src/main.cpp",
        "src/directory_validator.cpp",
        "src/Parser.cpp",
    };

    const modprofile = b.addModule("profile", .{
        .target = target,
        .optimize = optimize,
        .link_libcpp = true,
        .link_libc = false,
    });

    inline for (files) |file| {
        modprofile.addCSourceFile(.{
            .file = b.path(file),
            .language = .cpp,
            .flags = &.{
                "-MJ",
                file ++ ".json.tmp",
                "-std=c++20",
                // "-Wall",
                // "-Wextra",
                // "-Wpedantic",
                // "-Wshadow",
                // "-Wconversion",
                // "-Werror",
            },
        });
    }
    modprofile.addIncludePath(b.path("src/include/"));
    modprofile.addSystemIncludePath(modargparse.path("include/"));

    const exeprofile = b.addExecutable(.{
        .name = "profile",
        .root_module = modprofile,
    });

    b.installArtifact(exeprofile);

    const run_step = b.step("run", "Run the app");
    const run_cmd = b.addRunArtifact(exeprofile);
    run_step.dependOn(&run_cmd.step);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
}
