import requests
import os
import zipfile

print('Getting latest MPV release info...')
try:
    response = requests.get('https://api.github.com/repos/mpv-player/mpv/releases/latest', timeout=30)
    response.raise_for_status()
    release = response.json()
    print(f'Latest version: {release["tag_name"]}')
    
    assets = release.get('assets', [])
    for asset in assets:
        name = asset.get('name', '')
        if 'x86_64' in name and 'mingw' in name.lower():
            url = asset.get('browser_download_url')
            print(f'Found asset: {name}')
            print(f'Download URL: {url}')
            
            output = 'd:/workspace/trae/day11/mpv.zip'
            print('Downloading...')
            response = requests.get(url, stream=True, timeout=120)
            response.raise_for_status()
            
            total_size = int(response.headers.get('content-length', 0))
            downloaded = 0
            
            with open(output, 'wb') as f:
                for chunk in response.iter_content(chunk_size=8192):
                    if chunk:
                        f.write(chunk)
                        downloaded += len(chunk)
                        if total_size > 0:
                            progress = (downloaded / total_size) * 100
                            print(f'Progress: {progress:.1f}%', end='\r')
            
            print(f'\nDownload complete! File size: {os.path.getsize(output) / 1024 / 1024:.2f} MB')
            
            os.makedirs('d:/workspace/trae/day11/lib', exist_ok=True)
            with zipfile.ZipFile(output, 'r') as zip_ref:
                zip_ref.extractall('d:/workspace/trae/day11/lib')
            print('Extraction complete!')
            
            mpv_dll = 'd:/workspace/trae/day11/lib/mpv-x86_64-w64-mingw32/bin/mpv-2.dll'
            if os.path.exists(mpv_dll):
                print(f'SUCCESS: Found mpv-2.dll at: {mpv_dll}')
            else:
                print('mpv-2.dll not found, checking other locations...')
                for root, dirs, files in os.walk('d:/workspace/trae/day11/lib'):
                    for f in files:
                        if 'mpv' in f.lower() and f.endswith('.dll'):
                            print(f'Found: {os.path.join(root, f)}')
            break
            
except Exception as e:
    print(f'Error: {e}')
    import traceback
    traceback.print_exc()