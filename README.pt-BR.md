# Blair Witch OpenSSL Fix

Mod de compatibilidade para **Blair Witch (2019)** no Windows de 64 bits. Permite aplicar a correção do OpenSSL ao abrir o jogo pelo botão **Jogar** da Steam.

**[Guia de instalação passo a passo, remoção e solução de problemas](INSTALL.pt-BR.md)**

## Instalação

1. Feche o jogo.
2. Baixe e extraia o ZIP na página de [Releases](https://github.com/marcosmaldonadoo/blair-witch-openssl-fix/releases).
3. Na Steam, entre em **Blair Witch → Propriedades → Arquivos instalados → Explorar**.
4. Abra `Blairwitch\Binaries\Win64`.
5. Copie a **`winmm.dll`** do ZIP para essa pasta, ao lado de `Blairwitch-Win64-Shipping.exe`.
6. Em **Propriedades → Geral → Opções de inicialização**, remova o comando antigo que usa `cmd /c` e `OPENSSL_ia32cap`. Preserve outras opções que você use.
7. Clique em **Jogar** na Steam.

Se essa pasta já tiver uma `winmm.dll`, preserve o arquivo: outro mod pode estar usando esse nome. Esta versão não combina duas DLLs de mods com o mesmo nome.

Para desinstalar, feche o jogo e apague somente a `winmm.dll` deste mod na pasta `Win64` do jogo. A verificação de arquivos da Steam pode deixar arquivos adicionais de mods na pasta.

## O que muda

O mod aplica `OPENSSL_ia32cap=:~0x20000000` dentro do processo do jogo para evitar uma rotina antiga do OpenSSL que pode travar em determinados processadores. A [Intel documentou o problema e essa solução](https://www.intel.com/content/www/us/en/developer/articles/troubleshooting/openssl-sha-crash-bug-requires-application-update.html).

A alteração vale para o processo do jogo. Não há alteração persistente nas variáveis do usuário ou do sistema. Se já houver um valor não vazio para `OPENSSL_ia32cap`, ele será respeitado.

O executável original permanece intacto. A DLL encaminha as funções de áudio e temporização para a DLL original do Windows, carregada da pasta do sistema. Nenhum arquivo do jogo ou do Windows é distribuído no projeto.

Esta correção trata a falha de inicialização relacionada ao OpenSSL. Resolução, posição dos cliques e outros problemas gráficos precisam de ajustes próprios.

## Testes e limites

A primeira validação foi feita no Windows 11 x64, com a versão Steam de build **5340689**. O jogo foi iniciado pelo executável original sem uma variável OpenSSL herdada; o processo carregou o mod e permaneceu respondendo. Em 19/10/2026, o usuário também confirmou que o jogo abriu pelo botão **Jogar**, seguindo as instruções para remover o comando antigo, e que o menu funcionou normalmente. Uma sessão completa ainda não foi validada.

Os testes nativos verificam a variável visível ao `getenv` antes de `main`, os dois nomes de executável reconhecidos, a preservação de um valor existente, um processo não relacionado, 181 exportações e o encaminhamento de chamadas para a DLL do Windows.

Outras lojas, versões do jogo e combinações com outros mods não foram verificadas. O nome alternativo de executável `-Win7` é reconhecido, mas isso não significa que o Windows 7 tenha sido testado.

## Compilar

Com PowerShell e GCC MinGW-w64 x64 com bibliotecas UCRT disponíveis:

```powershell
.\build.ps1
.\test.ps1
```

O resultado fica em `build\winmm.dll`. Copie apenas essa DLL para o jogo. Os executáveis de teste não devem ser instalados na pasta do jogo.

Consulte o [README em inglês](README.md) para detalhes técnicos e [VALIDATION.md](VALIDATION.md) para o escopo da validação.

Licença MIT. Mod não oficial da comunidade.
