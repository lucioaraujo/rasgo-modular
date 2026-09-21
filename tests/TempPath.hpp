#pragma once

// Caminho de arquivo temporário PORTÁVEL para os testes.
//
// ---- por que existe ------------------------------------------------------
//
// Achado da CI em 21 set. 2026, na PRIMEIRA vez que ela rodou de verdade
// (até a extração pra repositório próprio o workflow estava inerte, porque
// o GitHub Actions só lê `.github/workflows/` da raiz do repositório).
//
// Os testes de WAV escreviam em `/tmp/...` fixo. No Windows `/tmp` não
// existe: o `fopen` falha, o arquivo nunca é escrito, e a leitura de volta
// devolve vetor vazio. Isso produziu DOIS defeitos de gravidade diferente:
//
//   - os testes novos de PCM24 indexavam `pcm[0]` direto e o job morreu
//     com SEGFAULT — barulhento, mas honesto;
//   - os testes antigos percorriam com `i < pcm.size()`, então com zero
//     amostras o laço simplesmente não rodava e eles **passavam sem testar
//     nada**. Esse é o pior dos dois: um teste verde que não verificou
//     coisa alguma mente para quem confia nele.
//
// Zero-dep de propósito, no idioma do resto do projeto: nada de
// `<filesystem>`, cuja disponibilidade varia com a versão do libc++ da
// Apple e com o deployment target — não vale arriscar uma dependência de
// plataforma justamente no código que existe para ser portável.

#include <cstdlib>
#include <string>

namespace rasgo::test {

// Diretório temporário do sistema, na ordem em que cada um é
// convencionado: `TMPDIR` em Unix, `TEMP`/`TMP` no Windows. O último
// recurso é o diretório de trabalho — que sempre existe e é gravável onde
// os testes rodam, e é melhor que um caminho absoluto inventado.
inline std::string tempDir() {
    for (const char* var : {"TMPDIR", "TEMP", "TMP"}) {
        const char* v = std::getenv(var);
        if (v != nullptr && v[0] != '\0') {
            std::string s(v);
            while (!s.empty() && (s.back() == '/' || s.back() == '\\'))
                s.pop_back();
            return s;
        }
    }
#ifdef _WIN32
    return ".";
#else
    return "/tmp";
#endif
}

// `tempPath("wav24_header", ".wav")` -> "<tmp>/rasgo_wav24_header.wav".
// A barra normal funciona também no Windows nas APIs de arquivo.
inline std::string tempPath(const std::string& name,
                            const std::string& ext = ".wav") {
    return tempDir() + "/rasgo_" + name + ext;
}

}  // namespace rasgo::test
