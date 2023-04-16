from platformio import fs
import os
import subprocess

Import("env")


def intersperse(lst, item):
    result = [item] * (len(lst) * 2)
    result[1::2] = lst
    return result


def postProgramAction(source, target, env):
    project_dir = env["PROJECT_DIR"]
    libdeps_dir = env["PROJECT_LIBDEPS_DIR"]
    board = env["BOARD"]
    nanopb_protos = env.GetProjectOption("custom_nanopb_protos", "")
    nanopb_python_loc = env.GetProjectOption("custom_nanopb_python_loc", "")

    nanopb_dir = f"{libdeps_dir}/{board}/Nanopb/generator/proto"
    proto_files = fs.match_src_files(project_dir, nanopb_protos)

    proto_files_abs = set()
    proto_include_dirs = set()

    for proto_file in proto_files:
        proto_file_abs = os.path.join(project_dir, proto_file)
        proto_dir = os.path.dirname(proto_file_abs)
        proto_files_abs.add(proto_file_abs)
        proto_include_dirs.add(proto_dir)

    proto_include_opts = intersperse(proto_include_dirs, "-I")
    proto_include_opts.extend(["-I", nanopb_dir])
    proto_proto_files = list(proto_files_abs)

    python_loc = os.path.join(project_dir, nanopb_python_loc)

    subprocess.run(
        [
            "protoc",
            *proto_include_opts,
            "--python_out",
            python_loc,
            *proto_proto_files,
        ]
    )


env.AddPostAction("buildprog", postProgramAction)
