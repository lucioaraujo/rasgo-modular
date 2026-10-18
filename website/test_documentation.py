#!/usr/bin/env python3
"""Regressões locais do guia: python3 -B -m unittest discover -s website -p 'test_*.py'."""
import json
import pathlib
import re
import tempfile
import unittest
from unittest import mock

import gerar_modulos as gerar
import seo
import verificar


class ModuleGuideTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalogue = json.loads((gerar.AQUI / "modulos.json").read_text(encoding="utf-8"))
        cls.types = [m["tipo"] for f in cls.catalogue for m in f["modulos"]]

    def test_sixty_distinct_modules_and_eight_families(self):
        self.assertEqual(len(self.catalogue), 8)
        self.assertEqual(len(self.types), 60)
        self.assertEqual(len(set(self.types)), 60)
        self.assertTrue({"LISTEN", "PHRASE"}.issubset(self.types))

    def test_complete_guides_and_real_images_in_each_language(self):
        for language in gerar.IDIOMAS:
            for module in self.types:
                with self.subTest(language=language, module=module):
                    sections = gerar.ler_verbete(language, module)
                    self.assertIsNotNone(sections)
                    for key in gerar.PARTES:
                        self.assertTrue(" ".join(sections[key]).strip())
                    self.assertTrue((gerar.AQUI / "assets" / "modulos" / (module + ".webp")).is_file())

    def test_generated_articles_are_complete_without_old_development_badges(self):
        for language, data in gerar.IDIOMAS.items():
            page = (gerar.AQUI / data["arquivo"]).read_text(encoding="utf-8")
            articles = re.findall(r'<article class="modulo" id="mod-([^"]+)">(.*?)</article>', page, re.S)
            self.assertEqual([name for name, _ in articles], self.types)
            self.assertNotIn("{total}", page)
            self.assertNotIn("{familias}", page)
            for module, article in articles:
                with self.subTest(language=language, module=module):
                    self.assertEqual(article.count("<h4>"), 4)
                    self.assertIn('assets/modulos/%s.webp' % module, article)
                    self.assertIn('loading="lazy"', article)
                    self.assertNotIn("0.1.7-dev", article)
            self.assertNotIn("development-note", page)

    def test_generation_is_reproducible(self):
        for language, data in gerar.IDIOMAS.items():
            page = gerar.pagina(language, data, self.catalogue, gerar.wordmark())
            # SEO é acrescentado depois; o restante deve bater com o arquivo.
            disk = (gerar.AQUI / data["arquivo"]).read_text(encoding="utf-8")
            self.assertEqual(page, seo.BLOCO.sub("", disk))
            self.assertEqual(page.count('<article class="modulo"'), 60)

    def test_new_recipes_use_actual_drum_and_vca_labels(self):
        for language in gerar.IDIOMAS:
            listen = (gerar.AQUI / "guia" / language / "LISTEN.md").read_text(encoding="utf-8")
            phrase = (gerar.AQUI / "guia" / language / "PHRASE.md").read_text(encoding="utf-8")
            self.assertIn("DRUM.GATE", listen)
            self.assertNotIn("DRUM.TRIG", listen)
            self.assertNotIn("LEVEL1=", listen + phrase)
            self.assertIn("LVL1=0", phrase)
            self.assertIn("VCA.IN1", phrase)
            self.assertIn("VCA.O1", phrase)
            self.assertIn("RANGE=1", phrase)


class PublishedVersionTests(unittest.TestCase):
    def check_version(self, local, published, development):
        with tempfile.TemporaryDirectory(prefix="rasgo-site-test-") as folder:
            root = pathlib.Path(folder)
            (root / "CMakeLists.txt").write_text(
                'project(Test VERSION %s)\nset(RASGO_MODULAR_VERSION_LABEL "${PROJECT_VERSION}%s")\n'
                % (local, "-dev" if development else ""), encoding="utf-8")
            (root / "seo.py").write_text('VERSAO = "%s"\n' % published, encoding="utf-8")
            errors = []
            with mock.patch.object(verificar, "RAIZ", root), mock.patch.object(verificar, "AQUI", root), \
                    mock.patch.object(verificar, "problemas", errors):
                version = verificar.versao_publicada()
            return version, errors

    def test_published_version_may_precede_explicit_development(self):
        self.assertEqual(self.check_version("0.1.7", "0.1.6", True), ("0.1.6", []))

    def test_release_versions_must_match(self):
        self.assertEqual(self.check_version("0.1.6", "0.1.6", False), ("0.1.6", []))
        self.assertTrue(self.check_version("0.1.7", "0.1.6", False)[1])

    def test_future_public_version_is_rejected_even_in_development(self):
        self.assertTrue(self.check_version("0.1.7", "0.2.0", True)[1])

    def test_semantic_numeric_order_not_lexical(self):
        self.assertEqual(self.check_version("0.1.10", "0.1.9", True), ("0.1.9", []))
        self.assertTrue(self.check_version("0.1.9", "0.1.10", True)[1])

    def test_missing_version_is_rejected(self):
        self.assertTrue(self.check_version("0.1.7", "invalid", True)[1])


if __name__ == "__main__":
    unittest.main()
