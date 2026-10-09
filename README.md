# Household Budget

Consolidação de receitas e despesas por mês e categoria, com valores inteiros em centavos e validação estrita de datas e registros.

## Executar

Requisitos: C17.

```sh
make
build/budget ledger.txt 2026-10 > resultado.json
```

## Funcionamento

Formato: `data|categoria|valor|descrição|income` ou `expense`. Sem o quinto campo, o registro é uma despesa. Exemplo: `2026-10-09|alimentacao|35.90|mercado|expense`. Linhas inválidas interrompem o relatório.

## Persistência de resultados

O arquivo de operações está em [vercel-home-telemetry-api.vercel.app](https://vercel-home-telemetry-api.vercel.app/laboratory.html?project=c-household-budget). As migrações Supabase estão no [repositório da API](https://github.com/brunnojob/vercel-home-telemetry-api/tree/main/supabase/migrations).

```sh
python cloud/sync.py enqueue resultado.json --project c-household-budget
python cloud/sync.py sync
```

Defina `BRUNNODEV_ACCESS_TOKEN` com sua sessão. A fila SQLite conserva os relatórios até confirmação do servidor; o mesmo conteúdo não gera registros duplicados. Tokens não são gravados no código.
