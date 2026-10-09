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

## Optional report archive

Export a JSON report from the command above, then run `python cloud/sync.py enqueue result.json --project c-household-budget` and `python cloud/sync.py sync`. Synchronization requires `BRUNNODEV_ACCESS_TOKEN` and the external operations API; the local outbox retains unacknowledged reports.

## License

Original source and documentation are MIT licensed; see [LICENSE](LICENSE). Third-party dependencies and media retain their respective terms. Maintained by [Brunno Dev](https://brunnodev.store).
