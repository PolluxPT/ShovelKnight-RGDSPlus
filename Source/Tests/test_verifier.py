import hashlib,importlib.util,json,pathlib,sys,tempfile,unittest
sys.dont_write_bytecode=True
source=pathlib.Path(__file__).resolve().parents[1]/'verify-install.py';spec=importlib.util.spec_from_file_location('verifier',source);mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
class CacheChecks(unittest.TestCase):
 def root(self,tmp):
  root=pathlib.Path(tmp);(root/'runtime').mkdir();(root/'runtime/essential.bin').write_bytes(b'correct')
  m={'sizes':{'runtime/essential.bin':7,'runtime/__pycache__/normalize-fmod.cpython-312.pyc':19357},'sha256':{'runtime/essential.bin':hashlib.sha256(b'correct').hexdigest(),'runtime/__pycache__/normalize-fmod.cpython-312.pyc':'0'*64}}
  (root/'runtime/install-manifest.json').write_text(json.dumps(m));return root
 def test_absent_cache(self):
  with tempfile.TemporaryDirectory() as tmp:self.assertEqual(mod.check(self.root(tmp)),[])
 def test_foreign_python_cache(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=self.root(tmp);p=root/'runtime/__pycache__/normalize-fmod.cpython-311.pyc';p.parent.mkdir();p.write_bytes(b'other interpreter cache');self.assertEqual(mod.check(root),[])
 def test_mismatched_old_cache(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=self.root(tmp);p=root/'runtime/__pycache__/normalize-fmod.cpython-312.pyc';p.parent.mkdir();p.write_bytes(b'generated');self.assertEqual(mod.check(root),[])
 def test_missing_required_file(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=self.root(tmp);(root/'runtime/essential.bin').unlink();self.assertIn('runtime/essential.bin',mod.check(root))
 def test_same_size_corruption(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=self.root(tmp);(root/'runtime/essential.bin').write_bytes(b'changed');self.assertIn('runtime/essential.bin (checksum)',mod.check(root))
if __name__=='__main__':unittest.main()
