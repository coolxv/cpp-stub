#!/bin/bash
# macos_enable_stub.sh
#
# Post-link step required on macOS (both Intel and Apple Silicon) so that a
# binary using cpp-stub can actually patch its own __TEXT segment at runtime.
#
# macOS does not let mprotect raise __TEXT above its maxprot field (r-x by
# default) and, additionally, Apple Silicon enforces W^X on __TEXT at the
# page-table level. cpp-stub works around the W^X enforcement by writing to
# the __TEXT segment through a remapped writable alias (see src/stub.h, the
# __APPLE__ branch). For the kernel to allow the alias to be made writable,
# maxprot must first be escalated to rwx, which is what this script does.
#
# Because the Mach-O contents change, the existing code signature is
# invalidated; the binary is therefore re-signed with an ad-hoc signature.
#
# Usage: tool/macos_enable_stub.sh <path/to/binary> [<path/to/binary> ...]

set -e

if [ "$#" -lt 1 ]; then
    echo "usage: $0 <binary> [<binary> ...]" >&2
    exit 1
fi

patch_one() {
    local bin="$1"
    if [ ! -f "$bin" ]; then
        echo "skip: not a file: $bin" >&2
        return
    fi

    # Compute the file offset of the __TEXT segment's maxprot field by
    # walking the Mach-O load commands. This is portable across arm64 and
    # x86_64 binaries (their header layouts differ).
    local offset
    offset=$(python3 - "$bin" <<'PY'
import struct, sys
path = sys.argv[1]
with open(path, 'rb') as f:
    data = f.read()
magic = struct.unpack_from('<I', data, 0)[0]
# LC_SEGMENT_64 = 0x19, header size = 32 (64-bit fat-less Mach-O)
if magic not in (0xfeedfacf,):
    sys.stderr.write("unsupported Mach-O magic 0x%x (only 64-bit thin binaries are supported)\n" % magic)
    sys.exit(2)
ncmds = struct.unpack_from('<I', data, 16)[0]
off = 32
for _ in range(ncmds):
    cmd, cmdsize = struct.unpack_from('<II', data, off)
    if cmd == 0x19:  # LC_SEGMENT_64
        name = data[off+8:off+24].rstrip(b'\x00')
        if name == b'__TEXT':
            # maxprot lives at +56 inside LC_SEGMENT_64
            print(off + 56)
            sys.exit(0)
    off += cmdsize
sys.stderr.write("__TEXT segment not found\n")
sys.exit(3)
PY
    )

    # Write 0x07 (VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE) into maxprot.
    printf '\x07' | dd of="$bin" bs=1 seek="$offset" count=1 conv=notrunc status=none

    # Re-sign ad-hoc since we just mutated the Mach-O.
    codesign -s - --force "$bin" >/dev/null 2>&1

    echo "enabled cpp-stub for: $bin"
}

for bin in "$@"; do
    patch_one "$bin"
done
