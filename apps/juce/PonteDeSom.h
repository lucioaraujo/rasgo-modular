// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (c) 2026 Lúcio de Araújo
//
// Cópia no Rasgo Modular: 15/11/2026, do monorepo privado
// rasgo-instruments (RASGO/PROJETOS_AUTORAIS/MAESTRUC/receptor), SHA-256 da
// origem f7de1ae1896d…; ponte de som, mixer e senha. Ao atualizar, copiar de lá e anotar.
//
// Ponte de som do Maestruc (maestruc.link/1, 15/11/2026): o som de um
// instrumento chega ao Maestruc por memória compartilhada, nos três sistemas
// (Linux, Windows, macOS), sem driver de áudio virtual. Fonte:
// RASGO/PROJETOS_AUTORAIS/MAESTRUC/receptor/PonteDeSom.h. Cada instrumento
// leva uma cópia (junto de MaestrucLink.h), registrando de onde veio.
//
// Um arquivo mapeado na memória por instrumento, na pasta temporária do
// usuário (só ele lê e escreve). Dentro: um anel de quadros de áudio, de um
// só produtor (o callback de áudio do instrumento) para um só consumidor (o
// callback de áudio do Maestruc), sem trava e sem alocação nos dois lados.
//
//   canais 0-1  = a mistura do instrumento (o que ele tocaria nas caixas)
//   canais 2-3, 4-5… = canais internos dele, se mandar (opcional)
//
// O instrumento continua com a própria placa de som, que vira só o relógio:
// enquanto o Maestruc estiver lendo, ele pede ao instrumento que cale a
// própria saída (para o som não sair em dobro). Se o Maestruc parar de ler
// por ~0,5 s (fechou, travou), o instrumento volta a tocar sozinho.
//
// Os relógios das duas placas nunca batem exatos: o Leitor corrige o desvio
// reamostrando de leve (e converte taxas diferentes, ex.: 44,1 → 48 kHz).
// Só C++17 e o sistema; sem JUCE.

#pragma once

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>

#if defined(_WIN32)
 // O windows.h define macros com nomes comuns; o receptor não pode vazá-las
 // para o código do instrumento (15/11/2026: `near` quebrou o Asterismus no
 // MSVC). Versão enxuta, sem min/max, e near/far desfeitas logo depois.
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #define RASGO_PONTE_LEAN_DEFINIDO
 #endif
 #include <windows.h>
 #ifdef RASGO_PONTE_LEAN_DEFINIDO
  #undef WIN32_LEAN_AND_MEAN
  #undef RASGO_PONTE_LEAN_DEFINIDO
 #endif
 #undef near
 #undef far
#else
 #include <dirent.h>
 #include <fcntl.h>
 #include <signal.h>
 #include <sys/mman.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif

namespace rasgo::maestruc
{

namespace ponte
{
inline constexpr char kMagica[8] = { 'R', 'A', 'S', 'G', 'O', 'P', 'D', 'S' };
inline constexpr std::uint32_t kVersao = 1;
inline constexpr int kMaxCanais = 32;          // 16 pares estéreo
inline constexpr std::uint32_t kMaxQuadros = 1u << 18;   // ~5,5 s a 48 kHz
inline constexpr int kNomeMax = 24;

static_assert (std::atomic<std::uint64_t>::is_always_lock_free, "a ponte precisa de atômicos de 64 bits sem trava");
static_assert (std::atomic<std::uint32_t>::is_always_lock_free, "a ponte precisa de atômicos de 32 bits sem trava");

struct Cabecalho
{
    char magica[8];
    std::uint32_t versao;
    std::uint32_t canais;          // total de canais (pares estéreo × 2)
    std::uint32_t capacidade;      // quadros no anel (potência de 2)
    float taxa;                    // taxa de amostragem do instrumento
    std::atomic<std::uint64_t> escrito;    // quadros escritos (produtor)
    std::atomic<std::uint64_t> lido;       // quadros lidos (consumidor)
    std::atomic<std::uint32_t> perdidos;   // quadros descartados: anel cheio
    std::atomic<std::uint32_t> pulsoProdutor;    // +1 a cada bloco do instrumento
    std::atomic<std::uint32_t> pulsoConsumidor;  // +1 a cada bloco do Maestruc
    std::atomic<std::uint32_t> calar;            // 1: o Maestruc pede que o instrumento cale a própria saída
    char nomes[kMaxCanais / 2][kNomeMax];        // nome de cada par estéreo ("mix", "bumbo"…)
};

inline std::size_t tamanhoTotal (std::uint32_t canais, std::uint32_t capacidade)
{
    return sizeof (Cabecalho) + sizeof (float) * (std::size_t) canais * capacidade;
}

inline bool potenciaDe2 (std::uint32_t v) { return v != 0 && (v & (v - 1)) == 0; }

// Mapeamento de arquivo, nos três sistemas.
class Mapa
{
public:
    Mapa() = default;
    Mapa (const Mapa&) = delete;
    Mapa& operator= (const Mapa&) = delete;
    ~Mapa() { fechar(); }

    void* dados() const noexcept { return dados_; }
    std::size_t tamanho() const noexcept { return tamanho_; }

    // Cria o arquivo novo (falha se já existir) com só o dono podendo ler.
    bool criar (const std::string& caminho, std::size_t tamanho)
    {
        fechar();
#if defined(_WIN32)
        const std::wstring w = largo (caminho);
        arquivo_ = CreateFileW (w.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);
        if (arquivo_ == INVALID_HANDLE_VALUE) return false;
        return mapear (tamanho, true);
#else
        fd_ = ::open (caminho.c_str(), O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
        if (fd_ < 0) return false;
        if (::ftruncate (fd_, (off_t) tamanho) != 0) { fechar(); ::unlink (caminho.c_str()); return false; }
        return mapear (tamanho, true);
#endif
    }

    // Abre um arquivo existente; confere que é arquivo comum, do mesmo
    // usuário e do tamanho mínimo. O tamanho exato o Leitor confere depois.
    bool abrir (const std::string& caminho)
    {
        fechar();
#if defined(_WIN32)
        const std::wstring w = largo (caminho);
        arquivo_ = CreateFileW (w.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (arquivo_ == INVALID_HANDLE_VALUE) return false;
        LARGE_INTEGER t {};
        if (! GetFileSizeEx (arquivo_, &t) || t.QuadPart < (LONGLONG) sizeof (Cabecalho)
            || t.QuadPart > (LONGLONG) tamanhoTotal (kMaxCanais, kMaxQuadros)) { fechar(); return false; }
        return mapear ((std::size_t) t.QuadPart, false);
#else
        fd_ = ::open (caminho.c_str(), O_RDWR | O_CLOEXEC | O_NOFOLLOW);
        if (fd_ < 0) return false;
        struct stat st {};
        if (::fstat (fd_, &st) != 0 || ! S_ISREG (st.st_mode) || st.st_uid != ::getuid()
            || (st.st_mode & 077) != 0
            || st.st_size < (off_t) sizeof (Cabecalho)
            || st.st_size > (off_t) tamanhoTotal (kMaxCanais, kMaxQuadros)) { fechar(); return false; }
        return mapear ((std::size_t) st.st_size, false);
#endif
    }

    void fechar() noexcept
    {
#if defined(_WIN32)
        if (dados_ != nullptr) UnmapViewOfFile (dados_);
        if (mapeamento_ != nullptr) CloseHandle (mapeamento_);
        if (arquivo_ != INVALID_HANDLE_VALUE) CloseHandle (arquivo_);
        mapeamento_ = nullptr;
        arquivo_ = INVALID_HANDLE_VALUE;
#else
        if (dados_ != nullptr) ::munmap (dados_, tamanho_);
        if (fd_ >= 0) ::close (fd_);
        fd_ = -1;
#endif
        dados_ = nullptr;
        tamanho_ = 0;
    }

private:
#if defined(_WIN32)
    static std::wstring largo (const std::string& s)
    {
        const int n = MultiByteToWideChar (CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        std::wstring w ((std::size_t) (n > 0 ? n : 1), L'\0');
        if (n > 0) MultiByteToWideChar (CP_UTF8, 0, s.c_str(), -1, w.data(), n);
        if (! w.empty() && w.back() == L'\0') w.pop_back();
        return w;
    }
    bool mapear (std::size_t tamanho, bool novo)
    {
        const ULONGLONG t = (ULONGLONG) tamanho;
        mapeamento_ = CreateFileMappingW (arquivo_, nullptr, PAGE_READWRITE, (DWORD) (t >> 32), (DWORD) (t & 0xffffffffu), nullptr);
        if (mapeamento_ == nullptr) { fechar(); return false; }
        dados_ = MapViewOfFile (mapeamento_, FILE_MAP_ALL_ACCESS, 0, 0, tamanho);
        if (dados_ == nullptr) { fechar(); return false; }
        tamanho_ = tamanho;
        if (novo) std::memset (dados_, 0, tamanho);
        return true;
    }
    HANDLE arquivo_ = INVALID_HANDLE_VALUE;
    HANDLE mapeamento_ = nullptr;
#else
    bool mapear (std::size_t tamanho, bool novo)
    {
        void* p = ::mmap (nullptr, tamanho, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (p == MAP_FAILED) { fechar(); return false; }
        dados_ = p;
        tamanho_ = tamanho;
        if (novo) std::memset (dados_, 0, tamanho);
        return true;
    }
    int fd_ = -1;
#endif
    void* dados_ = nullptr;
    std::size_t tamanho_ = 0;
};

// A pasta das pontes: na pasta temporária do usuário, só dele.
inline std::string pastaDasPontes()
{
#if defined(_WIN32)
    wchar_t buf[MAX_PATH + 1] {};
    const DWORD n = GetTempPathW (MAX_PATH, buf);
    std::wstring w (buf, n);
    w += L"rasgo-maestruc";
    CreateDirectoryW (w.c_str(), nullptr);
    const int m = WideCharToMultiByte (CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s ((std::size_t) (m > 0 ? m : 1), '\0');
    if (m > 0) WideCharToMultiByte (CP_UTF8, 0, w.c_str(), -1, s.data(), m, nullptr, nullptr);
    if (! s.empty() && s.back() == '\0') s.pop_back();
    return s;
#else
    const char* t = std::getenv ("TMPDIR");
    std::string base = (t != nullptr && *t != '\0') ? t : "/tmp";
    if (base.back() == '/') base.pop_back();
    const std::string pasta = base + "/rasgo-maestruc-" + std::to_string ((unsigned long) ::getuid());
    ::mkdir (pasta.c_str(), 0700);
    struct stat st {};
    // Recusa uma pasta que não seja do usuário ou que outros possam ler.
    if (::lstat (pasta.c_str(), &st) != 0 || ! S_ISDIR (st.st_mode) || st.st_uid != ::getuid()
        || (st.st_mode & 077) != 0)
        return {};
    return pasta;
#endif
}

inline std::string separador()
{
#if defined(_WIN32)
    return "\\";
#else
    return "/";
#endif
}

// Um caminho só é aceito se estiver dentro da pasta das pontes, sem "..",
// com o nome terminado em ".pds".
inline bool caminhoValido (const std::string& caminho)
{
    const std::string pasta = pastaDasPontes();
    if (pasta.empty() || caminho.size() <= pasta.size() + 5) return false;
    if (caminho.compare (0, pasta.size(), pasta) != 0) return false;
    const std::string resto = caminho.substr (pasta.size());
    if (resto.compare (0, 1, separador()) != 0) return false;
    const std::string nome = resto.substr (1);
    if (nome.find ("..") != std::string::npos || nome.find ('/') != std::string::npos
        || nome.find ('\\') != std::string::npos)
        return false;
    return nome.size() > 4 && nome.compare (nome.size() - 4, 4, ".pds") == 0;
}

// O processo do nome "<app>-<pid>-<ns>.pds" (0 = não é nome de ponte).
inline unsigned long long processoDoNome (const std::string& nome)
{
    if (nome.size() <= 4 || nome.compare (nome.size() - 4, 4, ".pds") != 0) return 0;
    const std::string base = nome.substr (0, nome.size() - 4);
    const auto b = base.rfind ('-');
    if (b == std::string::npos || b == 0) return 0;
    const auto a = base.rfind ('-', b - 1);
    if (a == std::string::npos) return 0;
    const std::string pid = base.substr (a + 1, b - a - 1);
    if (pid.empty() || pid.size() > 10 || pid.find_first_not_of ("0123456789") != std::string::npos) return 0;
    return std::stoull (pid);
}

// O processo ainda existe? Na dúvida (sem permissão para olhar), sim.
inline bool processoVivo (unsigned long long pid)
{
#if defined(_WIN32)
    HANDLE h = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD) pid);
    if (h == nullptr) return GetLastError() != ERROR_INVALID_PARAMETER;
    DWORD codigo = 0;
    const bool vivo = ! GetExitCodeProcess (h, &codigo) || codigo == STILL_ACTIVE;
    CloseHandle (h);
    return vivo;
#else
    return ::kill ((pid_t) pid, 0) == 0 || errno != ESRCH;
#endif
}

// Apaga as pontes deixadas por instrumentos que já não existem (fechados à
// força, travados): só arquivos .pds da pasta das pontes, do próprio usuário,
// cujo processo do nome acabou. 15/11/2026.
inline int limparOrfaos()
{
    const std::string pasta = pastaDasPontes();
    if (pasta.empty()) return 0;
    int apagados = 0;
#if defined(_WIN32)
    const int n = MultiByteToWideChar (CP_UTF8, 0, pasta.c_str(), -1, nullptr, 0);
    std::wstring w ((std::size_t) (n > 0 ? n : 1), L'\0');
    if (n > 0) MultiByteToWideChar (CP_UTF8, 0, pasta.c_str(), -1, w.data(), n);
    if (! w.empty() && w.back() == L'\0') w.pop_back();
    WIN32_FIND_DATAW d {};
    HANDLE h = FindFirstFileW ((w + L"\\*.pds").c_str(), &d);
    if (h == INVALID_HANDLE_VALUE) return 0;
    for (int i = 0; i < 256; ++i)
    {
        if ((d.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0)
        {
            const int m = WideCharToMultiByte (CP_UTF8, 0, d.cFileName, -1, nullptr, 0, nullptr, nullptr);
            std::string nome ((std::size_t) (m > 0 ? m : 1), '\0');
            if (m > 0) WideCharToMultiByte (CP_UTF8, 0, d.cFileName, -1, nome.data(), m, nullptr, nullptr);
            if (! nome.empty() && nome.back() == '\0') nome.pop_back();
            const auto pid = processoDoNome (nome);
            if (pid != 0 && ! processoVivo (pid) && DeleteFileW ((w + L"\\" + d.cFileName).c_str()))
                ++apagados;
        }
        if (! FindNextFileW (h, &d)) break;
    }
    FindClose (h);
#else
    DIR* dir = ::opendir (pasta.c_str());
    if (dir == nullptr) return 0;
    for (int i = 0; i < 256; ++i)   // um limite: a varredura nunca é longa
    {
        const dirent* e = ::readdir (dir);
        if (e == nullptr) break;
        const std::string nome = e->d_name;
        const auto pid = processoDoNome (nome);
        if (pid == 0 || processoVivo (pid)) continue;
        const std::string caminho = pasta + "/" + nome;
        struct stat st {};
        if (::lstat (caminho.c_str(), &st) == 0 && S_ISREG (st.st_mode) && st.st_uid == ::getuid()
            && ::unlink (caminho.c_str()) == 0)
            ++apagados;
    }
    ::closedir (dir);
#endif
    return apagados;
}
} // namespace ponte

// O arquivo da senha da sessão do Maestruc (na pasta das pontes).
inline std::string caminhoDaSenhaMaestruc()
{
    const auto pasta = ponte::pastaDasPontes();
    return pasta.empty() ? std::string() : pasta + ponte::separador() + "sessao.txt";
}

// ---------------------------------------------------------------------------
// Lado do instrumento: escreve o som no anel, dentro do callback de áudio.
class EmissorDeSom
{
public:
    EmissorDeSom() = default;
    ~EmissorDeSom() { fechar(); }

    // Fio de mensagens, antes de o áudio usar. `pares`: 1 = só a mistura;
    // mais = a mistura e os canais internos. `nomes`: um por par (opcional).
    // Devolve o caminho do arquivo (vazio = falhou).
    std::string abrir (const std::string& nomeDoApp, int pares, double taxa,
                       const char* const* nomes = nullptr, std::uint32_t capacidade = 1u << 15)
    {
        fechar();
        if (pares < 1 || pares > ponte::kMaxCanais / 2 || taxa < 8000.0 || taxa > 384000.0
            || ! ponte::potenciaDe2 (capacidade) || capacidade > ponte::kMaxQuadros)
            return {};
        const std::string pasta = ponte::pastaDasPontes();
        if (pasta.empty()) return {};
        ponte::limparOrfaos();   // pontes de instrumentos fechados à força
        std::string limpo;
        for (char c : nomeDoApp)
            limpo += (std::isalnum ((unsigned char) c) != 0 || c == '-') ? c : '-';
        // único entre aberturas: o relógio do sistema em ns e o processo
        const auto ns = (unsigned long long) std::chrono::duration_cast<std::chrono::nanoseconds> (
                            std::chrono::system_clock::now().time_since_epoch()).count();
#if defined(_WIN32)
        const auto pid = (unsigned long long) GetCurrentProcessId();
#else
        const auto pid = (unsigned long long) ::getpid();
#endif
        const auto unico = std::to_string (pid) + "-" + std::to_string (ns);
        caminho_ = pasta + ponte::separador() + (limpo.empty() ? std::string ("app") : limpo) + "-" + unico + ".pds";
        const auto canais = (std::uint32_t) (pares * 2);
        if (! mapa_.criar (caminho_, ponte::tamanhoTotal (canais, capacidade))) { caminho_.clear(); return {}; }
        auto* c = new (mapa_.dados()) ponte::Cabecalho();
        std::memcpy (c->magica, ponte::kMagica, sizeof (c->magica));
        c->canais = canais;
        c->capacidade = capacidade;
        c->taxa = (float) taxa;
        for (int i = 0; i < pares; ++i)
        {
            const char* n = (nomes != nullptr && nomes[i] != nullptr) ? nomes[i] : (i == 0 ? "mix" : "canal");
            std::strncpy (c->nomes[i], n, ponte::kNomeMax - 1);
        }
        cab_ = c;
        dados_ = reinterpret_cast<float*> (static_cast<char*> (mapa_.dados()) + sizeof (ponte::Cabecalho));
        canais_ = canais;
        mascara_ = capacidade - 1;
        // a versão por último: o Leitor só aceita o arquivo depois disso
        std::atomic_thread_fence (std::memory_order_release);
        c->versao = ponte::kVersao;
        return caminho_;
    }

    void fechar()
    {
        cab_ = nullptr;
        dados_ = nullptr;
        mapa_.fechar();
        if (! caminho_.empty())
        {
#if defined(_WIN32)
            const int n = MultiByteToWideChar (CP_UTF8, 0, caminho_.c_str(), -1, nullptr, 0);
            std::wstring w ((std::size_t) (n > 0 ? n : 1), L'\0');
            if (n > 0) MultiByteToWideChar (CP_UTF8, 0, caminho_.c_str(), -1, w.data(), n);
            DeleteFileW (w.c_str());
#else
            ::unlink (caminho_.c_str());
#endif
            caminho_.clear();
        }
    }

    bool aberto() const noexcept { return cab_ != nullptr; }
    const std::string& caminho() const noexcept { return caminho_; }

    // Callback de áudio. `canais[k]` (k < canais da ponte; nullptr = silêncio).
    // Devolve true se o instrumento deve calar a própria saída agora: o
    // Maestruc está lendo e pediu. Sem trava, sem alocação.
    bool escrever (const float* const* canais, int numCanais, int quadros) noexcept
    {
        if (cab_ == nullptr || quadros <= 0) return false;
        const std::uint64_t w = cab_->escrito.load (std::memory_order_relaxed);
        const std::uint64_t r = cab_->lido.load (std::memory_order_acquire);
        const std::uint64_t livre = (std::uint64_t) (mascara_ + 1) - (w - r);
        const int n = (std::uint64_t) quadros > livre ? (int) livre : quadros;
        if (n < quadros) cab_->perdidos.fetch_add ((std::uint32_t) (quadros - n), std::memory_order_relaxed);
        for (int i = 0; i < n; ++i)
        {
            float* q = dados_ + (std::size_t) ((w + (std::uint64_t) i) & mascara_) * canais_;
            for (std::uint32_t k = 0; k < canais_; ++k)
            {
                const float* src = (int) k < numCanais ? canais[k] : nullptr;
                q[k] = src != nullptr ? src[i] : 0.0f;
            }
        }
        cab_->escrito.store (w + (std::uint64_t) n, std::memory_order_release);
        cab_->pulsoProdutor.fetch_add (1, std::memory_order_relaxed);

        // O Maestruc está vivo? O pulso dele tem de ter mudado nos últimos
        // ~0,5 s de blocos do instrumento; senão o instrumento volta a tocar.
        const std::uint32_t p = cab_->pulsoConsumidor.load (std::memory_order_relaxed);
        if (p != ultimoPulso_) { ultimoPulso_ = p; quadrosSemPulso_ = 0; }
        else quadrosSemPulso_ += (std::uint64_t) quadros;
        const bool vivo = quadrosSemPulso_ < (std::uint64_t) (cab_->taxa * 0.5f);
        return vivo && cab_->calar.load (std::memory_order_relaxed) != 0;
    }

private:
    ponte::Mapa mapa_;
    ponte::Cabecalho* cab_ = nullptr;
    float* dados_ = nullptr;
    std::uint32_t canais_ = 0;
    std::uint64_t mascara_ = 0;
    std::uint32_t ultimoPulso_ = 0;
    std::uint64_t quadrosSemPulso_ = 0;
    std::string caminho_;
};

// ---------------------------------------------------------------------------
// Lado do Maestruc: lê o anel no próprio callback de áudio, corrigindo o
// desvio entre os relógios e convertendo a taxa (interpolação de Hermite de
// 4 pontos). Um Leitor por instrumento.
class LeitorDeSom
{
public:
    LeitorDeSom() = default;
    ~LeitorDeSom() { fechar(); }

    // Fio de mensagens. Só aceita caminhos dentro da pasta das pontes e
    // arquivos coerentes (mágica, versão, tamanho exato, limites).
    bool abrir (const std::string& caminho, double taxaDeSaida)
    {
        fechar();
        if (! ponte::caminhoValido (caminho) || taxaDeSaida < 8000.0) return false;
        if (! mapa_.abrir (caminho)) return false;
        auto* c = static_cast<ponte::Cabecalho*> (mapa_.dados());
        std::atomic_thread_fence (std::memory_order_acquire);
        if (std::memcmp (c->magica, ponte::kMagica, sizeof (c->magica)) != 0 || c->versao != ponte::kVersao
            || c->canais < 2 || c->canais > (std::uint32_t) ponte::kMaxCanais || (c->canais % 2) != 0
            || ! ponte::potenciaDe2 (c->capacidade) || c->capacidade > ponte::kMaxQuadros
            || ! (c->taxa >= 8000.0f && c->taxa <= 384000.0f)
            || mapa_.tamanho() != ponte::tamanhoTotal (c->canais, c->capacidade))
        {
            mapa_.fechar();
            return false;
        }
        cab_ = c;
        dados_ = reinterpret_cast<const float*> (static_cast<const char*> (mapa_.dados()) + sizeof (ponte::Cabecalho));
        canais_ = c->canais;
        mascara_ = c->capacidade - 1;
        razaoNominal_ = (double) c->taxa / taxaDeSaida;
        reiniciar();
        return true;
    }

    void fechar()
    {
        if (cab_ != nullptr) cab_->calar.store (0, std::memory_order_relaxed);
        cab_ = nullptr;
        dados_ = nullptr;
        mapa_.fechar();
    }

    bool aberto() const noexcept { return cab_ != nullptr; }
    int pares() const noexcept { return (int) canais_ / 2; }
    std::string nomeDoPar (int i) const
    {
        if (cab_ == nullptr || i < 0 || i >= pares()) return {};
        char n[ponte::kNomeMax + 1] {};
        std::memcpy (n, cab_->nomes[i], ponte::kNomeMax);
        return n;
    }

    // Pede (ou deixa de pedir) que o instrumento cale a própria saída.
    void pedirCalar (bool sim) noexcept { if (cab_ != nullptr) cab_->calar.store (sim ? 1u : 0u, std::memory_order_relaxed); }

    // Callback de áudio do Maestruc: `saida[k]` recebe `quadros` amostras do
    // canal k (k < numCanais; nullptr = ignora). Sem trava, sem alocação.
    // Enquanto o anel não junta a reserva (ao começar, depois de faltar
    // som), sai silêncio.
    void ler (float* const* saida, int numCanais, int quadros) noexcept
    {
        for (int k = 0; k < numCanais; ++k)
            if (saida[k] != nullptr) std::fill (saida[k], saida[k] + quadros, 0.0f);
        if (cab_ == nullptr || quadros <= 0) return;
        cab_->pulsoConsumidor.fetch_add (1, std::memory_order_relaxed);

        const std::uint64_t w = cab_->escrito.load (std::memory_order_acquire);
        std::uint64_t r = cab_->lido.load (std::memory_order_relaxed);
        const double reserva = std::max (4.0 * quadros * razaoNominal_, 0.03 * cab_->taxa);
        const double disponivel = (double) (w - r) - pos_;

        if (esperando_)
        {
            if (disponivel < reserva) return;
            esperando_ = false;
            integral_ = 0.0;
            nivel_ = disponivel;
        }
        // Muito atrasado (o Maestruc parou um tempo): pula para perto da reserva.
        if (disponivel > reserva * 4.0 + quadros * razaoNominal_ * 4.0)
        {
            const auto pular = (std::uint64_t) (disponivel - reserva);
            r += pular;
            cab_->lido.store (r, std::memory_order_release);
            ++saltos_;
            nivel_ = reserva;
        }

        // Controle do desvio: mede o quanto o anel está acima/abaixo da
        // reserva e ajusta a razão de leitura devagar, para a afinação não
        // oscilar (o enchimento sobe e desce a cada bloco; isso não é desvio).
        // A medida é suavizada em ~2 s de tempo real; o ajuste fica em ±0,2 %
        // (desvios de placas reais são de dezenas a centenas de ppm).
        //
        // É um controle PI sobre um integrador (o anel): com g = taxa/reserva,
        // o erro relativo e segue e'' + g·Kp·e' + g·Ki·e = 0. Os ganhos saem de
        // uma frequência natural lenta (0,05 rad/s, ~2 min) e amortecimento
        // 0,8, para não ir e voltar (oscilar a afinação), qualquer que seja o
        // tamanho do bloco ou da reserva.
        const double blocoSeg = quadros / (cab_->taxa / razaoNominal_);
        const double a = 1.0 - std::exp (-blocoSeg / 2.0);
        nivel_ += a * (((double) (w - r) - pos_) - nivel_);
        const double erro = (nivel_ - reserva) / reserva;
        const double g = cab_->taxa / reserva;
        constexpr double omega = 0.05, zeta = 0.8;
        const double kp = 2.0 * zeta * omega / g;
        const double ki = omega * omega / g;
        integral_ = std::clamp (integral_ + erro * ki * blocoSeg, -0.002, 0.002);
        const double razao = razaoNominal_ * (1.0 + std::clamp (erro * kp + integral_, -0.002, 0.002));
        razaoAtual_ = razao;

        const int nc = std::min<int> (numCanais, (int) canais_);
        for (int i = 0; i < quadros; ++i)
        {
            // precisa de 2 quadros à frente da posição para a interpolação
            const double base = std::floor (pos_);
            if ((double) (w - r) < base + 3.0)
            {
                esperando_ = true;   // faltou som: silêncio até juntar a reserva de novo
                ++faltas_;
                break;
            }
            const double f = pos_ - base;
            const std::uint64_t i0 = r + (std::uint64_t) base;
            for (int k = 0; k < nc; ++k)
            {
                if (saida[k] == nullptr) continue;
                const float xm1 = anterior_[k];
                const float x0 = amostra (i0, k);
                const float x1 = amostra (i0 + 1, k);
                const float x2 = amostra (i0 + 2, k);
                saida[k][i] = hermite (xm1, x0, x1, x2, (float) f);
            }
            pos_ += razao;
            const auto avancar = (std::uint64_t) std::floor (pos_);
            if (avancar > 0)
            {
                for (int k = 0; k < nc && k < ponte::kMaxCanais; ++k)
                    anterior_[k] = amostra (r + avancar - 1, k);
                r += avancar;
                pos_ -= (double) avancar;
            }
        }
        cab_->lido.store (r, std::memory_order_release);
    }

    // Para a tela e os ensaios.
    std::uint32_t perdidosNoInstrumento() const noexcept { return cab_ ? cab_->perdidos.load (std::memory_order_relaxed) : 0; }
    std::uint32_t faltas() const noexcept { return faltas_; }
    std::uint32_t saltos() const noexcept { return saltos_; }
    double razaoAtual() const noexcept { return razaoAtual_; }
    double nivel() const noexcept { return nivel_; }
    bool esperando() const noexcept { return esperando_; }
    // O instrumento ainda escreve? (o pulso dele mudou desde a última olhada)
    bool produtorVivo() noexcept
    {
        if (cab_ == nullptr) return false;
        const auto p = cab_->pulsoProdutor.load (std::memory_order_relaxed);
        const bool vivo = p != ultimoPulsoProdutor_;
        ultimoPulsoProdutor_ = p;
        return vivo;
    }

private:
    float amostra (std::uint64_t quadro, int canal) const noexcept
    {
        return dados_[(std::size_t) (quadro & mascara_) * canais_ + (std::size_t) canal];
    }
    static float hermite (float xm1, float x0, float x1, float x2, float t) noexcept
    {
        const float c1 = 0.5f * (x1 - xm1);
        const float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
        const float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
        return ((c3 * t + c2) * t + c1) * t + x0;
    }
    void reiniciar() noexcept
    {
        esperando_ = true;
        pos_ = 0.0;
        nivel_ = 0.0;
        integral_ = 0.0;
        razaoAtual_ = razaoNominal_;
        std::fill (std::begin (anterior_), std::end (anterior_), 0.0f);
    }

    ponte::Mapa mapa_;
    ponte::Cabecalho* cab_ = nullptr;
    const float* dados_ = nullptr;
    std::uint32_t canais_ = 0;
    std::uint64_t mascara_ = 0;
    double razaoNominal_ = 1.0, razaoAtual_ = 1.0;
    double pos_ = 0.0;        // fração entre o quadro lido e o próximo
    double nivel_ = 0.0, integral_ = 0.0;
    bool esperando_ = true;
    float anterior_[ponte::kMaxCanais] {};
    std::uint32_t faltas_ = 0, saltos_ = 0;
    std::uint32_t ultimoPulsoProdutor_ = 0;
};

} // namespace rasgo::maestruc
