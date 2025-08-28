const std = @import("std");
const builtin = @import("builtin");

const SourceFile = struct {
    name: []const u8,
    directory: []const u8,
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const modargparse = b.dependency("argparse", .{});

    const files: []const SourceFile = comptime &.{
        .{ .name = "main.cpp", .directory = "src/" },
        .{ .name = "directory_validator.cpp", .directory = "src/" },
        .{ .name = "Parser.cpp", .directory = "src/" },
    };

    const modprofile = b.addModule("profile", .{
        .target = target,
        .optimize = optimize,
        .link_libcpp = true,
        .link_libc = false,
    });

    inline for (files) |file| {
        modprofile.addCSourceFile(.{
            .file = b.path(file.directory ++ file.name),
            .language = .cpp,
            .flags = &.{
                "-MJ",
                file.name ++ ".json.tmp",
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

    const modcompiledb = b.createModule(.{
        .optimize = optimize,
        .target = b.resolveTargetQuery(
            .fromTarget(&builtin.target), // must be native
        ),
        .root_source_file = b.path("compiledb/main.zig"),
    });
    const compiledb = b.addExecutable(.{
        .name = "compiledb",
        .root_module = modcompiledb,
    });
    const cleanup_step = b.step(
        "compiledb",
        "Cleanup json fragments and form them into compile_commands.json",
    );
    const cleanup_command = b.addRunArtifact(compiledb);
    cleanup_command.addArgs(
        &.{
            std.process.getCwdAlloc(b.allocator) catch unreachable,
        },
    );
    cleanup_step.dependOn(&cleanup_command.step);
    b.getInstallStep().dependOn(cleanup_step);
}
