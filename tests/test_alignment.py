import importlib.util
from pathlib import Path
import unittest

spec=importlib.util.spec_from_file_location('aligner',Path(__file__).parents[1]/'engine'/'align.py')
aligner=importlib.util.module_from_spec(spec)
spec.loader.exec_module(aligner)

class AlignmentTests(unittest.TestCase):
    def test_exact_text_with_spaces_and_punctuation(self):
        rows,warnings=aligner.map_alignment(['你 好！','world'],[{'word':'你好','start':1,'end':3},{'word':' world','start':4,'end':6}],8)
        self.assertEqual(''.join(w['text'] for w in rows[0]['words']),'你 好！')
        self.assertEqual(rows[0]['words'][0]['start'],1)
        self.assertEqual(rows[0]['words'][2]['end'],3)
        self.assertEqual(rows[1]['start'],4)
        self.assertTrue(warnings)
    def test_mismatched_text_rejected(self):
        with self.assertRaises(ValueError): aligner.map_alignment(['不同'],[{'word':'你好','start':1,'end':2}],3)
    def test_invalid_timing_rejected(self):
        with self.assertRaises(ValueError): aligner.map_alignment(['你'],[{'word':'你','start':2,'end':1}],3)
        with self.assertRaises(ValueError): aligner.map_alignment(['你'],[{'word':'你','start':0,'end':9}],3)
    def test_zero_duration_not_advertised_as_success(self):
        with self.assertRaises(ValueError): aligner.map_alignment(['你好'],[{'word':'你','start':1,'end':1},{'word':'好','start':1,'end':1}],3)
    def test_line_boundaries_and_repeat(self):
        rows,_=aligner.map_alignment(['你','你'],[{'word':'你你','start':1,'end':3}],4)
        self.assertEqual(rows[0]['end'],2)
        self.assertEqual(rows[1]['start'],2)

if __name__=='__main__':unittest.main()
