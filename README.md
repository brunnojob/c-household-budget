# Household Budget

Monthly income and expense totals by category, using integer cents and strict date and record validation.

## Run

Requirements: C17.

```sh
make
build/budget ledger.txt 2026-10 > result.json
```

## Behavior

Input format: `date|category|amount|description|income` or `expense`. A record without the fifth field is treated as an expense. Example: `2026-10-09|food|35.90|groceries|expense`. Invalid lines stop report generation.

## Result synchronization

The [operations archive](https://vercel-home-telemetry-api.vercel.app/laboratory.html?project=c-household-budget) stores execution results. Supabase migrations are in the [API repository](https://github.com/brunnojob/vercel-home-telemetry-api/tree/main/supabase/migrations).

```sh
python cloud/sync.py enqueue result.json --project c-household-budget
python cloud/sync.py sync
```

Set `BRUNNODEV_ACCESS_TOKEN` to your session token. The SQLite outbox retains reports until the server confirms persistence; identical content does not create duplicate records. Tokens are not stored in source code. To run the synchronization tests:

```sh
python -m unittest discover -s cloud
```
