#!/usr/bin/env python3
"""Importa un ZIP/carpeta de samples a Assets/Samples/<CATEGORIA>/<CATEGORIA>-<Nombre>.wav

Uso:  python tools/import_samples.py ruta/TRAP_Files.zip
      python tools/import_samples.py ruta/carpeta_con_wavs

- Solo copia .wav (ignora .zpa, imagenes, etc.)
- La categoria se deduce del nombre de la carpeta (808s, KICKS, HI-HATS, OHs...).
- Para anadir sonidos en el futuro: vuelve a ejecutarlo con otro ZIP y recompila.
"""
import os, re, shutil, sys, tempfile, zipfile

CATEGORY_MAP = {
    "808": "808", "808S": "808", "KICKS": "KICKS", "KICK": "KICKS",
    "SNARES": "SNARES", "SNARE": "SNARES", "CLAPS": "CLAPS", "CLAP": "CLAPS",
    "HIHATS": "HIHATS", "HI-HATS": "HIHATS", "HATS": "HIHATS",
    "OHS": "OPEN_HATS", "OPEN HATS": "OPEN_HATS", "OPENHATS": "OPEN_HATS", "OPEN_HATS": "OPEN_HATS",
    "ROLLS": "ROLLS", "PERCS": "PERCUSSIONS", "PERCUSSIONS": "PERCUSSIONS", "PERC": "PERCUSSIONS",
    "RIMS": "RIMS", "VOCALS": "VOCALS", "FX": "FX",
}

def clean_name(stem):
    stem = re.sub(r"\s*\[[^\]]*\]\s*$", "", stem)      # quita " [KICK]"
    stem = stem.strip().replace(" ", "_")
    return re.sub(r"[^A-Za-z0-9_.]", "", stem) or "Sample"

def main():
    if len(sys.argv) < 2:
        print(__doc__); sys.exit(1)
    src = sys.argv[1]
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Assets", "Samples")
    tmp = None
    if zipfile.is_zipfile(src):
        tmp = tempfile.mkdtemp(); zipfile.ZipFile(src).extractall(tmp); src = tmp
    count, seen = 0, set()
    for root, _, files in os.walk(src):
        folder = os.path.basename(root).upper()
        for f in sorted(files):
            if not f.lower().endswith(".wav"): continue
            cat = CATEGORY_MAP.get(folder)
            if cat is None:
                print(f"  [?] carpeta sin categoria conocida '{folder}': {f} -> se omite"); continue
            name = clean_name(os.path.splitext(f)[0])
            if cat == "SNARES" and name.lower().startswith("roll"): cat = "ROLLS"
            key = (cat, name.lower())
            if key in seen: print(f"  [!] duplicado {cat}-{name}, se omite"); continue
            seen.add(key)
            dst_dir = os.path.join(out, cat); os.makedirs(dst_dir, exist_ok=True)
            shutil.copy2(os.path.join(root, f), os.path.join(dst_dir, f"{cat}-{name}.wav")); count += 1
    if tmp: shutil.rmtree(tmp)
    print(f"{count} samples importados en Assets/Samples")

if __name__ == "__main__":
    main()
