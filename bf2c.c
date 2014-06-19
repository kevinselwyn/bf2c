/*
 * Utility for converting brainfuck programs into C programs
 *
 * Copyright (C) 2014, Kevin Selwyn <kevinselwyn at gmail dot com>
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__APPLE__)
#include <malloc.h>
#endif

#define VERSION "1.0.1"

void strrepeat(char *buf, char *chr, int count) {
	int i;
	char indentation[30] = "";

	if (count == 0) {
		indentation[0] = '\0';
	} else {
		for (i = 0; i < count; i++) {
			if (i == 0) {
				strcpy(indentation, chr);
			} else {
				strcat(indentation, chr);
			}
		}
	}

	strcat(buf, indentation);
}

int main(int argc, char *argv[]) {
	int i = 0, rc = 0, bf_size = 0, c_size = 0, counter = 0, depth = 0;
	int plus = 0, minus = 0, right = 0, left = 0, dot = 0;
	char *bf_program = NULL, *bf_data = NULL, *bf_chars = NULL, *c_program = NULL;
	char output[30000] = "", line[300] = "";
	char indent1[30] = "", indent2[30] = "", pattern[60] = "";
	FILE *bf = NULL;
	FILE *c = NULL;

	if (argc < 2) {
		printf("bf2c (v%s)\n\nUsage: bf2c <program.bf> [<output.c>]\n", VERSION);
		rc = 1;
		goto cleanup;
	}

	bf_program = argv[1];
	bf = fopen(bf_program, "r");

	if (!bf) {
		printf("Could not open %s\n", bf_program);
		rc = 1;
		goto cleanup;
	}

	fseek(bf, 0, SEEK_END);
	bf_size = ftell(bf);
	fseek(bf, 0, SEEK_SET);

	if (!bf_size || bf_size == 0) {
		printf("%s is empty\n", bf_program);
		rc = 1;
		goto cleanup;
	}

	bf_data = malloc(sizeof(char) * bf_size);

	if (fread(bf_data, 1, bf_size, bf) != bf_size) {
		printf("Could not read %s\n", bf_program);
		rc = 1;
		goto cleanup;
	}

	bf_chars = malloc(sizeof(char) * bf_size);

	for (i = 0, counter = 0; i < bf_size; i++) {
		switch (bf_data[i]) {
		case ']':
		case '[':
		case '+':
		case '-':
		case '>':
		case '<':
		case '.':
		case ',':
			bf_chars[counter++] = bf_data[i];
		default:
			break;
		}
	}

	bf_size = counter;

	strcpy(output, "#include <stdio.h>\n\nint main() {\n\tint c = 0, i = 0;\n");
	strcat(output, "\tstatic int b[30000];\n\n");

	for (i = 0; i < bf_size; i++) {
		switch (bf_chars[i]) {
		case ']':
			depth--;
			strrepeat(output, "\t", depth);
			strcat(output, "\t}\n\n");

			break;
		case '[':
			strcat(output, "\n");
			strrepeat(output, "\t", depth);
			strcat(output, "\twhile(b[c] != 0) {\n");
			depth++;

			break;
		case '+':
			if (i + 1 < bf_size && bf_chars[i + 1] == '+') {
				plus++;
			} else {
				strrepeat(output, "\t", depth);

				if (plus > 0) {
					sprintf(line, "\tb[c] += %d;\n", ++plus);
					strcat(output, line);
					plus = 0;
				} else {
					strcat(output, "\t++b[c];\n");
				}
			}

			break;
		case '-':
			if (i + 1 < bf_size && bf_chars[i + 1] == '-') {
				minus++;
			} else {
				strrepeat(output, "\t", depth);

				if (minus > 0) {
					sprintf(line, "\tb[c] -= %d;\n", ++minus);
					strcat(output, line);
					minus = 0;
				} else {
					strcat(output, "\t--b[c];\n");
				}
			}

			break;
		case '>':
			if (i + 1 < bf_size && bf_chars[i + 1] == '>') {
				right++;
			} else {
				strrepeat(output, "\t", depth);

				if (right > 0) {
					sprintf(line, "\tc += %d;\n", ++right);
					strcat(output, line);
					right = 0;
				} else {
					strcat(output, "\t++c;\n");
				}
			}

			break;
		case '<':
			if (i + 1 < bf_size && bf_chars[i + 1] == '<') {
				left++;
			} else {
				strrepeat(output, "\t", depth);

				if (left > 0) {
					sprintf(line, "\tc -= %d;\n", ++left);
					strcat(output, line);
					left = 0;
				} else {
					strcat(output, "\t--c;\n");
				}
			}

			break;
		case '.':
			indent1[0] = '\0';
			indent2[0] = '\0';

			if (i + 1 < bf_size && bf_chars[i + 1] == '.') {
				dot++;
			} else {
				strrepeat(output, "\t", depth);

				if (dot > 0) {
					strrepeat(indent1, "\t", depth + 1);
					strrepeat(indent2, "\t", depth);

					strcpy(pattern, "\n\tfor (i = 0; i < %d; i++) {");
					strcat(pattern, "\n%s\tputchar(b[c]);\n%s\t}\n\n");

					sprintf(line, pattern, ++dot, indent1, indent2);
					strcat(output, line);
					dot = 0;
				} else {
					strcat(output, "\tputchar(b[c]);\n");
				}
			}

			break;
		case ',':
			strrepeat(output, "\t", depth);
			strcat(output, "\tb[c] = getchar();\n");

			break;
		default:
			break;
		}
	}

	strcat(output, "\n\treturn 0;\n");
	strcat(output, "}");

	if (argc < 3) {
		printf("%s\n", output);
		rc = 0;
		goto cleanup;
	}

	c_program = argv[2];
	c = fopen(c_program, "w+");

	if (!c) {
		printf("Could not open %s\n", c_program);
		rc = 1;
		goto cleanup;
	}

	c_size = strlen(output);

	if (fwrite(output, 1, c_size, c) != c_size) {
		printf("Could not write to %s\n", c_program);
		rc = 1;
		goto cleanup;
	}

cleanup:
	if (bf) {
		fclose(bf);
	}

	if (c) {
		fclose(c);
	}

	if (bf_data) {
		free(bf_data);
	}

	if (bf_chars) {
		free(bf_chars);
	}

	return 0;
}
