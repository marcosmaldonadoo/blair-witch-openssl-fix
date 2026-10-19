# Como instalar o Blair Witch OpenSSL Fix

Este guia é para a versão de **Blair Witch (2019) da Steam, no Windows de 64 bits**. Você instala uma DLL e depois abre o jogo normalmente pela Steam.

## 1. Baixar o mod

1. Feche o Blair Witch se ele estiver aberto.
2. Abra a página de [Releases do projeto](https://github.com/marcosmaldonadoo/blair-witch-openssl-fix/releases).
3. Na versão desejada, expanda **Assets** e baixe o arquivo que termina em **`-win64.zip`**.
4. Clique com o botão direito no ZIP e escolha **Extrair Tudo**.

Os arquivos **Source code** são o código para desenvolvedores. O ZIP com `-win64` contém a DLL pronta para instalar.

## 2. Encontrar a pasta certa

1. Abra a **Biblioteca** da Steam.
2. Clique com o botão direito em **Blair Witch → Propriedades**.
3. Entre em **Arquivos instalados → Explorar**.
4. Na pasta que abrir, entre em **Blairwitch → Binaries → Win64**.
5. Confirme que existe um arquivo chamado **`Blairwitch-Win64-Shipping.exe`** nessa pasta.

## 3. Copiar a DLL

Copie somente a **`winmm.dll`** extraída do ZIP para essa pasta `Win64`, ao lado do executável:

```text
Blair Witch/
└── Blairwitch/
    └── Binaries/
        └── Win64/
            ├── Blairwitch-Win64-Shipping.exe
            └── winmm.dll                       ← arquivo do mod
```

Não é necessário abrir a DLL. Ela será carregada quando o jogo iniciar.

**Se já existir uma `winmm.dll` nessa pasta, não substitua o arquivo.** Outro mod pode estar usando esse nome. Esta versão não permite combinar essas duas DLLs.

## 4. Retirar o comando antigo

1. Volte a **Propriedades → Geral → Opções de inicialização** na Steam.
2. Remova o comando antigo que contém **`cmd /c`** e **`OPENSSL_ia32cap`**, caso você tenha usado essa solução.
3. Se esse era o único comando, deixe o campo vazio. Preserve outras opções de inicialização que você use.
4. Feche a janela de propriedades.

O arquivo `.bat` usado antes pode ser dispensado. O mod será carregado junto com o jogo.

## 5. Abrir e conferir

Clique em **Jogar** na Steam. Confira se o jogo abre, se os botões do menu respondem e se você consegue entrar na sua partida.

A abertura pelo botão Jogar e o funcionamento do menu foram confirmados por um usuário na instalação testada. Uma campanha completa ainda não foi validada. Veja o [registro dos testes](VALIDATION.md).

## Se não funcionar

| O que aconteceu | O que conferir |
| --- | --- |
| O jogo continua sem abrir | A DLL precisa estar na mesma pasta de `Blairwitch-Win64-Shipping.exe`. Verifique se você extraiu o ZIP e copiou o arquivo para `Blairwitch\Binaries\Win64`. |
| Não encontrei a DLL no download | Baixe o arquivo `-win64.zip` em Assets; os arquivos Source code contêm o código do projeto. |
| A pasta já tem uma `winmm.dll` | Preserve a DLL existente. Não há suporte a combinar dois mods com esse nome. |
| O jogo abre, mas a resolução ou os cliques estão errados | São ajustes de vídeo e escala separados. Este mod trata a falha de inicialização do OpenSSL. |
| O problema começou depois da instalação | Feche o jogo e remova somente a DLL deste mod, seguindo a seção abaixo. |

Para relatar uma falha, abra uma [issue no projeto](https://github.com/marcosmaldonadoo/blair-witch-openssl-fix/issues). Informe a versão do mod, sua versão do Windows, o modelo do processador e em que momento ocorre o erro. Não publique senhas, chaves ou dados pessoais.

## Como desinstalar

1. Feche o jogo.
2. Volte à pasta `Blairwitch\Binaries\Win64`.
3. Apague **somente a `winmm.dll` que você copiou deste mod**.

O executável original e as DLLs do Windows permanecem intactos. A verificação de arquivos da Steam pode deixar arquivos adicionais de mods na pasta, por isso a remoção acima é manual.

Se o jogo voltar a apresentar a falha de OpenSSL após remover o mod, você pode restaurar nas opções de inicialização a solução que funcionava antes:

```cmd
cmd /c "set OPENSSL_ia32cap=:~0x20000000 && %command%"
```
