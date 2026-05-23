Import("env")
import os
import glob
import subprocess

def generate_hex(source, target, env):
    packages_dir = env.subst("$PROJECT_PACKAGES_DIR")
    build_dir    = env.subst("$BUILD_DIR")

    pattern = os.path.join(
        packages_dir,
        "toolchain-xtensa-esp32*",
        "bin",
        "xtensa-esp32-elf-objcopy*"
    )

    resultados = glob.glob(pattern)

    if not resultados:
        print(">>> ERROR: objcopy no encontrado")
        return

    objcopy  = resultados[0]
    elf_file = os.path.join(build_dir, "firmware.elf")
    hex_file = os.path.join(build_dir, "firmware.hex")

    # subprocess maneja espacios en rutas correctamente
    resultado = subprocess.run(
        [objcopy, "-O", "ihex", elf_file, hex_file],
        capture_output=True,
        text=True
    )

    if resultado.returncode == 0 and os.path.exists(hex_file):
        size = os.path.getsize(hex_file)
        print(f">>> HEX generado correctamente: {hex_file}")
        print(f">>> Tamaño: {size} bytes")
    else:
        print(f">>> ERROR al generar HEX")
        print(f">>> stderr: {resultado.stderr}")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", generate_hex)