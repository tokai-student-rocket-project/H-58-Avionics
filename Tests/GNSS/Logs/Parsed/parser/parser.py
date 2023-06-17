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
    output.write("Latitude,Longitude\n")

    for line in input:
        latitude = match(r"{\"lat\":(\d+(?:\.\d+)?),", line)
        longitude = match(r"\"lon\":(\d+(?:\.\d+)?)}", line)

        if not all([latitude, longitude]):
            continue

        output.write(f"{latitude},{longitude}\n")
