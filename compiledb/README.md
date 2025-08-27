# CompileDB
Zig does not have a built in way yet to specify outputting of
compile_commands.json. Instead, we have to use -MJ as a compile flag, then
combine all the fragments manually after the build as part of a build step. The
program that does that lives here.
