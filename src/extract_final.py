import py7zr
import os

lib_dir = 'd:/workspace/trae/day11/lib'
source_file = 'd:/workspace/trae/day11/mpv-dev-x86_64-20260528-git-cce65c7.7z'

print(f'Extracting {source_file}...')
with py7zr.SevenZipFile(source_file, mode='r') as z:
    z.extractall(lib_dir)
print('Extraction complete!')

print('\nLooking for mpv DLL files:')
found = False
mpv_dll_path = None
for root, dirs, files in os.walk(lib_dir):
    for f in files:
        if f.lower().startswith('mpv') and f.endswith('.dll'):
            mpv_dll_path = os.path.join(root, f)
            print(f'Found: {mpv_dll_path}')
            found = True

if found and mpv_dll_path:
    print('\nSUCCESS! MPV library found.')
    # 复制到lib根目录以便python-mpv能找到
    dest_path = os.path.join(lib_dir, 'mpv-2.dll')
    import shutil
    shutil.copy(mpv_dll_path, dest_path)
    print(f'Copied mpv-2.dll to {dest_path}')
else:
    print('\nERROR: No mpv DLL found!')