#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int base64_value(int c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

int main(int argc, char **argv) {
    static const char marker[] = "SPK Binary Data Follows -- base64 encoded:";
    FILE *input, *output;
    char line[4096];
    int found = 0, quartet[4], count = 0, padding = 0, c, value;
    unsigned char header[8];
    size_t header_size = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: %s RESPONSE OUTPUT\n", argv[0]);
        return 2;
    }
    input = fopen(argv[1], "rb");
    if (!input) return 3;
    output = fopen(argv[2], "wb");
    if (!output) { fclose(input); return 4; }

    while (fgets(line, sizeof(line), input)) {
        if (strstr(line, marker)) { found = 1; break; }
    }
    if (!found) goto invalid;

    while ((c = fgetc(input)) != EOF) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        if (c == '=') { value = 0; ++padding; }
        else { value = base64_value(c); if (value < 0 || padding) goto invalid; }
        quartet[count++] = value;
        if (count == 4) {
            unsigned char bytes[3];
            if (padding > 2) goto invalid;
            bytes[0] = (unsigned char)((quartet[0] << 2) | (quartet[1] >> 4));
            bytes[1] = (unsigned char)((quartet[1] << 4) | (quartet[2] >> 2));
            bytes[2] = (unsigned char)((quartet[2] << 6) | quartet[3]);
            if (fwrite(bytes, 1, (size_t)(3 - padding), output) != (size_t)(3 - padding))
                goto invalid;
            if (header_size < sizeof(header)) {
                size_t take = sizeof(header) - header_size;
                if (take > (size_t)(3 - padding)) take = (size_t)(3 - padding);
                memcpy(header + header_size, bytes, take);
                header_size += take;
            }
            count = 0;
        }
    }
    if (count != 0 || header_size != sizeof(header) ||
        memcmp(header, "DAF/SPK ", sizeof(header)) != 0 || fclose(output) != 0) {
        fclose(input); remove(argv[2]); return 5;
    }
    fclose(input);
    return 0;

invalid:
    fclose(input); fclose(output); remove(argv[2]);
    return 5;
}
