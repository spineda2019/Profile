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

    var dir: std.fs.Dir = try std.fs.cwd().openDir(
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

        var write_buffer: [4096]u8 = undefined;
        var read_buffer: [4096]u8 = undefined;

        var file_writer: std.fs.File.Writer = compiledb_file.writerStreaming(
            &write_buffer,
        );
        const writer = &file_writer.interface;
        defer writer.flush() catch unreachable;

        _ = try writer.write("[\n");
        defer _ = writer.write("]") catch unreachable;

        for (fragments.items) |fragment| {
            std.debug.print("Found: {s}\n", .{fragment});
            var fragment_file: std.fs.File = try dir.openFile(fragment, .{});
            defer dir.deleteFile(fragment) catch unreachable;
            defer fragment_file.close();
            std.debug.print("Size: {}\n", .{try fragment_file.getEndPos()});

            var file_reader: std.fs.File.Reader = fragment_file.readerStreaming(
                &read_buffer,
            );
            const reader = &file_reader.interface;
            const read = try reader.streamRemaining(writer);

            std.debug.print("Processed {} bytes...\n\n", .{read});
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
