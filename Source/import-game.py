#!/usr/bin/env python3
"""Import only the owner's supported game files; never execute a PC installer."""
import hashlib,json,os,pathlib,shutil,sys,tempfile,zipfile
CHUNK=1024*1024
OLD=b'libfmodex64-4.44.33.so\0'
NEW=b'libfmodex64.so\0'
def load_files(root):
 files=json.loads((root/'runtime/game-files.json').read_text())['files']
 for name in files:
  p=pathlib.PurePosixPath(name)
  if p.is_absolute() or '..' in p.parts or not name.startswith('game/'):
   raise ValueError('Invalid game file list.')
 return files
def present(root,files):
 return all((root/name).is_file() and (root/name).stat().st_size==spec['size'] for name,spec in files.items())
def import_game(root):
 root=pathlib.Path(root).resolve();files=load_files(root)
 if present(root,files):
  print('Game files found.',flush=True);return False
 incoming=root/'gamedata';source_dir=None;archive=None
 for candidate in [incoming/'game',incoming]:
  if (candidate/'64/ShovelKnight').is_file() and (candidate/'data').is_dir():source_dir=candidate;break
 if source_dir is None:
  expected=incoming/'shovel_knight_treasure_trove_4_1_36252.sh'
  candidates=sorted(incoming.glob('*.sh'))
  if expected.is_file():archive=expected
  elif len(candidates)==1:archive=candidates[0]
  elif len(candidates)>1:raise ValueError('Keep only the supported Shovel Knight Linux installer in gamedata.')
  else:raise ValueError('Game files are needed. Copy your GOG Linux 4.1 installer into ShovelKnight_RGDSPlus/gamedata, then launch again. See INSTALL.txt.')
 print('Preparing your game. Please wait; the first launch can take several minutes.',flush=True)
 stage=pathlib.Path(tempfile.mkdtemp(prefix='.shovel-import-',dir=root));z=None
 try:
  if archive is not None:z=zipfile.ZipFile(archive)
  for index,(name,spec) in enumerate(files.items(),1):
   rel=name[len('game/'):];dest=stage/name;dest.parent.mkdir(parents=True,exist_ok=True)
   if z is not None:
    member='data/noarch/game/'+rel;info=z.getinfo(member)
    if info.file_size!=spec['size']:raise ValueError('This installer is not the supported GOG Linux 4.1 release.')
    source=z.open(info)
   else:source=(source_dir/rel).open('rb')
   digest=hashlib.sha256();total=0
   with source, dest.open('wb') as out:
    while True:
     data=source.read(CHUNK)
     if not data:break
     total+=len(data)
     if total>spec['size']:raise ValueError('Unexpected game file size: '+rel)
     digest.update(data);out.write(data)
   if total!=spec['size'] or digest.hexdigest()!=spec['source_sha256']:
    raise ValueError('Unsupported or incomplete game file: '+rel+'. Use the original GOG Linux Treasure Trove 4.1 files.')
   if name=='game/64/lib/libfmodevent64.so':
    data=dest.read_bytes()
    if data.count(OLD)!=1:raise ValueError('Unsupported FMOD event library.')
    data=data.replace(OLD,NEW+bytes(len(OLD)-len(NEW)))
    if hashlib.sha256(data).hexdigest()!=spec['sha256']:raise ValueError('Audio library preparation failed.')
    dest.write_bytes(data)
   elif spec['source_sha256']!=spec['sha256']:raise ValueError('Invalid game file checksum list.')
   if index%75==0:print('Preparing files:',index,'of',len(files),flush=True)
  # Every input is validated before replacing installed files. Saves are outside game/.
  for name in files:
   dest=root/name;dest.parent.mkdir(parents=True,exist_ok=True);os.replace(stage/name,dest)
  (root/'game/64/ShovelKnight').chmod(0o755)
  print('Game import complete. Starting Shovel Knight.',flush=True);return True
 finally:
  if z is not None:z.close()
  shutil.rmtree(stage,ignore_errors=True)
def main():
 try:import_game(sys.argv[1]);return 0
 except (OSError,ValueError,KeyError,zipfile.BadZipFile) as e:
  print('Setup could not finish:',str(e),flush=True);return 3
if __name__=='__main__':sys.exit(main())
