#!/usr/bin/env python3
"""Package the public application only; reject user-owned content and state."""
from pathlib import Path
import hashlib,json,stat,sys,zipfile
repo=Path(__file__).resolve().parents[1];app=repo/'APPS/ShovelKnight_RGDSPlus'
output=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else repo.parent/'ReleaseAssets'
output.mkdir(parents=True,exist_ok=True)
files=[p for p in sorted((repo/'APPS').rglob('*')) if p.is_file()]
for p in files:
 rel=p.relative_to(app) if app in p.parents else None
 if p.is_symlink():raise SystemExit('Refusing symlink: '+str(p))
 if rel is not None:
  if any(x in rel.parts for x in ['game','savedata','logs','__pycache__']):raise SystemExit('Remove private/generated content before packaging: '+str(rel))
  if rel.parts[0]=='gamedata' and str(rel)!='gamedata/README.txt':raise SystemExit('Remove private installer/game files from gamedata.')
 if p.suffix in ['.fsb','.fev','.pak','.pyc','.pyo'] or 'PERSONAL' in p.name or 'TestLogs' in p.name:raise SystemExit('Private content rejected: '+str(p))
manifest=json.loads((app/'runtime/install-manifest.json').read_text())
for rel,digest in manifest['sha256'].items():
 if rel.startswith('game/'):continue
 p=app/rel
 if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest()!=digest:raise SystemExit('Public file checksum mismatch: '+rel)
name='ShovelKnight_RGDSPlus_Beta1_Install.zip';archive=output/name
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,9) as z:
 for p in files:
  info=zipfile.ZipInfo.from_file(p,str(p.relative_to(repo)))
  mode=0o755 if p.suffix=='.sh' or p.name=='box64-box32' else 0o644
  info.external_attr=(stat.S_IFREG|mode)<<16;info.compress_type=zipfile.ZIP_DEFLATED;z.writestr(info,p.read_bytes())
 z.writestr('README_INSTALL.txt',(app/'INSTALL.txt').read_bytes())
with zipfile.ZipFile(archive) as z:
 bad=z.testzip()
 if bad:raise SystemExit('Archive error: '+bad)
(output/'SHA256SUMS.txt').write_text(hashlib.sha256(archive.read_bytes()).hexdigest()+'  '+name+'\n')
print(archive,archive.stat().st_size)
