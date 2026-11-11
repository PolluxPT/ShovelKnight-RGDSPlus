import pathlib,sys,json,hashlib

def generated_cache(name):
 p=pathlib.PurePosixPath(name)
 return '__pycache__' in p.parts or p.suffix in ('.pyc','.pyo')

def check(root):
 root=pathlib.Path(root);m=json.loads((root/'runtime/install-manifest.json').read_text());bad=[]
 for name,size in m['sizes'].items():
  if name=='game/config.ini' or generated_cache(name):continue
  p=root/name
  if not p.is_file() or p.stat().st_size!=size:bad.append(name)
 for name,digest in m['sha256'].items():
  if generated_cache(name):continue
  p=root/name
  if not p.is_file():
   if name not in bad:bad.append(name)
  elif hashlib.sha256(p.read_bytes()).hexdigest()!=digest:bad.append(name+' (checksum)')
 return bad

if __name__=='__main__':
 bad=check(sys.argv[1])
 if bad:
  print('Incomplete or changed installation. Extract the complete ZIP again.');print('\n'.join(bad));sys.exit(1)
 print('Installation check passed.')
