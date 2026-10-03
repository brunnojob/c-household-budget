# c-household-budget

C17 command-line expense ledger. Imports one `YYYY-MM-DD|category|amount|description` row per line and groups spending by month and category.

Build with `cc -std=c17 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror src/main.c -o budget`. Run `./budget data/ledger.txt`.

Project by [Brunno Dev](https://brunnodev.store).