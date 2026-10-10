import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

def run(argv, expected=0):
    result = subprocess.run(argv, cwd=ROOT, capture_output=True, text=True, timeout=30)
    if result.returncode != expected:
        raise RuntimeError(result.stderr + result.stdout)
    return result.stdout

with tempfile.TemporaryDirectory() as temp:
    path = Path(temp) / 'ledger.txt'
    path.write_text('2026-10-01|salary|1000.00|receipt|income\n2026-10-02|food|125.25|groceries|expense\n2026-10-03|food|24.75|groceries|expense\n')
    report = json.loads(run(['build/budget', str(path)]))
    assert report['summary']['incomeMinor'] == 100000
    assert report['summary']['expenseMinor'] == 15000
    assert report['summary']['netMinor'] == 85000
    print(json.dumps(report, sort_keys=True))
    path.write_text('2026-02-30|food|1.00|invalid|expense\n')
    run(['build/budget', str(path)], 2)
    print('invalid calendar date rejected')
