# Trabalho EDA

- ler um arquivo informações de atores, produtores e filmes. [x]
- ler um arquivo de relacionamentos entre eles. [x]


gcc (Get-ChildItem -Recurse -Filter *.c | Select-Object -ExpandProperty FullName) -o main ; .\main.exe


- Filme
    - Nome_Filme
    - Atores []
    - Diretor []
    - Produtor []
    - Roteirista []

- FilmeAtor
    - Filme
    - PessoasQueAtuou

- FilmeRoteiro
    - Filme
    - PessoasQueRoteirizou

- FilmeDiretor
    - Filme
    - PessoasQueDirigiu

- FilmeProdutor
    - Filme
    - PessoaQueProduziu