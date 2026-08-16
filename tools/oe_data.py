#!/usr/bin/env python3
"""Reproducibly fetch OpenEphemeris data; never used by the C runtime."""
import argparse, base64, hashlib, json, pathlib, urllib.parse, urllib.request

DE440_URL = "https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de440.bsp"
DE440_SHA256 = "a4ce9bf9b3282becc9f4b2ac3cebe03a2ae7599981aabd7265fd8482fff7c4b5"
HORIZONS_URL = "https://ssd.jpl.nasa.gov/api/horizons.api"

def fetch(url):
    with urllib.request.urlopen(url) as response:
        return response.read()

def store_checked(path, payload, expected=None):
    digest = hashlib.sha256(payload).hexdigest()
    if expected and digest != expected:
        raise SystemExit(f"checksum mismatch: expected {expected}, got {digest}")
    path.write_bytes(payload)
    return digest

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=pathlib.Path, default=pathlib.Path("data"))
    parser.add_argument("--chiron", action="store_true")
    args = parser.parse_args(); args.output.mkdir(parents=True, exist_ok=True)
    manifest = {"schema": 1, "files": []}
    payload = fetch(DE440_URL)
    digest = store_checked(args.output / "de440.bsp", payload, DE440_SHA256)
    manifest["files"].append({"name":"de440.bsp","source":DE440_URL,"sha256":digest,"coverage":"1550-2650"})
    if args.chiron:
        query = {"format":"json", "COMMAND":"'DES=2060;'", "EPHEM_TYPE":"SPK",
                 "START_TIME":"'1800-01-01'", "STOP_TIME":"'2200-12-31'", "OBJ_DATA":"YES"}
        url = HORIZONS_URL + "?" + urllib.parse.urlencode(query)
        response = json.loads(fetch(url));
        if "spk" not in response: raise SystemExit(response.get("error", response.get("result", "Horizons error")))
        payload = base64.b64decode(response["spk"]); name = "chiron-2060-1800-2200.bsp"
        digest = store_checked(args.output / name, payload)
        manifest["files"].append({"name":name,"source":url,"sha256":digest,"coverage":"1800-2200",
                                  "spk_file_id":response.get("spk_file_id"),"horizons_result":response.get("result")})
    (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(args.output / "manifest.json")
if __name__ == "__main__": main()
