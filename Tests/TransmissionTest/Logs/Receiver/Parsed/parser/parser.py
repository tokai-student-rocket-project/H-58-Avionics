import sys
import re


def match(pattern: str, string: str) -> float:
    match = re.search(pattern, string)

    if not match:
        return None

    return float(match.group(1))


input_path = sys.argv[1]
output_path = sys.argv[2]

with open(input_path) as input, open(output_path, mode="w") as output:
    output.write("Frequency,Bandwidth,Length,RSSI,Routine\n")

    for line in input:
        frequency = match(r"Frequency (\d+(?:\.\d+)?) MHz", line)
        bandwidth = match(r"Bandwidth ([+-]?\d+) kHz", line)
        length = match(r"Length ([+-]?\d+) Bytes", line)
        rssi = match(r"RSSI ([+-]?\d+) dBm", line)
        routine = match(r"Routine (\d+(?:\.\d+)?) Hz", line)

        if not all([frequency, bandwidth, length, rssi, routine]):
            continue

        output.write(f"{frequency},{bandwidth},{length},{rssi},{routine}\n")
