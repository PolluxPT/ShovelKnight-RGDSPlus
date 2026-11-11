import sys,unittest,tempfile,pathlib,json,hashlib,zipfile,importlib.util,contextlib,io
sys.dont_write_bytecode=True
spec=importlib.util.spec_from_file_location('import_game',pathlib.Path(__file__).parents[1]/'import-game.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
class ImportTests(unittest.TestCase):
 def setUp(self):
  self.temp=tempfile.TemporaryDirectory();self.root=pathlib.Path(self.temp.name);(self.root/'runtime').mkdir();(self.root/'gamedata').mkdir();(self.root/'savedata').mkdir();(self.root/'savedata/keep').write_text('save')
  self.raw={'game/64/ShovelKnight':b'test executable','game/64/lib/libfmodevent64.so':b'prefix'+module.OLD+b'suffix','game/data/example.pak':b'example content'}
  files={}
  for n,b in self.raw.items():
   normalized=b.replace(module.OLD,module.NEW+bytes(len(module.OLD)-len(module.NEW))) if 'fmodevent64' in n else b
   files[n]={'size':len(b),'source_sha256':hashlib.sha256(b).hexdigest(),'sha256':hashlib.sha256(normalized).hexdigest()}
  (self.root/'runtime/game-files.json').write_text(json.dumps({'files':files}));self.files=files
 def tearDown(self):self.temp.cleanup()
 def archive(self,corrupt=False):
  p=self.root/'gamedata/shovel_knight_treasure_trove_4_1_36252.sh';p.write_bytes(b'#!/bin/sh\n# never run this file\n')
  with zipfile.ZipFile(p,'a') as z:
   for n,b in self.raw.items():
    if corrupt and n.endswith('.pak'):b=bytes([b[0]^1])+b[1:]
    z.writestr('data/noarch/'+n,b)
  return p
 def run_import(self):
  with contextlib.redirect_stdout(io.StringIO()):return module.import_game(self.root)
 def test_embedded_zip_import_and_idempotence(self):
  self.archive();self.assertTrue(self.run_import());self.assertFalse(self.run_import());self.assertEqual((self.root/'savedata/keep').read_text(),'save')
  for n,s in self.files.items():self.assertEqual(hashlib.sha256((self.root/n).read_bytes()).hexdigest(),s['sha256'])
 def test_corrupt_input_rolls_back_and_preserves_saves(self):
  self.archive(True)
  with self.assertRaises(ValueError):self.run_import()
  self.assertFalse((self.root/'game').exists());self.assertEqual((self.root/'savedata/keep').read_text(),'save');self.assertFalse(list(self.root.glob('.shovel-import-*')))
 def test_extracted_directory(self):
  for n,b in self.raw.items():
   p=self.root/'gamedata'/n;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b)
  self.assertTrue(self.run_import())
 def test_missing_source_is_clear(self):
  with self.assertRaisesRegex(ValueError,'Game files are needed'):self.run_import()
 def test_path_escape_rejected(self):
  (self.root/'runtime/game-files.json').write_text(json.dumps({'files':{'game/../../outside':{'size':0}}}))
  with self.assertRaisesRegex(ValueError,'Invalid game file list'):self.run_import()
 def test_multiple_installers_rejected(self):
  for n in ['one.sh','two.sh']:(self.root/'gamedata'/n).write_bytes(b'bad')
  with self.assertRaisesRegex(ValueError,'Keep only'):self.run_import()
if __name__=='__main__':unittest.main()
