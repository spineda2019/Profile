const std = @import("std");

const CleanupError = error{
    bad_args,
};

const ParsedArgs = struct {
    help_requested: bool,
    path_to_clean: []const u8,
};

const usage = "Usage: compiledb <PATH>";

pub fn main() !void {
    var debug_allocator: std.heap.DebugAllocator(.{}) = .init;
    const allocator: std.mem.Allocator = debug_allocator.allocator();

    const args: [][:0]u8 = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    const parsed_args: ParsedArgs = try parseArgs(args);
    if (parsed_args.help_requested) {
        printHelp();
        return;
    }

    std.debug.print("Cleaning up: {s}\n", .{parsed_args.path_to_clean});

    var dir: std.fs.Dir = try std.fs.openDirAbsolute(
        parsed_args.path_to_clean,
        .{
            .iterate = true,
        },
    );
    defer dir.close();

    var iterator: std.fs.Dir.Iterator = dir.iterate();

    var fragments: std.array_list.Aligned([]const u8, null) = .empty;

    while (try iterator.next()) |entry| {
        if (entry.kind == .file and std.mem.endsWith(u8, entry.name, ".json.tmp")) {
            try fragments.append(allocator, entry.name);
        }
    }

    if (fragments.items.len == 0) {
        std.debug.print("No Json fragments found, exiting...\n", .{});
        return;
    } else {
        var compiledb_file: std.fs.File = try dir.createFile(
            "compile_commands.json",
            .{ .truncate = true },
        );
        defer compiledb_file.close();

        var write_buffer: [4096]u8 = .{0} ** 4096;
        var read_buffer: [4096]u8 = .{0} ** 4096;
        var writer: std.fs.File.Writer = compiledb_file.writer(&write_buffer);
        defer writer.end() catch unreachable;

        _ = try writer.interface.write("[");
        defer _ = writer.interface.write("]") catch unreachable;

        for (fragments.items) |fragment| {
            std.debug.print("Found: {s}\n", .{fragment});
            var fragment_file: std.fs.File = try dir.openFile(fragment, .{});
            var reader: std.fs.File.Reader = fragment_file.reader(&read_buffer);

            defer dir.deleteFile(fragment) catch unreachable;
            defer fragment_file.close();

            var buf: [512]u8 = .{0} ** 512;
            start: switch (try reader.interface.readSliceShort(&buf)) {
                buf.len => {
                    _ = try writer.interface.write(&buf);
                    continue :start try reader.interface.readSliceShort(&buf);
                },
                else => |num_read| {
                    _ = try writer.interface.write(buf[0..num_read]);
                },
            }
        }
    }
}

fn printHelp() void {
    std.debug.print("{s}\n", .{usage});
}

fn parseArgs(args: [][:0]u8) CleanupError!ParsedArgs {
    if (args.len != 2) {
        std.debug.print("{s}\n", .{usage});
        return CleanupError.bad_args;
    } else {
        var parsed_args: ParsedArgs = .{
            .path_to_clean = args[1],
            .help_requested = false,
        };

        for (args) |arg| {
            if (std.mem.eql(u8, "-h", arg)) {
                parsed_args.help_requested = true;
            }
        }

        return parsed_args;
    }
}
